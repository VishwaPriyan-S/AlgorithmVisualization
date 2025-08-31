#pragma once
#include <QWidget>
#include <QHBoxLayout>
#include <QPushButton>
#include "AlgorithmHighlighter.h"

class VisualizationPage : public QWidget {
    Q_OBJECT
public:
    VisualizationPage(QWidget *parent = nullptr);

    void loadAlgorithm(const QString &name);
    void runDemo();

private:
    AlgorithmHighlighter *algoPanel;
    QPushButton *dummyVisualizer; // placeholder for your actual visualization canvas
};
