#include "ASTVisualizer.h"
#include <QPainter>
#include <QtMath>
#include <QDebug>
#include <QFont>
#include <QBrush>
#include <QPen>
#include <QJsonValue>
#include <QGraphicsDropShadowEffect>
#include <QPainterPath>
#include <functional>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>

// ==================== NodeItem (Draggable) ====================
NodeItem::NodeItem(QGraphicsItem* parent) : QGraphicsEllipseItem(-24, -24, 48, 48, parent) {
    setFlag(QGraphicsItem::ItemIsMovable);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges);
    setZValue(10);
    setPen(QPen(QColor(0, 212, 255, 100), 2));
}

void NodeItem::addEdge(QGraphicsLineItem* line, bool isStart) {
    edges.append({line, isStart});
}

QVariant NodeItem::itemChange(GraphicsItemChange change, const QVariant &value) {
    if (change == ItemPositionChange && scene()) {
        QPointF newPos = value.toPointF();
        for (const auto& edge : edges) {
            QLineF line = edge.line->line();
            if (edge.isStart) line.setP1(newPos);
            else              line.setP2(newPos);
            edge.line->setLine(line);
        }
    }
    return QGraphicsItem::itemChange(change, value);
}

// ==================== AnimatedBarItem ====================
AnimatedBarItem::AnimatedBarItem(QGraphicsItem* parent) : QGraphicsObject(parent), m_color(Qt::blue) {
    auto* shadow = new QGraphicsDropShadowEffect();
    shadow->setBlurRadius(20);
    shadow->setOffset(0, 0);
    shadow->setColor(QColor(0, 0, 0, 150));
    setGraphicsEffect(shadow);
}

void AnimatedBarItem::setVisuals(const QColor& color, const QRectF& /*rect*/, const QString& text) {
    m_color = color;
    // NOTE: Do NOT set m_rect here — it is animated via QPropertyAnimation.
    // Setting it here would make start == end, killing the animation.
    m_text = text;
    auto* shadow = static_cast<QGraphicsDropShadowEffect*>(graphicsEffect());
    if (shadow) shadow->setColor(QColor(color.red(), color.green(), color.blue(), 150));
    update();
}

QRectF AnimatedBarItem::boundingRect() const {
    return m_rect;
}

void AnimatedBarItem::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) {
    if (m_rect.isEmpty()) return;
    painter->setRenderHint(QPainter::Antialiasing, true);

    const qreal radius = 8.0;
    QPainterPath path;
    path.addRoundedRect(m_rect, radius, radius);

    // === 4-stop cyberpunk gradient ===
    QLinearGradient baseGrad(m_rect.topLeft(), m_rect.bottomLeft());
    baseGrad.setColorAt(0.00, m_color.lighter(155));
    baseGrad.setColorAt(0.15, m_color.lighter(120));
    baseGrad.setColorAt(0.60, m_color);
    baseGrad.setColorAt(1.00, m_color.darker(150));

    painter->setBrush(baseGrad);
    painter->setPen(Qt::NoPen);
    painter->drawPath(path);

    // === Neon outer border ===
    painter->setPen(QPen(m_color.lighter(180), 1.2));
    painter->setBrush(Qt::NoBrush);
    painter->drawPath(path);

    // === Specular highlight band at top ===
    qreal specH = qMin(m_rect.height() * 0.25, 14.0);
    QRectF specRect(m_rect.x() + 3, m_rect.y() + 2, m_rect.width() - 6, specH);
    QPainterPath specPath;
    specPath.addRoundedRect(specRect, radius - 2, radius - 2);
    QLinearGradient specGrad(specRect.topLeft(), specRect.bottomLeft());
    specGrad.setColorAt(0, QColor(255, 255, 255, 65));
    specGrad.setColorAt(1, QColor(255, 255, 255, 0));
    painter->setPen(Qt::NoPen);
    painter->setBrush(specGrad);
    painter->drawPath(specPath);

    // === Value text with drop shadow ===
    QFont font("Consolas", 11, QFont::Bold);
    painter->setFont(font);
    painter->setPen(QColor(0, 0, 0, 130));
    painter->drawText(m_rect.adjusted(1, 1, 1, 1), Qt::AlignCenter, m_text);
    painter->setPen(QColor("#e6edf3"));
    painter->drawText(m_rect, Qt::AlignCenter, m_text);
}

// ==================== GraphItem ====================
GraphItem::GraphItem(const QString& name, QGraphicsItem* parent)
    : QGraphicsItem(parent), m_name(name) {
    m_nameText = new QGraphicsTextItem(name, this);
    m_nameText->setDefaultTextColor(QColor("#8b949e"));
    m_nameText->setFont(QFont("Consolas", 13, QFont::Bold));
    m_nameText->setPos(-50, -30);
}

QRectF GraphItem::boundingRect() const { return childrenBoundingRect(); }
void GraphItem::paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*) {}

void GraphItem::setTheme(const QJsonObject& theme) {
    if (theme.contains("primary")) m_primaryColor = QColor(theme["primary"].toString());
    if (theme.contains("highlight")) m_highlightColor = QColor(theme["highlight"].toString());
    if (theme.contains("sorted")) m_sortedColor = QColor(theme["sorted"].toString());
}

void GraphItem::updateData(const QJsonObject& adjList, const QJsonObject& metadata) {
    prepareGeometryChange();
    // Clear edge references from NodeItems BEFORE deleting the edges,
    // otherwise NodeItem::itemChange may access deleted QGraphicsLineItems.
    for (auto& node : m_nodes) {
        if (node.circle) {
            node.circle->clearEdges();
        }
    }
    qDeleteAll(m_edges);
    m_edges.clear();
    for (auto& node : m_nodes) {
        if (node.circle) delete node.circle;
    }
    m_nodes.clear();

    const bool treeMode = metadata.value("tree_mode").toBool(false);
    if (treeMode) {
        QString rootName;
        QJsonObject rootObj;
        for (auto it = adjList.begin(); it != adjList.end(); ++it) {
            if (it.value().isObject()) {
                rootName = it.key();
                rootObj = it.value().toObject();
                break;
            }
        }
        if (rootObj.isEmpty()) {
            update();
            return;
        }

        // --- Tree Node Structure ---
        struct TreeNode {
            QString uniqueId;
            QString label;
            QPointF pos;
            QList<TreeNode*> children;
        };

        auto getLabel = [](const QJsonObject& node) -> QString {
            if (node.contains("value")) return node.value("value").toVariant().toString();
            if (node.contains("val")) return node.value("val").toVariant().toString();
            if (node.contains("data")) return node.value("data").toVariant().toString();
            return QString();
        };

        // --- Pass 1: Build internal tree structure ---
        int idCounter = 0;
        QList<TreeNode*> allNodes;

        std::function<TreeNode*(const QJsonObject&)> buildNode;
        buildNode = [&](const QJsonObject& json) -> TreeNode* {
            TreeNode* node = new TreeNode();
            allNodes.append(node);
            node->label = getLabel(json);
            if (node->label.isEmpty()) node->label = QString("N%1").arg(idCounter);
            node->uniqueId = node->label + "_" + QString::number(idCounter++);

            if (json.contains("left") && json.value("left").isObject()) {
                node->children.append(buildNode(json.value("left").toObject()));
            }
            if (json.contains("right") && json.value("right").isObject()) {
                node->children.append(buildNode(json.value("right").toObject()));
            }
            if (json.contains("children") && json.value("children").isArray()) {
                for (const QJsonValue& child : json.value("children").toArray()) {
                    if (child.isObject()) {
                        node->children.append(buildNode(child.toObject()));
                    }
                }
            }
            return node;
        };

        TreeNode* treeRoot = buildNode(rootObj);

        // --- Pass 2: Calculate subtree widths ---
        const double hSpacing = 65.0;
        const double vSpacing = 90.0;

        std::function<double(TreeNode*)> subtreeWidth;
        subtreeWidth = [&](TreeNode* n) -> double {
            if (n->children.isEmpty()) return 1.0;
            double total = 0;
            for (TreeNode* child : n->children) {
                total += subtreeWidth(child);
            }
            return total;
        };

        // --- Pass 3: Assign centered positions ---
        struct EdgeLink { QString from; QString to; };
        QList<EdgeLink> links;

        std::function<void(TreeNode*, double, double)> layoutTree;
        layoutTree = [&](TreeNode* n, double x, double y) {
            n->pos = QPointF(x, y);
            if (n->children.isEmpty()) return;

            double totalChildWidth = 0;
            for (TreeNode* child : n->children) {
                totalChildWidth += subtreeWidth(child);
            }

            double startX = x - (totalChildWidth * hSpacing) / 2.0;
            for (TreeNode* child : n->children) {
                double childWidth = subtreeWidth(child);
                double childX = startX + (childWidth * hSpacing) / 2.0;
                layoutTree(child, childX, y + vSpacing);
                links.append({n->uniqueId, child->uniqueId});
                startX += childWidth * hSpacing;
            }
        };

        layoutTree(treeRoot, 0, 0);

        // --- Pass 4: Create graphics items ---
        for (TreeNode* n : allNodes) {
            auto* circle = new NodeItem(this);
            circle->setPos(n->pos);

            QColor nodeColor = m_primaryColor;
            QRadialGradient gradient(n->pos.x(), n->pos.y(), 24);
            gradient.setColorAt(0, nodeColor.lighter(150));
            gradient.setColorAt(0.6, nodeColor);
            gradient.setColorAt(1, nodeColor.darker(140));
            circle->setBrush(gradient);

            auto* shadow = new QGraphicsDropShadowEffect();
            shadow->setBlurRadius(22);
            shadow->setColor(QColor(nodeColor.red(), nodeColor.green(), nodeColor.blue(), 140));
            shadow->setOffset(0, 0);
            circle->setGraphicsEffect(shadow);

            auto* label = new QGraphicsTextItem(n->label, circle);
            label->setDefaultTextColor(QColor("#e6edf3"));
            label->setFont(QFont("Consolas", 10, QFont::Bold));
            const QRectF r = label->boundingRect();
            label->setPos(-r.width() / 2, -r.height() / 2);

            m_nodes[n->uniqueId] = {circle, label, n->pos, n->label};
        }

        for (const EdgeLink& edge : links) {
            if (!m_nodes.contains(edge.from) || !m_nodes.contains(edge.to)) continue;
            NodeItem* n1 = m_nodes[edge.from].circle;
            NodeItem* n2 = m_nodes[edge.to].circle;
            auto* line = new QGraphicsLineItem(n1->x(), n1->y(), n2->x(), n2->y(), this);
            line->setPen(QPen(QColor(0, 212, 255, 70), 2));
            line->setZValue(-1);
            n1->addEdge(line, true);
            n2->addEdge(line, false);
            m_edges.append(line);
        }

        // Reposition name text centered above the tree root
        m_nameText->setPos(
            treeRoot->pos.x() - m_nameText->boundingRect().width() / 2,
            treeRoot->pos.y() - 70);

        qDeleteAll(allNodes);

        update();
        return;
    }

    QStringList nodeKeys = adjList.keys();
    if (nodeKeys.isEmpty()) return;

    int N = nodeKeys.size();
    double radius = 130.0;
    double angleStep = 2 * M_PI / N;

    QSet<QString> visitedSet;
    QSet<QString> frontierSet;
    QSet<QString> pathSet;
    const QString currentNode = metadata.value("current").toString();

    const QJsonArray visitedArray = metadata.value("visited").toArray();
    for (const QJsonValue& v : visitedArray) {
        visitedSet.insert(v.toVariant().toString());
    }

    const QJsonArray frontierArray = metadata.value("frontier").toArray();
    for (const QJsonValue& v : frontierArray) {
        frontierSet.insert(v.toVariant().toString());
    }

    const QJsonArray pathArray = metadata.value("path").toArray();
    for (const QJsonValue& v : pathArray) {
        pathSet.insert(v.toVariant().toString());
    }

    // Create Nodes
    for (int i = 0; i < N; ++i) {
        QString key = nodeKeys[i];
        double angle = i * angleStep;
        double x = radius * qCos(angle);
        double y = radius * qSin(angle);

        auto* circle = new NodeItem(this);
        circle->setPos(x, y);

        auto* label = new QGraphicsTextItem(key, circle);
        label->setDefaultTextColor(QColor("#e6edf3"));
        label->setFont(QFont("Consolas", 10, QFont::Bold));
        QRectF r = label->boundingRect();
        label->setPos(-r.width()/2, -r.height()/2);

        QColor nodeColor = m_primaryColor;
        if (visitedSet.contains(key)) {
            nodeColor = m_sortedColor;
        }
        if (frontierSet.contains(key)) {
            nodeColor = m_highlightColor;
        }
        if (!currentNode.isEmpty() && currentNode == key) {
            nodeColor = QColor("#ff79c6"); // Cyberpunk hot pink
        }
        if (pathSet.contains(key)) {
            nodeColor = QColor("#bd93f9"); // Neon purple
        }

        // Radial gradient for depth
        QRadialGradient gradient(x, y, 24);
        gradient.setColorAt(0, nodeColor.lighter(150));
        gradient.setColorAt(0.6, nodeColor);
        gradient.setColorAt(1, nodeColor.darker(140));
        circle->setBrush(gradient);

        auto* shadow = new QGraphicsDropShadowEffect();
        shadow->setBlurRadius(22);
        shadow->setColor(QColor(nodeColor.red(), nodeColor.green(), nodeColor.blue(), 140));
        shadow->setOffset(0, 0);
        circle->setGraphicsEffect(shadow);

        m_nodes[key] = {circle, label, QPointF(x, y), key};
    }

    // Create Edges
    QSet<QString> processedEdges;
    for (const QString& source : nodeKeys) {
        QJsonArray neighbors = adjList[source].toArray();
        for (const QJsonValue& val : neighbors) {
            QString target = val.isString() ? val.toString() : QString::number(val.toInt());
            QString edgeKey = source < target ? source + "-" + target : target + "-" + source;

            if (m_nodes.contains(source) && m_nodes.contains(target) && !processedEdges.contains(edgeKey)) {
                NodeItem* n1 = m_nodes[source].circle;
                NodeItem* n2 = m_nodes[target].circle;
                auto* line = new QGraphicsLineItem(n1->x(), n1->y(), n2->x(), n2->y(), this);
                line->setPen(QPen(QColor(0, 212, 255, 70), 2.0));
                line->setZValue(-1);
                n1->addEdge(line, true);
                n2->addEdge(line, false);
                m_edges.append(line);
                processedEdges.insert(edgeKey);
            }
        }
    }
    update();
}

void GraphItem::highlightNode(const QString& label) {
    for (auto it = m_nodes.begin(); it != m_nodes.end(); ++it) {
        NodeData& nd = it.value();
        QColor nodeColor = (nd.displayLabel == label) ? m_highlightColor : m_primaryColor;

        QRadialGradient gradient(nd.pos.x(), nd.pos.y(), 24);
        gradient.setColorAt(0, nodeColor.lighter(150));
        gradient.setColorAt(0.6, nodeColor);
        gradient.setColorAt(1, nodeColor.darker(140));
        nd.circle->setBrush(gradient);

        auto* shadow = static_cast<QGraphicsDropShadowEffect*>(nd.circle->graphicsEffect());
        if (shadow) shadow->setColor(QColor(nodeColor.red(), nodeColor.green(), nodeColor.blue(), 140));
    }
}

void GraphItem::setDisplayName(const QString& name) {
    m_nameText->setPlainText(name);
}

// ==================== ArrayItem ====================
ArrayItem::ArrayItem(const QString& name, QGraphicsItem* parent) 
    : QGraphicsItem(parent), m_name(name), m_mode(VisualizationMode::Generic), m_sortedStart(-1), m_maxObservedValue(1) 
{
    m_nameText = new QGraphicsTextItem(name, this);
    m_nameText->setDefaultTextColor(QColor("#79c0ff"));
    m_nameText->setFont(QFont("Consolas", 11, QFont::Bold));
    m_nameText->setPos(0, -28);
    setDataStructureType(name);
}

void ArrayItem::setMode(VisualizationMode mode) {
    m_mode = mode;
    updateLayout();
}

void ArrayItem::setDataStructureType(const QString& name) {
    QString lower = name.toLower();
    m_isStack = (lower.contains("stack") || lower == "stk");
    m_isQueue = (lower.contains("queue") || lower == "deque" || lower == "q");
}

void ArrayItem::setTheme(const QJsonObject& theme) {
    if (theme.contains("primary")) m_primaryColor = QColor(theme["primary"].toString());
    if (theme.contains("highlight")) m_highlightColor = QColor(theme["highlight"].toString());
    if (theme.contains("sorted")) m_sortedColor = QColor(theme["sorted"].toString());
}

void ArrayItem::updateData(const QVector<int>& newValues) {
    // 1. Detect Changes
    m_highlightIndices.clear();
    for(int i=0; i<newValues.size() && i<m_values.size(); ++i) {
        if(newValues[i] != m_values[i]) {
            m_highlightIndices.insert(i);
        }
    }
    m_values = newValues;

    // Track max for proportional height
    for (int val : m_values) {
        if (val > m_maxObservedValue) m_maxObservedValue = val;
    }

    updateLayout();
}

void ArrayItem::updateStepContext(const QJsonObject& vars) {
    m_compareIndices.clear();
    m_sortedStart = -1;

    const int i = vars.value("i").toInt(-1);
    const int j = vars.value("j").toInt(-1);
    const int left = vars.value("left").toInt(-1);
    const int right = vars.value("right").toInt(-1);
    const int low = vars.value("low").toInt(-1);
    const int high = vars.value("high").toInt(-1);
    const int n = vars.value("n").toInt(-1);

    if (i >= 0) m_compareIndices.insert(i);
    if (j >= 0) m_compareIndices.insert(j);
    if (j + 1 >= 0 && j + 1 < m_values.size()) m_compareIndices.insert(j + 1);
    if (left >= 0) m_compareIndices.insert(left);
    if (right >= 0) m_compareIndices.insert(right);
    if (low >= 0) m_compareIndices.insert(low);
    if (high >= 0) m_compareIndices.insert(high);

    // Bubble-sort heuristic: suffix [n-i, n-1] is already sorted after each pass.
    if (n > 0 && i >= 0 && i < n) {
        m_sortedStart = qBound(0, n - i, m_values.size());
    } else if (vars.value("swapped").isBool() && !vars.value("swapped").toBool()) {
        m_sortedStart = 0; // Early-exit pass means full array sorted.
    }
    // NOTE: Do NOT call updateLayout() here — updateData() will call it.
}

void ArrayItem::updateLayout() {
    if (m_animGroup) {
        m_animGroup->stop();
        m_animGroup = nullptr;
    }

    // Manage visual elements pool
    while (m_visualElements.size() < m_values.size()) {
        auto* bar = new AnimatedBarItem(this);
        m_visualElements.append(bar);
    }
    while (m_visualElements.size() > m_values.size()) {
        delete m_visualElements.takeLast();
    }

    m_animGroup = new QParallelAnimationGroup();

    auto animateBar = [this](AnimatedBarItem* bar, const QRectF& targetRect, int targetX, int targetY, const QColor& color, const QString& text) {
        bar->setVisuals(color, targetRect, text);
        bool isNew = bar->rect().isEmpty();
        if (isNew) {
            bar->setRect(targetRect);
            bar->setPos(targetX, targetY);
        } else {
            QPropertyAnimation* rectAnim = new QPropertyAnimation(bar, "rect");
            rectAnim->setDuration(280);
            rectAnim->setEasingCurve(QEasingCurve::OutCubic);
            rectAnim->setStartValue(bar->rect());
            rectAnim->setEndValue(targetRect);
            m_animGroup->addAnimation(rectAnim);

            QPropertyAnimation* posAnim = new QPropertyAnimation(bar, "pos");
            posAnim->setDuration(280);
            posAnim->setEasingCurve(QEasingCurve::OutCubic);
            posAnim->setStartValue(bar->pos());
            posAnim->setEndValue(QPointF(targetX, targetY));
            m_animGroup->addAnimation(posAnim);
        }
    };

    if (m_isStack) {
        // ===== VERTICAL STACK LAYOUT (bottom-up) =====
        int cardW = 90, cardH = 40, gap = 6;
        int totalH = m_values.size() * (cardH + gap) - gap;
        m_nameText->setPos(0, -32);

        for (int i = 0; i < m_values.size(); ++i) {
            int visualRow = m_values.size() - 1 - i; // index 0 at bottom
            int targetX = 0;
            int targetY = visualRow * (cardH + gap);

            bool isTop = (i == m_values.size() - 1);
            QColor c = isTop ? QColor("#00d4ff") : QColor("#21262d");
            if (m_highlightIndices.contains(i)) c = m_highlightColor;

            QRectF rect(0, 0, cardW, cardH);
            animateBar(m_visualElements[i], rect, targetX, targetY, c, QString::number(m_values[i]));
        }

    } else if (m_isQueue) {
        // ===== HORIZONTAL QUEUE LAYOUT =====
        int cardW = 70, cardH = 42, gap = 24;
        m_nameText->setPos(0, -32);

        for (int i = 0; i < m_values.size(); ++i) {
            int targetX = i * (cardW + gap);
            int targetY = 0;

            bool isFront = (i == 0);
            bool isRear = (i == m_values.size() - 1);
            QColor c = m_primaryColor;
            if (isFront) c = QColor("#50fa7b");
            if (isRear) c = QColor("#ff79c6");
            if (m_highlightIndices.contains(i)) c = QColor("#f1fa8c");

            QRectF rect(0, 0, cardW, cardH);
            animateBar(m_visualElements[i], rect, targetX, targetY, c, QString::number(m_values[i]));
        }

    } else {
        // ===== SORTING / GENERIC BAR LAYOUT =====
        int boxWidth = 55, baseHeight = 45, maxHeight = 130;

        for (int i = 0; i < m_values.size(); ++i) {
            int val = m_values[i];
            QColor barColor = m_primaryColor;
            if (m_highlightIndices.contains(i)) barColor = m_highlightColor;
            if (m_compareIndices.contains(i)) barColor = QColor("#f1fa8c");
            if (m_sortedStart >= 0 && i >= m_sortedStart) barColor = m_sortedColor;

            QRectF targetRect;
            int targetX = i * boxWidth;
            int targetY = 0;

            if (m_mode == VisualizationMode::Sorting) {
                int h = std::max(20, (int)((double)val / m_maxObservedValue * maxHeight));
                targetY = maxHeight - h;
                targetRect = QRectF(0, 0, boxWidth - 4, h);
                m_nameText->setPos(0, maxHeight + 28);
            } else {
                targetRect = QRectF(0, 0, boxWidth - 4, baseHeight - 4);
                m_nameText->setPos(0, -28);
            }

            animateBar(m_visualElements[i], targetRect, targetX, targetY, barColor, QString::number(val));
        }
    }

    QParallelAnimationGroup** groupPtr = &m_animGroup;
    QObject::connect(m_animGroup, &QObject::destroyed, [groupPtr, grp = m_animGroup]() {
        if (*groupPtr == grp) *groupPtr = nullptr;
    });
    m_animGroup->start(QAbstractAnimation::DeleteWhenStopped);
}

QRectF ArrayItem::boundingRect() const {
    QRectF base = childrenBoundingRect();
    return base.adjusted(-15, -40, 60, 35);
}

void ArrayItem::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) {
    if (m_values.isEmpty()) return;
    painter->setRenderHint(QPainter::Antialiasing, true);

    if (m_isStack) {
        // --- U-shaped container outline ---
        int cardW = 90, cardH = 40, gap = 6, margin = 10;
        int totalH = m_values.size() * (cardH + gap) - gap;
        QRectF container(-margin, -margin, cardW + 2 * margin, totalH + 2 * margin);

        QPen containerPen(QColor(0, 212, 255, 60), 1.5, Qt::DashLine);
        painter->setPen(containerPen);
        painter->setBrush(Qt::NoBrush);
        painter->drawLine(container.topLeft(), container.bottomLeft());
        painter->drawLine(container.bottomLeft(), container.bottomRight());
        painter->drawLine(container.bottomRight(), container.topRight());

        // "TOP" label
        painter->setPen(QColor("#00d4ff"));
        painter->setFont(QFont("Consolas", 9, QFont::Bold));
        painter->drawText(cardW + margin + 6, cardH / 2 + 5, QString::fromUtf8("\u2190 TOP"));

    } else if (m_isQueue) {
        // --- Arrows between queue elements ---
        int cardW = 70, cardH = 42, gap = 24;
        QPen arrowPen(QColor(255, 121, 198, 160), 2);
        painter->setPen(arrowPen);
        for (int i = 0; i < m_values.size() - 1; ++i) {
            int x1 = i * (cardW + gap) + cardW + 3;
            int x2 = (i + 1) * (cardW + gap) - 3;
            int y = cardH / 2;
            painter->drawLine(x1, y, x2, y);
            painter->drawLine(x2, y, x2 - 6, y - 4);
            painter->drawLine(x2, y, x2 - 6, y + 4);
        }
        // FRONT / REAR labels
        painter->setFont(QFont("Consolas", 8, QFont::Bold));
        painter->setPen(QColor("#50fa7b"));
        painter->drawText(0, cardH + 20, "FRONT");
        if (m_values.size() > 1) {
            int lastX = (m_values.size() - 1) * (cardW + gap);
            painter->setPen(QColor("#ff79c6"));
            painter->drawText(lastX, cardH + 20, "REAR");
        }

    } else if (m_mode == VisualizationMode::Sorting) {
        // --- Index labels below sorting bars ---
        int boxWidth = 55, maxHeight = 130;
        painter->setFont(QFont("Consolas", 8));
        painter->setPen(QColor("#484f58"));
        for (int i = 0; i < m_values.size(); ++i) {
            painter->drawText(i * boxWidth, maxHeight + 12, boxWidth - 4, 14, Qt::AlignCenter, QString::number(i));
        }
    }
}

// ==================== VariableItem ====================
VariableItem::VariableItem(const QString& name, QGraphicsItem* parent) : QGraphicsRectItem(parent) {
    setRect(0, 0, 220, 42);
    setBrush(QColor(22, 27, 34, 230));
    setPen(QPen(QColor("#30363d"), 1.0));

    // Cyan accent stripe on left
    auto* accent = new QGraphicsRectItem(0, 0, 4, 42, this);
    accent->setBrush(QColor("#00d4ff"));
    accent->setPen(Qt::NoPen);

    m_nameText = new QGraphicsTextItem(name + " =", this);
    m_nameText->setDefaultTextColor(QColor("#79c0ff"));
    m_nameText->setFont(QFont("Consolas", 11, QFont::DemiBold));
    m_nameText->setPos(14, 8);
    m_valueText = new QGraphicsTextItem("", this);
    m_valueText->setDefaultTextColor(QColor("#7ee787"));
    m_valueText->setFont(QFont("Consolas", 11, QFont::Bold));
    m_valueText->setPos(100, 8);

    auto* shadow = new QGraphicsDropShadowEffect();
    shadow->setBlurRadius(16);
    shadow->setOffset(0, 2);
    shadow->setColor(QColor(0, 212, 255, 35));
    setGraphicsEffect(shadow);
}
void VariableItem::updateValue(const QString& value) { m_valueText->setPlainText(value); }

// ==================== CallStackItem ====================
CallStackItem::CallStackItem(const QString& funcName, int depth, QGraphicsItem* parent) : QGraphicsRectItem(parent) {
    setRect(0, 0, 210, 32);
    setBrush(QColor(22, 27, 34, 220));
    setPen(QPen(QColor("#30363d"), 1));

    // Depth-based accent
    auto* accent = new QGraphicsRectItem(0, 0, 3, 32, this);
    QColor accentColor = (depth == 0) ? QColor("#79c0ff") : QColor("#bd93f9");
    accent->setBrush(accentColor);
    accent->setPen(Qt::NoPen);

    auto* text = new QGraphicsTextItem(QString::fromUtf8("\u0192  ") + funcName, this);
    text->setDefaultTextColor(QColor("#e6edf3"));
    text->setFont(QFont("Consolas", 10, QFont::DemiBold));
    text->setPos(12, 4);
}

// ==================== ASTVisualizer Core ====================
ASTVisualizer::ASTVisualizer(QGraphicsScene* scene, QObject* parent)
    : QObject(parent), m_scene(scene), m_currentStep(-1), m_mode(VisualizationMode::Generic),
    m_errorItem(nullptr), m_consoleItem(nullptr)
{
    reset();
}

void ASTVisualizer::reset() {
    m_currentStep = -1;
    if(m_scene) m_scene->clear();

    m_variableItems.clear();
    m_arrayItems.clear();
    m_stackItems.clear();
    m_graphItems.clear();
    m_treeOwnerScope.clear();

    m_errorItem = new QGraphicsTextItem();
    m_errorItem->setDefaultTextColor(QColor("#ff5555"));
    m_errorItem->setFont(QFont("Consolas", 11));
    m_errorItem->setPos(10, 10);
    m_scene->addItem(m_errorItem);

    // Console Item
    m_consoleItem = new QGraphicsTextItem("Console Output:\n");
    m_consoleItem->setDefaultTextColor(QColor("#8b949e"));
    m_consoleItem->setFont(QFont("Consolas", 10));
    m_consoleItem->setPos(10, m_scene->height() - 100);
    m_scene->addItem(m_consoleItem);
}

void ASTVisualizer::logMessage(const QString& msg, bool clear) {
    if(!m_consoleItem) return;
    if (clear) {
        m_consoleItem->setPlainText("Console Output:\n" + msg);
    } else {
        QString current = m_consoleItem->toPlainText();
        QStringList lines = current.split("\n");
        if (lines.size() > 6) { // keep max 6 lines
            lines.removeAt(1);
        }
        m_consoleItem->setPlainText(lines.join("\n") + "\n" + msg);
    }
}

void ASTVisualizer::setMode(VisualizationMode mode) {
    m_mode = mode;
    for(auto* item : m_arrayItems) item->setMode(mode);
}

void ASTVisualizer::setAIMetadata(const QJsonObject& metadata) {
    m_aiMetadata = metadata;
    if (metadata.contains("algorithm_class")) {
        QString algoClass = metadata["algorithm_class"].toString().toLower();
        if (algoClass == "sorting") setMode(VisualizationMode::Sorting);
        else if (algoClass == "graph") setMode(VisualizationMode::Graph);
        else if (algoClass == "recursion") setMode(VisualizationMode::Recursion);

    }
}

void ASTVisualizer::loadSteps(const QJsonArray& steps) {
    reset();
    m_steps = steps;
}

void ASTVisualizer::executeStep() {
    if (m_currentStep >= m_steps.size() - 1) {
        emit executionFinished();
        return;
    }
    m_currentStep++;
    processStep(m_steps[m_currentStep].toObject());
    emit stepExecuted(m_currentStep, m_steps.size());
}

void ASTVisualizer::processStep(const QJsonObject& step) {
    QString eventType = step["event"].toString();

    // 1. Highlight Line
    if (step.contains("line")) {
        emit highlightLine(step["line"].toInt());
    }

    // 2. Handle Errors
    if (eventType == "error") {
        if(m_errorItem) m_errorItem->setPlainText("Runtime Error:\n" + step["message"].toString());
        return;
    }

    if (eventType == "print_output") {
        logMessage(step["message"].toString());
        return;
    }

    // Skip constructor internals — they produce confusing variable cards
    // (e.g., "data = 5" from Node.__init__) and have no useful algorithm state.
    if (step.value("func").toString() == "__init__") return;

    QJsonObject graphMetadata;
    if (step.contains("graph_state") && step["graph_state"].isObject()) {
        graphMetadata = step["graph_state"].toObject();
    }

    // 4. Variables / Arrays / Graphs
    QJsonObject vars = step["variables"].toObject();
    QJsonObject semanticVars = vars;

    if (!m_aiMetadata.isEmpty() && m_aiMetadata.contains("pointers")) {
        QJsonObject pointers = m_aiMetadata["pointers"].toObject();
        for (auto it = pointers.begin(); it != pointers.end(); ++it) {
            QString role = it.key(); // e.g., "left", "right", "i"
            QString actualName = it.value().toString();
            if (vars.contains(actualName)) {
                semanticVars[role] = vars[actualName];
            }
        }
    }

    bool hasNumericArray = false;
    for (auto it = vars.begin(); it != vars.end(); ++it) {
        if (!it.value().isArray()) continue;
        const QJsonArray arr = it.value().toArray();
        if (arr.isEmpty()) continue;

        bool numeric = true;
        for (const QJsonValue& v : arr) {
            if (!v.isDouble()) {
                numeric = false;
                break;
            }
        }
        if (numeric) {
            hasNumericArray = true;
            break;
        }
    }

    const bool sortMarkers =
        semanticVars.contains("i") || semanticVars.contains("j") || semanticVars.contains("swapped") || semanticVars.contains("left") || semanticVars.contains("right") || semanticVars.contains("mid");
    if (m_mode == VisualizationMode::Generic && hasNumericArray && sortMarkers) {
        setMode(VisualizationMode::Sorting);
    }

    if (graphMetadata.isEmpty()) {
        const QStringList currentCandidates = {"current", "node", "u", "v", "vertex"};
        for (const QString& key : currentCandidates) {
            if (semanticVars.contains(key)) {
                graphMetadata["current"] = semanticVars.value(key).toVariant().toString();
                break;
            }
        }

        const QStringList visitedCandidates = {"visited", "seen", "closed"};
        for (const QString& key : visitedCandidates) {
            if (semanticVars.contains(key) && semanticVars.value(key).isArray()) {
                graphMetadata["visited"] = semanticVars.value(key).toArray();
                break;
            }
        }

        const QStringList frontierCandidates = {"queue", "stack", "frontier", "open"};
        for (const QString& key : frontierCandidates) {
            if (semanticVars.contains(key) && semanticVars.value(key).isArray()) {
                graphMetadata["frontier"] = semanticVars.value(key).toArray();
                break;
            }
        }
    }

    // Determine the current function scope for tree ownership tracking
    const QString currentFunc = step.value("func").toString();

    for (auto it = vars.begin(); it != vars.end(); ++it) {
        QString name = it.key();
        QJsonValue val = it.value();

        if (val.isObject()) {
            const QJsonObject obj = val.toObject();
            const bool looksLikeTree =
                obj.contains("value") || obj.contains("val") || obj.contains("data") ||
                obj.contains("left") || obj.contains("right") ||
                obj.contains("children");
            if (looksLikeTree) {
                QString treeKey = name + "_tree";

                // Only update the tree from the scope that originally created it.
                // This prevents recursive functions (which see subtrees under the
                // same variable name) from overwriting the full tree visualization.
                if (m_graphItems.contains(treeKey) && m_treeOwnerScope.contains(treeKey)
                    && m_treeOwnerScope[treeKey] != currentFunc) {
                    // Highlight the currently visited node in the existing tree
                    QString nodeLabel;
                    if (obj.contains("data")) nodeLabel = obj.value("data").toVariant().toString();
                    else if (obj.contains("value")) nodeLabel = obj.value("value").toVariant().toString();
                    else if (obj.contains("val")) nodeLabel = obj.value("val").toVariant().toString();
                    if (!nodeLabel.isEmpty()) {
                        m_graphItems[treeKey]->highlightNode(nodeLabel);
                    }
                    continue;  // Skip — a different scope owns this tree
                }

                // Record ownership on first creation
                if (!m_treeOwnerScope.contains(treeKey)) {
                    m_treeOwnerScope[treeKey] = currentFunc;
                }

                QJsonObject normalized;
                normalized[name] = obj;
                QJsonObject treeMeta;
                treeMeta.insert("tree_mode", true);
                syncGraph(treeKey, normalized, treeMeta, name);
            } else {
                syncGraph(name, obj, graphMetadata);
            }
        } else if (val.isArray()) {
            syncArray(name, val.toArray(), semanticVars);
        } else {
            // Skip tree parameter variables that are null (e.g., recursive base case)
            // when a tree graph already exists for this name, to avoid creating
            // a spurious scalar variable card.
            if (val.isNull() && m_graphItems.contains(name + "_tree")) {
                continue;
            }
            syncVariable(name, val.toVariant());
        }
    }

    // 5. Stack
    if (step.contains("stack")) {
        syncStack(step["stack"].toArray());
    }
}

void ASTVisualizer::syncGraph(const QString& name, const QJsonObject& graphData, const QJsonObject& metadata, const QString& displayName) {
    if (!m_graphItems.contains(name)) {
        auto* item = new GraphItem(displayName.isEmpty() ? name : displayName);
        // Place graph below any arrays/variables to avoid overlap
        // Calculate vertical offset based on existing array items
        int graphY = 350;
        for (auto it = m_arrayItems.begin(); it != m_arrayItems.end(); ++it) {
            QRectF arrayBounds = it.value()->boundingRect();
            int arrayBottom = it.value()->pos().y() + arrayBounds.height() + 40;
            if (arrayBottom > graphY) graphY = arrayBottom;
        }
        item->setPos(550, graphY);
        m_scene->addItem(item);
        m_graphItems[name] = item;
    }
    if (m_aiMetadata.contains("theme")) {
        m_graphItems[name]->setTheme(m_aiMetadata["theme"].toObject());
    }
    m_graphItems[name]->updateData(graphData, metadata);
}

void ASTVisualizer::syncVariable(const QString& name, const QVariant& value) {
    if (m_arrayItems.contains(name)) return;
    if (!m_variableItems.contains(name)) {
        auto* item = new VariableItem(name);
        item->setPos(20, 50 + m_variableItems.size() * 50);
        m_scene->addItem(item);
        m_variableItems[name] = item;
    }
    m_variableItems[name]->updateValue(value.toString());
}


void ASTVisualizer::syncArray(const QString& name, const QJsonArray& listData, const QJsonObject& vars) {
    QVector<int> values;
    for (auto v : listData)
        values.append(v.toInt());

    // Create new array visualization if not exists
    if (!m_arrayItems.contains(name)) {
        auto* item = new ArrayItem(name);
        item->setMode(m_mode);

        //  Layout settings
        int baseX = 200;
        int baseY = 50;    // start higher
        int gapY  = 180;   // Vertical spacing
        int gapX  = 400;   // Horizontal spacing for left/right splits

        //  Fixed positions based on array name
        if (name == "arr") {
            item->setPos(baseX + 200, baseY); // Centered
        }
        else if (name == "left") {
            item->setPos(baseX, baseY + gapY); // Left branch
        }
        else if (name == "right") {
            item->setPos(baseX + gapX, baseY + gapY); // Right branch
        }
        else if (name == "result") {
            item->setPos(baseX + 200, baseY + 2 * gapY); // Re-merged
        }
        else if (name == "temp") {
            item->setPos(baseX + 200, baseY + 3 * gapY);
        }
        else {
            // Place generic arrays (visited, etc.) in a horizontal row
            // above the graph area, with enough clearance
            int index = m_arrayItems.size();
            int arrayX = 400;
            int arrayY = baseY + index * 80;
            item->setPos(arrayX, arrayY);
        }

        m_scene->addItem(item);
        m_arrayItems[name] = item;
    }

    if (m_aiMetadata.contains("theme")) {
        m_arrayItems[name]->setTheme(m_aiMetadata["theme"].toObject());
    }

    //  Update visualization data
    m_arrayItems[name]->updateStepContext(vars);
    m_arrayItems[name]->updateData(values);



}

void ASTVisualizer::syncStack(const QJsonArray& stackData) {
    for(auto* item : m_stackItems) {
        m_scene->removeItem(item);
        delete item;
    }
    m_stackItems.clear();
    int depth = 0;
    for (auto v : stackData) {
        auto* item = new CallStackItem(v.toString(), depth);
        item->setPos(850, 50 + depth * 35);
        m_scene->addItem(item);
        m_stackItems.append(item);
        depth++;
    }
}

