#include "graph_visualizer.h"
#include <QPainter>
#include <QMouseEvent>
#include <QFontMetrics>
#include <QtMath>
#include <QRandomGenerator>

GraphVisualizer::GraphVisualizer(QWidget *parent)
    : QWidget(parent)
    , m_dragging(false)
    , m_draggedNode(-1)
    , m_backgroundColor(QColor(45, 45, 45))
    , m_defaultNodeColor(QColor(100, 150, 200))
    , m_highlightedNodeColor(QColor(255, 215, 0))
    , m_visitedNodeColor(QColor(144, 238, 144))
    , m_defaultEdgeColor(QColor(180, 180, 180))
    , m_highlightedEdgeColor(QColor(255, 69, 0))
    , m_animation(new QPropertyAnimation(this))
    , m_isAnimating(false)
{
    setMinimumSize(400, 300);
    setMouseTracking(true);

    connect(m_animation, &QPropertyAnimation::finished,
            this, &GraphVisualizer::animationFinished);
}

void GraphVisualizer::setData(const VisualizationData& data)
{
    // Parse graph data from visualization data
    // This assumes the graph structure is stored in additionalData
    QVariantMap graphData = data.additionalData;

    if (graphData.contains("nodes") && graphData.contains("edges")) {
        clearGraph();

        // Add nodes
        QVariantList nodes = graphData["nodes"].toList();
        for (const QVariant& nodeVar : nodes) {
            QVariantMap nodeData = nodeVar.toMap();
            int id = nodeData["id"].toInt();
            QPoint pos = nodeData["position"].toPoint();
            QString label = nodeData["label"].toString();
            addNode(id, pos, label);
        }

        // Add edges
        QVariantList edges = graphData["edges"].toList();
        for (const QVariant& edgeVar : edges) {
            QVariantMap edgeData = edgeVar.toMap();
            int from = edgeData["from"].toInt();
            int to = edgeData["to"].toInt();
            bool directed = edgeData["directed"].toBool();
            int weight = edgeData.value("weight", 1).toInt();
            addEdge(from, to, directed, weight);
        }
    }

    // Apply highlights
    for (int i = 0; i < data.highlightedIndices.size(); ++i) {
        int nodeId = data.highlightedIndices[i];
        QColor color = (i < data.highlightColors.size()) ?
                           data.highlightColors[i] : m_highlightedNodeColor;
        highlightNode(nodeId, color);
    }

    update();
}

void GraphVisualizer::animateToData(const VisualizationData& newData, int duration)
{
    // For now, just set data directly
    // In a full implementation, you would interpolate node positions
    setData(newData);
}

void GraphVisualizer::addNode(int id, const QPoint& position, const QString& label)
{
    GraphNode node(id, position, label.isEmpty() ? QString::number(id) : label);
    node.color = m_defaultNodeColor;
    m_nodes[id] = node;
    update();
}

void GraphVisualizer::addEdge(int fromNode, int toNode, bool directed, int weight)
{
    GraphEdge edge(fromNode, toNode, directed, weight);
    edge.color = m_defaultEdgeColor;
    m_edges.append(edge);
    update();
}

void GraphVisualizer::clearGraph()
{
    m_nodes.clear();
    m_edges.clear();
    update();
}

void GraphVisualizer::highlightNode(int nodeId, const QColor& color)
{
    GraphNode* node = getNode(nodeId);
    if (node) {
        node->color = color;
        node->isHighlighted = true;
        update();
    }
}

void GraphVisualizer::highlightEdge(int fromNode, int toNode, const QColor& color)
{
    for (GraphEdge& edge : m_edges) {
        if ((edge.fromNode == fromNode && edge.toNode == toNode) ||
            (!edge.isDirected && edge.fromNode == toNode && edge.toNode == fromNode)) {
            edge.color = color;
            edge.isHighlighted = true;
            update();
            break;
        }
    }
}

void GraphVisualizer::setNodeVisited(int nodeId, bool visited)
{
    GraphNode* node = getNode(nodeId);
    if (node) {
        node->isVisited = visited;
        if (visited && !node->isHighlighted) {
            node->color = m_visitedNodeColor;
        }
        update();
    }
}

void GraphVisualizer::autoLayout()
{
    if (m_nodes.isEmpty()) return;

    // Simple force-directed layout
    const int iterations = 100;
    const double k = 100.0; // Ideal spring length
    const double repulsion = 10000.0;

    QMap<int, QPointF> forces;
    QMap<int, QPointF> positions;

    // Initialize positions and forces
    for (auto it = m_nodes.begin(); it != m_nodes.end(); ++it) {
        positions[it.key()] = QPointF(it.value().position);
        forces[it.key()] = QPointF(0, 0);
    }

    for (int iter = 0; iter < iterations; ++iter) {
        // Reset forces
        for (auto& force : forces) {
            force = QPointF(0, 0);
        }

        // Repulsive forces between all nodes
        for (auto it1 = positions.begin(); it1 != positions.end(); ++it1) {
            for (auto it2 = it1 + 1; it2 != positions.end(); ++it2) {
                QPointF diff = it1.value() - it2.value();
                double distance = qSqrt(diff.x() * diff.x() + diff.y() * diff.y());
                if (distance > 0) {
                    double force = repulsion / (distance * distance);
                    diff *= force / distance;
                    forces[it1.key()] += diff;
                    forces[it2.key()] -= diff;
                }
            }
        }

        // Attractive forces for connected nodes
        for (const GraphEdge& edge : m_edges) {
            QPointF pos1 = positions[edge.fromNode];
            QPointF pos2 = positions[edge.toNode];
            QPointF diff = pos2 - pos1;
            double distance = qSqrt(diff.x() * diff.x() + diff.y() * diff.y());
            if (distance > 0) {
                double force = (distance - k) * 0.1;
                diff *= force / distance;
                forces[edge.fromNode] += diff;
                forces[edge.toNode] -= diff;
            }
        }

        // Apply forces with damping
        const double damping = 0.9;
        for (auto it = positions.begin(); it != positions.end(); ++it) {
            it.value() += forces[it.key()] * damping;

            // Keep within bounds
            QRectF bounds = rect().adjusted(50, 50, -50, -50);
            it.value().setX(qBound(bounds.left(), it.value().x(), bounds.right()));
            it.value().setY(qBound(bounds.top(), it.value().y(), bounds.bottom()));
        }
    }

    // Update node positions
    for (auto it = positions.begin(); it != positions.end(); ++it) {
        m_nodes[it.key()].position = it.value().toPoint();
    }

    update();
}

void GraphVisualizer::circularLayout()
{
    if (m_nodes.isEmpty()) return;

    QPoint center = rect().center();
    int radius = qMin(width(), height()) / 3;
    int nodeCount = m_nodes.size();

    int i = 0;
    for (auto it = m_nodes.begin(); it != m_nodes.end(); ++it, ++i) {
        double angle = 2 * M_PI * i / nodeCount;
        int x = center.x() + radius * qCos(angle);
        int y = center.y() + radius * qSin(angle);
        it.value().position = QPoint(x, y);
    }

    update();
}

void GraphVisualizer::gridLayout(int columns)
{
    if (m_nodes.isEmpty() || columns <= 0) return;

    int margin = 50;
    int cellWidth = (width() - 2 * margin) / columns;
    int cellHeight = 80;

    int i = 0;
    for (auto it = m_nodes.begin(); it != m_nodes.end(); ++it, ++i) {
        int row = i / columns;
        int col = i % columns;
        int x = margin + col * cellWidth + cellWidth / 2;
        int y = margin + row * cellHeight + cellHeight / 2;
        it.value().position = QPoint(x, y);
    }

    update();
}

void GraphVisualizer::resetVisualization()
{
    for (auto& node : m_nodes) {
        node.color = m_defaultNodeColor;
        node.isHighlighted = false;
        node.isVisited = false;
    }

    for (auto& edge : m_edges) {
        edge.color = m_defaultEdgeColor;
        edge.isHighlighted = false;
    }

    update();
}

void GraphVisualizer::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Clear background
    painter.fillRect(rect(), m_backgroundColor);

    if (m_nodes.isEmpty()) {
        painter.setPen(Qt::white);
        painter.setFont(QFont("Arial", 14));
        painter.drawText(rect(), Qt::AlignCenter, "No graph data to visualize");
        return;
    }

    // Draw edges first
    for (const GraphEdge& edge : m_edges) {
        drawEdge(painter, edge);
    }

    // Draw nodes on top
    for (const GraphNode& node : m_nodes) {
        drawNode(painter, node);
    }
}

void GraphVisualizer::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        int nodeId = getNodeAt(event->pos());
        if (nodeId >= 0) {
            m_dragging = true;
            m_draggedNode = nodeId;
            m_lastMousePos = event->pos();
            emit nodeClicked(nodeId);
        }
    }
    QWidget::mousePressEvent(event);
}

void GraphVisualizer::mouseMoveEvent(QMouseEvent *event)
{
    if (m_dragging && m_draggedNode >= 0) {
        GraphNode* node = getNode(m_draggedNode);
        if (node) {
            QPoint delta = event->pos() - m_lastMousePos;
            node->position += delta;
            m_lastMousePos = event->pos();
            update();
        }
    }
    QWidget::mouseMoveEvent(event);
}

void GraphVisualizer::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_dragging = false;
        m_draggedNode = -1;
    }
    QWidget::mouseReleaseEvent(event);
}

void GraphVisualizer::drawNode(QPainter& painter, const GraphNode& node)
{
    // Draw node circle
    painter.setBrush(node.color);
    painter.setPen(QPen(node.color.darker(150), 2));
    painter.drawEllipse(node.position - QPoint(node.radius, node.radius),
                        node.radius * 2, node.radius * 2);

    // Draw node label
    painter.setPen(Qt::white);
    painter.setFont(QFont("Arial", 10, QFont::Bold));
    QFontMetrics fm(painter.font());
    QRect textRect = fm.boundingRect(node.label);
    QPoint textPos = node.position - QPoint(textRect.width() / 2, -textRect.height() / 4);
    painter.drawText(textPos, node.label);

    // Draw highlight ring if highlighted
    if (node.isHighlighted) {
        painter.setBrush(Qt::NoBrush);
        painter.setPen(QPen(Qt::yellow, 3));
        painter.drawEllipse(node.position - QPoint(node.radius + 5, node.radius + 5),
                            (node.radius + 5) * 2, (node.radius + 5) * 2);
    }
}

void GraphVisualizer::drawEdge(QPainter& painter, const GraphEdge& edge)
{
    const GraphNode* fromNode = getNode(edge.fromNode);
    const GraphNode* toNode = getNode(edge.toNode);

    if (!fromNode || !toNode) return;

    QPoint start = calculateEdgeStart(*fromNode, *toNode);
    QPoint end = calculateEdgeEnd(*fromNode, *toNode);

    // Draw edge line
    painter.setPen(QPen(edge.color, edge.isHighlighted ? 3 : 2));
    painter.drawLine(start, end);

    // Draw arrow for directed edges
    if (edge.isDirected) {
        QVector2D direction(end - start);
        direction.normalize();
        QVector2D perpendicular(-direction.y(), direction.x());

        QPoint arrowHead = end - QPoint(direction.x() * 15, direction.y() * 15);
        QPoint arrowSide1 = arrowHead + QPoint(perpendicular.x() * 8, perpendicular.y() * 8);
        QPoint arrowSide2 = arrowHead - QPoint(perpendicular.x() * 8, perpendicular.y() * 8);

        painter.setBrush(edge.color);
        QPolygon arrow;
        arrow << end << arrowSide1 << arrowSide2;
        painter.drawPolygon(arrow);
    }

    // Draw edge weight
    if (edge.weight != 1) {
        drawEdgeLabel(painter, edge);
    }
}

void GraphVisualizer::drawEdgeLabel(QPainter& painter, const GraphEdge& edge)
{
    const GraphNode* fromNode = getNode(edge.fromNode);
    const GraphNode* toNode = getNode(edge.toNode);

    if (!fromNode || !toNode) return;

    QPoint midPoint = (fromNode->position + toNode->position) / 2;

    painter.setPen(Qt::white);
    painter.setFont(QFont("Arial", 9));
    painter.setBrush(QColor(0, 0, 0, 150));

    QString weightText = QString::number(edge.weight);
    QFontMetrics fm(painter.font());
    QRect textRect = fm.boundingRect(weightText);
    textRect.moveCenter(midPoint);

    painter.drawRect(textRect.adjusted(-2, -1, 2, 1));
    painter.drawText(textRect, Qt::AlignCenter, weightText);
}

int GraphVisualizer::getNodeAt(const QPoint& pos) const
{
    for (auto it = m_nodes.constBegin(); it != m_nodes.constEnd(); ++it) {
        QPoint diff = pos - it.value().position;
        if (diff.x() * diff.x() + diff.y() * diff.y() <= it.value().radius * it.value().radius) {
            return it.key();
        }
    }
    return -1;
}

GraphNode* GraphVisualizer::getNode(int nodeId)
{
    auto it = m_nodes.find(nodeId);
    return (it != m_nodes.end()) ? &it.value() : nullptr;
}

const GraphNode* GraphVisualizer::getNode(int nodeId) const
{
    auto it = m_nodes.find(nodeId);
    return (it != m_nodes.end()) ? &it.value() : nullptr;
}

QPoint GraphVisualizer::calculateEdgeStart(const GraphNode& from, const GraphNode& to) const
{
    QVector2D direction(to.position - from.position);
    direction.normalize();
    return from.position + QPoint(direction.x() * from.radius, direction.y() * from.radius);
}

QPoint GraphVisualizer::calculateEdgeEnd(const GraphNode& from, const GraphNode& to) const
{
    QVector2D direction(from.position - to.position);
    direction.normalize();
    return to.position + QPoint(direction.x() * to.radius, direction.y() * to.radius);
}
