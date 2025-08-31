#pragma once
#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QGroupBox>
#include <QScrollArea>

class HomePage : public QWidget {
    Q_OBJECT
public:
    HomePage(QWidget *parent = nullptr);

signals:
    void visualizeAlgorithm(const QString &name);  // signal when a card is clicked
};
