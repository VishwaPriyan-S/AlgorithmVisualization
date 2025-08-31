#include <QVBoxLayout>
#include <QGroupBox>
#include <QSplitter>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QFileDialog>
#include <QMessageBox>
#include <QCloseEvent>
#include <QSettings>
#include <QTextStream>
#include <QDockWidget>
#include <QRegularExpression>
#include <QDebug>
#include <qcombobox.h>
#include <qtextedit.h>
#include <QRandomGenerator>

#include "mainwindow.h"
#include "Widgets/code_highlighter.h"
#include "Widgets/control_panel.h"
#include "Widgets/visualization_widget.h"
#include "algorithm_manager.h"
#include "buuble_sort.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_mainSplitter(nullptr)
    , m_algorithmSelector(nullptr)
    , m_codeEditor(nullptr)
    , m_codeHighlighter(nullptr)
    , m_algorithmDescriptionLabel(nullptr)
    , m_complexityLabel(nullptr)
    , m_visualizationWidget(nullptr)
    , m_dataInputEdit(nullptr)
    , m_arraySizeSpinBox(nullptr)
    , m_generateDataButton(nullptr)
    , m_executeButton(nullptr)
    , m_controlPanel(nullptr)
    , m_algorithmManager(new AlgorithmManager(this))
    , m_statusProgressBar(nullptr)
    , m_statusLabel(nullptr)
    , m_hasExecutedAlgorithm(false)
    , m_showStatistics(true)
    , m_showOperationInfo(true)
    , m_enableAnimations(true)
    , m_defaultArraySize(20)
    , m_maxArraySize(200)
{
    setWindowTitle("Algorithm Visualizer");
    setMinimumSize(1200, 800);
    resize(1600, 1000);

    setupUI();
    setupMenuBar();
    setupToolBar();
    setupStatusBar();
    setupConnections();

    registerAlgorithms();

    QSettings settings("MySoft", "AlgoViz");
    restoreGeometry(settings.value("geometry").toByteArray());
    restoreState(settings.value("windowState").toByteArray());

    if (m_algorithmSelector->count() > 0) {
        m_algorithmSelector->setCurrentIndex(0);
        onAlgorithmChanged(0);
    } else {
        setUIEnabled(false);
    }
    generateRandomData();
    updateStatusBar();
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupUI()
{
    m_mainSplitter = new QSplitter(Qt::Horizontal, this);
    setCentralWidget(m_mainSplitter);

    QWidget* leftPanel = createLeftPanel();
    QWidget* rightPanel = createRightPanel();

    m_mainSplitter->addWidget(leftPanel);
    m_mainSplitter->addWidget(rightPanel);
    m_mainSplitter->setSizes({500, 700});
    m_mainSplitter->setStretchFactor(0, 0);
    m_mainSplitter->setStretchFactor(1, 1);

    m_controlPanel = new ControlPanel(this);
    QDockWidget* controlDock = new QDockWidget("Controls", this);
    controlDock->setWidget(m_controlPanel);
    controlDock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    addDockWidget(Qt::BottomDockWidgetArea, controlDock);
}

QWidget* MainWindow::createLeftPanel()
{
    QWidget* leftPanel = new QWidget();
    leftPanel->setMinimumWidth(400);
    leftPanel->setMaximumWidth(600);
    QVBoxLayout* leftLayout = new QVBoxLayout(leftPanel);

    QGroupBox* algorithmGroup = new QGroupBox("Algorithm Selection");
    QVBoxLayout* algorithmLayout = new QVBoxLayout(algorithmGroup);
    m_algorithmSelector = new QComboBox();
    algorithmLayout->addWidget(m_algorithmSelector);
    leftLayout->addWidget(algorithmGroup);

    leftLayout->addWidget(createAlgorithmInfoPanel());

    QGroupBox* codeGroup = new QGroupBox("Algorithm Code");
    QVBoxLayout* codeLayout = new QVBoxLayout(codeGroup);
    m_codeEditor = new QTextEdit();
    m_codeEditor->setReadOnly(true);
    m_codeEditor->setFont(QFont("Consolas", 10));
    m_codeHighlighter = new CodeHighlighter(m_codeEditor->document());
    codeLayout->addWidget(m_codeEditor);
    leftLayout->addWidget(codeGroup, 1);

    return leftPanel;
}

QWidget* MainWindow::createRightPanel()
{
    QWidget* rightPanel = new QWidget();
    QVBoxLayout* rightLayout = new QVBoxLayout(rightPanel);

    rightLayout->addWidget(createDataInputPanel());

    QGroupBox* visualizationGroup = new QGroupBox("Visualization");
    QVBoxLayout* visualizationLayout = new QVBoxLayout(visualizationGroup);
    m_visualizationWidget = new VisualizationWidget();
    visualizationLayout->addWidget(m_visualizationWidget);
    rightLayout->addWidget(visualizationGroup, 1);

    return rightPanel;
}

QWidget* MainWindow::createDataInputPanel()
{
    QGroupBox* dataGroup = new QGroupBox("Input Data");
    QVBoxLayout* dataLayout = new QVBoxLayout(dataGroup);

    QHBoxLayout* sizeLayout = new QHBoxLayout();
    sizeLayout->addWidget(new QLabel("Array Size:"));
    m_arraySizeSpinBox = new QSpinBox();
    m_arraySizeSpinBox->setRange(1, m_maxArraySize);
    m_arraySizeSpinBox->setValue(m_defaultArraySize);
    sizeLayout->addWidget(m_arraySizeSpinBox);
    m_generateDataButton = new QPushButton("Generate Random");
    sizeLayout->addWidget(m_generateDataButton);
    sizeLayout->addStretch();
    dataLayout->addLayout(sizeLayout);

    dataLayout->addWidget(new QLabel("Data (comma-separated):"));
    m_dataInputEdit = new QTextEdit();
    m_dataInputEdit->setMaximumHeight(80);
    dataLayout->addWidget(m_dataInputEdit);

    QHBoxLayout* executeLayout = new QHBoxLayout();
    executeLayout->addStretch();
    m_executeButton = new QPushButton("Execute Algorithm");
    executeLayout->addWidget(m_executeButton);
    dataLayout->addLayout(executeLayout);

    return dataGroup;
}

QWidget* MainWindow::createAlgorithmInfoPanel()
{
    QGroupBox* infoGroup = new QGroupBox("Algorithm Information");
    QVBoxLayout* infoLayout = new QVBoxLayout(infoGroup);
    m_algorithmDescriptionLabel = new QLabel("Select an algorithm.");
    m_algorithmDescriptionLabel->setWordWrap(true);
    infoLayout->addWidget(m_algorithmDescriptionLabel);
    m_complexityLabel = new QLabel("");
    m_complexityLabel->setWordWrap(true);
    infoLayout->addWidget(m_complexityLabel);
    infoGroup->setMaximumHeight(150);
    return infoGroup;
}

void MainWindow::setupMenuBar()
{
    QMenu* fileMenu = menuBar()->addMenu("&File");
    m_newDataAction = fileMenu->addAction("&New Data");
    fileMenu->addSeparator();
    m_loadDataAction = fileMenu->addAction("&Load Data...");
    m_saveDataAction = fileMenu->addAction("&Save Data...");
    fileMenu->addSeparator();
    m_exportImageAction = fileMenu->addAction("Export &Image...");
    fileMenu->addSeparator();
    m_exitAction = fileMenu->addAction("E&xit");

    QMenu* viewMenu = menuBar()->addMenu("&View");
    m_showStatisticsAction = viewMenu->addAction("Show Statistics");
    m_showStatisticsAction->setCheckable(true);
    m_showStatisticsAction->setChecked(m_showStatistics);
    m_showOperationInfoAction = viewMenu->addAction("Show Operation Info");
    m_showOperationInfoAction->setCheckable(true);
    m_showOperationInfoAction->setChecked(m_showOperationInfo);
    m_enableAnimationsAction = viewMenu->addAction("Enable Animations");
    m_enableAnimationsAction->setCheckable(true);
    m_enableAnimationsAction->setChecked(m_enableAnimations);

    QMenu* helpMenu = menuBar()->addMenu("&Help");
    m_aboutAction = helpMenu->addAction("&About");
    m_settingsAction = helpMenu->addAction("&Settings");
}

void MainWindow::setupToolBar()
{
    QToolBar* toolBar = addToolBar("Main Toolbar");
    toolBar->setMovable(false);
    toolBar->addAction(m_newDataAction);
    toolBar->addAction(m_loadDataAction);
    toolBar->addAction(m_saveDataAction);
    toolBar->addSeparator();
    toolBar->addAction(m_exportImageAction);
}

void MainWindow::setupStatusBar()
{
    m_statusLabel = new QLabel("Ready");
    statusBar()->addWidget(m_statusLabel);
    m_statusProgressBar = new QProgressBar();
    m_statusProgressBar->setMaximumWidth(200);
    m_statusProgressBar->setVisible(false);
    statusBar()->addPermanentWidget(m_statusProgressBar);
}

void MainWindow::setupConnections()
{
    connect(m_algorithmSelector, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onAlgorithmChanged);
    connect(m_generateDataButton, &QPushButton::clicked, this, &MainWindow::onGenerateDataClicked);
    connect(m_executeButton, &QPushButton::clicked, this, &MainWindow::onExecuteClicked);
    connect(m_dataInputEdit, &QTextEdit::textChanged, this, &MainWindow::onDataInputChanged);
    connect(m_arraySizeSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::onArraySizeChanged);
    connect(m_exitAction, &QAction::triggered, this, &MainWindow::close);
    connect(m_aboutAction, &QAction::triggered, this, &MainWindow::onAbout);

    connect(m_controlPanel, &ControlPanel::playClicked, this, &MainWindow::onPlayClicked);
    connect(m_controlPanel, &ControlPanel::pauseClicked, this, &MainWindow::onPauseClicked);
    connect(m_controlPanel, &ControlPanel::stepForwardClicked, this, &MainWindow::onStepForward);
    connect(m_controlPanel, &ControlPanel::stepBackwardClicked, this, &MainWindow::onStepBackward);
    connect(m_controlPanel, &ControlPanel::resetClicked, this, &MainWindow::onResetClicked);
    connect(m_controlPanel, &ControlPanel::speedChanged, this, &MainWindow::onSpeedChanged);

    connect(m_algorithmManager, &AlgorithmManager::stepChanged, this, &MainWindow::onAlgorithmStepChanged);
    connect(m_algorithmManager, &AlgorithmManager::algorithmFinished, this, &MainWindow::onAlgorithmFinished);
    connect(m_algorithmManager, &AlgorithmManager::algorithmError, this, &MainWindow::onAlgorithmError);
    connect(m_algorithmManager, &AlgorithmManager::executionStarted, this, &MainWindow::onExecutionStarted);
    connect(m_algorithmManager, &AlgorithmManager::playbackStateChanged, this, &MainWindow::onPlaybackStateChanged);
}

void MainWindow::registerAlgorithms()
{

    m_algorithmSelector->addItems(m_algorithmManager->getAvailableAlgorithms());
}

void MainWindow::generateRandomData()
{
    int size = m_arraySizeSpinBox->value();
    QVector<int> data;
    data.reserve(size);
    for (int i = 0; i < size; ++i) {
        data.append(QRandomGenerator::global()->bounded(1, 101));
    }
    setInputData(data);
}

void MainWindow::onAlgorithmChanged(int index)
{
    if (index < 0) return;
    QString algorithmName = m_algorithmSelector->itemText(index);
    if (m_algorithmManager->setCurrentAlgorithm(algorithmName)) {
        loadAlgorithmCode(algorithmName);
        updateAlgorithmInfo();
        setUIEnabled(true);
    } else {
        setUIEnabled(false);
    }
}

void MainWindow::onExecuteClicked()
{
    QVector<int> input = getCurrentInputData();
    if (input.isEmpty()) {
        QMessageBox::warning(this, "Input Error", "Please provide valid input data.");
        return;
    }
    m_algorithmManager->executeAlgorithm(input);
    m_hasExecutedAlgorithm = true;
    updateControlPanelState();
}

void MainWindow::onGenerateDataClicked() { generateRandomData(); }
void MainWindow::onPlayClicked() { m_algorithmManager->play(); }
void MainWindow::onPauseClicked() { m_algorithmManager->pause(); }
void MainWindow::onStepForward() { m_algorithmManager->stepForward(); }
void MainWindow::onStepBackward() { m_algorithmManager->stepBackward(); }
void MainWindow::onResetClicked()
{
    m_algorithmManager->reset();
    // After reset, re-visualize the initial data
    setInputData(m_currentData);
    VisualizationData initialViz;
    initialViz.array = m_currentData;
    m_visualizationWidget->updateVisualization(initialViz);
    updateStatusBar();
    updateControlPanelState();
}
void MainWindow::onSpeedChanged(int speed) { m_algorithmManager->setPlaybackSpeed(speed); }

void MainWindow::onAlgorithmStepChanged(int currentStep, int totalSteps)
{
    updateCodeHighlighting();
    updateVisualization();
    updateStatusBar();
    updateControlPanelState();
}

void MainWindow::onAlgorithmFinished()
{
    updateStatusBar();
    updateControlPanelState();
    m_statusProgressBar->setVisible(false);
    QMessageBox::information(this, "Execution Complete", "Algorithm has finished executing.");
}

void MainWindow::onAlgorithmError(const QString& error) { QMessageBox::critical(this, "Error", error); }
void MainWindow::onExecutionStarted()
{
    m_statusProgressBar->setValue(0);
    m_statusProgressBar->setVisible(true);
}
void MainWindow::onPlaybackStateChanged(bool isPlaying) { updateControlPanelState(); }

void MainWindow::updateCodeHighlighting()
{
    if (!m_codeHighlighter || !m_algorithmManager->getCurrentAlgorithm()) return;
    const StepData& stepData = m_algorithmManager->getCurrentAlgorithm()->getCurrentStepData();
    m_codeHighlighter->highlightLine(stepData.getHighlightInfo().getLineNumber());
}

void MainWindow::updateVisualization()
{
    if (!m_visualizationWidget || !m_algorithmManager->getCurrentAlgorithm()) return;
    const StepData& stepData = m_algorithmManager->getCurrentAlgorithm()->getCurrentStepData();
    m_visualizationWidget->updateVisualization(stepData.getVisualizationData());
}

void MainWindow::updateStatusBar()
{
    QString statusText;
    int currentStep = m_algorithmManager->getCurrentStep();
    int totalSteps = m_algorithmManager->getTotalSteps();
    AlgorithmBase* algorithm = m_algorithmManager->getCurrentAlgorithm();

    if (!algorithm) {
        statusText = "No algorithm selected";
    } else if (!m_hasExecutedAlgorithm || currentStep < 0) {
        statusText = QString("%1 | Ready to execute.").arg(m_algorithmManager->getCurrentAlgorithmName());
    } else if (algorithm->isFinished()) {
        statusText = QString("%1 | Finished.").arg(m_algorithmManager->getCurrentAlgorithmName());
    } else {
        statusText = QString("%1 | Step %2 of %3").arg(m_algorithmManager->getCurrentAlgorithmName()).arg(currentStep + 1).arg(totalSteps);
    }
    m_statusLabel->setText(statusText);

    if (totalSteps > 0 && m_hasExecutedAlgorithm) {
        m_statusProgressBar->setVisible(true);
        m_statusProgressBar->setValue((currentStep + 1) * 100 / totalSteps);
    } else {
        m_statusProgressBar->setVisible(false);
    }
}

void MainWindow::updateControlPanelState()
{
    if(!m_controlPanel || !m_algorithmManager) return;
    //bool canStepFwd = m_algorithmManager->canStepForward();
  //  bool canStepBack = m_algorithmManager->canStepBackward();
    bool isPlaying = m_algorithmManager->isPlaying();
    m_controlPanel->setPlaybackState(isPlaying);
}

void MainWindow::loadAlgorithmCode(const QString& algorithmName)
{
    QString sourceCode = m_algorithmManager->getCurrentAlgorithmSourceCode();
    m_codeEditor->setPlainText(sourceCode.isEmpty() ? "// Source code not available" : sourceCode);
    m_codeHighlighter->clearHighlight();
}

void MainWindow::updateAlgorithmInfo()
{
    QString name = m_algorithmManager->getCurrentAlgorithmName();
    if (name.isEmpty()) {
        m_algorithmDescriptionLabel->setText("No algorithm selected.");
        m_complexityLabel->setText("");
        return;
    }
    m_algorithmDescriptionLabel->setText(m_algorithmManager->getAlgorithmDescription(name));
    m_complexityLabel->setText(m_algorithmManager->getAlgorithmComplexity(name));
}

void MainWindow::setUIEnabled(bool enabled)
{
    m_executeButton->setEnabled(enabled);
    m_dataInputEdit->setEnabled(enabled);
    m_generateDataButton->setEnabled(enabled);
    m_arraySizeSpinBox->setEnabled(enabled);
}

QVector<int> MainWindow::getCurrentInputData() const
{
    QVector<int> data;
    QStringList parts = m_dataInputEdit->toPlainText().split(QRegularExpression("[\\s,]+"), Qt::SkipEmptyParts);
    for (const QString& part : parts) {
        bool ok;
        int value = part.toInt(&ok);
        if (ok) data.append(value);
    }
    return data;
}

void MainWindow::setInputData(const QVector<int>& data)
{
    m_currentData = data;
    m_dataInputEdit->setPlainText(dataToString(data));
    VisualizationData vizData;
    vizData.array = data;
    m_visualizationWidget->updateVisualization(vizData);
}

QString MainWindow::dataToString(const QVector<int>& data) const
{
    QStringList stringList;
    for (int value : data) stringList.append(QString::number(value));
    return stringList.join(", ");
}

void MainWindow::onNewData() { generateRandomData(); }
void MainWindow::onLoadData() { /* ... */ }
void MainWindow::onSaveData() { /* ... */ }
void MainWindow::onExportImage() { /* ... */ }
void MainWindow::onAbout()
{
    QMessageBox::about(this, "About Algorithm Visualizer",
                       "A tool for visualizing algorithms built with Qt and C++.");
}
void MainWindow::onSettings() { /* ... */ }
void MainWindow::onShowStatistics(bool show) { /* ... */ }
void MainWindow::onShowOperationInfo(bool show) { /* ... */ }
void MainWindow::onEnableAnimations(bool enable) { /* ... */ }
void MainWindow::onDataInputChanged()
{
    QVector<int> data = getCurrentInputData();
    if (!data.isEmpty() && data.size() != m_arraySizeSpinBox->value()) {
        m_arraySizeSpinBox->blockSignals(true);
        m_arraySizeSpinBox->setValue(data.size());
        m_arraySizeSpinBox->blockSignals(false);
    }
}
void MainWindow::onArraySizeChanged(int size)
{
    if (getCurrentInputData().size() != size) {
        generateRandomData();
    }
}
void MainWindow::closeEvent(QCloseEvent *event)
{
    m_algorithmManager->stop();
    QSettings settings("MySoft", "AlgoViz");
    settings.setValue("geometry", saveGeometry());
    settings.setValue("windowState", saveState());
    event->accept();
}
