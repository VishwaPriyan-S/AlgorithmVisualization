#ifndef ARRAY_VISUALIZER_H
#define ARRAY_VISUALIZER_H

#include <QWidget>
#include <QPainter>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QEasingCurve>
#include <QTimer>
#include "../step_data.h"

class ArrayVisualizer : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(qreal animationProgress READ animationProgress WRITE setAnimationProgress)

public:
    explicit ArrayVisualizer(QWidget *parent = nullptr);

    void setData(const VisualizationData& data);
    void animateToData(const VisualizationData& newData, int duration = 300);

    // Visual settings
    void setBarColors(const QColor& normal, const QColor& highlight,
                      const QColor& compare, const QColor& swap);
    void setDrawNumbers(bool draw) { m_drawNumbers = draw; update(); }
    void setDrawIndices(bool draw) { m_drawIndices = draw; update(); }
    void setBarSpacing(int spacing) { m_barSpacing = spacing; calculateLayout(); }

    // Animation properties
    qreal animationProgress() const { return m_animationProgress; }
    void setAnimationProgress(qreal progress);

public slots:
    void resetStatistics();

signals:
    void elementClicked(int index);
    void animationFinished();

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private:
    struct BarInfo {
        QRect rect;
        int value;
        QColor color;
        bool isAnimating;
        QPoint startPos;
        QPoint targetPos;
    };

    void calculateLayout();
    void updateBarColors();
    QColor getBarColor(int index) const;
    int getBarIndexAt(const QPoint& pos) const;
    void drawBar(QPainter& painter, const BarInfo& bar, int index);
    void drawValue(QPainter& painter, const BarInfo& bar, int index);
    void drawIndex(QPainter& painter, const BarInfo& bar, int index);
    void drawComparisons(QPainter& painter);
    void drawSwaps(QPainter& painter);

private slots:
    void onAnimationFinished();

private:
    VisualizationData m_currentData;
    VisualizationData m_targetData;
    QVector<BarInfo> m_bars;

    // Layout
    QRect m_drawingRect;
    int m_barWidth;
    int m_barSpacing;
    int m_maxValue;
    int m_minValue;

    // Visual settings
    QColor m_normalColor;
    QColor m_highlightColor;
    QColor m_compareColor;
    QColor m_swapColor;
    bool m_drawNumbers;
    bool m_drawIndices;

    // Animation
    QPropertyAnimation* m_animation;
    qreal m_animationProgress;
    bool m_isAnimating;
};

#endif
