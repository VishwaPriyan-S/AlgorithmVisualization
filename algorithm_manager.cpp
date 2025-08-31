#include "algorithm_manager.h"
#include <QDebug>
#include <algorithm>

const int AlgorithmManager::MIN_STEP_DELAY;
const int AlgorithmManager::MAX_STEP_DELAY;

AlgorithmManager::AlgorithmManager(QObject *parent)
    : QObject(parent)
    , m_currentAlgorithm(nullptr)
    , m_autoStepTimer(new QTimer(this))
    , m_isPlaying(false)
    , m_playbackSpeed(50)
{
    m_autoStepTimer->setSingleShot(false);
    connect(m_autoStepTimer, &QTimer::timeout, this, &AlgorithmManager::autoStep);
}

void AlgorithmManager::registerAlgorithm(std::unique_ptr<AlgorithmBase> algorithm)
{
    if (algorithm) {
        QString name = algorithm->name();
        registerAlgorithm(name, std::move(algorithm));
    }
}

void AlgorithmManager::registerAlgorithm(const QString& name, std::unique_ptr<AlgorithmBase> algorithm)
{
    if (algorithm && !name.isEmpty()) {
        m_algorithms.insert_or_assign(name, std::move(algorithm));
        qDebug() << "Registered algorithm:" << name;
    }
}

QStringList AlgorithmManager::getAvailableAlgorithms() const
{
    QStringList result;
    result.reserve(m_algorithms.size());
    for (const auto& pair : m_algorithms) {
        result.append(pair.first);
    }
    return result;
}

QStringList AlgorithmManager::getAlgorithmsByType(AlgorithmBase::AlgorithmType type) const
{
    QStringList result;
    for (const auto& pair : m_algorithms) {
        if (pair.second->type() == type) {
            result.append(pair.first);
        }
    }
    return result;
}

bool AlgorithmManager::setCurrentAlgorithm(const QString& name)
{
    auto it = m_algorithms.find(name);
    if (it != m_algorithms.end()) {
        if (m_currentAlgorithm) {
            disconnectAlgorithmSignals(m_currentAlgorithm);
        }

        m_currentAlgorithm = it->second.get();
        m_currentAlgorithmName = name;

        connectAlgorithmSignals(m_currentAlgorithm);
        stop();
        emit algorithmChanged(name);
        return true;
    }

    qWarning() << "Algorithm not found:" << name;
    return false;
}

void AlgorithmManager::executeAlgorithm(const QVector<int>& input)
{
    if (!m_currentAlgorithm) {
        qWarning() << "No algorithm selected";
        return;
    }
    stop();
    m_currentAlgorithm->reset();
    m_currentAlgorithm->execute(input);
    emit executionStarted();
    emit stepChanged(getCurrentStep(), getTotalSteps());
}

void AlgorithmManager::play()
{
    if (!m_currentAlgorithm) return;
    if (m_currentAlgorithm->isFinished()) {
        goToBeginning();
    }
    m_isPlaying = true;
    m_autoStepTimer->start(calculateStepDelay());
    emit playbackStateChanged(true);
}

void AlgorithmManager::pause()
{
    m_isPlaying = false;
    m_autoStepTimer->stop();
    if (m_currentAlgorithm) {
        m_currentAlgorithm->pause();
    }
    emit playbackStateChanged(false);
}

void AlgorithmManager::stop()
{
    pause();
    if (m_currentAlgorithm) {
        m_currentAlgorithm->stop();
    }
}

void AlgorithmManager::reset()
{
    stop();
    if (m_currentAlgorithm) {
        m_currentAlgorithm->reset();
        emit stepChanged(getCurrentStep(), getTotalSteps());
    }
}

void AlgorithmManager::stepForward()
{
    if (m_currentAlgorithm && m_currentAlgorithm->stepForward()) {
        emit stepChanged(getCurrentStep(), getTotalSteps());
        if (m_currentAlgorithm->isFinished()) {
            pause();
            emit algorithmFinished();
        }
    }
}

void AlgorithmManager::stepBackward()
{
    if (m_currentAlgorithm && m_currentAlgorithm->stepBackward()) {
        emit stepChanged(getCurrentStep(), getTotalSteps());
    }
}

void AlgorithmManager::goToStep(int step)
{
    if (m_currentAlgorithm && m_currentAlgorithm->goToStep(step)) {
        emit stepChanged(getCurrentStep(), getTotalSteps());
    }
}

void AlgorithmManager::goToBeginning()
{
    goToStep(-1);
}

void AlgorithmManager::goToEnd()
{
    if (m_currentAlgorithm) {
        goToStep(m_currentAlgorithm->getTotalSteps() - 1);
    }
}

void AlgorithmManager::setPlaybackSpeed(int speed)
{
    m_playbackSpeed = qBound(1, speed, 100);
    if (m_autoStepTimer->isActive()) {
        m_autoStepTimer->setInterval(calculateStepDelay());
    }
}

int AlgorithmManager::getCurrentStep() const
{
    return m_currentAlgorithm ? m_currentAlgorithm->getCurrentStepIndex() : -1;
}

int AlgorithmManager::getTotalSteps() const
{
    return m_currentAlgorithm ? m_currentAlgorithm->getTotalSteps() : 0;
}

bool AlgorithmManager::canStepForward() const
{
    return m_currentAlgorithm ? m_currentAlgorithm->canStepForward() : false;
}

bool AlgorithmManager::canStepBackward() const
{
    return m_currentAlgorithm ? m_currentAlgorithm->canStepBackward() : false;
}

QString AlgorithmManager::getAlgorithmDescription(const QString& name) const
{
    auto it = m_algorithms.find(name);
    return (it != m_algorithms.end()) ? it->second->description() : QString();
}

QString AlgorithmManager::getAlgorithmComplexity(const QString& name) const
{
    auto it = m_algorithms.find(name);
    return (it != m_algorithms.end()) ? it->second->getComplexityString() : QString();
}

QString AlgorithmManager::getCurrentAlgorithmSourceCode() const
{
    return m_currentAlgorithm ? m_currentAlgorithm->getSourceCode() : QString();
}

void AlgorithmManager::onAlgorithmStateChanged(AlgorithmBase::ExecutionState state)
{
    if (state == AlgorithmBase::Finished) {
        pause();
        emit algorithmFinished();
    } else if (state == AlgorithmBase::Error) {
        pause();
        emit algorithmError("Algorithm execution error");
    }
}

void AlgorithmManager::autoStep()
{
    if (m_isPlaying && canStepForward()) {
        stepForward();
    } else {
        pause();
    }
}

int AlgorithmManager::calculateStepDelay() const
{
    double normalized = (100.0 - m_playbackSpeed) / 99.0;
    return MIN_STEP_DELAY + static_cast<int>(normalized * (MAX_STEP_DELAY - MIN_STEP_DELAY));
}

void AlgorithmManager::connectAlgorithmSignals(AlgorithmBase* algorithm)
{
    if (algorithm) {
        connect(algorithm, &AlgorithmBase::stepChanged, this, &AlgorithmManager::stepChanged);
        connect(algorithm, &AlgorithmBase::algorithmFinished, this, &AlgorithmManager::algorithmFinished);
        connect(algorithm, &AlgorithmBase::algorithmError, this, &AlgorithmManager::algorithmError);
        connect(algorithm, &AlgorithmBase::stateChanged, this, &AlgorithmManager::onAlgorithmStateChanged);
    }
}

void AlgorithmManager::disconnectAlgorithmSignals(AlgorithmBase* algorithm)
{
    if (algorithm) {
        disconnect(algorithm, nullptr, this, nullptr);
    }
}
