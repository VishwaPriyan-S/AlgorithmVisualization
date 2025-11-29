#include "HomePage.h"

HomePage::HomePage(QWidget *parent) : QWidget(parent) {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    QLabel *title = new QLabel("Algorithm Visualizer");
    title->setStyleSheet("font-size: 28px; font-weight: bold; margin: 10px;");
    mainLayout->addWidget(title, 0, Qt::AlignHCenter);

    QLabel *subtitle = new QLabel("Explore Data Structures and Algorithms interactively");
    subtitle->setStyleSheet("color: gray; font-size: 14px;");
    mainLayout->addWidget(subtitle, 0, Qt::AlignHCenter);

    // Scrollable area
    QScrollArea *scroll = new QScrollArea;
    QWidget *contentWidget = new QWidget;
    QVBoxLayout *contentLayout = new QVBoxLayout(contentWidget);

    // Example algorithms
    QStringList algorithms = {"Bubble Sort", "Merge Sort", "Binary Search", "Graph Traversal"};

    for (auto &algo : algorithms) {
        QFrame *card = new QFrame;
        card->setStyleSheet(
            "QFrame {"
            "   border: 1px solid #aaa;"
            "   border-radius: 15px;"
            "   background: white;"
            "   padding: 12px;"
            "   margin: 8px;"
            "   box-shadow: 2px 2px 6px rgba(0,0,0,0.2);"
            "}"
            );

        QVBoxLayout *cardLayout = new QVBoxLayout(card);
        QLabel *desc = new QLabel("Description about " + algo);
        QPushButton *btn = new QPushButton("Visualize " + algo);
        cardLayout->addWidget(desc);
        cardLayout->addWidget(btn);

        contentLayout->addWidget(card);

    }

    contentWidget->setLayout(contentLayout);
    scroll->setWidget(contentWidget);
    scroll->setWidgetResizable(true);

    mainLayout->addWidget(scroll);
}
