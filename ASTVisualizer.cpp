#include "ASTVisualizer.h"
#include <QPainter>
#include <QDebug>

// ==================== ArrayItem Implementation ====================

ArrayItem::ArrayItem(const QString& name, QGraphicsItem* parent)
    : QGraphicsItem(parent), m_name(name), m_mode(VisualizationMode::Generic)
{
    // Initialize the label for the array
    m_nameText = new QGraphicsTextItem(name, this);
    m_nameText->setDefaultTextColor(Qt::white);
    m_nameText->setFont(QFont("Consolas", 12, QFont::Bold));
    m_nameText->setPos(0, -30);
}

void ArrayItem::setMode(VisualizationMode mode) {
    m_mode = mode;
    updateLayout();
    update(); // Trigger repaint
}

void ArrayItem::updateData(const QVector<int>& values) {
    // Only update layout if data actually changed
    if (m_values != values) {
        m_values = values;
        updateLayout();
    }
}

void ArrayItem::updateLayout() {
    // 1. Clear existing visual elements safely
    // QGraphicsItem's destructor handles children, but we are managing them manually in lists
    qDeleteAll(m_visualElements);
    qDeleteAll(m_textElements);
    m_visualElements.clear();
    m_textElements.clear();

    // 2. Constants for layout
    int spacing = 5;
    int boxSize = 40;
    int barWidth = 25;

    // 3. Create new elements based on values
    for (int i = 0; i < m_values.size(); ++i) {
        auto* rect = new QGraphicsRectItem(this);
        auto* text = new QGraphicsTextItem(QString::number(m_values[i]), this);

        m_visualElements.append(rect);
        m_textElements.append(text);

        if (m_mode == VisualizationMode::Sorting) {
            // --- BAR CHART MODE ---
            // Scale height: value * 5 (Min height 5px)
            int h = qMax(5, m_values[i] * 5);

            rect->setRect(i * (barWidth + spacing), -h, barWidth, h);
            rect->setBrush(QColor(100, 149, 237)); // Cornflower Blue

            // Number goes below the bar
            text->setPos(i * (barWidth + spacing), 5);
            text->setDefaultTextColor(Qt::white);
        } else {
            // --- GENERIC BOX MODE ---
            rect->setRect(i * (boxSize + spacing), 0, boxSize, boxSize);
            rect->setBrush(QColor(60, 60, 65)); // Dark Gray
            rect->setPen(QPen(Qt::white));

            // Center number inside box
            text->setPos(rect->rect().x() + 10, rect->rect().y() + 10);
            text->setDefaultTextColor(Qt::white);
        }
    }
}

QRectF ArrayItem::boundingRect() const { return childrenBoundingRect(); }
void ArrayItem::paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*) {}


// ==================== VariableItem Implementation ====================

VariableItem::VariableItem(const QString& name, QGraphicsItem* parent)
    : QGraphicsRectItem(parent)
{
    setRect(0, 0, 150, 40);
    setBrush(QColor(50, 50, 50));
    setPen(QPen(Qt::gray));

    m_nameText = new QGraphicsTextItem(name + " =", this);
    m_nameText->setDefaultTextColor(QColor(200, 200, 200));
    m_nameText->setPos(5, 10);

    m_valueText = new QGraphicsTextItem("", this);
    m_valueText->setDefaultTextColor(QColor(86, 156, 214)); // VS Code Blue
    m_valueText->setFont(QFont("Consolas", 11, QFont::Bold));
    m_valueText->setPos(80, 10);
}

void VariableItem::updateValue(const QString& value) {
    m_valueText->setPlainText(value);
}


// ==================== CallStackItem Implementation ====================

CallStackItem::CallStackItem(const QString& funcName, int depth, QGraphicsItem* parent)
    : QGraphicsRectItem(parent)
{
    setRect(0, 0, 180, 30);
    setBrush(QColor(218, 165, 32)); // Goldenrod color
    setPen(QPen(Qt::white));

    auto* text = new QGraphicsTextItem(funcName, this);
    text->setDefaultTextColor(Qt::black);
    text->setPos(10, 5);
}

// ==================== GraphItem Implementation ====================

GraphItem::GraphItem(const QString& name, QGraphicsItem* parent)
    : QGraphicsItem(parent), m_name(name)
{
    m_nameText = new QGraphicsTextItem(name, this);
    m_nameText->setDefaultTextColor(Qt::white);
    m_nameText->setFont(QFont("Consolas", 14, QFont::Bold));
    m_nameText->setPos(-50, -50); // Label top-left
}

QRectF GraphItem::boundingRect() const { return childrenBoundingRect(); }
void GraphItem::paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*) {}

void GraphItem::updateData(const QJsonObject& adjList) {
    // 1. Cleanup old items
    for(auto& node : m_nodes) {
        delete node.circle;
        delete node.label;
    }
    m_nodes.clear();
    qDeleteAll(m_edges);
    m_edges.clear();

    // 2. Identify all unique nodes
    QStringList nodeKeys = adjList.keys();
    if (nodeKeys.isEmpty()) return;

    // 3. Calculate Circular Layout
    int N = nodeKeys.size();
    double radius = 100.0 + (N * 10); // Grow radius with more nodes
    double angleStep = 2 * M_PI / N;
    double centerX = 150, centerY = 150;

    for (int i = 0; i < N; ++i) {
        QString key = nodeKeys[i];

        // Calculate Position
        double angle = i * angleStep;
        double x = centerX + radius * qCos(angle);
        double y = centerY + radius * qSin(angle);

        // Create Visual Node (Circle)
        auto* circle = new QGraphicsEllipseItem(-20, -20, 40, 40, this);
        circle->setPos(x, y);
        circle->setBrush(QColor(100, 149, 237)); // Blue
        circle->setPen(QPen(Qt::white, 2));
        circle->setZValue(10); // Nodes above edges

        // Create Label
        auto* label = new QGraphicsTextItem(key, circle);
        label->setDefaultTextColor(Qt::white);
        label->setFont(QFont("Arial", 10, QFont::Bold));
        // Center text in circle (approx)
        label->setPos(-10, -12);

        m_nodes[key] = {circle, label, QPointF(x, y)};
    }

    // 4. Create Edges (Lines)
    for (const QString& source : nodeKeys) {
        QJsonArray neighbors = adjList[source].toArray();
        for (const QJsonValue& val : neighbors) {
            // Support both int and string neighbor IDs
            QString target = val.isString() ? val.toString() : QString::number(val.toInt());

            if (m_nodes.contains(source) && m_nodes.contains(target)) {
                QPointF p1 = m_nodes[source].pos;
                QPointF p2 = m_nodes[target].pos;

                auto* line = new QGraphicsLineItem(p1.x(), p1.y(), p2.x(), p2.y(), this);
                line->setPen(QPen(Qt::gray, 2));
                line->setZValue(0); // Edges below nodes
                m_edges.append(line);
            }
        }
    }
}
// ==================== ASTVisualizer Implementation ====================

ASTVisualizer::ASTVisualizer(QGraphicsScene* scene, QObject* parent)
    : QObject(parent), m_scene(scene), m_currentStep(-1), m_mode(VisualizationMode::Generic),
    m_errorItem(nullptr), m_consoleItem(nullptr)
{
    // Initialize items safely
    reset();
}

void ASTVisualizer::setMode(VisualizationMode mode) {
    m_mode = mode;
    // Propagate mode change to all array items
    for(auto* item : m_arrayItems) {
        item->setMode(mode);
    }
}

void ASTVisualizer::loadSteps(const QJsonArray& steps) {
    reset();
    m_steps = steps;
}

void ASTVisualizer::reset() {
    m_currentStep = -1;

    // 1. CLEAR SCENE
    // This deletes ALL QGraphicsItems managed by the scene.
    // We must NOT delete pointers manually after this point if they were added to the scene.
    m_scene->clear();

    // 2. CLEAR POINTER CONTAINERS
    // Just clear the lists, don't call delete (pointers are already invalid/deleted by scene)
    m_variableItems.clear();
    m_arrayItems.clear();
    m_stackItems.clear();

    // 3. RE-CREATE PERMANENT ITEMS
    // Since scene->clear() deleted them, we allocate new ones.

    // Error Display
    m_errorItem = new QGraphicsTextItem();
    m_errorItem->setDefaultTextColor(Qt::red);
    m_errorItem->setFont(QFont("Arial", 14));
    m_errorItem->setPos(10, 10);
    m_scene->addItem(m_errorItem);

    // Console Output Display
    m_consoleItem = new QGraphicsTextItem("Console Output:\n");
    m_consoleItem->setDefaultTextColor(QColor(100, 255, 100)); // Matrix Green
    m_consoleItem->setFont(QFont("Consolas", 12));
    m_consoleItem->setPos(10, 600); // Place at bottom of canvas
    m_scene->addItem(m_consoleItem);
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

    // --- 1. Handle Runtime Errors ---
    if (eventType == "error") {
        if(m_errorItem) m_errorItem->setPlainText("Runtime Error:\n" + step["message"].toString());
        return;
    }

    // --- 2. Handle Console Print Output ---
    if (eventType == "print_output") {
        if(m_consoleItem) {
            QString output = step["message"].toString();
            m_consoleItem->setPlainText("Console Output:\n" + output);
        }
        return;
    }

    // --- 3. Sync Code Editor Line ---
    if (step.contains("line")) {
        emit highlightLine(step["line"].toInt());
    }

    // --- 4. Process Variables & Arrays ---
    QJsonObject vars = step["variables"].toObject();
    for (auto it = vars.begin(); it != vars.end(); ++it) {
        QString name = it.key();
        QJsonValue val = it.value();

        // LOGIC: If we are in Graph Mode, and the value is a Dictionary (Object), treat as Graph
        if (m_mode == VisualizationMode::Graph && val.isObject()) {
            syncGraph(name, val.toObject());
        }
        else if (val.isArray()) {
            syncArray(name, val.toArray());
        }
        else {
            syncVariable(name, val.toVariant());
        }
    }

    // --- 5. Process Call Stack ---
    if (step.contains("stack")) {
        syncStack(step["stack"].toArray());
    }
}

void ASTVisualizer::syncVariable(const QString& name, const QVariant& value) {
    // If it's already an array, don't overwrite it as a variable
    if (m_arrayItems.contains(name)) return;

    // Create if new
    if (!m_variableItems.contains(name)) {
        auto* item = new VariableItem(name);
        item->setPos(20, 50 + m_variableItems.size() * 50); // Stack vertically
        m_scene->addItem(item);
        m_variableItems[name] = item;
    }
    // Update value
    m_variableItems[name]->updateValue(value.toString());
}

void ASTVisualizer::syncArray(const QString& name, const QJsonArray& listData) {
    QVector<int> values;
    for (auto v : listData) values.append(v.toInt());

    // Create if new
    if (!m_arrayItems.contains(name)) {
        auto* item = new ArrayItem(name);
        item->setMode(m_mode);
        item->setPos(250, 150 + m_arrayItems.size() * 150); // Position in center
        m_scene->addItem(item);
        m_arrayItems[name] = item;
    }
    // Update data
    m_arrayItems[name]->updateData(values);
}

void ASTVisualizer::syncStack(const QJsonArray& stackData) {
    // 1. Remove old stack items safely
    // Since these change every step (pushes/pops), we just rebuild them.
    for(auto* item : m_stackItems) {
        m_scene->removeItem(item);
        delete item;
    }
    m_stackItems.clear();

    // 2. Build new stack visualization
    int depth = 0;
    for (auto v : stackData) {
        QString funcName = v.toString();
        auto* item = new CallStackItem(funcName, depth);

        // Position on the right side of the canvas
        item->setPos(900, 50 + depth * 35);

        m_scene->addItem(item);
        m_stackItems.append(item);
        depth++;
    }
}

void ASTVisualizer::syncGraph(const QString& name, const QJsonObject& graphData) {
    if (!m_graphItems.contains(name)) {
        auto* item = new GraphItem(name);
        item->setPos(400, 100); // Position to the right of arrays
        m_scene->addItem(item);
        m_graphItems[name] = item;
    }
    m_graphItems[name]->updateData(graphData);
}
