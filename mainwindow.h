#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSplitter>
#include <QTextEdit>
#include <QLabel>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QTimer>

// Include your custom visualizer headers
#include "Widgets/control_panel.h"
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
    // Core execution slots
    void onExecuteClicked();
    void onStepExecuted(int step, int total);
    void onExecutionFinished();

    // Control Panel integration slots
    void onPlayClicked();
    void onPauseClicked();
    void onResetClicked();
    void onSpeedChanged(int speed);
    void onStepForward();
    void onGoToStep(int step);

    // Rewind / Skipping slots
    void onStepBackward();
    void onGoToBeginning();
    void onGoToEnd();

private:
    // UI Setup helper functions
    void setupUI();
    void setupMenuBar();
    void setupToolBar();
    void setupStatusBar();
    void setupConnections();

    // UI Components
    QSplitter* m_mainSplitter;
    QTextEdit* m_codeEditor;
    QGraphicsScene* m_scene;
    QGraphicsView* m_graphicsView;
    QLabel* m_statusLabel;

    // New Control Panel
    ControlPanel* m_controlPanel;

    // Logic Components
    ASTVisualizer* m_visualizer;
    QTimer* m_playTimer;
    CodeHighlighter* m_codeHighlighter;

    // Track steps for scrub/goto functionality
    int m_totalSteps;
    int m_currentStep;
};

#endif // MAINWINDOW_H
