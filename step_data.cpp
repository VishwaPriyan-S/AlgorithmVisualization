#include "step_data.h"

StepData::StepData()
    : m_stepType(INITIALIZATION)
    , m_stepNumber(0)
{
}

StepData::StepData(const HighlightInfo& highlight, const VisualizationData& vizData, StepType type)
    : m_highlightInfo(highlight)
    , m_visualizationData(vizData)
    , m_stepType(type)
    , m_stepNumber(0)
{
}
