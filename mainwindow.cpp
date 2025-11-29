#include "mainwindow.h"
#include "Widgets/code_highlighter.h"
#include "Widgets/control_panel.h"
#include "interpreter/PythonParserEngine.h"

#include <QVBoxLayout>
#include <QGroupBox>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QMessageBox>
#include <QRegularExpression>
#include <QScrollArea>
#include <QRandomGenerator>
#include <QSettings>
#include <QDockWidget>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_mainSplitter(nullptr)
    , m_codeEditor(nullptr)
    , m_codeHighlighter(nullptr)
    , m_algorithmSelector(nullptr)
    , m_algorithmDescriptionLabel(nullptr)
    , m_complexityLabel(nullptr)
    , m_dataInputEdit(nullptr)
    , m_arraySizeSpinBox(nullptr)
    , m_generateDataButton(nullptr)
    , m_executeButton(nullptr)
    , m_visualizationPlaceholder(nullptr)
    , m_controlPanel(nullptr)
    , m_statusLabel(nullptr)
    , m_statusProgressBar(nullptr)
    , m_hasExecutedAlgorithm(false)
    , m_showStatistics(true)
    , m_showOperationInfo(true)
    , m_enableAnimations(true)
    , m_defaultArraySize(20)
    , m_maxArraySize(200)
{
    setWindowTitle("Algorithm Visualizer");
    resize(1600, 1000);

    setupUI();
    setupMenuBar();
    setupToolBar();
    setupStatusBar();
    setupConnections();

    setUIEnabled(false);           // Start with no algorithms
    updateStatusBar();
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
    m_mainSplitter->setStretchFactor(0, 0);
    m_mainSplitter->setStretchFactor(1, 1);

    // Control Panel Dock
    m_controlPanel = new ControlPanel(this);
    QDockWidget* controlDock = new QDockWidget("Controls", this);
    controlDock->setWidget(m_controlPanel);
    addDockWidget(Qt::BottomDockWidgetArea, controlDock);
}

QWidget* MainWindow::createLeftPanel()
{
    QWidget* leftPanel = new QWidget();
    QVBoxLayout* leftLayout = new QVBoxLayout(leftPanel);

    QGroupBox* algoGroup = new QGroupBox("Algorithm Selection");
    QVBoxLayout* algoLayout = new QVBoxLayout(algoGroup);

    m_algorithmSelector = new QComboBox();
    m_algorithmSelector->addItem("None");    // Default
    algoLayout->addWidget(m_algorithmSelector);

    leftLayout->addWidget(algoGroup);
    leftLayout->addWidget(createAlgorithmInfoPanel());

    QGroupBox* codeGroup = new QGroupBox("Algorithm Code");
    QVBoxLayout* codeLayout = new QVBoxLayout(codeGroup);

    m_codeEditor = new QTextEdit();
    m_codeEditor->setFont(QFont("Consolas", 11));
    m_codeHighlighter = new CodeHighlighter(m_codeEditor->document());
    codeLayout->addWidget(m_codeEditor);

    leftLayout->addWidget(codeGroup, 1);
    // JSON Output Panel
    QGroupBox* jsonGroup = new QGroupBox("Parsed JSON Output");
    QVBoxLayout* jsonLayout = new QVBoxLayout(jsonGroup);

    m_jsonOutputView = new QTextEdit();
    m_jsonOutputView->setReadOnly(true);
    m_jsonOutputView->setFont(QFont("Consolas", 10));
    jsonLayout->addWidget(m_jsonOutputView);

    leftLayout->addWidget(jsonGroup, 1);

    return leftPanel;
}

QWidget* MainWindow::createRightPanel()
{
    QWidget* rightPanel = new QWidget();
    QVBoxLayout* rightLayout = new QVBoxLayout(rightPanel);

    rightLayout->addWidget(createDataInputPanel());

    QGroupBox* visGroup = new QGroupBox("Visualization");
    QVBoxLayout* visLayout = new QVBoxLayout(visGroup);

    // NEW PLACEHOLDER
    m_visualizationPlaceholder = new QLabel("Visualization will appear here once implemented");
   // m_visualizationPlaceholder->setAlignment(Qt::AlignCenter);
    m_visualizationPlaceholder->setStyleSheet("background:#f0f0f0; border:1px dashed #aaa;");
    m_visualizationPlaceholder->setMinimumHeight(300);

    visLayout->addWidget(m_visualizationPlaceholder);
    rightLayout->addWidget(visGroup, 1);

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

    dataLayout->addLayout(sizeLayout);

    dataLayout->addWidget(new QLabel("Data (comma-separated):"));
    m_dataInputEdit = new QTextEdit();
    m_dataInputEdit->setMaximumHeight(80);
    dataLayout->addWidget(m_dataInputEdit);

    QHBoxLayout* execLayout = new QHBoxLayout();
    execLayout->addStretch();
    m_executeButton = new QPushButton("Execute");
    execLayout->addWidget(m_executeButton);
    dataLayout->addLayout(execLayout);

    return dataGroup;
}

QWidget* MainWindow::createAlgorithmInfoPanel()
{
    QGroupBox* group = new QGroupBox("Algorithm Information");
    QVBoxLayout* layout = new QVBoxLayout(group);

    m_algorithmDescriptionLabel = new QLabel("No algorithm selected.");
    layout->addWidget(m_algorithmDescriptionLabel);

    m_complexityLabel = new QLabel("");
    layout->addWidget(m_complexityLabel);

    return group;
}

void MainWindow::setupMenuBar()
{
    QMenu* fileMenu = menuBar()->addMenu("&File");
    fileMenu->addAction("&New");
    fileMenu->addSeparator();
    fileMenu->addAction("&Exit", this, &QWidget::close);

    QMenu* helpMenu = menuBar()->addMenu("&Help");
    helpMenu->addAction("&About", this, &MainWindow::onAbout);
}

void MainWindow::setupToolBar()
{
    QToolBar* bar = addToolBar("Toolbar");
    QAction* parseAction = bar->addAction("Parse & Load Code");
    connect(parseAction, &QAction::triggered, this, &MainWindow::onParseCustomCodeClicked);
}

void MainWindow::setupStatusBar()
{
    m_statusLabel = new QLabel("Ready");
    statusBar()->addWidget(m_statusLabel);

    m_statusProgressBar = new QProgressBar();
    m_statusProgressBar->setVisible(false);
    statusBar()->addPermanentWidget(m_statusProgressBar);
}

void MainWindow::setupConnections()
{
    connect(m_algorithmSelector, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onAlgorithmChanged);
    connect(m_generateDataButton, &QPushButton::clicked, this, &MainWindow::onGenerateDataClicked);
    connect(m_executeButton, &QPushButton::clicked, this, &MainWindow::onExecuteClicked);
    connect(m_dataInputEdit, &QTextEdit::textChanged, this, &MainWindow::onDataInputChanged);
    connect(m_arraySizeSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::onArraySizeChanged);
}

void MainWindow::onAlgorithmChanged(int index)
{
    if (index <= 0) {
        m_algorithmDescriptionLabel->setText("No algorithm selected.");
        m_complexityLabel->setText("");
        setUIEnabled(false);
        return;
    }

    setUIEnabled(true);
}

void MainWindow::onExecuteClicked()
{
    QMessageBox::information(this, "TODO", "Execution engine not yet implemented.\nAST → JSON → Visualization next.");
}

void MainWindow::onGenerateDataClicked()
{
    int size = m_arraySizeSpinBox->value();
    QVector<int> data;
    data.reserve(size);

    for (int i = 0; i < size; ++i)
        data.append(QRandomGenerator::global()->bounded(1, 100));

    setInputData(data);
}

void MainWindow::onParseCustomCodeClicked()
{
    QString code = m_codeEditor->toPlainText();
    if (code.trimmed().isEmpty()) {
        QMessageBox::warning(this, "Empty Code", "Paste code first!");
        return;
    }

    // Use Python parser
    PythonParserEngine parser("python");   // or "python3" if needed
    std::string jsonAst = parser.parseToAstJson(code.toStdString());

    // Display in output panel
    m_jsonOutputView->setPlainText(QString::fromStdString(jsonAst));

    // Simple feedback based on whether an "error" key exists
    if (jsonAst.find("\"error\"") != std::string::npos) {
        QMessageBox::warning(this,
                             "Parse Error",
                             "Python parser reported an error.\nCheck the JSON output for details.");
    } else {
        QMessageBox::information(this,
                                 "Parsed",
                                 "Python code parsed successfully.\nAST JSON displayed below.");
    }
}


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
    if (getCurrentInputData().size() != size)
        onGenerateDataClicked();
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

    for (auto &p : parts) {
        bool ok;
        int v = p.toInt(&ok);
        if (ok) data.append(v);
    }

    return data;
}

void MainWindow::setInputData(const QVector<int>& data)
{
    m_dataInputEdit->setPlainText(dataToString(data));
}

QString MainWindow::dataToString(const QVector<int>& data) const
{
    QStringList list;
    for (int v : data) list << QString::number(v);
    return list.join(", ");
}

void MainWindow::updateStatusBar()
{
    m_statusLabel->setText("Ready");
}

void MainWindow::onAbout()
{
    QMessageBox::about(this, "About", "Algorithm Visualization (New Engine)\nCreated in Qt.");
}
