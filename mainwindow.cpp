#include "mainwindow.h"
#include "Widgets/code_highlighter.h"
#include "Widgets/control_panel.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QToolBar>
#include <QStatusBar>
#include <QMessageBox>
#include <QComboBox>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonParseError>
#include <QCoreApplication>
#include <QSplitter>
#include <QLabel>
#include <QTimer>
#include <QGraphicsView>
#include <QDir>
#include <QFileInfo>
#include <QInputDialog>
#include <QSettings>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    m_visualizer(nullptr),
    m_playTimer(nullptr),
    m_aiClient(new OpenAIClient(this)),
    m_totalSteps(0),
    m_currentStep(0)
{
    QSettings settings("MyCompany", "AlgorithmVisualizer");
    m_aiClient->setApiKey(settings.value("groq_api_key", "").toString());
    m_isAIEnabled = settings.value("ai_enabled", false).toBool();

    setStyleSheet(R"(
        QMainWindow { background-color: #1e1e1e; }
        QWidget { background-color: #1e1e1e; color: #cccccc; font-family: 'Segoe UI', 'Consolas', sans-serif; font-size: 13px; }
        QTextEdit {
            background-color: #1e1e1e;
            color: #d4d4d4;
            border: 1px solid #3c3c3c;
            border-radius: 0px;
            font-family: 'Consolas', 'Courier New', monospace;
            font-size: 13px;
            selection-background-color: #264f78;
        }
        QGraphicsView {
            background-color: #1e1e1e;
            border: 1px solid #3c3c3c;
            border-radius: 0px;
        }
        QGroupBox {
            background-color: #252526;
            border: 1px solid #3c3c3c;
            border-radius: 0px;
            margin-top: 14px;
            padding-top: 14px;
            font-weight: 600;
            color: #cccccc;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 4px;
            color: #cccccc;
        }
        QToolBar {
            background-color: #3c3c3c;
            border: none;
            spacing: 4px;
            padding: 2px 6px;
        }
        QToolButton {
            background-color: transparent;
            border: none;
            border-radius: 4px;
            padding: 4px 10px;
            color: #cccccc;
            font-size: 12px;
        }
        QToolButton:hover { background-color: #505050; }
        QToolButton:pressed { background-color: #404040; }
        QComboBox {
            background-color: #3c3c3c;
            color: #cccccc;
            border: 1px solid #555555;
            border-radius: 0px;
            padding: 3px 8px;
        }
        QComboBox::drop-down { border: none; }
        QComboBox QAbstractItemView {
            background-color: #252526;
            color: #cccccc;
            selection-background-color: #094771;
        }
        QStatusBar {
            background-color: #007acc;
            color: white;
            border: none;
            font-size: 12px;
        }
        QLabel { color: #cccccc; }
        QScrollBar:vertical {
            background-color: #1e1e1e;
            width: 12px;
            margin: 0;
        }
        QScrollBar::handle:vertical {
            background-color: #424242;
            min-height: 30px;
            border-radius: 0px;
        }
        QScrollBar::handle:vertical:hover { background-color: #4f4f4f; }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0px; }
        QScrollBar:horizontal {
            background-color: #1e1e1e;
            height: 12px;
        }
        QScrollBar::handle:horizontal {
            background-color: #424242;
            min-width: 30px;
        }
        QScrollBar::handle:horizontal:hover { background-color: #4f4f4f; }
        QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal { width: 0px; }
        QSplitter::handle { background-color: #3c3c3c; }
    )");

    setWindowTitle("Algorithm Visualizer Pro");
    resize(1650,950);



    setupUI();
    setupToolBar();
    setupStatusBar();
    setupConnections();
}

MainWindow::~MainWindow()
{
    if (m_activeProcess && m_activeProcess->state() != QProcess::NotRunning) {
        m_activeProcess->kill();
    }
    delete m_tempCodeFile;
    m_tempCodeFile = nullptr;
}

void MainWindow::setupUI()
{
    QWidget* centralWidget = new QWidget(this);
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);

    mainLayout->setContentsMargins(6,6,6,6);
    mainLayout->setSpacing(6);

    m_mainSplitter = new QSplitter(Qt::Horizontal);
    m_mainSplitter->setHandleWidth(2);
    m_mainSplitter->setStyleSheet("QSplitter::handle{background:#3c3c3c;}");

    /*
    LEFT PANEL
    */

    QWidget* leftPanel = new QWidget();
    QVBoxLayout* leftLayout = new QVBoxLayout(leftPanel);

    QGroupBox* codeGroup = new QGroupBox("Algorithm Code");

    m_codeEditor = new QTextEdit();
    m_codeEditor->setFont(QFont("Consolas",13));
    m_codeEditor->setPlaceholderText("Paste or write your Python algorithm here...");

    m_codeHighlighter = new CodeHighlighter(m_codeEditor->document());

    QVBoxLayout* codeLayout = new QVBoxLayout(codeGroup);
    codeLayout->addWidget(m_codeEditor);

    leftLayout->addWidget(codeGroup);

    /*
    RIGHT PANEL
    */

    QWidget* rightPanel = new QWidget();
    QVBoxLayout* rightLayout = new QVBoxLayout(rightPanel);

    QGroupBox* visGroup = new QGroupBox("Algorithm Visualization");

    m_scene = new QGraphicsScene(0,0,1200,800);
    m_scene->setBackgroundBrush(QColor("#0d1117"));

    m_graphicsView = new QGraphicsView(m_scene);

    m_graphicsView->setRenderHints(
        QPainter::Antialiasing |
        QPainter::SmoothPixmapTransform |
        QPainter::TextAntialiasing
        );

    m_graphicsView->setDragMode(QGraphicsView::ScrollHandDrag);
    m_graphicsView->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);

    QVBoxLayout* visLayout = new QVBoxLayout(visGroup);
    visLayout->addWidget(m_graphicsView);

    // Console Group removed.
    
    rightLayout->addWidget(visGroup, 1);

    /*
    SPLITTER
    */

    m_mainSplitter->addWidget(leftPanel);
    m_mainSplitter->addWidget(rightPanel);

    m_mainSplitter->setStretchFactor(0, 1);
    m_mainSplitter->setStretchFactor(1, 4);

    /*
    VISUALIZER + CONTROLS
    */

    m_visualizer = new ASTVisualizer(m_scene,this);
    m_playTimer = new QTimer(this);
    m_controlPanel = new ControlPanel(this);

    mainLayout->addWidget(m_mainSplitter,1);
    mainLayout->addWidget(m_controlPanel,0);

    setCentralWidget(centralWidget);
}

void MainWindow::setupToolBar()
{
    QToolBar* bar = addToolBar("Controls");

    bar->setMovable(false);
    bar->setIconSize(QSize(24,24));

    QAction* runAction = bar->addAction("Run");
    QAction* pauseAction = bar->addAction("Pause");
    QAction* stopAction = bar->addAction("Stop");
    QAction* resetAction = bar->addAction("Reset");

    connect(runAction,&QAction::triggered,this,&MainWindow::onExecuteClicked);
    connect(pauseAction,&QAction::triggered,this,&MainWindow::onPauseClicked);
    connect(stopAction,&QAction::triggered,this,&MainWindow::onStopClicked);
    connect(resetAction,&QAction::triggered,this,&MainWindow::onResetClicked);



    bar->addSeparator();

    QComboBox* typeCombo = new QComboBox();

    typeCombo->addItem("Generic",(int)VisualizationMode::Generic);
    typeCombo->addItem("Sorting",(int)VisualizationMode::Sorting);
    typeCombo->addItem("Recursion",(int)VisualizationMode::Recursion);
    typeCombo->addItem("Graph",(int)VisualizationMode::Graph);

    connect(typeCombo,QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,[this,typeCombo](int index){

                auto mode=(VisualizationMode)typeCombo->itemData(index).toInt();
                m_visualizer->setMode(mode);

            });

    bar->addWidget(new QLabel(" Mode: "));
    bar->addWidget(typeCombo);
}

void MainWindow::setupStatusBar()
{
    m_statusLabel = new QLabel("Ready");
    statusBar()->addWidget(m_statusLabel);
}

void MainWindow::setupConnections()
{
    connect(m_visualizer,&ASTVisualizer::stepExecuted,this,&MainWindow::onStepExecuted);
    connect(m_visualizer,&ASTVisualizer::executionFinished,this,&MainWindow::onExecutionFinished);

    connect(m_aiClient, &OpenAIClient::analysisComplete, this, &MainWindow::onAnalysisComplete);
    connect(m_aiClient, &OpenAIClient::analysisFailed, this, &MainWindow::onAnalysisFailed);

    connect(m_visualizer,&ASTVisualizer::highlightLine,this,[this](int line){

        m_codeHighlighter->highlightLine(line);

        QTextCursor cursor(
            m_codeEditor->document()->findBlockByLineNumber(line-1)
            );

        m_codeEditor->setTextCursor(cursor);

    });

    connect(m_controlPanel,&ControlPanel::playClicked,this,&MainWindow::onPlayClicked);
    connect(m_controlPanel,&ControlPanel::pauseClicked,this,&MainWindow::onPauseClicked);
    connect(m_controlPanel,&ControlPanel::stepClicked,this,&MainWindow::onStepForward);
    connect(m_controlPanel,&ControlPanel::resetClicked,this,&MainWindow::onResetClicked);
    connect(m_controlPanel,&ControlPanel::speedChanged,this,&MainWindow::onSpeedChanged);
    connect(m_controlPanel,&ControlPanel::stopClicked,this,&MainWindow::onStopClicked);
    connect(m_controlPanel,&ControlPanel::stepBackwardClicked,this,&MainWindow::onStepBackward);
    connect(m_controlPanel,&ControlPanel::goToStepClicked,this,&MainWindow::onGoToStep);
    connect(m_controlPanel,&ControlPanel::goToBeginningClicked,this,&MainWindow::onGoToBeginning);
    connect(m_controlPanel,&ControlPanel::goToEndClicked,this,&MainWindow::onGoToEnd);

    connect(m_playTimer,&QTimer::timeout,m_visualizer,&ASTVisualizer::executeStep);
}

void MainWindow::onExecuteClicked()
{
    QString code = m_codeEditor->toPlainText();

    if(code.trimmed().isEmpty())
    {
        QMessageBox::information(this,"Input Required",
                                 "Please enter Python code first.");
        return;
    }

    onPauseClicked();
    m_visualizer->logMessage("Starting Python trace...", true);

    m_tracerFinished = false;
    m_aiFinished = false;
    m_aiMetadata = QJsonObject();
    m_visualizer->setAIMetadata(m_aiMetadata); // clear old metadata

    if (m_isAIEnabled) {
        m_aiClient->analyzeCode(code);
    } else {
        m_aiFinished = true; // Skip AI wait if disabled
    }

    if (m_activeProcess && m_activeProcess->state() != QProcess::NotRunning) {
        m_activeProcess->kill();
        m_activeProcess->deleteLater();
    }
    m_activeProcess = nullptr;

    m_processStdOut.clear();
    m_processStdErr.clear();
    delete m_tempCodeFile;
    m_tempCodeFile = new QTemporaryFile(QDir::tempPath() + "/algovisXXXXXX.py");

    if (!m_tempCodeFile || !m_tempCodeFile->open()) {
        QMessageBox::critical(this, "Execution Failed", "Failed to create temporary Python file.");
        return;
    }

    m_tempCodeFile->write(code.toUtf8());
    m_tempCodeFile->flush();
    m_tempCodeFile->close();

    auto resolveTracerPath = []() -> QString {
        const QString appDirPath = QCoreApplication::applicationDirPath();
        const QString cwdPath = QDir::currentPath();

        QStringList roots;
        roots << appDirPath << cwdPath;

        QDir appDir(appDirPath);
        for (int i = 0; i < 3 && appDir.cdUp(); ++i) {
            roots << appDir.absolutePath();
        }

        QDir cwdDir(cwdPath);
        for (int i = 0; i < 4 && cwdDir.cdUp(); ++i) {
            roots << cwdDir.absolutePath();
        }

        QStringList candidates;
        for (const QString& root : roots) {
            candidates << QDir(root).absoluteFilePath("python_tracer.py");
            candidates << QDir(root).absoluteFilePath("python/python_tracer.py");

            const QString buildDirPath = QDir(root).absoluteFilePath("build");
            QDir buildDir(buildDirPath);
            if (buildDir.exists()) {
                const QFileInfoList subdirs = buildDir.entryInfoList(
                    QDir::Dirs | QDir::NoDotAndDotDot
                );
                for (const QFileInfo& subdirInfo : subdirs) {
                    const QDir subdir(subdirInfo.absoluteFilePath());
                    candidates << subdir.absoluteFilePath("python_tracer.py");
                    candidates << subdir.absoluteFilePath("python/python_tracer.py");
                }
            }
        }

        for (const QString& candidate : candidates) {
            const QFileInfo info(candidate);
            if (info.exists() && info.isFile()) {
                return info.absoluteFilePath();
            }
        }
        return QString();
    };

    const QString scriptPath = resolveTracerPath();
    if (scriptPath.isEmpty()) {
        QMessageBox::critical(
            this,
            "Tracer Missing",
            "python_tracer.py not found.\n"
            "Searched app directory, working directory, and common build folders."
        );
        return;
    }
    m_visualizer->logMessage("Using tracer: " + scriptPath);

    QProcess* proc = new QProcess(this);
    m_activeProcess = proc;
    connect(proc, &QObject::destroyed, this, [this]() {
        m_activeProcess = nullptr;
    });

    connect(proc, &QProcess::readyReadStandardOutput, this, [this, proc]() {
        m_processStdOut.append(proc->readAllStandardOutput());
    });

    connect(proc, &QProcess::readyReadStandardError, this, [this, proc]() {
        QByteArray chunk = proc->readAllStandardError();
        m_processStdErr.append(chunk);
        const QString msg = QString::fromUtf8(chunk).trimmed();
        if (!msg.isEmpty()) {
            m_visualizer->logMessage("[stderr] " + msg);
        }
    });

    connect(proc, &QProcess::errorOccurred, this, [this](QProcess::ProcessError) {
        m_statusLabel->setText("Execution error");
        QMessageBox::critical(this, "Execution Failed", "Unable to start or run Python process.");
    });

    connect(proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [this, proc](int exitCode, QProcess::ExitStatus status) {
        m_statusLabel->setText("Trace finished");

        if (status != QProcess::NormalExit || exitCode != 0) {
            QString err = QString::fromUtf8(m_processStdErr).trimmed();
            if (err.isEmpty()) {
                err = "Python process failed without additional details.";
            }
            QMessageBox::critical(this, "Execution Failed", err);
            if (m_activeProcess == proc) {
                m_activeProcess = nullptr;
            }
            proc->deleteLater();
            return;
        }

        if (m_processStdOut.isEmpty()) {
            QMessageBox::critical(this, "Execution Failed", "Python returned no output.");
            if (m_activeProcess == proc) {
                m_activeProcess = nullptr;
            }
            proc->deleteLater();
            return;
        }

        QJsonParseError parseError;
        const QJsonDocument doc = QJsonDocument::fromJson(m_processStdOut, &parseError);
        if (parseError.error != QJsonParseError::NoError || !doc.isArray()) {
            const QString snippet = QString::fromUtf8(m_processStdOut.left(350));
            QMessageBox::critical(this, "Invalid Trace Output",
                                  "Tracer did not return a valid JSON array.\n\n" + snippet);
            if (m_activeProcess == proc) {
                m_activeProcess = nullptr;
            }
            proc->deleteLater();
            return;
        }

        const QJsonArray stepsArray = doc.array();
        m_totalSteps = stepsArray.size();
        m_currentStep = 0;

        m_visualizer->loadSteps(stepsArray);
        m_controlPanel->onAlgorithmChanged(true);
        m_controlPanel->setTotalSteps(m_totalSteps);
        m_controlPanel->setCurrentStep(0);

        m_statusLabel->setText(QString("Loaded %1 steps").arg(m_totalSteps));
        m_visualizer->logMessage(QString("Loaded %1 execution steps.").arg(m_totalSteps));

        m_tracerFinished = true;
        tryStartPlayback();

        if (m_activeProcess == proc) {
            m_activeProcess = nullptr;
        }
        proc->deleteLater();
    });

    m_statusLabel->setText("Running tracer...");
    proc->start("python", QStringList() << scriptPath << m_tempCodeFile->fileName());
}

void MainWindow::onPlayClicked()
{
    if(m_totalSteps==0) return;

    m_controlPanel->setPlayingState(true);

    int speedVal = m_controlPanel->getSpeed();
    int intervalMs = 2050 - (speedVal*20);

    m_playTimer->start(qMax(50,intervalMs));
    m_statusLabel->setText("Playing");
}

void MainWindow::onPauseClicked()
{
    m_controlPanel->setPlayingState(false);
    m_playTimer->stop();
    m_statusLabel->setText("Paused");
}

void MainWindow::onStopClicked()
{
    onPauseClicked();

    if (m_activeProcess && m_activeProcess->state() != QProcess::NotRunning) {
        m_activeProcess->kill();
        m_visualizer->logMessage("Execution stopped.");
    }

    m_statusLabel->setText("Stopped");
}

void MainWindow::onStepForward()
{
    onPauseClicked();
    m_visualizer->executeStep();
}

void MainWindow::onResetClicked()
{
    onPauseClicked();

    m_visualizer->reset();

    m_currentStep = 0;

    m_controlPanel->setCurrentStep(0);

    m_codeHighlighter->clearHighlight();

    m_statusLabel->setText("Reset");
}

void MainWindow::onSpeedChanged(int speed)
{
    if(m_playTimer->isActive())
    {
        int intervalMs = 2050 - (speed*20);
        m_playTimer->setInterval(qMax(50,intervalMs));
    }
}

void MainWindow::onStepExecuted(int step,int total)
{
    m_currentStep = step;

    m_statusLabel->setText(
        QString("Step %1 / %2").arg(step+1).arg(total)
        );

    m_controlPanel->updateStepCounter(step,total);
}

void MainWindow::onExecutionFinished()
{
    m_playTimer->stop();

    m_controlPanel->setPlayingState(false);

    m_statusLabel->setText("Execution Finished");

    m_codeHighlighter->clearHighlight();
}

void MainWindow::onGoToStep(int step)
{
    if(step < 0 || step >= m_totalSteps)
        return;

    onPauseClicked();

    m_visualizer->reset();

    for(int i = 0; i <= step; i++)
        m_visualizer->executeStep();
}


void MainWindow::onStepBackward()
{
    if(m_currentStep > 0)
        onGoToStep(m_currentStep - 1);
}


void MainWindow::onGoToBeginning()
{
    if(m_totalSteps > 0)
        onGoToStep(0);
}


void MainWindow::onGoToEnd()
{
    if(m_totalSteps > 0)
        onGoToStep(m_totalSteps - 1);
}

void MainWindow::onAIKeyActionClicked() {
    QSettings settings("MyCompany", "AlgorithmVisualizer");
    bool ok;
    QString key = QInputDialog::getText(this, "Groq API Key",
                                        "Enter your Groq API Key:", QLineEdit::Password,
                                        settings.value("groq_api_key").toString(), &ok);
    if (ok) {
        settings.setValue("groq_api_key", key);
        m_aiClient->setApiKey(key);
    }
    
    // Also ask to enable AI
    int ret = QMessageBox::question(this, "Enable AI", "Do you want to enable AI enhanced visualization for future runs?", QMessageBox::Yes | QMessageBox::No);
    m_isAIEnabled = (ret == QMessageBox::Yes);
    settings.setValue("ai_enabled", m_isAIEnabled);
}

void MainWindow::onAnalysisComplete(const QJsonObject& metadata) {
    m_aiMetadata = metadata;
    m_aiFinished = true;
    tryStartPlayback();
}

void MainWindow::onAnalysisFailed(const QString& errorString) {
    m_aiFinished = true;
    tryStartPlayback();
}

void MainWindow::tryStartPlayback() {
    if (m_tracerFinished && m_aiFinished) {
        if (!m_aiMetadata.isEmpty()) {
            m_visualizer->setAIMetadata(m_aiMetadata);
        }
        onPlayClicked();
    }
}
