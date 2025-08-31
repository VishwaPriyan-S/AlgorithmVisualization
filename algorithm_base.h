#ifndef ALGORITHM_BASE_H
#define ALGORITHM_BASE_H

#include <QObject>
#include <QString>
#include <QVector>
#include <QTimer>
#include <QVariantMap>

#include "step_data.h"   // ✅ use your detailed data structures

// Abstract base class for all algorithms
class AlgorithmBase : public QObject
{
    Q_OBJECT

public:
    enum ExecutionState { NotStarted, Running, Paused, Finished, Error };
    enum AlgorithmType { Sorting, Searching, Graph, Other };

    explicit AlgorithmBase(QObject *parent = nullptr);
    virtual ~AlgorithmBase() = default;

    // To be implemented by subclasses
    virtual QString name() const = 0;
    virtual QString description() const = 0;
    virtual QString getSourceCode() const = 0;
    virtual AlgorithmType type() const = 0;
    virtual QString getComplexityString() const = 0;
    virtual void execute(const QVector<int>& data) = 0;

    // Execution control
    void reset();
    void pause();
    void stop();
    bool stepForward();
    bool stepBackward();
    bool goToStep(int step);

    // State information
    int getCurrentStepIndex() const { return m_currentStep; }
    int getTotalSteps() const { return m_steps.size(); }
    bool isFinished() const { return m_state == Finished; }
    bool canStepForward() const;
    bool canStepBackward() const;
    const StepData& getCurrentStepData() const;

signals:
    void stepChanged(int currentStep, int totalSteps);
    void algorithmFinished();
    void algorithmError(const QString& error);
    void stateChanged(ExecutionState newState);

protected:
    void addStep(const HighlightInfo& highlight, const VisualizationData& vizData);
    void clearSteps();
    void setState(ExecutionState newState);

    QVector<StepData> m_steps;
    int m_currentStep;
    ExecutionState m_state;
};

#endif // ALGORITHM_BASE_H
