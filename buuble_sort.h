#ifndef BUBBLESORT_ALGORITHM_H
#define BUBBLESORT_ALGORITHM_H

#include "algorithm_base.h"

class BubbleSortAlgorithm : public AlgorithmBase
{
    Q_OBJECT
public:
    explicit BubbleSortAlgorithm(QObject *parent = nullptr);

    QString name() const override;
    QString description() const override;
    QString getSourceCode() const override;
    AlgorithmType type() const override;
    QString getComplexityString() const override;
    void execute(const QVector<int>& data) override;
};

#endif
