#ifndef GRAPHVIEW_H
#define GRAPHVIEW_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QPainter>
#include <QStyleOption>
#include <QScrollBar>
#include <QSharedPointer>
#include <QMap>
#include "dotparser.h"

class GraphView;

class NodeItem : public QGraphicsItem {
public:
    NodeItem(const QString& nodeId, const QString& label, const QString& shortLabel, int count, GraphView* view);
    
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    
    QString nodeId() const { return m_nodeId; }
    void setHighlighted(bool highlighted);
    bool isHighlighted() const { return m_highlighted; }
    
    static const int NODE_WIDTH = 180;
    static const int NODE_HEIGHT = 60;

protected:
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) override;
    void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;

private:
    QString m_nodeId;
    QString m_label;
    QString m_shortLabel;
    int m_count;
    bool m_highlighted = false;
    bool m_hovered = false;
    GraphView* m_view;
};

class EdgeItem : public QGraphicsItem {
public:
    EdgeItem(NodeItem* from, NodeItem* to, bool isBackEdge = false);
    
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

private:
    NodeItem* m_from;
    NodeItem* m_to;
    bool m_isBackEdge;
};

class GraphView : public QGraphicsView {
    Q_OBJECT

public:
    explicit GraphView(QWidget* parent = nullptr);
    ~GraphView();

    void setParser(QSharedPointer<DotParser> parser);
    void displayNodeAsTree(const QString& rootNodeId, int maxDepth = 3);
    void clearDisplay();
    
    void setMaxDepth(int depth) { m_maxDepth = depth; }
    int maxDepth() const { return m_maxDepth; }
    
    void setMaxNodes(int maxNodes) { m_maxNodes = maxNodes; }
    int maxNodes() const { return m_maxNodes; }
    
    NodeItem* nodeItem(const QString& nodeId) const { return m_nodeItems.value(nodeId); }

signals:
    void nodeClicked(const QString& nodeId);
    void nodeDoubleClicked(const QString& nodeId);
    void displayFinished(int nodeCount, int edgeCount);

protected:
    void wheelEvent(QWheelEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

private:
    void setupScene();
    void addNodeToTree(const QString& nodeId, qreal x, qreal y, int depth, QSet<QString>& visited);
    void addEdge(const QString& fromId, const QString& toId, bool isBackEdge);
    void layoutTree(const QString& nodeId, qreal x, qreal y, int depth, int siblingIndex, int siblingCount, QSet<QString>& visited);
    void updateNodePositions();
    void countNodes(const QString& nodeId, int depth, QSet<QString>& visited, int& count);
    
    QSharedPointer<DotParser> m_parser;
    QGraphicsScene* m_scene;
    QMap<QString, NodeItem*> m_nodeItems;
    QList<EdgeItem*> m_edgeItems;
    
    int m_maxDepth = 50;
    int m_maxNodes = 20000;  // Limit total nodes displayed for performance
    qreal m_currentSpacing = 200.0;
    bool m_panning = false;
    QPoint m_lastPanPoint;
    
    static constexpr qreal LEVEL_HEIGHT = 80.0;
    static constexpr qreal MIN_NODE_SPACING = 200.0;
};

#endif // GRAPHVIEW_H
