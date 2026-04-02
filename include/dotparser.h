#ifndef DOTPARSER_H
#define DOTPARSER_H

#include <QString>
#include <QMap>
#include <QSet>
#include <QVector>
#include <QSharedPointer>

struct Node {
    QString id;
    QString label;
    QString shortLabel;  // Extracted address/function info
    int count = 0;       // Execution count from label
    QSet<QString> successors;  // Outgoing edges
    QSet<QString> predecessors;  // Incoming edges
};

class DotParser {
public:
    DotParser();
    
    bool parseFile(const QString& filePath);
    bool parseContent(const QString& content);
    
    const QMap<QString, QSharedPointer<Node>>& getNodes() const { return m_nodes; }
    int getNodeCount() const { return m_nodes.size(); }
    int getEdgeCount() const { return m_edgeCount; }
    QString getGraphName() const { return m_graphName; }
    
    // Get neighbors with limit
    QVector<QString> getSuccessors(const QString& nodeId, int limit = 10) const;
    QVector<QString> getPredecessors(const QString& nodeId, int limit = 10) const;
    
    // Search nodes by label
    QVector<QString> searchNodes(const QString& pattern, int limit = 100) const;
    
    // Check if node exists
    bool hasNode(const QString& nodeId) const;
    
    // Get node
    QSharedPointer<Node> getNode(const QString& nodeId) const;

private:
    QMap<QString, QSharedPointer<Node>> m_nodes;
    int m_edgeCount = 0;
    QString m_graphName;
    
    QString extractAddress(const QString& label) const;
    int extractCount(const QString& label) const;
    QString extractShortLabel(const QString& label) const;
};

#endif // DOTPARSER_H
