#include "array_visualizer.h"
#include <QPainter>
#include <QMouseEvent>
#include <QPainterPath>

#include <QFontMetrics>
#include <algorithm>

ArrayVisualizer::ArrayVisualizer(QWidget *parent)
    : QWidget(parent)
    , m_barWidth(0)
    , m_barSpacing(2)
    , m_maxValue(100)
    , m_minValue(0)
    , m_normalColor(QColor(70, 130, 180))
    , m_highlightColor(QColor(255, 215, 0))
    , m_compareColor(QColor(255, 69, 0))
    , m_swapColor(QColor(50, 205, 50))
    , m_drawNumbers(true)
    , m_drawIndices(true)
    , m_animation(new QPropertyAnimation(this, "animationProgress"))
    , m_animationProgress(0.0)
    , m_isAnimating(false)
{
    setMinimumHeight(200);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    connect(m_animation, &QPropertyAnimation::finished,
            this, &ArrayVisualizer::onAnimationFinished);

    m_animation->setEasingCurve(QEasingCurve::OutCubic);
}

void ArrayVisualizer::setData(const VisualizationData& data)
{
    m_currentData = data;

    if (!data.array.isEmpty()) {
        m_maxValue = *std::max_element(data.array.begin(), data.array.end());
        m_minValue = *std::min_element(data.array.begin(), data.array.end());
    }

    calculateLayout();
    updateBarColors();
    update();
}

void ArrayVisualizer::animateToData(const VisualizationData& newData, int duration)
{
    if (m_isAnimating) {
        m_animation->stop();
    }

    m_targetData = newData;
    m_isAnimating = true;

    m_animation->setDuration(duration);
    m_animation->setStartValue(0.0);
    m_animation->setEndValue(1.0);
    m_animation->start();
}

void ArrayVisualizer::setBarColors(const QColor& normal, const QColor& highlight,
                                   const QColor& compare, const QColor& swap)
{
    m_normalColor = normal;
    m_highlightColor = highlight;
    m_compareColor = compare;
    m_swapColor = swap;
    updateBarColors();
    update();
}

void ArrayVisualizer::setAnimationProgress(qreal progress)
{
    m_animationProgress = progress;

    // Interpolate between current and target data
    if (m_isAnimating) {
        VisualizationData interpolatedData = m_currentData;

        // Interpolate array values if sizes match
        if (m_currentData.array.size() == m_targetData.array.size()) {
            for (int i = 0; i < interpolatedData.array.size(); ++i) {
                int currentValue = m_currentData.array[i];
                int targetValue = m_targetData.array[i];
                interpolatedData.array[i] = currentValue +
                                            (targetValue - currentValue) * progress;
            }
        }

        // Update highlights, comparisons, etc. at the end of animation
        if (progress >= 1.0) {
            interpolatedData.highlightedIndices = m_targetData.highlightedIndices;
            interpolatedData.highlightColors = m_targetData.highlightColors;
            interpolatedData.comparisons = m_targetData.comparisons;
            interpolatedData.swaps = m_targetData.swaps;
            interpolatedData.currentOperation = m_targetData.currentOperation;
        }

        // Update visualization with interpolated data
        if (!interpolatedData.array.isEmpty()) {
            m_maxValue = *std::max_element(interpolatedData.array.begin(), interpolatedData.array.end());
            m_minValue = *std::min_element(interpolatedData.array.begin(), interpolatedData.array.end());
        }

        calculateLayout();
        updateBarColors();
    }

    update();
}

void ArrayVisualizer::resetStatistics()
{
    // This could be used to reset any internal statistics
    update();
}

void ArrayVisualizer::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Clear background
    painter.fillRect(rect(), QColor(40, 40, 40));

    if (m_bars.isEmpty()) {
        // Draw placeholder message
        painter.setPen(Qt::white);
        painter.setFont(QFont("Arial", 14));
        painter.drawText(rect(), Qt::AlignCenter, "No data to visualize");
        return;
    }

    // Draw bars
    for (int i = 0; i < m_bars.size(); ++i) {
        drawBar(painter, m_bars[i], i);

        if (m_drawNumbers) {
            drawValue(painter, m_bars[i], i);
        }

        if (m_drawIndices) {
            drawIndex(painter, m_bars[i], i);
        }
    }

    // Draw comparisons and swaps
    drawComparisons(painter);
    drawSwaps(painter);
}

void ArrayVisualizer::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    calculateLayout();
}

void ArrayVisualizer::mousePressEvent(QMouseEvent *event)
{
    int index = getBarIndexAt(event->pos());
    if (index >= 0) {
        emit elementClicked(index);
    }
    QWidget::mousePressEvent(event);
}

void ArrayVisualizer::calculateLayout()
{
    if (m_currentData.array.isEmpty()) {
        m_bars.clear();
        return;
    }

    // Calculate drawing rectangle (leave space for labels)
    int margin = 20;
    int bottomMargin = m_drawIndices ? 40 : 20;
    int topMargin = m_drawNumbers ? 30 : 20;

    m_drawingRect = rect().adjusted(margin, topMargin, -margin, -bottomMargin);

    if (m_drawingRect.width() <= 0 || m_drawingRect.height() <= 0) {
        return;
    }

    // Calculate bar dimensions
    int arraySize = m_currentData.array.size();
    int totalSpacing = (arraySize - 1) * m_barSpacing;
    m_barWidth = (m_drawingRect.width() - totalSpacing) / arraySize;
    m_barWidth = qMax(1, m_barWidth);

    // Create bars
    m_bars.clear();
    m_bars.reserve(arraySize);

    int valueRange = m_maxValue - m_minValue;
    if (valueRange == 0) valueRange = 1;

    for (int i = 0; i < arraySize; ++i) {
        BarInfo bar;
        bar.value = m_currentData.array[i];
        bar.isAnimating = false;

        // Calculate bar height
        int barHeight = ((bar.value - m_minValue) * m_drawingRect.height()) / valueRange;
        barHeight = qMax(5, barHeight); // Minimum height

        // Calculate bar position
        int x = m_drawingRect.left() + i * (m_barWidth + m_barSpacing);
        int y = m_drawingRect.bottom() - barHeight;

        bar.rect = QRect(x, y, m_barWidth, barHeight);
        bar.color = getBarColor(i);

        m_bars.append(bar);
    }
}

void ArrayVisualizer::updateBarColors()
{
    for (int i = 0; i < m_bars.size(); ++i) {
        m_bars[i].color = getBarColor(i);
    }
}

QColor ArrayVisualizer::getBarColor(int index) const
{
    // Check for swaps first (highest priority)
    for (const auto& swap : m_currentData.swaps) {
        if (swap.index1 == index || swap.index2 == index) {
            return swap.color.isValid() ? swap.color : m_swapColor;
        }
    }

    // Check for comparisons
    for (const auto& comparison : m_currentData.comparisons) {
        if (comparison.index1 == index || comparison.index2 == index) {
            return comparison.color.isValid() ? comparison.color : m_compareColor;
        }
    }

    // Check for highlights
    for (int i = 0; i < m_currentData.highlightedIndices.size(); ++i) {
        if (m_currentData.highlightedIndices[i] == index) {
            if (i < m_currentData.highlightColors.size()) {
                return m_currentData.highlightColors[i];
            }
            return m_highlightColor;
        }
    }

    return m_normalColor;
}

int ArrayVisualizer::getBarIndexAt(const QPoint& pos) const
{
    for (int i = 0; i < m_bars.size(); ++i) {
        if (m_bars[i].rect.contains(pos)) {
            return i;
        }
    }
    return -1;
}

void ArrayVisualizer::drawBar(QPainter& painter, const BarInfo& bar, int index)
{
    // Draw bar with gradient effect
    QLinearGradient gradient(bar.rect.topLeft(), bar.rect.topRight());
    gradient.setColorAt(0, bar.color);
    gradient.setColorAt(1, bar.color.darker(120));

    painter.setBrush(QBrush(gradient));
    painter.setPen(QPen(bar.color.darker(150), 1));
    painter.drawRect(bar.rect);

    // Draw border highlight for selected bars
    if (bar.color != m_normalColor) {
        painter.setPen(QPen(Qt::white, 2));
        painter.drawRect(bar.rect);
    }
}

void ArrayVisualizer::drawValue(QPainter& painter, const BarInfo& bar, int index)
{
    painter.setPen(Qt::white);
    painter.setFont(QFont("Arial", 10, QFont::Bold));

    QString valueText = QString::number(bar.value);
    QFontMetrics fm(painter.font());
    QRect textRect = fm.boundingRect(valueText);

    // Position text above the bar
    QPoint textPos;
    textPos.setX(bar.rect.center().x() - textRect.width() / 2);
    textPos.setY(bar.rect.top() - 5);

    painter.drawText(textPos, valueText);
}

void ArrayVisualizer::drawIndex(QPainter& painter, const BarInfo& bar, int index)
{
    painter.setPen(QColor(180, 180, 180));
    painter.setFont(QFont("Arial", 9));

    QString indexText = QString::number(index);
    QFontMetrics fm(painter.font());
    QRect textRect = fm.boundingRect(indexText);

    // Position text below the bar
    QPoint textPos;
    textPos.setX(bar.rect.center().x() - textRect.width() / 2);
    textPos.setY(bar.rect.bottom() + 15);

    painter.drawText(textPos, indexText);
}

void ArrayVisualizer::drawComparisons(QPainter& painter)
{
    for (const auto& comparison : m_currentData.comparisons) {
        if (comparison.index1 >= 0 && comparison.index1 < m_bars.size() &&
            comparison.index2 >= 0 && comparison.index2 < m_bars.size()) {

            // Draw comparison arc
            QPoint start = m_bars[comparison.index1].rect.center();
            QPoint end = m_bars[comparison.index2].rect.center();
            start.setY(m_bars[comparison.index1].rect.top() - 10);
            end.setY(m_bars[comparison.index2].rect.top() - 10);

            painter.setPen(QPen(comparison.color, 3));

            // Draw arc using cubic bezier curve
            QPainterPath path;
            path.moveTo(start);

            QPoint control1 = start;
            QPoint control2 = end;
            control1.setY(start.y() - 30);
            control2.setY(end.y() - 30);

            path.cubicTo(control1, control2, end);
            painter.drawPath(path);

            // Draw arrows
            painter.setBrush(comparison.color);
            QPolygon arrow1, arrow2;
            arrow1 << QPoint(start.x()-3, start.y()) << QPoint(start.x()+3, start.y())
                   << QPoint(start.x(), start.y()-6);
            arrow2 << QPoint(end.x()-3, end.y()) << QPoint(end.x()+3, end.y())
                   << QPoint(end.x(), end.y()-6);
            painter.drawPolygon(arrow1);
            painter.drawPolygon(arrow2);
        }
    }
}

void ArrayVisualizer::drawSwaps(QPainter& painter)
{
    for (const auto& swap : m_currentData.swaps) {
        if (swap.index1 >= 0 && swap.index1 < m_bars.size() &&
            swap.index2 >= 0 && swap.index2 < m_bars.size()) {

            // Draw swap arrows
            QPoint start = m_bars[swap.index1].rect.center();
            QPoint end = m_bars[swap.index2].rect.center();

            painter.setPen(QPen(swap.color, 4));
            painter.drawLine(start, end);

            // Draw arrow heads
            QVector2D direction(end - start);
            direction.normalize();
            QVector2D perpendicular(-direction.y(), direction.x());

            QPoint arrowHead1 = end - QPoint(direction.x() * 10, direction.y() * 10);
            QPoint arrowSide1 = arrowHead1 + QPoint(perpendicular.x() * 5, perpendicular.y() * 5);
            QPoint arrowSide2 = arrowHead1 - QPoint(perpendicular.x() * 5, perpendicular.y() * 5);

            painter.setBrush(swap.color);
            QPolygon arrowHead;
            arrowHead << end << arrowSide1 << arrowSide2;
            painter.drawPolygon(arrowHead);
        }
    }
}

void ArrayVisualizer::onAnimationFinished()
{
    m_isAnimating = false;
    m_currentData = m_targetData;
    calculateLayout();
    updateBarColors();
    update();
    emit animationFinished();
}
