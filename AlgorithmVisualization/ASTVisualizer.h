#ifndef ASTVISUALIZER_H
#define ASTVISUALIZER_H

#include <QObject>
#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QGraphicsRectItem>
#include <QGraphicsTextItem>
#include <QGraphicsLineItem>
#include <QJsonObject>
#include <QJsonArray>
#include <QMap>
#include <QVector>
#include <QSet>

// 1. FIX: Added 'Recursion' to VisualizationMode
enum class VisualizationMode { Generic, Sorting, Graph, Recursion };

// ==================== NodeItem (Draggable Circle) ====================
class NodeItem : public QGraphicsEllipseItem {
public:
    NodeItem(QGraphicsItem* parent = nullptr);
    void addEdge(QGraphicsLineItem* line, bool isStart);
protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;
private:
    struct EdgeInfo { QGraphicsLineItem* line; bool isStart; };
    QVector<EdgeInfo> edges;
};

// ==================== GraphItem ====================
class GraphItem : public QGraphicsItem {
public:
    GraphItem(const QString& name, QGraphicsItem* parent = nullptr);
    void updateData(const QJsonObject& adjList);
    QRectF boundingRect() const override;
    void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*) override;

private:
    QString m_name;
    QGraphicsTextItem* m_nameText;
    struct NodeData { NodeItem* circle; QGraphicsTextItem* label; QPointF pos; };
    QMap<QString, NodeData> m_nodes;
    QList<QGraphicsLineItem*> m_edges;
};

// ==================== ArrayItem ====================
class ArrayItem : public QGraphicsItem {
public:
    ArrayItem(const QString& name, QGraphicsItem* parent = nullptr);
    void setMode(VisualizationMode mode);
    void updateData(const QVector<int>& values); // Now detects changes
    QRectF boundingRect() const override;
    void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*) override;

private:
    void updateLayout();
    QString m_name;
    VisualizationMode m_mode;
    QVector<int> m_values;
    QSet<int> m_highlightIndices; // 2. FIX: Track changed indices
    QGraphicsTextItem* m_nameText;
    QList<QGraphicsRectItem*> m_visualElements;
    QList<QGraphicsTextItem*> m_textElements;
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

// ==================== ASTVisualizer (Main) ====================
class ASTVisualizer : public QObject {
    Q_OBJECT
public:
    explicit ASTVisualizer(QGraphicsScene* scene, QObject* parent = nullptr);
    void loadSteps(const QJsonArray& steps);
    void executeStep();
    void setMode(VisualizationMode mode);
    void reset();

signals:
    void stepExecuted(int currentStep, int totalSteps);
    void executionFinished();
    void highlightLine(int line);

private:
    void processStep(const QJsonObject& step);
    void syncVariable(const QString& name, const QVariant& value);
    void syncArray(const QString& name, const QJsonArray& listData);
    void syncStack(const QJsonArray& stackData);
    void syncGraph(const QString& name, const QJsonObject& graphData);

    QGraphicsScene* m_scene;
    QJsonArray m_steps;
    int m_currentStep;
    VisualizationMode m_mode;
    QGraphicsTextItem* m_errorItem;
    QGraphicsTextItem* m_consoleItem;

    QMap<QString, VariableItem*> m_variableItems;
    QMap<QString, ArrayItem*> m_arrayItems;
    QList<CallStackItem*> m_stackItems;
    QMap<QString, GraphItem*> m_graphItems;
};

#endif // ASTVISUALIZER_H
