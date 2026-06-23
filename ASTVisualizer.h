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
#include <QGraphicsObject>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>

// 1. FIX: Added 'Recursion' to VisualizationMode
enum class VisualizationMode { Generic, Sorting, Graph, Recursion };

// ==================== NodeItem (Draggable Circle) ====================
class NodeItem : public QGraphicsEllipseItem {
public:
    NodeItem(QGraphicsItem* parent = nullptr);
    void addEdge(QGraphicsLineItem* line, bool isStart);
    void clearEdges() { edges.clear(); }
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
    void updateData(const QJsonObject& adjList, const QJsonObject& metadata = QJsonObject());
    void setTheme(const QJsonObject& theme);
    void highlightNode(const QString& label);
    void setDisplayName(const QString& name);
    QRectF boundingRect() const override;
    void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*) override;

private:
    QString m_name;
    QGraphicsTextItem* m_nameText;
    struct NodeData { NodeItem* circle; QGraphicsTextItem* label; QPointF pos; QString displayLabel; };
    QMap<QString, NodeData> m_nodes;
    QList<QGraphicsLineItem*> m_edges;
    QColor m_primaryColor = QColor("#00d4ff");
    QColor m_highlightColor = QColor("#ff79c6");
    QColor m_sortedColor = QColor("#50fa7b");
};

// ==================== AnimatedBarItem ====================
class AnimatedBarItem : public QGraphicsObject {
    Q_OBJECT
    Q_PROPERTY(QRectF rect READ rect WRITE setRect)
public:
    AnimatedBarItem(QGraphicsItem* parent = nullptr);
    void setVisuals(const QColor& color, const QRectF& rect, const QString& text);
    QRectF rect() const { return m_rect; }
    void setRect(const QRectF& r) { m_rect = r; update(); }
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
private:
    QRectF m_rect;
    QColor m_color;
    QString m_text;
};

// ==================== ArrayItem ====================
class ArrayItem : public QGraphicsItem {
public:
    ArrayItem(const QString& name, QGraphicsItem* parent = nullptr);
    void setMode(VisualizationMode mode);
    void setDataStructureType(const QString& name);
    void updateData(const QVector<int>& values);
    void updateStepContext(const QJsonObject& vars);
    void setTheme(const QJsonObject& theme);
    QRectF boundingRect() const override;
    void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*) override;

private:
    void updateLayout();
    QString m_name;
    VisualizationMode m_mode;
    bool m_isStack = false;
    bool m_isQueue = false;
    QVector<int> m_values;
    QSet<int> m_highlightIndices;
    QSet<int> m_compareIndices;
    int m_sortedStart;
    int m_maxObservedValue;
    QParallelAnimationGroup* m_animGroup = nullptr;
    QGraphicsTextItem* m_nameText;
    QList<AnimatedBarItem*> m_visualElements;
    QColor m_primaryColor = QColor("#00d4ff");
    QColor m_highlightColor = QColor("#ff79c6");
    QColor m_sortedColor = QColor("#50fa7b");
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
    void logMessage(const QString& msg, bool clear = false);
    void setAIMetadata(const QJsonObject& metadata);

signals:
    void stepExecuted(int currentStep, int totalSteps);
    void executionFinished();
    void highlightLine(int line);

private:
    void processStep(const QJsonObject& step);
    void syncVariable(const QString& name, const QVariant& value);
    void syncArray(const QString& name, const QJsonArray& listData, const QJsonObject& vars);
    void syncStack(const QJsonArray& stackData);
    void syncGraph(const QString& name, const QJsonObject& graphData, const QJsonObject& metadata = QJsonObject(), const QString& displayName = QString());

    QGraphicsScene* m_scene;
    QJsonArray m_steps;
    int m_currentStep;
    VisualizationMode m_mode;
    QGraphicsTextItem* m_errorItem;
    QGraphicsTextItem* m_consoleItem;
    QJsonObject m_aiMetadata;

    QMap<QString, VariableItem*> m_variableItems;
    QMap<QString, ArrayItem*> m_arrayItems;
    QList<CallStackItem*> m_stackItems;
    QMap<QString, GraphItem*> m_graphItems;
    QMap<QString, QString> m_treeOwnerScope; // tracks which function scope owns each tree graph
};

#endif // ASTVISUALIZER_H
