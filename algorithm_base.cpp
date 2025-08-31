#include "algorithm_base.h"

AlgorithmBase::AlgorithmBase(QObject *parent)
    : QObject(parent)
    , m_currentStep(-1)
    , m_state(NotStarted)
{
}

void AlgorithmBase::reset()
{
    m_currentStep = -1;
    setState(NotStarted);
    clearSteps();
    emit stepChanged(m_currentStep, m_steps.size());
}

void AlgorithmBase::pause()
{
    if (m_state == Running) {
        setState(Paused);
    }
}

void AlgorithmBase::stop()
{
    setState(NotStarted);
    m_currentStep = -1;
    emit stepChanged(m_currentStep, m_steps.size());
}

bool AlgorithmBase::stepForward()
{
    if (canStepForward()) {
        m_currentStep++;
        emit stepChanged(m_currentStep, m_steps.size());

        if (m_currentStep >= m_steps.size() - 1) {
            setState(Finished);
        }
        return true;
    }
    return false;
}

bool AlgorithmBase::stepBackward()
{
    if (canStepBackward()) {
        m_currentStep--;
        emit stepChanged(m_currentStep, m_steps.size());
        return true;
    }
    return false;
}

bool AlgorithmBase::goToStep(int step)
{
    if (step >= -1 && step < m_steps.size()) {
        m_currentStep = step;
        emit stepChanged(m_currentStep, m_steps.size());

        if (m_currentStep >= m_steps.size() - 1 && m_steps.size() > 0) {
            setState(Finished);
        } else if (m_state != NotStarted) {
            setState(Paused);
        }
        return true;
    }
    return false;
}

bool AlgorithmBase::canStepForward() const
{
    return m_currentStep < m_steps.size() - 1;
}

bool AlgorithmBase::canStepBackward() const
{
    return m_currentStep > -1;
}

const StepData& AlgorithmBase::getCurrentStepData() const
{
    static StepData emptyStep;
    if (m_currentStep >= 0 && m_currentStep < m_steps.size()) {
        return m_steps[m_currentStep];
    }
    return emptyStep;
}

void AlgorithmBase::addStep(const HighlightInfo& highlight, const VisualizationData& vizData)
{
    StepData step(highlight, vizData);
    step.setStepNumber(m_steps.size());
    m_steps.append(step);
}

void AlgorithmBase::clearSteps()
{
    m_steps.clear();
}

void AlgorithmBase::setState(ExecutionState newState)
{
    if (m_state != newState) {
        m_state = newState;
        emit stateChanged(newState);
    }
}
