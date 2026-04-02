#include "graphview.h"
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QApplication>
#include <QCursor>
#include <cmath>

// ============================================================================
// NodeItem Implementation
// ============================================================================

NodeItem::NodeItem(const QString& nodeId, const QString& label, 
                   const QString& shortLabel, int count, GraphView* view)
    : m_nodeId(nodeId)
    , m_label(label)
    , m_shortLabel(shortLabel)
    , m_count(count)
    , m_view(view)
{
    setFlag(ItemIsSelectable, true);
    setFlag(ItemIsMovable, false);
    setFlag(ItemSendsGeometryChanges, true);
    setAcceptHoverEvents(true);
    setCursor(Qt::PointingHandCursor);
}

QRectF NodeItem::boundingRect() const {
    return QRectF(0, 0, NODE_WIDTH, NODE_HEIGHT);
}

void NodeItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    Q_UNUSED(widget);
    
    painter->setRenderHint(QPainter::Antialiasing, true);
    
    // Colors based on state
    QColor bgColor, borderColor, textColor;
    
    if (m_highlighted) {
        bgColor = QColor(255, 200, 100);  // Orange for highlighted
        borderColor = QColor(200, 100, 0);
    } else if (m_hovered) {
        bgColor = QColor(200, 230, 255);  // Light blue for hover
        borderColor = QColor(100, 150, 200);
    } else {
        bgColor = QColor(240, 240, 240);  // Light gray default
        borderColor = QColor(100, 100, 100);
    }
    
    textColor = QColor(0, 0, 0);
    
    // Draw shadow
    painter->setBrush(QColor(0, 0, 0, 30));
    painter->setPen(Qt::NoPen);
    painter->drawRoundedRect(3, 3, NODE_WIDTH, NODE_HEIGHT, 8, 8);
    
    // Draw node box
    painter->setBrush(bgColor);
    painter->setPen(QPen(borderColor, m_highlighted ? 3 : 2));
    painter->drawRoundedRect(0, 0, NODE_WIDTH, NODE_HEIGHT, 8, 8);
    
    // Draw text
    painter->setPen(textColor);
    QFont font = painter->font();
    
    // Address (first line)
    font.setBold(true);
    font.setPointSize(10);
    painter->setFont(font);
    QString addrText = m_label.split("\\n")[0];
    if (addrText.contains('[')) {
        addrText = addrText.split('[')[0].trimmed();
    }
    painter->drawText(8, 18, NODE_WIDTH - 16, 16, Qt::AlignLeft, addrText);
    
    // Instruction (second line)
    font.setBold(false);
    font.setPointSize(9);
    painter->setFont(font);
    painter->drawText(8, 36, NODE_WIDTH - 16, 14, Qt::AlignLeft, m_shortLabel);
    
    // Count (third line)
    font.setPointSize(8);
    painter->setFont(font);
    painter->drawText(8, NODE_HEIGHT - 8, NODE_WIDTH - 16, 14, Qt::AlignLeft, 
                      QString("count=%1").arg(m_count));
    
    // Selection indicator
    if (option->state & QStyle::State_Selected) {
        painter->setPen(QPen(QColor(0, 100, 255), 3));
        painter->setBrush(Qt::NoBrush);
        painter->drawRoundedRect(0, 0, NODE_WIDTH, NODE_HEIGHT, 8, 8);
    }
}

void NodeItem::setHighlighted(bool highlighted) {
    m_highlighted = highlighted;
    update();
}

void NodeItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) {
    emit m_view->nodeDoubleClicked(m_nodeId);
    QGraphicsItem::mouseDoubleClickEvent(event);
}

void NodeItem::hoverEnterEvent(QGraphicsSceneHoverEvent* event) {
    m_hovered = true;
    update();
    QGraphicsItem::hoverEnterEvent(event);
}

void NodeItem::hoverLeaveEvent(QGraphicsSceneHoverEvent* event) {
    m_hovered = false;
    update();
    QGraphicsItem::hoverLeaveEvent(event);
}

// ============================================================================
// EdgeItem Implementation
// ============================================================================

EdgeItem::EdgeItem(NodeItem* from, NodeItem* to, bool isBackEdge)
    : m_from(from)
    , m_to(to)
    , m_isBackEdge(isBackEdge)
{
    setFlag(ItemIsSelectable, false);
    setZValue(-1);  // Edges behind nodes
}

QRectF EdgeItem::boundingRect() const {
    if (!m_from || !m_to) {
        return QRectF();
    }
    
    QPointF fromPos = m_from->scenePos() + QPointF(NodeItem::NODE_WIDTH / 2, NodeItem::NODE_HEIGHT);
    QPointF toPos = m_to->scenePos() + QPointF(NodeItem::NODE_WIDTH / 2, 0);
    
    // Add padding for arrow head
    qreal padding = 10;
    return QRectF(fromPos, toPos).normalized().adjusted(-padding, -padding, padding, padding);
}

void EdgeItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    Q_UNUSED(option);
    Q_UNUSED(widget);
    
    if (!m_from || !m_to) {
        return;
    }
    
    painter->setRenderHint(QPainter::Antialiasing, true);
    
    QPointF fromPos = m_from->scenePos() + QPointF(NodeItem::NODE_WIDTH / 2, NodeItem::NODE_HEIGHT);
    QPointF toPos = m_to->scenePos() + QPointF(NodeItem::NODE_WIDTH / 2, 0);
    
    // Color based on edge type
    QColor edgeColor = m_isBackEdge ? QColor(200, 100, 100) : QColor(100, 100, 100);
    
    // Draw line
    QPen pen(edgeColor, m_isBackEdge ? 2 : 1, m_isBackEdge ? Qt::DashLine : Qt::SolidLine);
    painter->setPen(pen);
    painter->drawLine(fromPos, toPos);
    
    // Draw arrow head
    double angle = std::atan2(toPos.y() - fromPos.y(), toPos.x() - fromPos.x());
    double arrowAngle = M_PI / 6;  // 30 degrees
    double arrowLength = 10;
    
    QPointF arrowPoint1(
        toPos.x() - arrowLength * std::cos(angle - arrowAngle),
        toPos.y() - arrowLength * std::sin(angle - arrowAngle)
    );
    QPointF arrowPoint2(
        toPos.x() - arrowLength * std::cos(angle + arrowAngle),
        toPos.y() - arrowLength * std::sin(angle + arrowAngle)
    );
    
    painter->setBrush(edgeColor);
    painter->drawPolygon(QPolygonF() << toPos << arrowPoint1 << arrowPoint2);
}

// ============================================================================
// GraphView Implementation
// ============================================================================

GraphView::GraphView(QWidget* parent)
    : QGraphicsView(parent)
    , m_scene(new QGraphicsScene(this))
{
    setScene(m_scene);
    setupScene();
    
    // Enable interactions
    setRenderHint(QPainter::Antialiasing, true);
    setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setResizeAnchor(QGraphicsView::AnchorUnderMouse);
    setDragMode(QGraphicsView::ScrollHandDrag);
    
    // Background
    m_scene->setBackgroundBrush(QColor(250, 250, 250));
    
    // Connect signals
    connect(this, &GraphView::nodeDoubleClicked, this, [this](const QString& nodeId) {
        // Emit to parent for handling expansion
        emit nodeDoubleClicked(nodeId);
    });
}

GraphView::~GraphView() {
    clearDisplay();
    delete m_scene;
}

void GraphView::setupScene() {
    m_scene->clear();
    qDeleteAll(m_edgeItems);
    m_edgeItems.clear();
    m_nodeItems.clear();
}

void GraphView::setParser(QSharedPointer<DotParser> parser) {
    m_parser = parser;
}

void GraphView::clearDisplay() {
    setupScene();
}

void GraphView::displayNodeAsTree(const QString& rootNodeId, int maxDepth) {
    if (!m_parser || !m_parser->hasNode(rootNodeId)) {
        return;
    }

    clearDisplay();
    m_maxDepth = maxDepth;

    // First pass: count nodes to determine optimal spacing
    QSet<QString> tempVisited;
    int estimatedNodes = 0;
    countNodes(rootNodeId, 0, tempVisited, estimatedNodes);
    
    // Adaptive spacing based on node count
    qreal adaptiveSpacing = qMax(50.0, qMin(200.0, 100000.0 / qMax(1, estimatedNodes)));
    qreal sceneWidth = estimatedNodes * adaptiveSpacing * 1.5;
    qreal sceneHeight = (maxDepth + 2) * LEVEL_HEIGHT + 100;

    m_scene->setSceneRect(-sceneWidth / 2, -50, sceneWidth, sceneHeight);

    // Second pass: layout with adaptive spacing
    QSet<QString> visited;
    m_currentSpacing = adaptiveSpacing;
    layoutTree(rootNodeId, 0, 0, 0, 0, 1, visited);

    // Fit view to scene
    fitInView(m_scene->sceneRect(), Qt::KeepAspectRatio);
    
    // Emit display finished signal with counts
    emit displayFinished(m_nodeItems.size(), m_edgeItems.size());
}

void GraphView::countNodes(const QString& nodeId, int depth, QSet<QString>& visited, int& count) {
    if (count >= m_maxNodes || depth > m_maxDepth || !m_parser || !m_parser->hasNode(nodeId)) {
        return;
    }
    if (visited.contains(nodeId)) {
        return;
    }
    visited.insert(nodeId);
    count++;
    
    auto successors = m_parser->getSuccessors(nodeId, 50);
    for (const QString& childId : successors) {
        countNodes(childId, depth + 1, visited, count);
    }
}

void GraphView::layoutTree(const QString& nodeId, qreal x, qreal y, int depth,
                           int siblingIndex, int siblingCount, QSet<QString>& visited) {
    // Check node limit
    if (m_nodeItems.size() >= m_maxNodes) {
        return;
    }
    
    if (!m_parser || !m_parser->hasNode(nodeId) || depth > m_maxDepth) {
        return;
    }

    // Check for cycles
    if (visited.contains(nodeId)) {
        return;
    }
    visited.insert(nodeId);

    auto node = m_parser->getNode(nodeId);

    // Create node item
    QString displayLabel = node->label.split("\\n")[0];
    NodeItem* nodeItem = new NodeItem(nodeId, displayLabel, node->shortLabel, node->count, this);
    nodeItem->setPos(x - NodeItem::NODE_WIDTH / 2, y);
    nodeItem->setZValue(1);
    m_scene->addItem(nodeItem);
    m_nodeItems[nodeId] = nodeItem;

    // Get children (successors)
    auto successors = m_parser->getSuccessors(nodeId, 50);  // Limit children displayed
    
    if (successors.isEmpty() || depth >= m_maxDepth) {
        return;
    }
    
    // Calculate positions for children
    int childCount = successors.size();
    qreal totalWidth = childCount * m_currentSpacing;
    qreal startX = x - totalWidth / 2 + m_currentSpacing / 2;

    for (int i = 0; i < childCount; ++i) {
        const QString& childId = successors[i];
        bool isBackEdge = visited.contains(childId);

        // Create child node
        qreal childX = startX + i * m_currentSpacing;
        qreal childY = y + LEVEL_HEIGHT;

        layoutTree(childId, childX, childY, depth + 1, i, childCount, visited);

        // Create edge
        if (m_nodeItems.contains(childId)) {
            addEdge(nodeId, childId, isBackEdge);
        }
    }
}

void GraphView::addEdge(const QString& fromId, const QString& toId, bool isBackEdge) {
    if (!m_nodeItems.contains(fromId) || !m_nodeItems.contains(toId)) {
        return;
    }
    
    EdgeItem* edge = new EdgeItem(m_nodeItems[fromId], m_nodeItems[toId], isBackEdge);
    m_scene->addItem(edge);
    m_edgeItems.append(edge);
}

void GraphView::wheelEvent(QWheelEvent* event) {
    // Zoom with wheel
    if (event->modifiers() & Qt::ControlModifier) {
        qreal factor = 1.1;
        if (event->angleDelta().y() > 0) {
            scale(factor, factor);
        } else {
            scale(1.0 / factor, 1.0 / factor);
        }
        event->accept();
    } else {
        QGraphicsView::wheelEvent(event);
    }
}

void GraphView::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::MiddleButton) {
        m_panning = true;
        m_lastPanPoint = event->pos();
        setCursor(Qt::ClosedHandCursor);
        event->accept();
    } else {
        QGraphicsView::mousePressEvent(event);
    }
}

void GraphView::mouseMoveEvent(QMouseEvent* event) {
    if (m_panning) {
        QPointF delta = mapToScene(event->pos()) - mapToScene(m_lastPanPoint);
        m_lastPanPoint = event->pos();
        horizontalScrollBar()->setValue(horizontalScrollBar()->value() - delta.x());
        verticalScrollBar()->setValue(verticalScrollBar()->value() - delta.y());
        event->accept();
    } else {
        QGraphicsView::mouseMoveEvent(event);
    }
}

void GraphView::keyPressEvent(QKeyEvent* event) {
    switch (event->key()) {
        case Qt::Key_Plus:
        case Qt::Key_Equal:
            scale(1.2, 1.2);
            break;
        case Qt::Key_Minus:
            scale(1.0 / 1.2, 1.0 / 1.2);
            break;
        case Qt::Key_0:
            fitInView(m_scene->sceneRect(), Qt::KeepAspectRatio);
            break;
        case Qt::Key_Home:
            centerOn(0, 0);
            break;
        default:
            QGraphicsView::keyPressEvent(event);
    }
}
