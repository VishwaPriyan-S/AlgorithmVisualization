#ifndef CONTROL_PANEL_H
#define CONTROL_PANEL_H

#include <QWidget>
#include <QPushButton>
#include <QSlider>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QProgressBar>
#include <QSpinBox>

class ControlPanel : public QWidget
{
    Q_OBJECT

public:
    explicit ControlPanel(QWidget *parent = nullptr);

    // --- Methods expected by MainWindow ---
    void setPlayingState(bool isPlaying);
    void updateStepCounter(int currentStep, int totalSteps);
    int getSpeed() const;
    void setSpeed(int speed);
    // --------------------------------------

    // Control states
    void setPlayEnabled(bool enabled);
    void setPauseEnabled(bool enabled);
    void setStepEnabled(bool enabled);
    void setResetEnabled(bool enabled);

    // Progress
    void setProgress(int current, int total);
    void setCurrentStep(int step);
    void setTotalSteps(int total);

public slots:
    void onAlgorithmChanged(bool hasAlgorithm);
    void onExecutionStateChanged(bool isRunning);

signals:
    // --- Signals expected by MainWindow ---
    void playClicked();
    void stepClicked(); // Maps to step forward
    void resetClicked();
    void speedChanged(int speed);
    // --------------------------------------

    void pauseClicked();
    void stopClicked();
    void stepBackwardClicked();
    void goToStepClicked(int step);
    void goToBeginningClicked();
    void goToEndClicked();

private slots:
    void onPlayPauseClicked();
    void onSpeedSliderChanged(int value);
    void onStepSpinBoxChanged(int value);
    void onStepForwardClicked(); // Internal handler

private:
    void setupUI();
    void updatePlayPauseButton();
    void updateProgressDisplay();

private:
    // Control buttons
    QPushButton* m_playPauseButton;
    QPushButton* m_stopButton;
    QPushButton* m_resetButton;
    QPushButton* m_stepBackwardButton;
    QPushButton* m_stepForwardButton;
    QPushButton* m_goToBeginningButton;
    QPushButton* m_goToEndButton;

    // Progress display
    QProgressBar* m_progressBar;
    QSpinBox* m_stepSpinBox;
    QLabel* m_progressLabel;

    // Speed control
    QSlider* m_speedSlider;
    QLabel* m_speedLabel;

    // Layout
    QVBoxLayout* m_mainLayout;
    QHBoxLayout* m_controlButtonsLayout;
    QHBoxLayout* m_progressLayout;
    QHBoxLayout* m_speedLayout;

    // State
    bool m_isPlaying;
    int m_currentStep;
    int m_totalSteps;
    int m_currentSpeed;
};

#endif // CONTROL_PANEL_H
