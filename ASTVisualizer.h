#ifndef ASTVISUALIZER_H
#define ASTVISUALIZER_H

#include <QObject>
#include <QJsonArray>
#include <QJsonObject>
#include <QGraphicsScene>
#include <QGraphicsRectItem>
#include <QGraphicsTextItem>
#include <QPropertyAnimation>
#include <QSequentialAnimationGroup>
#include <QMap>
#include <QVector>
#include <QVariant>

// ==================== VariableItem ====================

class VariableItem : public QObject, public QGraphicsRectItem {
    Q_OBJECT
    Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity)

public:
    VariableItem(const QString& name,
                 const QVariant& value,
                 QGraphicsItem* parent = nullptr);

    void updateValue(const QVariant& value);
    QString getName() const { return m_name; }

private:
    QString m_name;
    QGraphicsTextItem* m_nameText;
    QGraphicsTextItem* m_valueText;
};

// ==================== ArrayItem ====================

class ArrayItem : public QGraphicsItem {
public:
    ArrayItem(const QString& name,
              const QVector<int>& values,
              QGraphicsItem* parent = nullptr);

    void updateElement(int index, int value);
    void highlightElement(int index, const QColor& color);
    void clearHighlights();

    QRectF boundingRect() const override;
    void paint(QPainter* painter,
               const QStyleOptionGraphicsItem* option,
               QWidget* widget) override;

    QGraphicsRectItem* getElementRect(int index);

private:
    QString m_name;
    QVector<QGraphicsRectItem*> m_elements;
    QVector<QGraphicsTextItem*> m_valueTexts;
    QVector<QGraphicsTextItem*> m_indexTexts;
    QGraphicsTextItem* m_nameText;
};

// ==================== ASTVisualizer (Step Player) ====================

class ASTVisualizer : public QObject {
    Q_OBJECT

public:
    explicit ASTVisualizer(QGraphicsScene* scene,
                           QObject* parent = nullptr);

    // Load execution steps from Python
    bool loadExecutionSteps(const QJsonArray& steps);

    // Execution control
    void reset();
    void executeStep();

    // Step info
    int getCurrentStep() const { return m_currentStep; }
    int getTotalSteps() const { return m_steps.size(); }

signals:
    void stepExecuted(int stepNumber, int totalSteps);
    void executionFinished();
    void variableChanged(const QString& name, const QVariant& value);

private:
    // Step execution
    void executeStepInternal(const QJsonObject& step);

    // Internal Variable Management
    void setVariable(const QString& name, const QVariant& value);
    QVariant getVariable(const QString& name);

    // Visual helpers
    void createVariableVisual(const QString& name,
                              const QVariant& value);
    void updateVariableVisual(const QString& name,
                              const QVariant& value);
    void createArrayVisual(const QString& name,
                           const QVector<int>& values);
    void updateArrayVisual(const QString& name,
                           int index,
                           int value);
    void highlightArrayElements(const QString& name,
                                const QVector<int>& indices,
                                const QColor& color);
    void animateSwap(const QString& arrayName,
                     int idx1,
                     int idx2);
    void showMessage(const QString& message);

    // State
    QGraphicsScene* m_scene;
    QJsonArray m_steps;
    int m_currentStep;

    QMap<QString, QVariant> m_variables;
    QMap<QString, QVector<int>> m_arrays;

    QMap<QString, VariableItem*> m_variableItems;
    QMap<QString, ArrayItem*> m_arrayItems;

    QGraphicsTextItem* m_messageText;
};

#endif // ASTVISUALIZER_H
