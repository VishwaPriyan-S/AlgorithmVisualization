#ifndef GRAPH_VISUALIZER_H
#define GRAPH_VISUALIZER_H

#include <QWidget>
#include <QPainter>
#include <QPropertyAnimation>
#include <QPoint>
#include <QMap>
#include "../step_data.h"

struct GraphNode {
    int id;
    QPoint position;
    QColor color;
    QString label;
    int radius;
    bool isHighlighted;
    bool isVisited;

    GraphNode() : id(-1), radius(20), isHighlighted(false), isVisited(false) {}
    GraphNode(int nodeId, const QPoint& pos, const QString& text = "")
        : id(nodeId), position(pos), label(text), radius(20),
        isHighlighted(false), isVisited(false), color(QColorConstants::Svg::lightblue) {}
};

struct GraphEdge {
    int fromNode;
    int toNode;
    QColor color;
    bool isHighlighted;
    bool isDirected;
    int weight;

    GraphEdge() : fromNode(-1), toNode(-1), isHighlighted(false),
        isDirected(false), weight(1), color(Qt::black) {}
    GraphEdge(int from, int to, bool directed = false, int w = 1)
        : fromNode(from), toNode(to), isHighlighted(false),
        isDirected(directed), weight(w), color(Qt::black) {}
};

class GraphVisualizer : public QWidget
{
    Q_OBJECT

public:
    explicit GraphVisualizer(QWidget *parent = nullptr);

    void setData(const VisualizationData& data);
    void animateToData(const VisualizationData& newData, int duration = 300);

    // Graph structure
    void addNode(int id, const QPoint& position, const QString& label = "");
    void addEdge(int fromNode, int toNode, bool directed = false, int weight = 1);
    void clearGraph();

    // Node highlighting
    void highlightNode(int nodeId, const QColor& color = Qt::yellow);
    void highlightEdge(int fromNode, int toNode, const QColor& color = Qt::red);
    void setNodeVisited(int nodeId, bool visited = true);

    // Layout algorithms
    void autoLayout();
    void circularLayout();
    void gridLayout(int columns);

public slots:
    void resetVisualization();

signals:
    void nodeClicked(int nodeId);
    void edgeClicked(int fromNode, int toNode);
    void animationFinished();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    void drawNode(QPainter& painter, const GraphNode& node);
    void drawEdge(QPainter& painter, const GraphEdge& edge);
    void drawEdgeLabel(QPainter& painter, const GraphEdge& edge);

    int getNodeAt(const QPoint& pos) const;
    GraphNode* getNode(int nodeId);
    const GraphNode* getNode(int nodeId) const;

    QPoint calculateEdgeStart(const GraphNode& from, const GraphNode& to) const;
    QPoint calculateEdgeEnd(const GraphNode& from, const GraphNode& to) const;

private:
    QMap<int, GraphNode> m_nodes;
    QVector<GraphEdge> m_edges;

    // Mouse interaction
    bool m_dragging;
    int m_draggedNode;
    QPoint m_lastMousePos;

    // Visual settings
    QColor m_backgroundColor;
    QColor m_defaultNodeColor;
    QColor m_highlightedNodeColor;
    QColor m_visitedNodeColor;
    QColor m_defaultEdgeColor;
    QColor m_highlightedEdgeColor;

    // Animation
    QPropertyAnimation* m_animation;
    bool m_isAnimating;
};

#endif
