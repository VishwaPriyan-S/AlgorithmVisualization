#include "mainwindow.h"
#include "Widgets/code_highlighter.h"
#include "interpreter/PythonParserEngine.h"

#include <QVBoxLayout>
#include <QGroupBox>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QMessageBox>
#include <QRegularExpression>
#include <QRandomGenerator>
#include <QDockWidget>
#include <QTimer>
#include <QGraphicsView>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_visualizer(nullptr)
    , m_scene(nullptr)
    , m_graphicsView(nullptr)
    , m_playTimer(nullptr)
{
    setWindowTitle("Algorithm Visualizer");
    resize(1600, 1000);

    setupUI();
    setupMenuBar();
    setupToolBar();
    setupStatusBar();
    setupConnections();
}

MainWindow::~MainWindow() {}

void MainWindow::setupUI()
{
    m_mainSplitter = new QSplitter(Qt::Horizontal, this);
    setCentralWidget(m_mainSplitter);

    QWidget* leftPanel = createLeftPanel();
    QWidget* rightPanel = createRightPanel();

    m_mainSplitter->addWidget(leftPanel);
    m_mainSplitter->addWidget(rightPanel);
}

QWidget* MainWindow::createLeftPanel()
{
    QWidget* panel = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(panel);

    QGroupBox* codeGroup = new QGroupBox("Python Code");
    QVBoxLayout* codeLayout = new QVBoxLayout(codeGroup);

    m_codeEditor = new QTextEdit();
    m_codeEditor->setFont(QFont("Consolas", 11));
    m_codeHighlighter = new CodeHighlighter(m_codeEditor->document());
    codeLayout->addWidget(m_codeEditor);

    layout->addWidget(codeGroup);

    QGroupBox* jsonGroup = new QGroupBox("Execution Steps (JSON)");
    QVBoxLayout* jsonLayout = new QVBoxLayout(jsonGroup);

    m_jsonOutputView = new QTextEdit();
    m_jsonOutputView->setReadOnly(true);
    m_jsonOutputView->setFont(QFont("Consolas", 10));
    jsonLayout->addWidget(m_jsonOutputView);

    layout->addWidget(jsonGroup);

    return panel;
}

QWidget* MainWindow::createRightPanel()
{
    QWidget* panel = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(panel);

    QGroupBox* visGroup = new QGroupBox("Visualization");
    QVBoxLayout* visLayout = new QVBoxLayout(visGroup);

    m_scene = new QGraphicsScene(0, 0, 1200, 600);
    m_scene->setBackgroundBrush(QColor(40, 44, 52));

    m_graphicsView = new QGraphicsView(m_scene);
    m_graphicsView->setRenderHint(QPainter::Antialiasing);
    visLayout->addWidget(m_graphicsView);

    layout->addWidget(visGroup);

    m_visualizer = new ASTVisualizer(m_scene, this);
    m_playTimer = new QTimer(this);

    return panel;
}

void MainWindow::setupMenuBar()
{
    QMenu* fileMenu = menuBar()->addMenu("&File");
    fileMenu->addAction("Exit", this, &QWidget::close);
}

void MainWindow::setupToolBar()
{
    QToolBar* bar = addToolBar("Toolbar");
    QAction* runAction = bar->addAction("Run");
    connect(runAction, &QAction::triggered, this, &MainWindow::onExecuteClicked);
}

void MainWindow::setupStatusBar()
{
    m_statusLabel = new QLabel("Ready");
    statusBar()->addWidget(m_statusLabel);
}

void MainWindow::setupConnections()
{
    connect(m_visualizer, &ASTVisualizer::stepExecuted,
            this, &MainWindow::onStepExecuted);

    connect(m_visualizer, &ASTVisualizer::executionFinished,
            this, &MainWindow::onExecutionFinished);

    connect(m_playTimer, &QTimer::timeout, this, [this]() {
        if (m_visualizer->getCurrentStep() < m_visualizer->getTotalSteps() - 1)
            m_visualizer->executeStep();
        else
            m_playTimer->stop();
    });
}

void MainWindow::onExecuteClicked()
{
    QString code = m_codeEditor->toPlainText();
    if (code.trimmed().isEmpty()) {
        QMessageBox::warning(this, "Error", "Paste Python code first");
        return;
    }

    PythonParserEngine parser("python");
    std::string output = parser.parseToAstJson(code.toStdString());

    m_jsonOutputView->setPlainText(QString::fromStdString(output));

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(
        QByteArray::fromStdString(output), &err);

    if (err.error != QJsonParseError::NoError || !doc.isObject()) {
        QMessageBox::critical(this, "JSON Error", err.errorString());
        return;
    }

    QJsonObject root = doc.object();
    if (!root.contains("steps")) {
        QMessageBox::critical(this, "Error", "No steps found in JSON");
        return;
    }

    QJsonArray steps = root["steps"].toArray();
    if (!m_visualizer->loadExecutionSteps(steps)) {
        QMessageBox::critical(this, "Error", "Failed to load steps");
        return;
    }

    m_visualizer->executeStep();
    m_playTimer->start(600);
}

void MainWindow::onStepExecuted(int step, int total)
{
    statusBar()->showMessage(
        QString("Step %1 / %2").arg(step + 1).arg(total));
}

void MainWindow::onExecutionFinished()
{
    m_playTimer->stop();
    QMessageBox::information(this, "Done", "Execution finished");
}
