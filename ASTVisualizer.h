#ifndef ASTVISUALIZER_H
#define ASTVISUALIZER_H

#include <QObject>
#include <QJsonArray>
#include <QJsonObject>
#include <QGraphicsScene>
#include <QGraphicsRectItem>
#include <QGraphicsTextItem>
#include <QPropertyAnimation>
#include <QMap>
#include <QVector>

// ==================== Enums ====================
enum class VisualizationMode {
    Generic,    // Boxes
    Sorting,    // Bar Chart
    Recursion,   // Call Stack Focus
    Graph       //for Graphs
};

// ==================== ArrayItem ====================
class ArrayItem : public QGraphicsItem {
public:
    ArrayItem(const QString& name, QGraphicsItem* parent = nullptr);

    void setMode(VisualizationMode mode);
    void updateData(const QVector<int>& values); // Re-syncs entire array

    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

private:
    void updateLayout();

    QString m_name;
    QVector<int> m_values;
    QVector<QGraphicsRectItem*> m_visualElements;
    QVector<QGraphicsTextItem*> m_textElements;
    QGraphicsTextItem* m_nameText;
    VisualizationMode m_mode;
};

// ==================== VariableItem ====================
class VariableItem : public QGraphicsRectItem {
public:
    VariableItem(const QString& name, QGraphicsItem* parent = nullptr);
    void updateValue(const QString& value);

private:
    QGraphicsTextItem* m_nameText;
    QGraphicsTextItem* m_valueText;
};

// ==================== CallStackItem ====================
class CallStackItem : public QGraphicsRectItem {
public:
    CallStackItem(const QString& funcName, int depth, QGraphicsItem* parent = nullptr);
};

// ====================GraphItem========================
class GraphItem : public QGraphicsItem {
public:
    GraphItem(const QString& name, QGraphicsItem* parent = nullptr);
    void updateData(const QJsonObject& adjList); // Expects { "0": [1, 2], ... }

    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

private:
    QString m_name;
    QGraphicsTextItem* m_nameText;

    // Structure to hold visual parts of a node
    struct VisualNode {
        QGraphicsEllipseItem* circle;
        QGraphicsTextItem* label;
        QPointF pos;
    };

    QMap<QString, VisualNode> m_nodes;
    QVector<QGraphicsLineItem*> m_edges;
};
// ==================== ASTVisualizer ====================
class ASTVisualizer : public QObject {
    Q_OBJECT

public:
    explicit ASTVisualizer(QGraphicsScene* scene, QObject* parent = nullptr);

    void loadSteps(const QJsonArray& steps);
    void executeStep();
    void setMode(VisualizationMode mode);
    void reset();

signals:
    void stepExecuted(int step, int total);
    void highlightLine(int line);
    void executionFinished();

private:
    void processStep(const QJsonObject& step);

    // Sync functions
    void syncVariable(const QString& name, const QVariant& value);
    void syncArray(const QString& name, const QJsonArray& listData);
    void syncStack(const QJsonArray& stackData);
    void syncGraph(const QString& name, const QJsonObject& graphData);

    QGraphicsScene* m_scene;
    QJsonArray m_steps;
    int m_currentStep;
    VisualizationMode m_mode;

    QMap<QString, VariableItem*> m_variableItems;
    QMap<QString, ArrayItem*> m_arrayItems;
    QVector<CallStackItem*> m_stackItems;
    QMap<QString, GraphItem*> m_graphItems;

    QGraphicsTextItem* m_errorItem;
    QGraphicsTextItem* m_consoleItem; // <--- This was missing!
};

#endif // ASTVISUALIZER_H
