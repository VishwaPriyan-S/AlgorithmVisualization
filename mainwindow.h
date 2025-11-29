#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSplitter>
#include <QTextEdit>
#include <QComboBox>
#include <QLabel>
#include <QSpinBox>
#include <QProgressBar>
#include <QPushButton>
#include <QVBoxLayout>

class CodeHighlighter;
class ControlPanel;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    // UI
    QSplitter* m_mainSplitter;
    QTextEdit* m_codeEditor;
    CodeHighlighter* m_codeHighlighter;

    QComboBox* m_algorithmSelector;
    QLabel* m_algorithmDescriptionLabel;
    QLabel* m_complexityLabel;

    QTextEdit* m_dataInputEdit;
    QSpinBox* m_arraySizeSpinBox;
    QPushButton* m_generateDataButton;
    QPushButton* m_executeButton;
    QTextEdit* m_jsonOutputView;

    QWidget* m_visualizationPlaceholder;     // NEW

    ControlPanel* m_controlPanel;

    QLabel* m_statusLabel;
    QProgressBar* m_statusProgressBar;

    bool m_hasExecutedAlgorithm;
    bool m_showStatistics;
    bool m_showOperationInfo;
    bool m_enableAnimations;

    int m_defaultArraySize;
    int m_maxArraySize;

private:
    void setupUI();
    void setupMenuBar();
    void setupToolBar();
    void setupStatusBar();
    void setupConnections();

    QWidget* createLeftPanel();
    QWidget* createRightPanel();
    QWidget* createDataInputPanel();
    QWidget* createAlgorithmInfoPanel();

    void setUIEnabled(bool enabled);
    void updateStatusBar();

    QVector<int> getCurrentInputData() const;
    void setInputData(const QVector<int>& data);
    QString dataToString(const QVector<int>& data) const;

private slots:
    void onAlgorithmChanged(int index);
    void onExecuteClicked();
    void onGenerateDataClicked();
    void onParseCustomCodeClicked();
    void onDataInputChanged();
    void onArraySizeChanged(int size);
    void onAbout();
};

#endif // MAINWINDOW_H
