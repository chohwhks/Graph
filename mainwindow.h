#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "graph.h"
#include "graphwidget.h"

class QPushButton;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private slots:
    void openGraphFile();
    void saveMatrixFile();
    void runDijkstra();
    void onStartVertexSelected(int vertex);
    void onPathComputed(int start, int end, double distance, bool found);

private:
    Graph m_graph;

    GraphWidget *m_graphWidget;

    QPushButton *m_openButton;
    QPushButton *m_saveMatrixButton;
    QPushButton *m_dijkstraButton;
    QPushButton *m_carButton;
};

#endif // MAINWINDOW_H
