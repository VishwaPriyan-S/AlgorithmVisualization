#include "control_panel.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QIcon>
#include <QApplication>
#include <QStyle>

ControlPanel::ControlPanel(QWidget *parent)
    : QWidget(parent)
    , m_playPauseButton(nullptr)
    , m_stopButton(nullptr)
    , m_resetButton(nullptr)
    , m_stepBackwardButton(nullptr)
    , m_stepForwardButton(nullptr)
    , m_goToBeginningButton(nullptr)
    , m_goToEndButton(nullptr)
    , m_progressBar(nullptr)
    , m_stepSpinBox(nullptr)
    , m_progressLabel(nullptr)
    , m_speedSlider(nullptr)
    , m_speedLabel(nullptr)
    , m_mainLayout(nullptr)
    , m_controlButtonsLayout(nullptr)
    , m_progressLayout(nullptr)
    , m_speedLayout(nullptr)
    , m_isPlaying(false)
    , m_currentStep(0)
    , m_totalSteps(0)
    , m_currentSpeed(50)
{
    setupUI();
    setFixedHeight(80); // Compact layout
}

void ControlPanel::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setSpacing(8);
    m_mainLayout->setContentsMargins(10, 5, 10, 5);

    // --- Control buttons row ---
    m_controlButtonsLayout = new QHBoxLayout();
    m_controlButtonsLayout->setSpacing(5);

    // Go to beginning
    m_goToBeginningButton = new QPushButton(this);
    m_goToBeginningButton->setIcon(style()->standardIcon(QStyle::SP_MediaSkipBackward));
    m_goToBeginningButton->setToolTip("Go to Beginning");
    m_goToBeginningButton->setFixedSize(40, 30);
    connect(m_goToBeginningButton, &QPushButton::clicked, this, &ControlPanel::goToBeginningClicked);

    // Step backward
    m_stepBackwardButton = new QPushButton(this);
    m_stepBackwardButton->setIcon(style()->standardIcon(QStyle::SP_MediaSeekBackward));
    m_stepBackwardButton->setToolTip("Step Backward");
    m_stepBackwardButton->setFixedSize(40, 30);
    connect(m_stepBackwardButton, &QPushButton::clicked, this, &ControlPanel::stepBackwardClicked);

    // Play/Pause
    m_playPauseButton = new QPushButton(this);
    m_playPauseButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    m_playPauseButton->setToolTip("Play");
    m_playPauseButton->setFixedSize(50, 35);
    connect(m_playPauseButton, &QPushButton::clicked, this, &ControlPanel::onPlayPauseClicked);

    // Stop
    m_stopButton = new QPushButton(this);
    m_stopButton->setIcon(style()->standardIcon(QStyle::SP_MediaStop));
    m_stopButton->setToolTip("Stop");
    m_stopButton->setFixedSize(40, 30);
    connect(m_stopButton, &QPushButton::clicked, this, &ControlPanel::stopClicked);

    // Step forward (Mapped to both local handler and signal)
    m_stepForwardButton = new QPushButton(this);
    m_stepForwardButton->setIcon(style()->standardIcon(QStyle::SP_MediaSeekForward));
    m_stepForwardButton->setToolTip("Step Forward");
    m_stepForwardButton->setFixedSize(40, 30);
    connect(m_stepForwardButton, &QPushButton::clicked, this, &ControlPanel::onStepForwardClicked);

    // Go to end
    m_goToEndButton = new QPushButton(this);
    m_goToEndButton->setIcon(style()->standardIcon(QStyle::SP_MediaSkipForward));
    m_goToEndButton->setToolTip("Go to End");
    m_goToEndButton->setFixedSize(40, 30);
    connect(m_goToEndButton, &QPushButton::clicked, this, &ControlPanel::goToEndClicked);

    // Reset
    m_resetButton = new QPushButton("Reset", this);
    m_resetButton->setToolTip("Reset Algorithm");
    m_resetButton->setFixedSize(60, 30);
    connect(m_resetButton, &QPushButton::clicked, this, &ControlPanel::resetClicked);

    m_controlButtonsLayout->addWidget(m_goToBeginningButton);
    m_controlButtonsLayout->addWidget(m_stepBackwardButton);
    m_controlButtonsLayout->addWidget(m_playPauseButton);
    m_controlButtonsLayout->addWidget(m_stopButton);
    m_controlButtonsLayout->addWidget(m_stepForwardButton);
    m_controlButtonsLayout->addWidget(m_goToEndButton);
    m_controlButtonsLayout->addSpacing(10);
    m_controlButtonsLayout->addWidget(m_resetButton);
    m_controlButtonsLayout->addStretch();

    // --- Progress row ---
    m_progressLayout = new QHBoxLayout();
    m_progressLayout->setSpacing(10);

    m_progressLabel = new QLabel("Step:", this);
    m_progressLabel->setMinimumWidth(40);

    m_stepSpinBox = new QSpinBox(this);
    m_stepSpinBox->setMinimum(0);
    m_stepSpinBox->setMaximum(0);
    m_stepSpinBox->setValue(0);
    m_stepSpinBox->setFixedWidth(80);
    m_stepSpinBox->setToolTip("Current Step");
    connect(m_stepSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &ControlPanel::onStepSpinBoxChanged);

    m_progressBar = new QProgressBar(this);
    m_progressBar->setMinimum(0);
    m_progressBar->setMaximum(100);
    m_progressBar->setValue(0);
    m_progressBar->setTextVisible(true);
    m_progressBar->setFormat("%v / %m");

    m_progressLayout->addWidget(m_progressLabel);
    m_progressLayout->addWidget(m_stepSpinBox);
    m_progressLayout->addWidget(m_progressBar, 1);

    // --- Speed control row ---
    m_speedLayout = new QHBoxLayout();
    m_speedLayout->setSpacing(10);

    m_speedLabel = new QLabel("Speed:", this);
    m_speedLabel->setMinimumWidth(50);

    QLabel* slowLabel = new QLabel("Slow", this);
    slowLabel->setStyleSheet("color: gray; font-size: 10px;");

    m_speedSlider = new QSlider(Qt::Horizontal, this);
    m_speedSlider->setMinimum(1);
    m_speedSlider->setMaximum(100);
    m_speedSlider->setValue(m_currentSpeed);
    m_speedSlider->setTickPosition(QSlider::TicksBelow);
    m_speedSlider->setTickInterval(25);
    m_speedSlider->setToolTip("Playback Speed");
    connect(m_speedSlider, &QSlider::valueChanged, this, &ControlPanel::onSpeedSliderChanged);

    QLabel* fastLabel = new QLabel("Fast", this);
    fastLabel->setStyleSheet("color: gray; font-size: 10px;");

    QLabel* speedValueLabel = new QLabel(QString("%1%").arg(m_currentSpeed), this);
    speedValueLabel->setMinimumWidth(40);
    speedValueLabel->setAlignment(Qt::AlignCenter);

    connect(m_speedSlider, &QSlider::valueChanged, [speedValueLabel](int value) {
        speedValueLabel->setText(QString("%1%").arg(value));
    });

    m_speedLayout->addWidget(m_speedLabel);
    m_speedLayout->addWidget(slowLabel);
    m_speedLayout->addWidget(m_speedSlider, 1);
    m_speedLayout->addWidget(fastLabel);
    m_speedLayout->addWidget(speedValueLabel);

    // Add layouts
    m_mainLayout->addLayout(m_controlButtonsLayout);
    QHBoxLayout* secondRow = new QHBoxLayout();
    secondRow->addLayout(m_progressLayout, 2);
    secondRow->addSpacing(20);
    secondRow->addLayout(m_speedLayout, 1);
    m_mainLayout->addLayout(secondRow);

    // Initial state
    setPlayEnabled(false);
    setStepEnabled(false);
    setResetEnabled(false);

    // Styling
    setStyleSheet(R"(
        ControlPanel { background-color: #252526; border-top: 1px solid #3c3c3c; }
        QPushButton {
            background-color: #3c3c3c;
            border: 1px solid #555555;
            border-radius: 3px;
            color: #cccccc;
            font-weight: bold;
        }
        QPushButton:hover { background-color: #505050; border-color: #666666; }
        QPushButton:pressed { background-color: #333333; }
        QPushButton:disabled { background-color: #2d2d2d; color: #555555; border-color: #3c3c3c; }
        QProgressBar {
            border: 1px solid #3c3c3c;
            border-radius: 0px;
            background-color: #1e1e1e;
            color: #cccccc;
            text-align: center;
            font-size: 11px;
        }
        QProgressBar::chunk { background-color: #007acc; }
        QSpinBox {
            background-color: #3c3c3c;
            border: 1px solid #555555;
            border-radius: 0px;
            color: #cccccc;
            padding: 2px;
        }
        QSlider::groove:horizontal {
            border: 1px solid #3c3c3c;
            height: 4px;
            background-color: #1e1e1e;
            border-radius: 2px;
        }
        QSlider::handle:horizontal {
            background-color: #007acc;
            border: none;
            width: 14px;
            height: 14px;
            border-radius: 7px;
            margin: -5px 0px;
        }
        QSlider::handle:horizontal:hover { background-color: #1a8ad4; }
        QLabel { color: #cccccc; }
    )");
}

// --- Methods required by MainWindow ---

void ControlPanel::setPlayingState(bool isPlaying)
{
    m_isPlaying = isPlaying;
    updatePlayPauseButton();
    // When playing, we generally disable stepping to prevent conflicts
    if (m_stepForwardButton) m_stepForwardButton->setEnabled(!isPlaying && m_currentStep < m_totalSteps - 1);
    if (m_stepBackwardButton) m_stepBackwardButton->setEnabled(!isPlaying && m_currentStep > 0);
}

void ControlPanel::updateStepCounter(int currentStep, int totalSteps)
{
    // MainWindow passes 1-based steps usually, but our logic might be 0-based.
    // Adjust based on your visualizer. Assuming visualizer sends 0-based current index.
    // We update our internal state:
    setProgress(currentStep, totalSteps);
}

void ControlPanel::onStepForwardClicked()
{
    // Emit the signal MainWindow expects
    emit stepClicked();
}

// ---------------------------------------

void ControlPanel::setPlayEnabled(bool enabled)
{
    if (m_playPauseButton) m_playPauseButton->setEnabled(enabled);
}

void ControlPanel::setPauseEnabled(bool enabled)
{
    if (m_playPauseButton) m_playPauseButton->setEnabled(enabled);
}

void ControlPanel::setStepEnabled(bool enabled)
{
    if (m_stepForwardButton) m_stepForwardButton->setEnabled(enabled);
    if (m_stepBackwardButton) m_stepBackwardButton->setEnabled(enabled);
    if (m_goToBeginningButton) m_goToBeginningButton->setEnabled(enabled);
    if (m_goToEndButton) m_goToEndButton->setEnabled(enabled);
    if (m_stepSpinBox) m_stepSpinBox->setEnabled(enabled);
}

void ControlPanel::setResetEnabled(bool enabled)
{
    if (m_resetButton) m_resetButton->setEnabled(enabled);
    if (m_stopButton) m_stopButton->setEnabled(enabled);
}

void ControlPanel::setProgress(int current, int total)
{
    m_currentStep = current;
    m_totalSteps = total;
    updateProgressDisplay();
}

void ControlPanel::setCurrentStep(int step)
{
    m_currentStep = step;
    updateProgressDisplay();
}

void ControlPanel::setTotalSteps(int total)
{
    m_totalSteps = total;
    updateProgressDisplay();
}

int ControlPanel::getSpeed() const
{
    return m_speedSlider ? m_speedSlider->value() : m_currentSpeed;
}

void ControlPanel::setSpeed(int speed)
{
    m_currentSpeed = qBound(1, speed, 100);
    if (m_speedSlider) m_speedSlider->setValue(m_currentSpeed);
}

void ControlPanel::onAlgorithmChanged(bool hasAlgorithm)
{
    setPlayEnabled(hasAlgorithm);
    setStepEnabled(hasAlgorithm);
    setResetEnabled(hasAlgorithm);

    if (!hasAlgorithm) {
        m_isPlaying = false;
        updatePlayPauseButton();
        setProgress(0, 0);
    }
}

void ControlPanel::onExecutionStateChanged(bool isRunning)
{
    setPlayingState(isRunning);
}

void ControlPanel::onPlayPauseClicked()
{
    if (m_isPlaying) {
        emit pauseClicked();
    } else {
        emit playClicked();
    }
}

void ControlPanel::onSpeedSliderChanged(int value)
{
    m_currentSpeed = value;
    emit speedChanged(value);
}

void ControlPanel::onStepSpinBoxChanged(int value)
{
    if (value != m_currentStep) {
        emit goToStepClicked(value);
    }
}

void ControlPanel::updatePlayPauseButton()
{
    if (!m_playPauseButton) return;

    if (m_isPlaying) {
        m_playPauseButton->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
        m_playPauseButton->setToolTip("Pause");
    } else {
        m_playPauseButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
        m_playPauseButton->setToolTip("Play");
    }
}

void ControlPanel::updateProgressDisplay()
{
    if (m_progressBar) {
        m_progressBar->setMaximum(qMax(1, m_totalSteps));
        // Ensure we don't exceed max
        m_progressBar->setValue(qBound(0, m_currentStep + 1, m_totalSteps));
        m_progressBar->setFormat(QString("%1 / %2").arg(m_currentStep + 1).arg(m_totalSteps));
    }

    if (m_stepSpinBox) {
        // Step box is usually 0-indexed or 1-indexed. Let's assume 0-indexed for internal logic
        m_stepSpinBox->setMaximum(qMax(0, m_totalSteps - 1));

        bool blocked = m_stepSpinBox->blockSignals(true);
        m_stepSpinBox->setValue(m_currentStep);
        m_stepSpinBox->blockSignals(blocked);
    }

    // Update buttons based on bounds
    if (m_stepBackwardButton) m_stepBackwardButton->setEnabled(!m_isPlaying && m_currentStep > 0);
    if (m_goToBeginningButton) m_goToBeginningButton->setEnabled(!m_isPlaying && m_currentStep > 0);
    if (m_stepForwardButton) m_stepForwardButton->setEnabled(!m_isPlaying && m_currentStep < m_totalSteps - 1);
    if (m_goToEndButton) m_goToEndButton->setEnabled(!m_isPlaying && m_currentStep < m_totalSteps - 1);
}
