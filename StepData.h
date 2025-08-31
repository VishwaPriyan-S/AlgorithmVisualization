#ifndef STEP_DATA_H
#define STEP_DATA_H

#include <QString>
#include <QVector>
#include <QVariant>
#include <QColor>

struct HighlightInfo {
    int lineNumber;              // Line number to highlight (1-based)
    QString lineContent;         // Content of the line being executed
    QString explanation;         // Human-readable explanation of this step
    QString variableStates;      // Current variable states (e.g., "i=2, j=5")

    HighlightInfo() : lineNumber(0) {}
    HighlightInfo(int line, const QString& content, const QString& explain, const QString& vars = "")
        : lineNumber(line), lineContent(content), explanation(explain), variableStates(vars) {}
};

struct ComparisonInfo {
    int index1;
    int index2;
    QString operation;           // "compare", "equal", "greater", "less"
    QColor color;

    ComparisonInfo() : index1(-1), index2(-1) {}
    ComparisonInfo(int i1, int i2, const QString& op, const QColor& c = Qt::red)
        : index1(i1), index2(i2), operation(op), color(c) {}
};

struct SwapInfo {
    int index1;
    int index2;
    QColor color;

    SwapInfo() : index1(-1), index2(-1) {}
    SwapInfo(int i1, int i2, const QColor& c = Qt::blue)
        : index1(i1), index2(i2), color(c) {}
};

struct VisualizationData {
    QVector<int> array;                          // Current state of the array
    QVector<int> highlightedIndices;             // Indices to highlight
    QVector<QColor> highlightColors;             // Colors for highlighted indices
    QVector<ComparisonInfo> comparisons;         // Active comparisons
    QVector<SwapInfo> swaps;                     // Active swaps
    QString currentOperation;                    // Description of current operation
    QVariantMap additionalData;                 // Extra data for specific algorithms

    // Helper methods
    void clearHighlights() {
        highlightedIndices.clear();
        highlightColors.clear();
        comparisons.clear();
        swaps.clear();
    }

    void addHighlight(int index, const QColor& color = Qt::yellow) {
        highlightedIndices.append(index);
        highlightColors.append(color);
    }

    void addComparison(int i1, int i2, const QString& op = "compare", const QColor& color = Qt::red) {
        comparisons.append(ComparisonInfo(i1, i2, op, color));
    }

    void addSwap(int i1, int i2, const QColor& color = Qt::blue) {
        swaps.append(SwapInfo(i1, i2, color));
    }
};

class StepData
{
public:
    enum StepType {
        INITIALIZATION,
        COMPARISON,
        SWAP,
        ASSIGNMENT,
        LOOP_START,
        LOOP_END,
        FUNCTION_CALL,
        FUNCTION_RETURN,
        FINISHED
    };

public:
    StepData();
    StepData(const HighlightInfo& highlight, const VisualizationData& vizData, StepType type = COMPARISON);

    // Getters
    const HighlightInfo& getHighlightInfo() const { return m_highlightInfo; }
    const VisualizationData& getVisualizationData() const { return m_visualizationData; }
    StepType getStepType() const { return m_stepType; }
    int getStepNumber() const { return m_stepNumber; }
    const QString& getDescription() const { return m_description; }

    // Setters
    void setHighlightInfo(const HighlightInfo& info) { m_highlightInfo = info; }
    void setVisualizationData(const VisualizationData& data) { m_visualizationData = data; }
    void setStepType(StepType type) { m_stepType = type; }
    void setStepNumber(int number) { m_stepNumber = number; }
    void setDescription(const QString& desc) { m_description = desc; }

private:
    HighlightInfo m_highlightInfo;
    VisualizationData m_visualizationData;
    StepType m_stepType;
    int m_stepNumber;
    QString m_description;
};

#endif // STEP_DATA_H

// ========================= step_data.cpp =========================
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
