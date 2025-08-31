#include "buuble_sort.h"
#include <QVariantMap>
#include "algorithm_base.h"

BubbleSortAlgorithm::BubbleSortAlgorithm(QObject *parent) : AlgorithmBase(parent)
{
}

QString BubbleSortAlgorithm::name() const
{
    return "Bubble Sort";
}

QString BubbleSortAlgorithm::description() const
{
    return "A simple sorting algorithm that repeatedly steps through the list, "
           "compares adjacent elements and swaps them if they are in the wrong order.";
}

QString BubbleSortAlgorithm::getSourceCode() const
{
    return R"(
void bubbleSort(int arr[], int n) {
    bool swapped;
    for (int i = 0; i < n - 1; i++) {
        swapped = false;
        for (int j = 0; j < n - i - 1; j++) {
            if (arr[j] > arr[j + 1]) {
                swap(arr[j], arr[j + 1]);
                swapped = true;
            }
        }
        if (swapped == false)
            break;
    }
}
)";
}

AlgorithmBase::AlgorithmType BubbleSortAlgorithm::type() const
{
    return Sorting;
}

QString BubbleSortAlgorithm::getComplexityString() const
{
    return "Time: O(n^2) avg & worst, O(n) best | Space: O(1)";
}

void BubbleSortAlgorithm::execute(const QVector<int>& data)
{
    clearSteps();
    QVector<int> array = data;
    int n = array.size();
    long long comparisons = 0;
    long long swaps = 0;

    VisualizationData initialData;
    initialData.array = array;
    initialData.currentOperation = "Initial array";
    addStep(HighlightInfo(1, "Starting Bubble Sort"), initialData);

    for (int i = 0; i < n - 1; i++) {
        bool swapped = false;
        addStep(HighlightInfo(3, "Outer loop iteration"), {array, {}, QString("Pass %1").arg(i+1), comparisons, swaps});

        for (int j = 0; j < n - i - 1; j++) {
            comparisons++;
            VisualizationData compData;
            compData.array = array;
            compData.currentOperation = QString("Comparing %1 and %2").arg(array[j]).arg(array[j+1]);
            compData.highlights = {{"color", "yellow"}, {"indices", QVariant::fromValue(QVector<int>{j, j+1})}};
            compData.comparisons = comparisons;
            compData.swaps = swaps;
            addStep(HighlightInfo(6, "Comparing adjacent elements"), compData);

            if (array[j] > array[j + 1]) {
                swaps++;
                std::swap(array[j], array[j + 1]);
                swapped = true;

                VisualizationData swapData;
                swapData.array = array;
                swapData.currentOperation = QString("Swapping %1 and %2").arg(array[j+1]).arg(array[j]);
                swapData.highlights = {{"color", "red"}, {"indices", QVariant::fromValue(QVector<int>{j, j+1})}};
                swapData.comparisons = comparisons;
                swapData.swaps = swaps;
                addStep(HighlightInfo(7, "Elements are out of order, swapping"), swapData);
            }
        }
        if (!swapped) {
            addStep(HighlightInfo(12, "Array is sorted, early exit"), {array, {}, "Array sorted", comparisons, swaps});
            break;
        }
    }

    VisualizationData finalData;
    finalData.array = array;
    finalData.currentOperation = "Array is sorted!";
    finalData.highlights = {{"color", "lightgreen"}, {"indices", QVariant::fromValue(QVector<int>{})}}; // Highlight all
    finalData.comparisons = comparisons;
    finalData.swaps = swaps;
    addStep(HighlightInfo(15, "Finished sorting"), finalData);

    setState(Paused);
}
