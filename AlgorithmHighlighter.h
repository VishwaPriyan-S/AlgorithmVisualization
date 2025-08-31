#pragma once
#include <QWidget>
#include <QListWidget>
#include <QVBoxLayout>
#include <QStringList>

class AlgorithmHighlighter : public QWidget {
    Q_OBJECT
public:
    explicit AlgorithmHighlighter(QWidget *parent = nullptr);

    void loadAlgorithm(const QStringList &lines);
    void highlightLine(int index);

private:
    QListWidget *codeView;
};
