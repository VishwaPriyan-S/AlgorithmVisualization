#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVector>

// Forward declarations
class QSplitter;
class QComboBox;
class QTextEdit;
class QLabel;
class QGroupBox;
class QSpinBox;
class QPushButton;
class QProgressBar;
class QAction;
class QCloseEvent;

class CodeHighlighter;
class VisualizationWidget;
class ControlPanel;
class AlgorithmManager;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void onAlgorithmChanged(int index);
    void onExecuteClicked();
    void onGenerateDataClicked();
    void onPlayClicked();
    void onPauseClicked();
    void onStepForward();
    void onStepBackward();
    void onResetClicked();
    void onSpeedChanged(int speed);
    void onAlgorithmStepChanged(int currentStep, int totalSteps);
    void onAlgorithmFinished();
    void onAlgorithmError(const QString& error);
    void onExecutionStarted();
    void onNewData();
    void onLoadData();
    void onSaveData();
    void onExportImage();
    void onAbout();
    void onSettings();
    void onShowStatistics(bool show);
    void onShowOperationInfo(bool show);
    void onEnableAnimations(bool enable);
    void onDataInputChanged();
    void onArraySizeChanged(int size);
    void onPlaybackStateChanged(bool isPlaying);

private:
    void setupUI();
    void setupMenuBar();
    void setupToolBar();
    void setupStatusBar();
    void setupConnections();
    void loadAlgorithmCode(const QString& algorithmName);
    void registerAlgorithms();
    void generateRandomData();
    void updateAlgorithmInfo();
    void setUIEnabled(bool enabled);
    void updateCodeHighlighting();
    void updateVisualization();
    void updateStatusBar();
    void updateControlPanelState();

    QWidget* createLeftPanel();
    QWidget* createRightPanel();
    QWidget* createDataInputPanel();
    QWidget* createAlgorithmInfoPanel();

    QVector<int> getCurrentInputData() const;
    void setInputData(const QVector<int>& data);
    QString dataToString(const QVector<int>& data) const;
    QVector<int> stringToData(const QString& str) const;
 QGroupBox* m_algorithmInfoGroup;
    QSplitter* m_mainSplitter;
    QComboBox* m_algorithmSelector;
    QTextEdit* m_codeEditor;
    CodeHighlighter* m_codeHighlighter;
    QLabel* m_algorithmDescriptionLabel;
    QLabel* m_complexityLabel;
    VisualizationWidget* m_visualizationWidget;
    QTextEdit* m_dataInputEdit;
    QSpinBox* m_arraySizeSpinBox;
    QPushButton* m_generateDataButton;
    QPushButton* m_executeButton;
    ControlPanel* m_controlPanel;
    AlgorithmManager* m_algorithmManager;
    QProgressBar* m_statusProgressBar;
    QLabel* m_statusLabel;
    QAction* m_newDataAction;
    QAction* m_loadDataAction;
    QAction* m_saveDataAction;
    QAction* m_exportImageAction;
    QAction* m_exitAction;
    QAction* m_aboutAction;
    QAction* m_settingsAction;
    QAction* m_showStatisticsAction;
    QAction* m_showOperationInfoAction;
    QAction* m_enableAnimationsAction;
    bool m_hasExecutedAlgorithm;
    QString m_lastDataFile;
    QVector<int> m_currentData;
    bool m_showStatistics;
    bool m_showOperationInfo;
    bool m_enableAnimations;
    int m_defaultArraySize;
    int m_maxArraySize;
};

#endif // MAINWINDOW_H
