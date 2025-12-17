#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSplitter>
#include <QTextEdit>
#include <QLabel>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QTimer>

// Include your custom visualizer header
#include "ASTVisualizer.h"

// Forward declaration for the highlighter
class CodeHighlighter;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // Only declare slots that are actually implemented in mainwindow.cpp
    void onExecuteClicked();
    void onStepExecuted(int step, int total);
    void onExecutionFinished();

private:
    // UI Setup helper functions
    void setupUI();
    QWidget* createLeftPanel();
    QWidget* createRightPanel();
    void setupMenuBar();
    void setupToolBar();
    void setupStatusBar();
    void setupConnections();

    // UI Components
    QSplitter* m_mainSplitter;
    QTextEdit* m_codeEditor;
    QTextEdit* m_jsonOutputView;
    QGraphicsScene* m_scene;
    QGraphicsView* m_graphicsView;
    QLabel* m_statusLabel;

    // Logic Components
    ASTVisualizer* m_visualizer;
    QTimer* m_playTimer;
    CodeHighlighter* m_codeHighlighter;
};

#endif // MAINWINDOW_H
