#ifndef ALGORITHM_MANAGER_H
#define ALGORITHM_MANAGER_H

#include <QObject>
#include <QTimer>
#include <QStringList>
#include <QVector>
#include <memory>
#include <map>
#include "algorithm_base.h"

class AlgorithmManager : public QObject
{
    Q_OBJECT

public:
    explicit AlgorithmManager(QObject *parent = nullptr);

    void registerAlgorithm(std::unique_ptr<AlgorithmBase> algorithm);
    void registerAlgorithm(const QString& name, std::unique_ptr<AlgorithmBase> algorithm);

    QStringList getAvailableAlgorithms() const;
    QStringList getAlgorithmsByType(AlgorithmBase::AlgorithmType type) const;

    bool setCurrentAlgorithm(const QString& name);
    AlgorithmBase* getCurrentAlgorithm() const { return m_currentAlgorithm; }
    const QString& getCurrentAlgorithmName() const { return m_currentAlgorithmName; }

    // Algorithm Info
    QString getAlgorithmDescription(const QString& name) const;
    QString getAlgorithmComplexity(const QString& name) const;
    QString getCurrentAlgorithmSourceCode() const;

    void setCustomAlgorithm(AlgorithmBase* algo);


    // Execution Control
    void executeAlgorithm(const QVector<int>& input);
    void play();
    void pause();
    void stop();
    void reset();
    void stepForward();
    void stepBackward();
    void goToStep(int step);
    void goToBeginning();
    void goToEnd();
    void setPlaybackSpeed(int speed); // speed from 1-100

    // State Info
    int getCurrentStep() const;
    int getTotalSteps() const;
    bool isPlaying() const { return m_isPlaying; }
    bool canStepForward() const;
    bool canStepBackward() const;

signals:
    void algorithmChanged(const QString& newAlgorithmName);
    void executionStarted();
    void stepChanged(int currentStep, int totalSteps);
    void algorithmFinished();
    void algorithmError(const QString& error);
    void playbackStateChanged(bool isPlaying);

private slots:
    void onAlgorithmStateChanged(AlgorithmBase::ExecutionState state);
    void autoStep();

private:
    int calculateStepDelay() const;
    void connectAlgorithmSignals(AlgorithmBase* algorithm);
    void disconnectAlgorithmSignals(AlgorithmBase* algorithm);



    std::map<QString, std::unique_ptr<AlgorithmBase>> m_algorithms;
    AlgorithmBase* m_currentAlgorithm;
    QString m_currentAlgorithmName;

    QTimer* m_autoStepTimer;
    bool m_isPlaying;
    int m_playbackSpeed; // 1-100

    // Constants for playback speed calculation
    static const int MIN_STEP_DELAY = 10; // ms for speed 100
    static const int MAX_STEP_DELAY = 2000; // ms for speed 1
};

#endif // ALGORITHM_MANAGER_H
