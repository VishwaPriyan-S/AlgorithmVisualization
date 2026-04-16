#include "ASTVisualizer.h"
#include <QPainter>
#include <QtMath>
#include <QDebug>
#include <QFont>
#include <QBrush>
#include <QPen>
#include <QJsonValue>
#include <functional>

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

void GraphItem::updateData(const QJsonObject& adjList, const QJsonObject& metadata) {
    prepareGeometryChange();
    for(auto& node : m_nodes) if(node.circle) delete node.circle;
    m_nodes.clear();
    qDeleteAll(m_edges);
    m_edges.clear();

    const bool treeMode = metadata.value("tree_mode").toBool(false);
    if (treeMode) {
        QString rootName;
        QJsonObject rootObj;
        for (auto it = adjList.begin(); it != adjList.end(); ++it) {
            if (it.value().isObject()) {
                rootName = it.key();
                rootObj = it.value().toObject();
                break;
            }
        }
        if (rootObj.isEmpty()) {
            update();
            return;
        }

        struct EdgeLink { QString from; QString to; };
        QMap<QString, QPointF> positions;
        QList<EdgeLink> links;
        int visitIndex = 0;

        std::function<QString(const QJsonObject&, int)> buildTree;
        buildTree = [&](const QJsonObject& node, int depth) -> QString {
            QString nodeLabel;
            if (node.contains("value")) nodeLabel = node.value("value").toVariant().toString();
            else if (node.contains("val")) nodeLabel = node.value("val").toVariant().toString();
            else nodeLabel = QString("N%1").arg(visitIndex);

            if (nodeLabel.isEmpty()) nodeLabel = QString("N%1").arg(visitIndex);
            const QString uniqueId = nodeLabel + "_" + QString::number(visitIndex++);
            positions[uniqueId] = QPointF(visitIndex * 55.0, depth * 85.0);

            const auto attachChild = [&](const QJsonValue& childVal) {
                if (!childVal.isObject()) return;
                const QString childId = buildTree(childVal.toObject(), depth + 1);
                links.append({uniqueId, childId});
            };

            attachChild(node.value("left"));
            attachChild(node.value("right"));

            if (node.contains("children") && node.value("children").isArray()) {
                const QJsonArray children = node.value("children").toArray();
                for (const QJsonValue& child : children) {
                    attachChild(child);
                }
            }
            return uniqueId;
        };

        buildTree(rootObj, 0);

        for (auto it = positions.begin(); it != positions.end(); ++it) {
            const QString key = it.key();
            const QPointF pos = it.value();
            auto* circle = new NodeItem(this);
            circle->setPos(pos);
            circle->setBrush(QColor(59, 130, 246));

            QString labelText = key;
            const int suffixIdx = labelText.lastIndexOf('_');
            if (suffixIdx > 0) {
                labelText = labelText.left(suffixIdx);
            }

            auto* label = new QGraphicsTextItem(labelText, circle);
            label->setDefaultTextColor(Qt::white);
            label->setFont(QFont("Arial", 10, QFont::Bold));
            const QRectF r = label->boundingRect();
            label->setPos(-r.width() / 2, -r.height() / 2);

            m_nodes[key] = {circle, label, pos};
        }

        for (const EdgeLink& edge : links) {
            if (!m_nodes.contains(edge.from) || !m_nodes.contains(edge.to)) continue;
            NodeItem* n1 = m_nodes[edge.from].circle;
            NodeItem* n2 = m_nodes[edge.to].circle;
            auto* line = new QGraphicsLineItem(n1->x(), n1->y(), n2->x(), n2->y(), this);
            line->setPen(QPen(QColor(148, 163, 184), 2));
            line->setZValue(-1);
            n1->addEdge(line, true);
            n2->addEdge(line, false);
            m_edges.append(line);
        }

        update();
        return;
    }

    QStringList nodeKeys = adjList.keys();
    if (nodeKeys.isEmpty()) return;

    int N = nodeKeys.size();
    double radius = 130.0;
    double angleStep = 2 * M_PI / N;

    QSet<QString> visitedSet;
    QSet<QString> frontierSet;
    QSet<QString> pathSet;
    const QString currentNode = metadata.value("current").toString();

    const QJsonArray visitedArray = metadata.value("visited").toArray();
    for (const QJsonValue& v : visitedArray) {
        visitedSet.insert(v.toVariant().toString());
    }

    const QJsonArray frontierArray = metadata.value("frontier").toArray();
    for (const QJsonValue& v : frontierArray) {
        frontierSet.insert(v.toVariant().toString());
    }

    const QJsonArray pathArray = metadata.value("path").toArray();
    for (const QJsonValue& v : pathArray) {
        pathSet.insert(v.toVariant().toString());
    }

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

        QColor nodeColor(100, 149, 237);
        if (visitedSet.contains(key)) {
            nodeColor = QColor(34, 197, 94);
        }
        if (frontierSet.contains(key)) {
            nodeColor = QColor(245, 158, 11);
        }
        if (!currentNode.isEmpty() && currentNode == key) {
            nodeColor = QColor(239, 68, 68);
        }
        if (pathSet.contains(key)) {
            nodeColor = QColor(168, 85, 247);
        }
        circle->setBrush(nodeColor);

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
    : QGraphicsItem(parent), m_name(name), m_mode(VisualizationMode::Generic), m_sortedStart(-1) {
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

void ArrayItem::updateStepContext(const QJsonObject& vars) {
    m_compareIndices.clear();
    m_sortedStart = -1;

    const int i = vars.value("i").toInt(-1);
    const int j = vars.value("j").toInt(-1);
    const int left = vars.value("left").toInt(-1);
    const int right = vars.value("right").toInt(-1);
    const int low = vars.value("low").toInt(-1);
    const int high = vars.value("high").toInt(-1);
    const int n = vars.value("n").toInt(-1);

    if (i >= 0) m_compareIndices.insert(i);
    if (j >= 0) m_compareIndices.insert(j);
    if (j + 1 >= 0 && j + 1 < m_values.size()) m_compareIndices.insert(j + 1);
    if (left >= 0) m_compareIndices.insert(left);
    if (right >= 0) m_compareIndices.insert(right);
    if (low >= 0) m_compareIndices.insert(low);
    if (high >= 0) m_compareIndices.insert(high);

    // Bubble-sort heuristic: suffix [n-i, n-1] is already sorted after each pass.
    if (n > 0 && i >= 0 && i < n) {
        m_sortedStart = qBound(0, n - i, m_values.size());
    } else if (vars.value("swapped").isBool() && !vars.value("swapped").toBool()) {
        m_sortedStart = 0; // Early-exit pass means full array sorted.
    }

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
        if (m_compareIndices.contains(i)) {
            boxColor = QColor(251, 191, 36); // Compared elements in amber.
        }
        if (m_sortedStart >= 0 && i >= m_sortedStart) {
            boxColor = QColor(34, 197, 94); // Sorted section in green.
        }

        if (m_mode == VisualizationMode::Sorting) {
            int h = qMax(5, m_values[i] * 5);
            rect->setRect(i * (25 + spacing), -h, 25, h);
            QColor barColor = QColor(100, 149, 237);
            if (m_highlightIndices.contains(i) || m_compareIndices.contains(i) || (m_sortedStart >= 0 && i >= m_sortedStart)) {
                barColor = boxColor;
            }
            rect->setBrush(barColor);
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

    QJsonObject graphMetadata;
    if (step.contains("graph_state") && step["graph_state"].isObject()) {
        graphMetadata = step["graph_state"].toObject();
    }

    // 4. Variables / Arrays / Graphs
    QJsonObject vars = step["variables"].toObject();
    bool hasNumericArray = false;
    for (auto it = vars.begin(); it != vars.end(); ++it) {
        if (!it.value().isArray()) continue;
        const QJsonArray arr = it.value().toArray();
        if (arr.isEmpty()) continue;

        bool numeric = true;
        for (const QJsonValue& v : arr) {
            if (!v.isDouble()) {
                numeric = false;
                break;
            }
        }
        if (numeric) {
            hasNumericArray = true;
            break;
        }
    }

    const bool sortMarkers =
        vars.contains("i") || vars.contains("j") || vars.contains("swapped");
    if (m_mode == VisualizationMode::Generic && hasNumericArray && sortMarkers) {
        setMode(VisualizationMode::Sorting);
    }

    if (graphMetadata.isEmpty()) {
        const QStringList currentCandidates = {"current", "node", "u", "v", "vertex"};
        for (const QString& key : currentCandidates) {
            if (vars.contains(key)) {
                graphMetadata["current"] = vars.value(key).toVariant().toString();
                break;
            }
        }

        const QStringList visitedCandidates = {"visited", "seen", "closed"};
        for (const QString& key : visitedCandidates) {
            if (vars.contains(key) && vars.value(key).isArray()) {
                graphMetadata["visited"] = vars.value(key).toArray();
                break;
            }
        }

        const QStringList frontierCandidates = {"queue", "stack", "frontier", "open"};
        for (const QString& key : frontierCandidates) {
            if (vars.contains(key) && vars.value(key).isArray()) {
                graphMetadata["frontier"] = vars.value(key).toArray();
                break;
            }
        }
    }

    for (auto it = vars.begin(); it != vars.end(); ++it) {
        QString name = it.key();
        QJsonValue val = it.value();

        if (val.isObject()) {
            const QJsonObject obj = val.toObject();
            const bool looksLikeTree =
                obj.contains("value") || obj.contains("val") ||
                obj.contains("left") || obj.contains("right") ||
                obj.contains("children");
            if (looksLikeTree) {
                QJsonObject normalized;
                normalized[name] = obj;
                QJsonObject treeMeta;
                treeMeta.insert("tree_mode", true);
                syncGraph(name + "_tree", normalized, treeMeta);
            } else {
                syncGraph(name, obj, graphMetadata);
            }
        } else if (val.isArray()) {
            syncArray(name, val.toArray(), vars);
        } else {
            syncVariable(name, val.toVariant());
        }
    }

    // 5. Stack
    if (step.contains("stack")) {
        syncStack(step["stack"].toArray());
    }
}

void ASTVisualizer::syncGraph(const QString& name, const QJsonObject& graphData, const QJsonObject& metadata) {
    if (!m_graphItems.contains(name)) {
        auto* item = new GraphItem(name);
        item->setPos(550, 250); // Center-Right
        m_scene->addItem(item);
        m_graphItems[name] = item;
    }
    m_graphItems[name]->updateData(graphData, metadata);
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

void ASTVisualizer::syncArray(const QString& name, const QJsonArray& listData, const QJsonObject& vars) {
    QVector<int> values;
    for (auto v : listData) values.append(v.toInt());

    if (!m_arrayItems.contains(name)) {
        auto* item = new ArrayItem(name);
        item->setMode(m_mode);
        item->setPos(250, 100 + m_arrayItems.size() * 120);
        m_scene->addItem(item);
        m_arrayItems[name] = item;
    }
    m_arrayItems[name]->updateStepContext(vars);
    m_arrayItems[name]->updateData(values);

    // Keep array visualizations centered so tall bars don't render near/above the top edge.
    if (m_scene) {
        const QRectF sceneRect = m_scene->sceneRect();
        const QRectF itemRect = m_arrayItems[name]->boundingRect();
        const qreal centeredX = sceneRect.center().x() - (itemRect.width() / 2.0);
        const qreal centeredY = sceneRect.center().y() - (itemRect.height() / 2.0);
        m_arrayItems[name]->setPos(centeredX, centeredY);
    }
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
