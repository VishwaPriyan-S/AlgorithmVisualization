#include "ASTVisualizer.h"
#include <QPainter>
#include <QtMath>
#include <QDebug>
#include <QFont>
#include <QBrush>
#include <QPen>

// ==================== NodeItem (Draggable) ====================
NodeItem::NodeItem(QGraphicsItem* parent) : QGraphicsEllipseItem(parent) {
    setRect(-20, -20, 40, 40);
    setBrush(QColor(100, 149, 237));
    setPen(QPen(Qt::white, 2));
    setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemSendsGeometryChanges);
    setZValue(10);
}

void NodeItem::addEdge(QGraphicsLineItem* line, bool isStart) {
    edges.append({line, isStart});
}

QVariant NodeItem::itemChange(GraphicsItemChange change, const QVariant &value) {
    if (change == ItemPositionChange && scene()) {
        QPointF newPos = value.toPointF();
        for (const auto& edge : edges) {
            QLineF line = edge.line->line();
            if (edge.isStart) line.setP1(newPos);
            else              line.setP2(newPos);
            edge.line->setLine(line);
        }
    }
    return QGraphicsItem::itemChange(change, value);
}

// ==================== GraphItem ====================
GraphItem::GraphItem(const QString& name, QGraphicsItem* parent)
    : QGraphicsItem(parent), m_name(name) {
    m_nameText = new QGraphicsTextItem(name, this);
    m_nameText->setDefaultTextColor(Qt::gray);
    m_nameText->setFont(QFont("Consolas", 14, QFont::Bold));
    m_nameText->setPos(-50, -150);
}

QRectF GraphItem::boundingRect() const { return childrenBoundingRect(); }
void GraphItem::paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*) {}

void GraphItem::updateData(const QJsonObject& adjList) {
    prepareGeometryChange();
    for(auto& node : m_nodes) if(node.circle) delete node.circle;
    m_nodes.clear();
    qDeleteAll(m_edges);
    m_edges.clear();

    QStringList nodeKeys = adjList.keys();
    if (nodeKeys.isEmpty()) return;

    int N = nodeKeys.size();
    double radius = 130.0;
    double angleStep = 2 * M_PI / N;

    // Create Nodes
    for (int i = 0; i < N; ++i) {
        QString key = nodeKeys[i];
        double angle = i * angleStep;
        double x = radius * qCos(angle);
        double y = radius * qSin(angle);

        auto* circle = new NodeItem(this);
        circle->setPos(x, y);

        auto* label = new QGraphicsTextItem(key, circle);
        label->setDefaultTextColor(Qt::white);
        label->setFont(QFont("Arial", 10, QFont::Bold));
        QRectF r = label->boundingRect();
        label->setPos(-r.width()/2, -r.height()/2);

        m_nodes[key] = {circle, label, QPointF(x, y)};
    }

    // Create Edges
    QSet<QString> processedEdges;
    for (const QString& source : nodeKeys) {
        QJsonArray neighbors = adjList[source].toArray();
        for (const QJsonValue& val : neighbors) {
            QString target = val.isString() ? val.toString() : QString::number(val.toInt());
            QString edgeKey = source < target ? source + "-" + target : target + "-" + source;

            if (m_nodes.contains(source) && m_nodes.contains(target) && !processedEdges.contains(edgeKey)) {
                NodeItem* n1 = m_nodes[source].circle;
                NodeItem* n2 = m_nodes[target].circle;
                auto* line = new QGraphicsLineItem(n1->x(), n1->y(), n2->x(), n2->y(), this);
                line->setPen(QPen(Qt::lightGray, 2));
                line->setZValue(-1);
                n1->addEdge(line, true);
                n2->addEdge(line, false);
                m_edges.append(line);
                processedEdges.insert(edgeKey);
            }
        }
    }
    update();
}

// ==================== ArrayItem (Golden Highlight Logic) ====================
ArrayItem::ArrayItem(const QString& name, QGraphicsItem* parent)
    : QGraphicsItem(parent), m_name(name), m_mode(VisualizationMode::Generic) {
    m_nameText = new QGraphicsTextItem(name, this);
    m_nameText->setDefaultTextColor(Qt::white);
    m_nameText->setPos(0, -30);
}

void ArrayItem::setMode(VisualizationMode mode) {
    m_mode = mode;
    updateLayout();
}

void ArrayItem::updateData(const QVector<int>& newValues) {
    // 1. Detect Changes
    m_highlightIndices.clear();
    int commonSize = qMin(m_values.size(), newValues.size());

    for (int i = 0; i < commonSize; ++i) {
        if (m_values[i] != newValues[i]) {
            m_highlightIndices.insert(i);
        }
    }
    // Highlight any newly added elements
    for (int i = commonSize; i < newValues.size(); ++i) {
        m_highlightIndices.insert(i);
    }

    // 2. Update Data
    m_values = newValues;
    updateLayout();
    update();
}

void ArrayItem::updateLayout() {
    qDeleteAll(m_visualElements);
    qDeleteAll(m_textElements);
    m_visualElements.clear();
    m_textElements.clear();

    int spacing = 5;
    int boxSize = 40;

    for (int i = 0; i < m_values.size(); ++i) {
        auto* rect = new QGraphicsRectItem(this);
        auto* text = new QGraphicsTextItem(QString::number(m_values[i]), this);
        m_visualElements.append(rect);
        m_textElements.append(text);

        // 3. APPLY HIGHLIGHT COLOR
        QColor boxColor = QColor(60, 60, 65); // Default Dark Gray
        if (m_highlightIndices.contains(i)) {
            boxColor = QColor(255, 215, 0); // Golden Yellow for changed items
        }

        if (m_mode == VisualizationMode::Sorting) {
            int h = qMax(5, m_values[i] * 5);
            rect->setRect(i * (25 + spacing), -h, 25, h);
            rect->setBrush(m_highlightIndices.contains(i) ? boxColor : QColor(100, 149, 237));
            text->setPos(i * (25 + spacing), 5);
        } else {
            rect->setRect(i * (boxSize + spacing), 0, boxSize, boxSize);
            rect->setBrush(boxColor);
            rect->setPen(QPen(Qt::white));
            text->setPos(rect->rect().x() + 10, rect->rect().y() + 10);
        }

        // Text color needs to be black if background is Golden, white otherwise
        text->setDefaultTextColor(m_highlightIndices.contains(i) ? Qt::black : Qt::white);
    }
}

QRectF ArrayItem::boundingRect() const { return childrenBoundingRect(); }
void ArrayItem::paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*) {}

// ==================== VariableItem ====================
VariableItem::VariableItem(const QString& name, QGraphicsItem* parent) : QGraphicsRectItem(parent) {
    setRect(0, 0, 150, 40);
    setBrush(QColor(50, 50, 50));
    setPen(QPen(Qt::gray));
    m_nameText = new QGraphicsTextItem(name + " =", this);
    m_nameText->setDefaultTextColor(Qt::white);
    m_nameText->setPos(5, 10);
    m_valueText = new QGraphicsTextItem("", this);
    m_valueText->setDefaultTextColor(Qt::cyan);
    m_valueText->setPos(80, 10);
}
void VariableItem::updateValue(const QString& value) { m_valueText->setPlainText(value); }

// ==================== CallStackItem ====================
CallStackItem::CallStackItem(const QString& funcName, int depth, QGraphicsItem* parent) : QGraphicsRectItem(parent) {
    setRect(0, 0, 180, 30);
    setBrush(QColor(218, 165, 32));
    setPen(QPen(Qt::white));
    auto* text = new QGraphicsTextItem(funcName, this);
    text->setDefaultTextColor(Qt::black);
    text->setPos(10, 5);
}

// ==================== ASTVisualizer Core ====================
ASTVisualizer::ASTVisualizer(QGraphicsScene* scene, QObject* parent)
    : QObject(parent), m_scene(scene), m_currentStep(-1), m_mode(VisualizationMode::Generic),
    m_errorItem(nullptr), m_consoleItem(nullptr)
{
    reset();
}

void ASTVisualizer::reset() {
    m_currentStep = -1;
    if(m_scene) m_scene->clear();

    m_variableItems.clear();
    m_arrayItems.clear();
    m_stackItems.clear();
    m_graphItems.clear();

    m_errorItem = new QGraphicsTextItem();
    m_errorItem->setDefaultTextColor(Qt::red);
    m_errorItem->setFont(QFont("Arial", 12));
    m_errorItem->setPos(10, 10);
    m_scene->addItem(m_errorItem);

    // FIX: Console Item
    m_consoleItem = new QGraphicsTextItem("Console Output:\n");
    m_consoleItem->setDefaultTextColor(QColor(100, 255, 100)); // Matrix Green
    m_consoleItem->setFont(QFont("Consolas", 11));
    m_consoleItem->setPos(10, 600);
    m_scene->addItem(m_consoleItem);
}

void ASTVisualizer::setMode(VisualizationMode mode) {
    m_mode = mode;
    for(auto* item : m_arrayItems) item->setMode(mode);
}

void ASTVisualizer::loadSteps(const QJsonArray& steps) {
    reset();
    m_steps = steps;
}

void ASTVisualizer::executeStep() {
    if (m_currentStep >= m_steps.size() - 1) {
        emit executionFinished();
        return;
    }
    m_currentStep++;
    processStep(m_steps[m_currentStep].toObject());
    emit stepExecuted(m_currentStep, m_steps.size());
}

void ASTVisualizer::processStep(const QJsonObject& step) {
    QString eventType = step["event"].toString();

    // 1. Highlight Line
    if (step.contains("line")) {
        emit highlightLine(step["line"].toInt());
    }

    // 2. Handle Errors
    if (eventType == "error") {
        if(m_errorItem) m_errorItem->setPlainText("Runtime Error:\n" + step["message"].toString());
        return;
    }

    // 3. FIX: Handle Console Output
    if (eventType == "print_output") {
        if(m_consoleItem) {
            QString msg = step["message"].toString();
            // Optional: Keep history or just show last line?
            // This implementation keeps the header and adds the new message.
            m_consoleItem->setPlainText("Console Output:\n" + msg);
        }
        return;
    }

    // 4. Variables / Arrays / Graphs
    QJsonObject vars = step["variables"].toObject();
    for (auto it = vars.begin(); it != vars.end(); ++it) {
        QString name = it.key();
        QJsonValue val = it.value();

        if (val.isObject()) {
            syncGraph(name, val.toObject());
        } else if (val.isArray()) {
            syncArray(name, val.toArray());
        } else {
            syncVariable(name, val.toVariant());
        }
    }

    // 5. Stack
    if (step.contains("stack")) {
        syncStack(step["stack"].toArray());
    }
}

void ASTVisualizer::syncGraph(const QString& name, const QJsonObject& graphData) {
    if (!m_graphItems.contains(name)) {
        auto* item = new GraphItem(name);
        item->setPos(550, 250); // Center-Right
        m_scene->addItem(item);
        m_graphItems[name] = item;
    }
    m_graphItems[name]->updateData(graphData);
}

void ASTVisualizer::syncVariable(const QString& name, const QVariant& value) {
    if (m_arrayItems.contains(name)) return;
    if (!m_variableItems.contains(name)) {
        auto* item = new VariableItem(name);
        item->setPos(20, 50 + m_variableItems.size() * 50);
        m_scene->addItem(item);
        m_variableItems[name] = item;
    }
    m_variableItems[name]->updateValue(value.toString());
}

void ASTVisualizer::syncArray(const QString& name, const QJsonArray& listData) {
    QVector<int> values;
    for (auto v : listData) values.append(v.toInt());

    if (!m_arrayItems.contains(name)) {
        auto* item = new ArrayItem(name);
        item->setMode(m_mode);
        item->setPos(250, 100 + m_arrayItems.size() * 120);
        m_scene->addItem(item);
        m_arrayItems[name] = item;
    }
    m_arrayItems[name]->updateData(values);
}

void ASTVisualizer::syncStack(const QJsonArray& stackData) {
    for(auto* item : m_stackItems) {
        m_scene->removeItem(item);
        delete item;
    }
    m_stackItems.clear();
    int depth = 0;
    for (auto v : stackData) {
        auto* item = new CallStackItem(v.toString(), depth);
        item->setPos(850, 50 + depth * 35);
        m_scene->addItem(item);
        m_stackItems.append(item);
        depth++;
    }
}
