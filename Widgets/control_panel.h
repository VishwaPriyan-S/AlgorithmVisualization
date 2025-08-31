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

    // Control states
    void setPlayEnabled(bool enabled);
    void setPauseEnabled(bool enabled);
    void setStepEnabled(bool enabled);
    void setResetEnabled(bool enabled);

    // Progress
    void setProgress(int current, int total);
    void setCurrentStep(int step) { m_currentStep = step; updateProgressDisplay(); }
    void setTotalSteps(int total) { m_totalSteps = total; updateProgressDisplay(); }

    // Speed control
    int getSpeed() const;
    void setSpeed(int speed);

    void setPlaybackState(bool isPlaying) {
        playButton->setEnabled(!isPlaying);
        pauseButton->setEnabled(isPlaying);
    }

public slots:
    void onAlgorithmChanged(bool hasAlgorithm);
    void onExecutionStateChanged(bool isRunning);

signals:
    void playClicked();
    void pauseClicked();
    void stopClicked();
    void resetClicked();
    void stepForwardClicked();
    void stepBackwardClicked();
    void speedChanged(int speed);
    void goToStepClicked(int step);
    void goToBeginningClicked();
    void goToEndClicked();

private slots:
    void onPlayPauseClicked();
    void onSpeedSliderChanged(int value);
    void onStepSpinBoxChanged(int value);

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

    QPushButton *playButton;
    QPushButton *pauseButton;

};

#endif
