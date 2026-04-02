#ifndef GRAPHMODEL_H
#define GRAPHMODEL_H

#include <QAbstractItemModel>
#include <QSharedPointer>
#include "dotparser.h"

class GraphModel : public QAbstractItemModel {
    Q_OBJECT

public:
    explicit GraphModel(QObject* parent = nullptr);
    ~GraphModel();

    void setParser(QSharedPointer<DotParser> parser);
    void setRootNode(const QString& nodeId);
    void setNeighborLimit(int limit);
    int getNeighborLimit() const { return m_neighborLimit; }
    
    // Expand node to show neighbors
    void expandNode(const QString& nodeId);
    void collapseNode(const QString& nodeId);
    bool isExpanded(const QString& nodeId) const;

    // QAbstractItemModel interface
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& child) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

    // Get node ID from index
    QString nodeIdFromIndex(const QModelIndex& index) const;
    QModelIndex indexFromNodeId(const QString& nodeId) const;

signals:
    void nodeSelected(const QString& nodeId);

private:
    struct TreeItem {
        QString nodeId;
        TreeItem* parent = nullptr;
        QVector<TreeItem*> children;
        bool expanded = false;
        bool isNeighbor = false;  // True if this is a neighbor (not root)
        QString neighborType;     // "Successor" or "Predecessor"
    };

    TreeItem* getTreeItem(const QModelIndex& index) const;
    void clearTree();
    TreeItem* createRootItem();
    void addNeighbors(TreeItem* parentItem);
    QString generateLabel(const QString& nodeId) const;

    QSharedPointer<DotParser> m_parser;
    TreeItem* m_rootItem = nullptr;
    int m_neighborLimit = 20;
    QSet<QString> m_expandedNodes;
    QHash<QString, TreeItem*> m_nodeToItem;
};

#endif // GRAPHMODEL_H
