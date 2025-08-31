#ifndef VISUALIZATION_WIDGET_H
#define VISUALIZATION_WIDGET_H

#include <QWidget>
#include <QPainter>
#include <QPropertyAnimation>
#include <QTimer>
#include <QLabel>
#include <QVBoxLayout>
#include "../step_data.h"

class ArrayVisualizer;
class GraphVisualizer;

class VisualizationWidget : public QWidget
{
    Q_OBJECT

public:
    enum VisualizationType {
        ArrayType,
        GraphType,
        TreeType
    };

public:
    explicit VisualizationWidget(QWidget *parent = nullptr);

    void setVisualizationType(VisualizationType type);
    void setVisualizationData(const VisualizationData& data);
    void animateToStep(const VisualizationData& data, int duration = 300);
    void setAnimationEnabled(bool enabled) { m_animationEnabled = enabled; }

    // Display control
    void showOperationInfo(bool show) { m_showOperationInfo = show; updateOperationDisplay(); }
    void showStatistics(bool show) { m_showStatistics = show; updateStatisticsDisplay(); }

public slots:
    void updateVisualization(const VisualizationData& data);
    void clearVisualization();

signals:
    void visualizationClicked(int index);
    void animationFinished();

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private slots:
    void onAnimationFinished();

private:
    void setupLayout();
    void updateOperationDisplay();
    void updateStatisticsDisplay();

private:
    VisualizationType m_visualizationType;
    VisualizationData m_currentData;

    // Child widgets
    ArrayVisualizer* m_arrayVisualizer;
    GraphVisualizer* m_graphVisualizer;
    QWidget* m_currentVisualizer;

    // Information display
    QLabel* m_operationLabel;
    QLabel* m_statisticsLabel;
    QVBoxLayout* m_mainLayout;

    // Animation
    bool m_animationEnabled;
    bool m_showOperationInfo;
    bool m_showStatistics;

    // Statistics
    int m_comparisons;
    int m_swaps;
    int m_arrayAccesses;
};

#endif
