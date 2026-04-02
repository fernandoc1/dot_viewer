#include "graphmodel.h"
#include <QFont>

GraphModel::GraphModel(QObject* parent)
    : QAbstractItemModel(parent) {}

GraphModel::~GraphModel() {
    clearTree();
}

void GraphModel::setParser(QSharedPointer<DotParser> parser) {
    beginResetModel();
    m_parser = parser;
    clearTree();
    endResetModel();
}

void GraphModel::setRootNode(const QString& nodeId) {
    if (!m_parser || !m_parser->hasNode(nodeId)) {
        return;
    }
    
    beginResetModel();
    clearTree();
    m_rootItem = createRootItem();
    m_rootItem->nodeId = nodeId;
    m_nodeToItem[nodeId] = m_rootItem;
    endResetModel();
}

void GraphModel::setNeighborLimit(int limit) {
    m_neighborLimit = limit;
}

void GraphModel::expandNode(const QString& nodeId) {
    if (!m_parser || !m_nodeToItem.contains(nodeId)) {
        return;
    }
    
    TreeItem* item = m_nodeToItem[nodeId];
    if (item->expanded) {
        return;
    }
    
    item->expanded = true;
    m_expandedNodes.insert(nodeId);
    
    int pos = item->children.count();
    beginInsertRows(indexFromNodeId(nodeId), pos, pos + 1);
    addNeighbors(item);
    endInsertRows();
}

void GraphModel::collapseNode(const QString& nodeId) {
    if (!m_parser || !m_nodeToItem.contains(nodeId)) {
        return;
    }
    
    TreeItem* item = m_nodeToItem[nodeId];
    if (!item->expanded) {
        return;
    }
    
    // Remove all children
    if (!item->children.isEmpty()) {
        beginRemoveRows(indexFromNodeId(nodeId), 0, item->children.count() - 1);
        for (TreeItem* child : item->children) {
            // Recursively delete children
            QVector<TreeItem*> toDelete;
            toDelete.append(child);
            while (!toDelete.isEmpty()) {
                TreeItem* toDel = toDelete.takeFirst();
                for (TreeItem* grandChild : toDel->children) {
                    toDelete.append(grandChild);
                }
                delete toDel;
            }
        }
        item->children.clear();
        endRemoveRows();
    }
    
    item->expanded = false;
    m_expandedNodes.remove(nodeId);
}

bool GraphModel::isExpanded(const QString& nodeId) const {
    if (!m_nodeToItem.contains(nodeId)) {
        return false;
    }
    return m_nodeToItem[nodeId]->expanded;
}

void GraphModel::clearTree() {
    if (m_rootItem) {
        QVector<TreeItem*> toDelete;
        toDelete.append(m_rootItem);
        while (!toDelete.isEmpty()) {
            TreeItem* item = toDelete.takeFirst();
            for (TreeItem* child : item->children) {
                toDelete.append(child);
            }
            delete item;
        }
        m_rootItem = nullptr;
    }
    m_nodeToItem.clear();
    m_expandedNodes.clear();
}

GraphModel::TreeItem* GraphModel::createRootItem() {
    TreeItem* item = new TreeItem();
    return item;
}

void GraphModel::addNeighbors(TreeItem* parentItem) {
    if (!m_parser) return;
    
    const QString& nodeId = parentItem->nodeId;
    
    // Add successors
    QVector<QString> successors = m_parser->getSuccessors(nodeId, m_neighborLimit);
    for (const QString& succ : successors) {
        TreeItem* child = new TreeItem();
        child->nodeId = succ;
        child->parent = parentItem;
        child->isNeighbor = true;
        child->neighborType = "Successor";
        parentItem->children.append(child);
        m_nodeToItem[succ] = child;
    }
    
    // Add predecessors
    QVector<QString> predecessors = m_parser->getPredecessors(nodeId, m_neighborLimit);
    for (const QString& pred : predecessors) {
        TreeItem* child = new TreeItem();
        child->nodeId = pred;
        child->parent = parentItem;
        child->isNeighbor = true;
        child->neighborType = "Predecessor";
        parentItem->children.append(child);
        m_nodeToItem[pred] = child;
    }
}

QString GraphModel::generateLabel(const QString& nodeId) const {
    if (!m_parser || !m_parser->hasNode(nodeId)) {
        return nodeId;
    }
    
    auto node = m_parser->getNode(nodeId);
    // Format: [Address] Instruction (count)
    QString addr = node->label.split('\n')[0];
    if (addr.contains('[')) {
        addr = addr.split('[')[0].trimmed();
    }
    
    return QString("[%1] %2 (count=%3)")
        .arg(addr)
        .arg(node->shortLabel)
        .arg(node->count);
}

QModelIndex GraphModel::index(int row, int column, const QModelIndex& parent) const {
    if (!hasIndex(row, column, parent)) {
        return QModelIndex();
    }
    
    TreeItem* parentItem = getTreeItem(parent);
    if (row < 0 || row >= parentItem->children.count()) {
        return QModelIndex();
    }
    
    TreeItem* childItem = parentItem->children[row];
    return createIndex(row, column, childItem);
}

QModelIndex GraphModel::parent(const QModelIndex& child) const {
    if (!child.isValid()) {
        return QModelIndex();
    }
    
    TreeItem* childItem = getTreeItem(child);
    TreeItem* parentItem = childItem->parent;
    
    if (!parentItem || parentItem == m_rootItem) {
        return QModelIndex();
    }
    
    return createIndex(0, 0, parentItem);
}

int GraphModel::rowCount(const QModelIndex& parent) const {
    TreeItem* parentItem = getTreeItem(parent);
    return parentItem->children.count();
}

int GraphModel::columnCount(const QModelIndex& parent) const {
    Q_UNUSED(parent);
    return 2;  // Node info and neighbor type
}

QVariant GraphModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid()) {
        return QVariant();
    }
    
    TreeItem* item = getTreeItem(index);
    const QString& nodeId = item->nodeId;
    
    if (role == Qt::DisplayRole) {
        if (index.column() == 0) {
            QString label = generateLabel(nodeId);
            if (item->isNeighbor) {
                return QString("%1 ← %2").arg(label, item->neighborType);
            }
            return label;
        } else if (index.column() == 1) {
            if (!m_parser) return QVariant();
            auto node = m_parser->getNode(nodeId);
            if (node) {
                return QString("Succ: %1, Pred: %2")
                    .arg(node->successors.size())
                    .arg(node->predecessors.size());
            }
        }
    } else if (role == Qt::FontRole) {
        QFont font;
        if (!item->isNeighbor) {
            font.setBold(true);
        }
        return font;
    } else if (role == Qt::UserRole) {
        return nodeId;
    }
    
    return QVariant();
}

QVariant GraphModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        if (section == 0) {
            return "Node";
        } else if (section == 1) {
            return "Connections";
        }
    }
    return QVariant();
}

Qt::ItemFlags GraphModel::flags(const QModelIndex& index) const {
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QString GraphModel::nodeIdFromIndex(const QModelIndex& index) const {
    if (!index.isValid()) {
        return QString();
    }
    TreeItem* item = getTreeItem(index);
    return item->nodeId;
}

QModelIndex GraphModel::indexFromNodeId(const QString& nodeId) const {
    if (!m_nodeToItem.contains(nodeId)) {
        return QModelIndex();
    }
    TreeItem* item = m_nodeToItem[nodeId];
    if (!item->parent) {
        return createIndex(0, 0, item);
    }
    int row = item->parent->children.indexOf(item);
    return createIndex(row, 0, item);
}

GraphModel::TreeItem* GraphModel::getTreeItem(const QModelIndex& index) const {
    if (!index.isValid()) {
        return m_rootItem;
    }
    return static_cast<TreeItem*>(index.internalPointer());
}
