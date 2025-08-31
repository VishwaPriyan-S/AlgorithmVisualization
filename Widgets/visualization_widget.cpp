#include "visualization_widget.h"
#include "array_visualizer.h"
#include "graph_visualizer.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QMouseEvent>

VisualizationWidget::VisualizationWidget(QWidget *parent)
    : QWidget(parent)
    , m_visualizationType(ArrayType)
    , m_arrayVisualizer(nullptr)
    , m_graphVisualizer(nullptr)
    , m_currentVisualizer(nullptr)
    , m_operationLabel(nullptr)
    , m_statisticsLabel(nullptr)
    , m_mainLayout(nullptr)
    , m_animationEnabled(true)
    , m_showOperationInfo(true)
    , m_showStatistics(true)
    , m_comparisons(0)
    , m_swaps(0)
    , m_arrayAccesses(0)
{
    setupLayout();
    setVisualizationType(ArrayType);
}

void VisualizationWidget::setupLayout()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setSpacing(10);
    m_mainLayout->setContentsMargins(10, 10, 10, 10);

    // Operation info label
    m_operationLabel = new QLabel(this);
    m_operationLabel->setStyleSheet(
        "QLabel {"
        "   background-color: rgba(70, 130, 180, 100);"
        "   border: 1px solid #4682B4;"
        "   border-radius: 5px;"
        "   padding: 5px;"
        "   color: white;"
        "   font-weight: bold;"
        "}"
        );
    m_operationLabel->setAlignment(Qt::AlignCenter);
    m_operationLabel->setMinimumHeight(30);
    m_mainLayout->addWidget(m_operationLabel);

    // Statistics label
    m_statisticsLabel = new QLabel(this);
    m_statisticsLabel->setStyleSheet(
        "QLabel {"
        "   background-color: rgba(60, 60, 60, 150);"
        "   border: 1px solid #666;"
        "   border-radius: 5px;"
        "   padding: 5px;"
        "   color: white;"
        "   font-size: 12px;"
        "}"
        );
    m_statisticsLabel->setAlignment(Qt::AlignLeft);
    m_mainLayout->addWidget(m_statisticsLabel);

    // Visualization area (will be set by setVisualizationType)
    m_mainLayout->addStretch();
}

void VisualizationWidget::setVisualizationType(VisualizationType type)
{
    if (m_visualizationType == type && m_currentVisualizer) {
        return;
    }

    m_visualizationType = type;

    // Remove current visualizer
    if (m_currentVisualizer) {
        m_mainLayout->removeWidget(m_currentVisualizer);
        m_currentVisualizer->setParent(nullptr);
    }

    // Create new visualizer
    switch (type) {
    case ArrayType:
        if (!m_arrayVisualizer) {
            m_arrayVisualizer = new ArrayVisualizer(this);
            connect(m_arrayVisualizer, &ArrayVisualizer::elementClicked,
                    this, &VisualizationWidget::visualizationClicked);
            connect(m_arrayVisualizer, &ArrayVisualizer::animationFinished,
                    this, &VisualizationWidget::animationFinished);
        }
        m_currentVisualizer = m_arrayVisualizer;
        break;

    case GraphType:
        if (!m_graphVisualizer) {
            m_graphVisualizer = new GraphVisualizer(this);
            connect(m_graphVisualizer, &GraphVisualizer::nodeClicked,
                    this, &VisualizationWidget::visualizationClicked);
            connect(m_graphVisualizer, &GraphVisualizer::animationFinished,
                    this, &VisualizationWidget::animationFinished);
        }
        m_currentVisualizer = m_graphVisualizer;
        break;

    default:
        m_currentVisualizer = nullptr;
        break;
    }

    // Add new visualizer
    if (m_currentVisualizer) {
        m_mainLayout->insertWidget(m_mainLayout->count() - 1, m_currentVisualizer, 1);
    }

    update();
}

void VisualizationWidget::setVisualizationData(const VisualizationData& data)
{
    m_currentData = data;
    updateVisualization(data);
}

void VisualizationWidget::animateToStep(const VisualizationData& data, int duration)
{
    if (m_animationEnabled) {
        switch (m_visualizationType) {
        case ArrayType:
            if (m_arrayVisualizer) {
                m_arrayVisualizer->animateToData(data, duration);
            }
            break;
        case GraphType:
            if (m_graphVisualizer) {
                m_graphVisualizer->animateToData(data, duration);
            }
            break;
        }
    } else {
        updateVisualization(data);
    }
}

void VisualizationWidget::updateVisualization(const VisualizationData& data)
{
    m_currentData = data;

    // Update statistics
    for (const auto& comparison : data.comparisons) {
        m_comparisons++;
    }
    for (const auto& swap : data.swaps) {
        m_swaps++;
    }
    m_arrayAccesses += data.highlightedIndices.size();

    // Update displays
    updateOperationDisplay();
    updateStatisticsDisplay();

    // Update visualizer
    switch (m_visualizationType) {
    case ArrayType:
        if (m_arrayVisualizer) {
            m_arrayVisualizer->setData(data);
        }
        break;
    case GraphType:
        if (m_graphVisualizer) {
            m_graphVisualizer->setData(data);
        }
        break;
    }
}

void VisualizationWidget::clearVisualization()
{
    VisualizationData emptyData;
    m_comparisons = 0;
    m_swaps = 0;
    m_arrayAccesses = 0;

    updateVisualization(emptyData);
}

void VisualizationWidget::updateOperationDisplay()
{
    if (m_operationLabel) {
        QString text = m_currentData.currentOperation;
        if (text.isEmpty()) {
            text = "Ready";
        }
        m_operationLabel->setText(text);
        m_operationLabel->setVisible(m_showOperationInfo);
    }
}

void VisualizationWidget::updateStatisticsDisplay()
{
    if (m_statisticsLabel) {
        QString stats = QString("Comparisons: %1  |  Swaps: %2  |  Array Accesses: %3")
        .arg(m_comparisons)
            .arg(m_swaps)
            .arg(m_arrayAccesses);
        m_statisticsLabel->setText(stats);
        m_statisticsLabel->setVisible(m_showStatistics);
    }
}

void VisualizationWidget::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);
}

void VisualizationWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
}

void VisualizationWidget::mousePressEvent(QMouseEvent *event)
{
    QWidget::mousePressEvent(event);
}

void VisualizationWidget::onAnimationFinished()
{
    emit animationFinished();
}
