#include "ASTVisualizer.h"

#include <QTimer>
#include <QDebug>
#include <QPainter>
#include <QVariantAnimation>
#include <QFont>

// ==================== VariableItem ====================

VariableItem::VariableItem(const QString& name,
                           const QVariant& value,
                           QGraphicsItem* parent)
    : QObject(nullptr),
    QGraphicsRectItem(parent),
    m_name(name)
{
    setRect(0, 0, 200, 50);
    setBrush(QColor(70, 130, 180));
    setPen(QPen(Qt::black, 2));

    m_nameText = new QGraphicsTextItem(name, this);
    m_nameText->setPos(10, 5);
    m_nameText->setDefaultTextColor(Qt::white);

    QFont font;
    font.setPointSize(12);
    font.setBold(true);
    m_nameText->setFont(font);

    m_valueText = new QGraphicsTextItem(value.toString(), this);
    m_valueText->setPos(10, 25);
    m_valueText->setDefaultTextColor(Qt::white);
}

void VariableItem::updateValue(const QVariant& value)
{
    m_valueText->setPlainText(value.toString());

    auto* anim = new QPropertyAnimation(this, "opacity");
    anim->setDuration(300);
    anim->setStartValue(1.0);
    anim->setKeyValueAt(0.5, 0.3);
    anim->setEndValue(1.0);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

// ==================== ArrayItem ====================

ArrayItem::ArrayItem(const QString& name,
                     const QVector<int>& values,
                     QGraphicsItem* parent)
    : QGraphicsItem(parent),
    m_name(name)
{
    const int w = 60, h = 60, spacing = 5;

    m_nameText = new QGraphicsTextItem(name, this);
    m_nameText->setPos(0, -30);

    QFont f;
    f.setPointSize(12);
    f.setBold(true);
    m_nameText->setFont(f);

    for (int i = 0; i < values.size(); ++i) {
        auto* rect = new QGraphicsRectItem(0, 0, w, h, this);
        rect->setPos(i * (w + spacing), 0);
        rect->setBrush(QColor(100, 149, 237));
        rect->setPen(QPen(Qt::black, 2));
        m_elements.append(rect);

        auto* valText = new QGraphicsTextItem(QString::number(values[i]), this);
        valText->setPos(rect->pos().x() + 20, 15);
        m_valueTexts.append(valText);

        auto* idxText = new QGraphicsTextItem(QString::number(i), this);
        idxText->setPos(rect->pos().x() + 25, h + 5);
        idxText->setDefaultTextColor(Qt::gray);
        m_indexTexts.append(idxText);
    }
}

void ArrayItem::updateElement(int index, int value)
{
    if (index >= 0 && index < m_valueTexts.size())
        m_valueTexts[index]->setPlainText(QString::number(value));
}

void ArrayItem::highlightElement(int index, const QColor& color)
{
    if (index >= 0 && index < m_elements.size())
        m_elements[index]->setBrush(color);
}

void ArrayItem::clearHighlights()
{
    for (auto* e : m_elements)
        e->setBrush(QColor(100, 149, 237));
}

QRectF ArrayItem::boundingRect() const
{
    if (m_elements.isEmpty()) return QRectF();
    return QRectF(0, -30, m_elements.size() * 65, 100);
}

void ArrayItem::paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*) {}

QGraphicsRectItem* ArrayItem::getElementRect(int index)
{
    if (index >= 0 && index < m_elements.size())
        return m_elements[index];
    return nullptr;
}

// ==================== ASTVisualizer ====================

ASTVisualizer::ASTVisualizer(QGraphicsScene* scene, QObject* parent)
    : QObject(parent),
    m_scene(scene),
    m_currentStep(-1)
{
    m_messageText = new QGraphicsTextItem();
    m_messageText->setPos(20, scene->height() - 80);
    m_scene->addItem(m_messageText);
}

// ==================== Step Loader ====================

bool ASTVisualizer::loadExecutionSteps(const QJsonArray& steps)
{
    reset();
    if (steps.isEmpty()) return false;
    m_steps = steps;
    m_currentStep = -1;
    return true;
}

// ==================== Execution ====================

void ASTVisualizer::executeStep()
{
    if (m_currentStep >= m_steps.size() - 1) {
        emit executionFinished();
        return;
    }

    m_currentStep++;
    executeStepInternal(m_steps[m_currentStep].toObject());
    emit stepExecuted(m_currentStep, m_steps.size());
}

void ASTVisualizer::executeStepInternal(const QJsonObject& step)
{
    QString type = step["type"].toString();
    QJsonObject data = step["data"].toObject();
    m_messageText->setPlainText(step["desc"].toString());

    if (type == "array_init") {
        QVector<int> vals;
        for (auto v : data["values"].toArray()) vals.append(v.toInt());
        m_arrays[data["name"].toString()] = vals;
        createArrayVisual(data["name"].toString(), vals);
    }
    else if (type == "var_assign") {
        setVariable(data["name"].toString(), data["value"].toVariant());
        updateVariableVisual(data["name"].toString(), data["value"].toVariant());
    }
    else if (type == "array_access") {
        highlightArrayElements(data["array"].toString(),
                               { data["index"].toInt() },
                               QColor(255,215,0));
    }
    else if (type == "compare") {
        highlightArrayElements(data["array"].toString(),
                               { data["i"].toInt(), data["j"].toInt() },
                               QColor(135,206,250));
    }
    else if (type == "swap") {
        auto& arr = m_arrays[data["array"].toString()];
        int i = data["i"].toInt(), j = data["j"].toInt();
        std::swap(arr[i], arr[j]);
        updateArrayVisual(data["array"].toString(), i, arr[i]);
        updateArrayVisual(data["array"].toString(), j, arr[j]);
        animateSwap(data["array"].toString(), i, j);
    }
    else if (type == "print") {
        showMessage(data["value"].toString());
    }
    else if (type == "error") {
        showMessage("ERROR: " + step["desc"].toString());
        emit executionFinished();
    }
}

// ==================== Helpers ====================

void ASTVisualizer::setVariable(const QString& n, const QVariant& v)
{
    m_variables[n] = v;
    emit variableChanged(n, v);
}

QVariant ASTVisualizer::getVariable(const QString& n)
{
    return m_variables.value(n);
}

void ASTVisualizer::createVariableVisual(const QString& n, const QVariant& v)
{
    if (m_variableItems.contains(n)) return;
    auto* item = new VariableItem(n, v);
    item->setPos(20, 20 + m_variableItems.size() * 70);
    m_scene->addItem(item);
    m_variableItems[n] = item;
}

void ASTVisualizer::updateVariableVisual(const QString& n, const QVariant& v)
{
    if (!m_variableItems.contains(n))
        createVariableVisual(n, v);
    else
        m_variableItems[n]->updateValue(v);
}

void ASTVisualizer::createArrayVisual(const QString& n, const QVector<int>& v)
{
    if (m_arrayItems.contains(n)) return;
    auto* item = new ArrayItem(n, v);
    item->setPos(250, 20 + m_arrayItems.size() * 120);
    m_scene->addItem(item);
    m_arrayItems[n] = item;
}

void ASTVisualizer::updateArrayVisual(const QString& n, int i, int v)
{
    if (!m_arrayItems.contains(n)) return;
    m_arrayItems[n]->updateElement(i, v);
}

void ASTVisualizer::highlightArrayElements(const QString& n,
                                           const QVector<int>& idx,
                                           const QColor& c)
{
    if (!m_arrayItems.contains(n)) return;
    for (int i : idx) m_arrayItems[n]->highlightElement(i, c);
}

void ASTVisualizer::animateSwap(const QString&, int, int) {}

void ASTVisualizer::showMessage(const QString& msg)
{
    m_messageText->setPlainText("Output: " + msg);
}

void ASTVisualizer::reset()
{
    m_steps = QJsonArray();
    m_currentStep = -1;
    m_variables.clear();
    m_arrays.clear();

    qDeleteAll(m_variableItems);
    qDeleteAll(m_arrayItems);
    m_variableItems.clear();
    m_arrayItems.clear();

    if (m_messageText) m_messageText->setPlainText("");
}
