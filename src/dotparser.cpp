#include "dotparser.h"
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>
#include <QFileInfo>

DotParser::DotParser() {}

bool DotParser::parseFile(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }
    
    // Memory-mapped reading for large files
    uchar* mappedData = file.map(0, file.size());
    QString content;
    if (mappedData == nullptr) {
        // Fallback for very large files or if mmap fails
        QTextStream in(&file);
        content = in.readAll();
    } else {
        content = QString::fromUtf8(reinterpret_cast<const char*>(mappedData), file.size());
    }
    file.close();
    
    return parseContent(content);
}

bool DotParser::parseContent(const QString& content) {
    m_nodes.clear();
    m_edgeCount = 0;
    
    // Extract graph name
    QRegularExpression graphNameRegex(R"(digraph\s+(\w+))");
    QRegularExpressionMatch match = graphNameRegex.match(content);
    if (match.hasMatch()) {
        m_graphName = match.captured(1);
    }
    
    // Parse node definitions: n0 [label="..."];
    QRegularExpression nodeRegex(R"(n(\d+)\s*\[label=\"([^\"]+)\"\])");
    QRegularExpressionMatchIterator nodeIt = nodeRegex.globalMatch(content);
    
    while (nodeIt.hasNext()) {
        QRegularExpressionMatch nodeMatch = nodeIt.next();
        QString id = "n" + nodeMatch.captured(1);
        QString label = nodeMatch.captured(2);
        
        auto node = QSharedPointer<Node>::create();
        node->id = id;
        node->label = label;
        node->shortLabel = extractShortLabel(label);
        node->count = extractCount(label);
        
        m_nodes[id] = node;
    }
    
    // Parse edges: n0 -> n1; or n0 -> n1 [label="..."];
    QRegularExpression edgeRegex(R"(n(\d+)\s*->\s*n(\d+))");
    QRegularExpressionMatchIterator edgeIt = edgeRegex.globalMatch(content);
    
    while (edgeIt.hasNext()) {
        QRegularExpressionMatch edgeMatch = edgeIt.next();
        QString fromId = "n" + edgeMatch.captured(1);
        QString toId = "n" + edgeMatch.captured(2);
        
        if (m_nodes.contains(fromId) && m_nodes.contains(toId)) {
            m_nodes[fromId]->successors.insert(toId);
            m_nodes[toId]->predecessors.insert(fromId);
            m_edgeCount++;
        }
    }
    
    return true;
}

QString DotParser::extractAddress(const QString& label) const {
    // Extract address from label like "00ff00 [file@00ff00]\nsei\ncount=1"
    QRegularExpression addrRegex(R"(([0-9a-fA-F]{6})\s*\[)");
    QRegularExpressionMatch match = addrRegex.match(label);
    if (match.hasMatch()) {
        return match.captured(1);
    }
    return QString();
}

int DotParser::extractCount(const QString& label) const {
    // Extract count from label
    QRegularExpression countRegex(R"(count=(\d+))");
    QRegularExpressionMatch match = countRegex.match(label);
    if (match.hasMatch()) {
        return match.captured(1).toInt();
    }
    return 0;
}

QString DotParser::extractShortLabel(const QString& label) const {
    // Extract instruction from label (second line)
    QStringList lines = label.split('\n');
    if (lines.size() >= 2) {
        return lines[1];  // The instruction (e.g., "sei", "clc")
    }
    return label;
}

QVector<QString> DotParser::getSuccessors(const QString& nodeId, int limit) const {
    QVector<QString> result;
    if (!m_nodes.contains(nodeId)) {
        return result;
    }
    
    const QSet<QString>& succs = m_nodes[nodeId]->successors;
    int count = 0;
    for (const QString& succ : succs) {
        if (limit > 0 && count >= limit) break;
        result.append(succ);
        count++;
    }
    return result;
}

QVector<QString> DotParser::getPredecessors(const QString& nodeId, int limit) const {
    QVector<QString> result;
    if (!m_nodes.contains(nodeId)) {
        return result;
    }
    
    const QSet<QString>& preds = m_nodes[nodeId]->predecessors;
    int count = 0;
    for (const QString& pred : preds) {
        if (limit > 0 && count >= limit) break;
        result.append(pred);
        count++;
    }
    return result;
}

QVector<QString> DotParser::searchNodes(const QString& pattern, int limit) const {
    QVector<QString> result;
    if (pattern.isEmpty()) {
        return result;
    }
    
    QString searchPattern = pattern.toLower();
    int count = 0;
    
    for (auto it = m_nodes.begin(); it != m_nodes.end() && count < limit; ++it) {
        const auto& node = it.value();
        if (node->id.contains(searchPattern, Qt::CaseInsensitive) ||
            node->label.contains(searchPattern, Qt::CaseInsensitive) ||
            node->shortLabel.contains(searchPattern, Qt::CaseInsensitive)) {
            result.append(node->id);
            count++;
        }
    }
    
    return result;
}

bool DotParser::hasNode(const QString& nodeId) const {
    return m_nodes.contains(nodeId);
}

QSharedPointer<Node> DotParser::getNode(const QString& nodeId) const {
    return m_nodes.value(nodeId);
}
