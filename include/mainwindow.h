#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSharedPointer>
#include <QTreeView>
#include <QLineEdit>
#include <QLabel>
#include <QSpinBox>
#include <QPushButton>
#include <QListWidget>
#include <QTextEdit>
#include <QStatusBar>
#include <QProgressBar>

#include "dotparser.h"
#include "graphmodel.h"
#include "graphview.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    MainWindow(const QString& fileName, QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void loadFile();
    void loadFileInternal(const QString& fileName);
    void searchGraph();
    void onSearchTextChanged(const QString& text);
    void onGraphNodeDoubleClicked(const QString& nodeId);
    void onNeighborLimitChanged(int value);
    void onDepthChanged(int value);
    void onSearchResultClicked(QListWidgetItem* item);
    void showNodeDetails(const QString& nodeId);

private:
    void setupUi();
    void setupMenuBar();
    void setupToolBar();
    void setupCentralWidget();
    void updateStatus();

    QSharedPointer<DotParser> m_parser;
    GraphModel* m_model;
    
    // UI elements
    GraphView* m_graphView;
    QLineEdit* m_searchEdit;
    QListWidget* m_searchResults;
    QLabel* m_statusLabel;
    QLabel* m_graphInfoLabel;
    QSpinBox* m_neighborLimitSpin;
    QSpinBox* m_depthSpin;
    QTextEdit* m_nodeDetails;
    QPushButton* m_searchBtn;
    
    QWidget* m_centralWidget;
};

#endif // MAINWINDOW_H
