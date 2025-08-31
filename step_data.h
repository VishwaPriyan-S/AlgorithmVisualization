#ifndef STEP_DATA_H
#define STEP_DATA_H

#include <QString>
#include <QVector>
#include <QVariant>
#include <QColor>

struct HighlightInfo {
    int lineNumber;
    QString lineContent;
    QString explanation;
    QString variableStates;

    HighlightInfo() : lineNumber(0) {}
    HighlightInfo(int line, const QString& content, const QString& explain, const QString& vars = "")
        : lineNumber(line), lineContent(content), explanation(explain), variableStates(vars) {}

    int getLineNumber() const { return lineNumber; }   // <-- Add this
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
