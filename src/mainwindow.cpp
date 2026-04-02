#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QGroupBox>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QFileDialog>
#include <QMessageBox>
#include <QApplication>
#include <QCursor>
#include <QTimer>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_parser(QSharedPointer<DotParser>::create())
    , m_model(new GraphModel(this))
{
    setWindowTitle("DOT Graph Viewer - Execution Flow Analyzer");
    resize(1200, 800);

    setupUi();
    setupMenuBar();
    setupToolBar();
}

MainWindow::MainWindow(const QString& fileName, QWidget* parent)
    : QMainWindow(parent)
    , m_parser(QSharedPointer<DotParser>::create())
    , m_model(new GraphModel(this))
{
    setWindowTitle("DOT Graph Viewer - Execution Flow Analyzer");
    resize(1200, 800);

    setupUi();
    setupMenuBar();
    setupToolBar();
    
    // Load file after UI is set up
    QTimer::singleShot(100, [this, fileName]() {
        loadFileInternal(fileName);
    });
}

MainWindow::~MainWindow() {}

void MainWindow::setupUi() {
    m_centralWidget = new QWidget(this);
    setCentralWidget(m_centralWidget);
    
    auto* mainLayout = new QVBoxLayout(m_centralWidget);
    
    // Top toolbar with search and load
    auto* topBar = new QHBoxLayout();
    
    // Load button
    auto* loadBtn = new QPushButton("Load DOT File");
    connect(loadBtn, &QPushButton::clicked, this, &MainWindow::loadFile);
    topBar->addWidget(loadBtn);
    
    // Search box
    topBar->addWidget(new QLabel("Search:"));
    m_searchEdit = new QLineEdit();
    m_searchEdit->setPlaceholderText("Search by address, instruction, or node ID...");
    m_searchEdit->setMinimumWidth(300);
    connect(m_searchEdit, &QLineEdit::returnPressed, this, &MainWindow::searchGraph);
    connect(m_searchEdit, &QLineEdit::textChanged, this, &MainWindow::onSearchTextChanged);
    topBar->addWidget(m_searchEdit);
    
    m_searchBtn = new QPushButton("Search");
    connect(m_searchBtn, &QPushButton::clicked, this, &MainWindow::searchGraph);
    topBar->addWidget(m_searchBtn);

    // Neighbor limit
    topBar->addWidget(new QLabel("Neighbors:"));
    m_neighborLimitSpin = new QSpinBox();
    m_neighborLimitSpin->setRange(1, 1000);
    m_neighborLimitSpin->setValue(20);
    m_neighborLimitSpin->setToolTip("Maximum number of neighbors to display per node");
    connect(m_neighborLimitSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &MainWindow::onNeighborLimitChanged);
    topBar->addWidget(m_neighborLimitSpin);

    // Tree depth limit
    topBar->addWidget(new QLabel("Tree depth:"));
    m_depthSpin = new QSpinBox();
    m_depthSpin->setRange(1, 50);
    m_depthSpin->setValue(50);
    m_depthSpin->setToolTip("Maximum depth for tree visualization");
    connect(m_depthSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &MainWindow::onDepthChanged);
    topBar->addWidget(m_depthSpin);

    topBar->addStretch();
    
    // Graph info label
    m_graphInfoLabel = new QLabel("No file loaded");
    m_graphInfoLabel->setStyleSheet("font-weight: bold;");
    topBar->addWidget(m_graphInfoLabel);
    
    mainLayout->addLayout(topBar);

    // Tab widget for tree and graph views
    m_tabWidget = new QTabWidget();
    
    // Tab 1: Tree View
    auto* treeWidget = new QWidget();
    auto* treeLayout = new QVBoxLayout(treeWidget);
    treeLayout->setContentsMargins(0, 0, 0, 0);
    
    m_treeView = new QTreeView();
    m_treeView->setModel(m_model);
    m_treeView->setHeaderHidden(false);
    m_treeView->setAlternatingRowColors(true);
    m_treeView->setAnimated(true);
    m_treeView->setIndentation(20);
    m_treeView->setSortingEnabled(false);
    m_treeView->setColumnWidth(0, 500);
    m_treeView->setColumnWidth(1, 150);
    m_treeView->setContextMenuPolicy(Qt::CustomContextMenu);
    
    connect(m_treeView, &QTreeView::clicked, this, &MainWindow::onNodeSelected);
    connect(m_treeView, &QTreeView::doubleClicked, this, &MainWindow::onNodeDoubleClicked);
    
    treeLayout->addWidget(m_treeView);
    m_tabWidget->addTab(treeWidget, "Tree View");

    // Tab 2: Graph Visualization
    m_graphView = new GraphView();
    connect(m_graphView, &GraphView::nodeDoubleClicked, this, &MainWindow::onGraphNodeDoubleClicked);
    connect(m_graphView, &GraphView::displayFinished, this, [this](int nodeCount, int edgeCount) {
        m_statusLabel->setText(QString("Displaying %1 nodes, %2 edges").arg(nodeCount).arg(edgeCount));
    });
    m_tabWidget->addTab(m_graphView, "Graph Visualization");
    
    // Set Graph Visualization as the first/active tab
    m_tabWidget->setCurrentIndex(1);

    mainLayout->addWidget(m_tabWidget);

    // Bottom panel: Search results
    auto* searchGroup = new QGroupBox("Search Results");
    auto* searchLayout = new QVBoxLayout(searchGroup);
    
    m_searchResults = new QListWidget();
    m_searchResults->setAlternatingRowColors(true);
    connect(m_searchResults, &QListWidget::itemClicked,
            this, &MainWindow::onSearchResultClicked);
    searchLayout->addWidget(m_searchResults);
    mainLayout->addWidget(searchGroup);
    searchGroup->setMaximumHeight(200);

    // Side panel: Node details
    auto* detailsGroup = new QGroupBox("Node Details");
    auto* detailsLayout = new QVBoxLayout(detailsGroup);

    m_nodeDetails = new QTextEdit();
    m_nodeDetails->setReadOnly(true);
    m_nodeDetails->setFont(QFont("Courier", 10));
    m_nodeDetails->setPlaceholderText("Select a node to see details...");
    detailsLayout->addWidget(m_nodeDetails);

    mainLayout->addWidget(detailsGroup);
    detailsGroup->setMaximumWidth(350);

    // Status bar
    m_statusLabel = new QLabel("Ready");
    statusBar()->addWidget(m_statusLabel);

    updateStatus();
}

void MainWindow::setupMenuBar() {
    auto* fileMenu = menuBar()->addMenu("&File");
    
    auto* openAction = fileMenu->addAction("&Open DOT File...");
    openAction->setShortcut(QKeySequence::Open);
    connect(openAction, &QAction::triggered, this, &MainWindow::loadFile);
    
    fileMenu->addSeparator();
    
    auto* exitAction = fileMenu->addAction("E&xit");
    exitAction->setShortcut(QKeySequence::Quit);
    connect(exitAction, &QAction::triggered, qApp, &QApplication::quit);
    
    auto* helpMenu = menuBar()->addMenu("&Help");
    
    auto* aboutAction = helpMenu->addAction("&About");
    connect(aboutAction, &QAction::triggered, [this]() {
        QMessageBox::about(this, "About DOT Graph Viewer",
            "DOT Graph Viewer - Execution Flow Analyzer\n\n"
            "A Qt6 application for visualizing large DOT graph files.\n"
            "Designed for analyzing program execution flow graphs.\n\n"
            "Features:\n"
            "- Memory-efficient parsing of large DOT files\n"
            "- Tree-based navigation with expandable nodes\n"
            "- Search functionality for finding nodes\n"
            "- Configurable neighbor display limit");
    });
}

void MainWindow::setupToolBar() {
    // Toolbar can be added here if needed
}

void MainWindow::loadFile() {
    QString fileName = QFileDialog::getOpenFileName(
        this, "Open DOT File", QString(), "DOT Files (*.dot);;All Files (*)");

    if (fileName.isEmpty()) {
        return;
    }
    
    loadFileInternal(fileName);
}

void MainWindow::loadFileInternal(const QString& fileName) {
    m_statusLabel->setText("Loading file...");
    m_statusLabel->setStyleSheet("color: blue;");
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    QApplication::processEvents();

    // Parse in a separate step to allow UI update
    QTimer::singleShot(100, [this, fileName]() {
        bool success = m_parser->parseFile(fileName);

        QApplication::restoreOverrideCursor();

        if (!success) {
            QMessageBox::critical(this, "Error", "Failed to load DOT file: " + fileName);
            m_statusLabel->setText("Failed to load file");
            m_statusLabel->setStyleSheet("color: red;");
            return;
        }

        m_model->setParser(m_parser);
        m_model->setNeighborLimit(m_neighborLimitSpin->value());
        
        m_graphView->setParser(m_parser);
        m_graphView->setMaxDepth(m_depthSpin->value());
        m_graphView->setMaxNodes(20000);  // Show all nodes (graph has ~16K nodes)

        m_graphInfoLabel->setText(
            QString("Graph: %1 | Nodes: %2 | Edges: %3")
                .arg(m_parser->getGraphName())
                .arg(m_parser->getNodeCount())
                .arg(m_parser->getEdgeCount()));

        m_statusLabel->setText("File loaded successfully - displaying graph...");
        m_statusLabel->setStyleSheet("color: blue;");
        QApplication::processEvents();

        // Set first node as root and display entire graph
        auto nodes = m_parser->getNodes();
        if (!nodes.isEmpty()) {
            QString firstNode = nodes.firstKey();
            m_model->setRootNode(firstNode);
            // Display entire graph with max depth
            m_graphView->displayNodeAsTree(firstNode, 50);
            showNodeDetails(firstNode);
        }

        m_statusLabel->setText("File loaded successfully");
        m_statusLabel->setStyleSheet("color: green;");
        updateStatus();
    });
}

void MainWindow::searchGraph() {
    QString pattern = m_searchEdit->text().trimmed();
    if (pattern.isEmpty() || !m_parser) {
        return;
    }
    
    m_searchResults->clear();
    m_statusLabel->setText("Searching...");
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    QApplication::processEvents();
    
    QTimer::singleShot(50, [this, pattern]() {
        auto results = m_parser->searchNodes(pattern, 100);
        
        QApplication::restoreOverrideCursor();
        
        for (const QString& nodeId : results) {
            auto node = m_parser->getNode(nodeId);
            if (node) {
                // Extract address from label (first part before [)
                QString firstLine = node->label.split("\\n")[0];
                QString addr = firstLine.split('[')[0].trimmed();
                
                QString display = QString("[%1] %2 (count=%3)")
                    .arg(addr)
                    .arg(node->shortLabel)
                    .arg(node->count);
                auto* item = new QListWidgetItem(display);
                item->setData(Qt::UserRole, nodeId);
                m_searchResults->addItem(item);
            }
        }
        
        m_statusLabel->setText(QString("Found %1 node(s)").arg(results.size()));
    });
}

void MainWindow::onSearchTextChanged(const QString& text) {
    // Auto-search on text change with debounce could be added here
    Q_UNUSED(text);
}

void MainWindow::onNodeSelected(const QModelIndex& index) {
    if (!index.isValid()) {
        return;
    }
    
    QString nodeId = m_model->nodeIdFromIndex(index);
    if (!nodeId.isEmpty()) {
        showNodeDetails(nodeId);
    }
}

void MainWindow::onNodeDoubleClicked(const QModelIndex& index) {
    if (!index.isValid()) {
        return;
    }

    QString nodeId = m_model->nodeIdFromIndex(index);
    if (nodeId.isEmpty()) {
        return;
    }

    if (m_model->isExpanded(nodeId)) {
        m_model->collapseNode(nodeId);
    } else {
        m_model->expandNode(nodeId);
    }
}

void MainWindow::onGraphNodeDoubleClicked(const QString& nodeId) {
    // Refresh the graph visualization with the clicked node as root
    if (m_parser && m_parser->hasNode(nodeId)) {
        m_graphView->displayNodeAsTree(nodeId, m_depthSpin->value());
        showNodeDetails(nodeId);
    }
}

void MainWindow::onNeighborLimitChanged(int value) {
    m_model->setNeighborLimit(value);
    m_statusLabel->setText(QString("Neighbor limit set to %1").arg(value));
}

void MainWindow::onDepthChanged(int value) {
    m_graphView->setMaxDepth(value);
    m_statusLabel->setText(QString("Tree depth set to %1").arg(value));
    
    // Refresh current graph view if visible
    if (m_tabWidget->currentIndex() == 1 && m_parser) {
        // Get the current root node from model and refresh
        auto nodes = m_parser->getNodes();
        if (!nodes.isEmpty()) {
            // Find a visible node to use as root
            QString firstNode = nodes.firstKey();
            m_graphView->displayNodeAsTree(firstNode, value);
        }
    }
}

void MainWindow::onSearchResultClicked(QListWidgetItem* item) {
    QString nodeId = item->data(Qt::UserRole).toString();
    if (!nodeId.isEmpty() && m_parser && m_parser->hasNode(nodeId)) {
        m_model->setRootNode(nodeId);
        m_graphView->displayNodeAsTree(nodeId, m_depthSpin->value());
        showNodeDetails(nodeId);
    }
}

void MainWindow::showNodeDetails(const QString& nodeId) {
    if (!m_parser || !m_parser->hasNode(nodeId)) {
        return;
    }
    
    auto node = m_parser->getNode(nodeId);
    if (!node) {
        return;
    }
    
    QString details;
    details += QString("=== Node: %1 ===\n\n").arg(nodeId);
    
    // Full label
    details += "Full Label:\n";
    details += node->label.replace("\\n", "\n");
    details += "\n\n";
    
    // Statistics
    details += QString("Successors: %1\n").arg(node->successors.size());
    details += QString("Predecessors: %1\n").arg(node->predecessors.size());
    details += QString("Execution Count: %1\n\n").arg(node->count);
    
    // Quick neighbors preview
    auto succs = m_parser->getSuccessors(nodeId, 5);
    if (!succs.isEmpty()) {
        details += "First successors:\n";
        for (const QString& s : succs) {
            auto succNode = m_parser->getNode(s);
            if (succNode) {
                details += QString("  → %1: %2\n").arg(s, succNode->shortLabel);
            }
        }
        if (node->successors.size() > 5) {
            details += QString("  ... and %1 more\n").arg(node->successors.size() - 5);
        }
        details += "\n";
    }
    
    auto preds = m_parser->getPredecessors(nodeId, 5);
    if (!preds.isEmpty()) {
        details += "First predecessors:\n";
        for (const QString& p : preds) {
            auto predNode = m_parser->getNode(p);
            if (predNode) {
                details += QString("  ← %1: %2\n").arg(p, predNode->shortLabel);
            }
        }
        if (node->predecessors.size() > 5) {
            details += QString("  ... and %1 more\n").arg(node->predecessors.size() - 5);
        }
    }
    
    m_nodeDetails->setPlainText(details);
}

void MainWindow::updateStatus() {
    if (m_parser && m_parser->getNodeCount() > 0) {
        m_statusLabel->setText(
            QString("Loaded: %1 nodes, %2 edges | Neighbor limit: %3")
                .arg(m_parser->getNodeCount())
                .arg(m_parser->getEdgeCount())
                .arg(m_model->getNeighborLimit()));
    } else {
        m_statusLabel->setText("Ready - Load a DOT file to begin");
    }
}
