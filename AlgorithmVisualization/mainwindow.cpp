#include "mainwindow.h"
#include "Widgets/code_highlighter.h" // Adjust path if necessary
#include "Widgets/control_panel.h"
#include <QVBoxLayout>
#include <QGroupBox>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QMessageBox>
#include <QSplitter>
#include <QLabel>
#include <QComboBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QProcess>
#include <QTemporaryFile>
#include <QDir>
#include <QCoreApplication>
#include <QtMath>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_visualizer(nullptr)
    , m_playTimer(nullptr)
    , m_totalSteps(0)
    , m_currentStep(0)
{
    setWindowTitle("Algorithm Visualizer Pro");
    resize(1600, 1000);
    setStyleSheet("QMainWindow { background-color: #2b2b2b; color: white; }"
                  "QTextEdit { background-color: #1e1e1e; color: #d4d4d4; font-family: Consolas; }"
                  "QGraphicsView { border: none; }");

    setupUI();
    setupToolBar();
    setupStatusBar();
    setupConnections();
}

MainWindow::~MainWindow() {}

void MainWindow::setupUI() {
    // 1. Create a central widget and main layout
    QWidget* centralWidget = new QWidget(this);
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(5, 5, 5, 5);
    mainLayout->setSpacing(5);

    // 2. Setup Splitter for Code and Canvas
    m_mainSplitter = new QSplitter(Qt::Horizontal, centralWidget);

    // Left Panel (Code)
    QWidget* leftPanel = new QWidget();
    QVBoxLayout* leftLayout = new QVBoxLayout(leftPanel);
    QGroupBox* codeGroup = new QGroupBox("Python Code");
    codeGroup->setStyleSheet("color: white; font-weight: bold;");
    m_codeEditor = new QTextEdit();
    m_codeEditor->setFont(QFont("Consolas", 12));
    m_codeHighlighter = new CodeHighlighter(m_codeEditor->document());
    QVBoxLayout* l = new QVBoxLayout(codeGroup);
    l->addWidget(m_codeEditor);
    leftLayout->addWidget(codeGroup);

    // Right Panel (Canvas)
    QWidget* rightPanel = new QWidget();
    QVBoxLayout* rightLayout = new QVBoxLayout(rightPanel);
    QGroupBox* visGroup = new QGroupBox("Visualization Canvas");
    visGroup->setStyleSheet("color: white; font-weight: bold;");
    m_scene = new QGraphicsScene(0, 0, 1200, 800);
    m_scene->setBackgroundBrush(QColor(40, 44, 52));
    m_graphicsView = new QGraphicsView(m_scene);
    m_graphicsView->setRenderHint(QPainter::Antialiasing);
    QVBoxLayout* v = new QVBoxLayout(visGroup);
    v->addWidget(m_graphicsView);
    rightLayout->addWidget(visGroup);

    m_mainSplitter->addWidget(leftPanel);
    m_mainSplitter->addWidget(rightPanel);
    m_mainSplitter->setStretchFactor(1, 2);

    // 3. Setup Logic Components
    m_visualizer = new ASTVisualizer(m_scene, this);
    m_playTimer = new QTimer(this);

    // 4. Setup Control Panel
    m_controlPanel = new ControlPanel(this);

    // 5. Add to Main Layout (Splitter on top, Control Panel on bottom)
    mainLayout->addWidget(m_mainSplitter, 1); // stretch 1
    mainLayout->addWidget(m_controlPanel, 0); // stretch 0

    setCentralWidget(centralWidget);
}

void MainWindow::setupToolBar() {
    QToolBar* bar = addToolBar("Controls");
    bar->setMovable(false);

    QAction* runAction = bar->addAction("⚙ Compile & Load");
    runAction->setToolTip("Run Python tracer and load visualization data");
    connect(runAction, &QAction::triggered, this, &MainWindow::onExecuteClicked);

    bar->addSeparator();

    QComboBox* typeCombo = new QComboBox();
    typeCombo->addItem("Generic", QVariant::fromValue((int)VisualizationMode::Generic));
    typeCombo->addItem("Sorting", QVariant::fromValue((int)VisualizationMode::Sorting));
    typeCombo->addItem("Recursion", QVariant::fromValue((int)VisualizationMode::Recursion));
    typeCombo->addItem("Graph (Adjacency List)", QVariant::fromValue((int)VisualizationMode::Graph));

    connect(typeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this, typeCombo](int index){
        auto mode = (VisualizationMode)typeCombo->itemData(index).toInt();
        m_visualizer->setMode(mode);
    });
    bar->addWidget(typeCombo);
}

void MainWindow::setupStatusBar() {
    m_statusLabel = new QLabel("Ready");
    statusBar()->addWidget(m_statusLabel);
}

void MainWindow::setupConnections() {
    // Core Visualizer Connections
    connect(m_visualizer, &ASTVisualizer::stepExecuted, this, &MainWindow::onStepExecuted);
    connect(m_visualizer, &ASTVisualizer::executionFinished, this, &MainWindow::onExecutionFinished);

    // Sync Code Highlight
    connect(m_visualizer, &ASTVisualizer::highlightLine, this, [this](int line){
        m_codeHighlighter->highlightLine(line);
        QTextCursor cursor(m_codeEditor->document()->findBlockByLineNumber(line - 1));
        m_codeEditor->setTextCursor(cursor);
    });

    // Control Panel Connections
    connect(m_controlPanel, &ControlPanel::playClicked, this, &MainWindow::onPlayClicked);
    connect(m_controlPanel, &ControlPanel::pauseClicked, this, &MainWindow::onPauseClicked);
    connect(m_controlPanel, &ControlPanel::stepClicked, this, &MainWindow::onStepForward);
    connect(m_controlPanel, &ControlPanel::resetClicked, this, &MainWindow::onResetClicked);
    connect(m_controlPanel, &ControlPanel::speedChanged, this, &MainWindow::onSpeedChanged);
    connect(m_controlPanel, &ControlPanel::goToStepClicked, this, &MainWindow::onGoToStep);

    // Rewind Connections
    connect(m_controlPanel, &ControlPanel::stepBackwardClicked, this, &MainWindow::onStepBackward);
    connect(m_controlPanel, &ControlPanel::goToBeginningClicked, this, &MainWindow::onGoToBeginning);
    connect(m_controlPanel, &ControlPanel::goToEndClicked, this, &MainWindow::onGoToEnd);

    // Timer triggers single steps
    connect(m_playTimer, &QTimer::timeout, m_visualizer, &ASTVisualizer::executeStep);
}

void MainWindow::onExecuteClicked() {
    QString code = m_codeEditor->toPlainText();
    if (code.trimmed().isEmpty()) return;

    // 1. Save code to temp file
    QTemporaryFile tempFile;
    if (!tempFile.open()) return;
    tempFile.write(code.toUtf8());
    tempFile.close();

    // 2. Try to find the script in multiple likely locations
    QString scriptPath = QCoreApplication::applicationDirPath() + "/python_tracer.py";
    if (!QFile::exists(scriptPath)) {
        scriptPath = QCoreApplication::applicationDirPath() + "/../../python_tracer.py";
    }

    if (!QFile::exists(scriptPath)) {
        QMessageBox::critical(this, "Error", "Could not find python_tracer.py!\nPlease copy it to: " + QCoreApplication::applicationDirPath());
        return;
    }

    // 3. Run Python
    QProcess process;
    process.start("python", QStringList() << scriptPath << tempFile.fileName());
    process.waitForFinished();

    QByteArray output = process.readAllStandardOutput();
    QByteArray error = process.readAllStandardError();

    if (output.isEmpty()) {
        QString msg = "Python returned no output.";
        if (!error.isEmpty()) msg += "\nError: " + QString(error);
        QMessageBox::critical(this, "Execution Failed", msg);
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(output);

    // 4. Safety Checks
    if (doc.isNull()) {
        QMessageBox::critical(this, "Parse Error", "Output was not valid JSON:\n" + QString(output));
        return;
    }
    if (!doc.isArray()) {
        QMessageBox::critical(this, "Format Error", "Expected JSON Array but got something else.");
        return;
    }

    // 5. Load data and setup Control Panel
    QJsonArray stepsArray = doc.array();
    m_totalSteps = stepsArray.size();
    m_currentStep = 0;

    m_visualizer->loadSteps(stepsArray);

    // Unlock control panel UI
    m_controlPanel->onAlgorithmChanged(true);
    m_controlPanel->setTotalSteps(m_totalSteps);
    m_controlPanel->setCurrentStep(0);

    // Auto-start playback
    onPlayClicked();
}

// --- Control Panel Slots ---

void MainWindow::onPlayClicked() {
    if (m_totalSteps == 0) return;

    m_controlPanel->setPlayingState(true);

    // Convert slider value (1-100) to timer interval (e.g., 2000ms to 50ms)
    int speedVal = m_controlPanel->getSpeed();
    int intervalMs = 2050 - (speedVal * 20);

    m_playTimer->start(qMax(50, intervalMs));
}

void MainWindow::onPauseClicked() {
    m_controlPanel->setPlayingState(false);
    m_playTimer->stop();
}

void MainWindow::onStepForward() {
    onPauseClicked(); // Ensure timer is stopped if user manually steps
    m_visualizer->executeStep();
}

void MainWindow::onStepBackward() {
    if (m_currentStep > 0) {
        onGoToStep(m_currentStep - 1);
    }
}

void MainWindow::onGoToBeginning() {
    if (m_totalSteps > 0) {
        onGoToStep(0);
    }
}

void MainWindow::onGoToEnd() {
    if (m_totalSteps > 0) {
        onGoToStep(m_totalSteps - 1);
    }
}

void MainWindow::onResetClicked() {
    onPauseClicked();
    m_visualizer->reset();
    m_currentStep = 0;
    m_controlPanel->setCurrentStep(0);
    m_codeHighlighter->clearHighlight();
    m_statusLabel->setText("Reset");
}

void MainWindow::onGoToStep(int step) {
    if (step < 0 || step >= m_totalSteps) return;

    onPauseClicked();

    // To go backwards or skip forwards, we reset the visualizer
    // and rapidly fast-forward to the target step.
    m_visualizer->reset();
    for (int i = 0; i <= step; ++i) {
        m_visualizer->executeStep();
    }
}

void MainWindow::onSpeedChanged(int speed) {
    if (m_playTimer->isActive()) {
        int intervalMs = 2050 - (speed * 20);
        m_playTimer->setInterval(qMax(50, intervalMs));
    }
}

// --- Visualizer Response Slots ---

void MainWindow::onStepExecuted(int step, int total) {
    m_currentStep = step;
    m_statusLabel->setText(QString("Step %1 / %2").arg(step + 1).arg(total));
    m_controlPanel->updateStepCounter(step, total);
}

void MainWindow::onExecutionFinished() {
    m_playTimer->stop();
    m_controlPanel->setPlayingState(false);
    m_statusLabel->setText("Execution Finished");
    m_codeHighlighter->clearHighlight();
}
