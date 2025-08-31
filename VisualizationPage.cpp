#include "VisualizationPage.h"
#include <QTimer>

VisualizationPage::VisualizationPage(QWidget *parent) : QWidget(parent) {
    QHBoxLayout *layout = new QHBoxLayout(this);

    // Left side: visualization canvas (placeholder)
    dummyVisualizer = new QPushButton("Algorithm Animation Here");
    dummyVisualizer->setMinimumSize(400, 400);

    // Right side: pseudocode panel
    algoPanel = new AlgorithmHighlighter;

    layout->addWidget(dummyVisualizer, 3);
    layout->addWidget(algoPanel, 1);

    setLayout(layout);
}

void VisualizationPage::loadAlgorithm(const QString &name) {
    QStringList lines;

    if (name == "Bubble Sort") {
        lines << "for i = 0 to n-1:"
              << "   for j = 0 to n-i-1:"
              << "       if arr[j] > arr[j+1]:"
              << "           swap(arr[j], arr[j+1])";
    } else if (name == "Binary Search") {
        lines << "low = 0, high = n-1"
              << "while low <= high:"
              << "   mid = (low + high) / 2"
              << "   if arr[mid] == target: return mid"
              << "   else if arr[mid] < target: low = mid+1"
              << "   else: high = mid-1";
    } else {
        lines << "// Algorithm pseudocode not defined yet";
    }

    algoPanel->loadAlgorithm(lines);
}

void VisualizationPage::runDemo() {
    // Example: simulate step execution
    algoPanel->highlightLine(0);
    QTimer::singleShot(1000, [this]() { algoPanel->highlightLine(1); });
    QTimer::singleShot(2000, [this]() { algoPanel->highlightLine(2); });
    QTimer::singleShot(3000, [this]() { algoPanel->highlightLine(3); });
}
