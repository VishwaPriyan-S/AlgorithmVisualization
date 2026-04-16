#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSplitter>
#include <QTextEdit>
#include <QLabel>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QTimer>
#include <QPointer>
#include <QProcess>
#include <QTemporaryFile>
#include <QByteArray>

#include "Widgets/control_panel.h"
#include "ASTVisualizer.h"

class CodeHighlighter;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:

    void onExecuteClicked();
    void onStepExecuted(int step, int total);
    void onExecutionFinished();

    void onPlayClicked();
    void onPauseClicked();
    void onResetClicked();
    void onSpeedChanged(int speed);
    void onStepForward();
    void onGoToStep(int step);

    void onStepBackward();
    void onGoToBeginning();
    void onGoToEnd();
    void onStopClicked();

private:

    void setupUI();
    void setupToolBar();
    void setupStatusBar();
    void setupConnections();

    QSplitter* m_mainSplitter;

    QTextEdit* m_codeEditor;
    QTextEdit* m_console;

    QGraphicsScene* m_scene;
    QGraphicsView* m_graphicsView;

    QLabel* m_statusLabel;

    ControlPanel* m_controlPanel;

    ASTVisualizer* m_visualizer;
    QTimer* m_playTimer;
    CodeHighlighter* m_codeHighlighter;

    int m_totalSteps;
    int m_currentStep;

    QProcess* m_activeProcess = nullptr;
    QByteArray m_processStdOut;
    QByteArray m_processStdErr;
    QTemporaryFile* m_tempCodeFile = nullptr;
};

#endif
