#include "AlgorithmHighlighter.h"

AlgorithmHighlighter::AlgorithmHighlighter(QWidget *parent) : QWidget(parent) {
    QVBoxLayout *layout = new QVBoxLayout(this);

    codeView = new QListWidget;
    codeView->setStyleSheet("font-family: Consolas; font-size: 14px;");
    layout->addWidget(codeView);

    setLayout(layout);
}

void AlgorithmHighlighter::loadAlgorithm(const QStringList &lines) {
    codeView->clear();
    for (auto &line : lines) {
        codeView->addItem(line);
    }
}

void AlgorithmHighlighter::highlightLine(int index) {
    for (int i = 0; i < codeView->count(); ++i) {
        codeView->item(i)->setBackground(Qt::white);
    }
    if (index >= 0 && index < codeView->count()) {
        codeView->item(index)->setBackground(Qt::yellow);
    }
}
