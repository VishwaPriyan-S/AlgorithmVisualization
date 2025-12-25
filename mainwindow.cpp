#include "mainwindow.h"
#include "Widgets/code_highlighter.h"

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
#include <QtMath> // Needed for Sin/Cos in circular layout

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_visualizer(nullptr)
    , m_playTimer(nullptr)
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
    m_mainSplitter = new QSplitter(Qt::Horizontal, this);
    setCentralWidget(m_mainSplitter);

    // Left Panel
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

    // Right Panel
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

    m_visualizer = new ASTVisualizer(m_scene, this);
    m_playTimer = new QTimer(this);
}

void MainWindow::setupToolBar() {
    QToolBar* bar = addToolBar("Controls");
    bar->setMovable(false);

    QAction* runAction = bar->addAction("▶ Run");
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
    connect(m_visualizer, &ASTVisualizer::stepExecuted, this, &MainWindow::onStepExecuted);
    connect(m_visualizer, &ASTVisualizer::executionFinished, this, &MainWindow::onExecutionFinished);

    // Sync Highlight
    connect(m_visualizer, &ASTVisualizer::highlightLine, this, [this](int line){
        m_codeHighlighter->highlightLine(line);
        QTextCursor cursor(m_codeEditor->document()->findBlockByLineNumber(line - 1));
        m_codeEditor->setTextCursor(cursor);
    });

    connect(m_playTimer, &QTimer::timeout, this, [this]() {
        m_visualizer->executeStep();
    });
}

void MainWindow::onExecuteClicked() {
    QString code = m_codeEditor->toPlainText();
    if (code.trimmed().isEmpty()) return;

    // 1. Save code to temp file
    QTemporaryFile tempFile;
    if (!tempFile.open()) return;
    tempFile.write(code.toUtf8());
    tempFile.close();

    // 2. Run Python Tracer via QProcess
    QProcess process;
    // Assumes python_tracer.py is in the same directory as the executable// ... inside onExecuteClicked ...

    // 1. Try to find the script in multiple likely locations
    QString scriptPath = QCoreApplication::applicationDirPath() + "/python_tracer.py";

    if (!QFile::exists(scriptPath)) {
        // Fallback: Try looking in the source directory (adjust relative path as needed)
        // If build folder is "build/Debug", source is usually two levels up "../.."
        scriptPath = QCoreApplication::applicationDirPath() + "/../../python_tracer.py";
    }

    if (!QFile::exists(scriptPath)) {
        QMessageBox::critical(this, "Error", "Could not find python_tracer.py!\nPlease copy it to: " + QCoreApplication::applicationDirPath());
        return;
    }

    // 2. Run Python...
    process.start("python", QStringList() << scriptPath << tempFile.fileName());
    process.waitForFinished();

    // ... after process.waitForFinished() ...

    QByteArray output = process.readAllStandardOutput();
    QByteArray error = process.readAllStandardError();

    if (output.isEmpty()) {
        QString msg = "Python returned no output.";
        if (!error.isEmpty()) msg += "\nError: " + QString(error);
        QMessageBox::critical(this, "Execution Failed", msg);
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(output);

    // SAFETY CHECK: Ensure doc is valid and is an array
    if (doc.isNull()) {
        QMessageBox::critical(this, "Parse Error", "Output was not valid JSON:\n" + QString(output));
        return;
    }

    if (!doc.isArray()) {
        QMessageBox::critical(this, "Format Error", "Expected JSON Array but got something else.");
        return;
    }

    m_visualizer->loadSteps(doc.array());
    m_playTimer->start(400);
}

void MainWindow::onStepExecuted(int step, int total) {
    m_statusLabel->setText(QString("Step %1 / %2").arg(step + 1).arg(total));
}

void MainWindow::onExecutionFinished() {
    m_playTimer->stop();
    m_statusLabel->setText("Finished");
    m_codeHighlighter->clearHighlight();
}
