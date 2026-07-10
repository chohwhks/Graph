#ifndef GRAPHWIDGET_H
#define GRAPHWIDGET_H

#include <QWidget>
#include <QVector>
#include <QPointF>
#include <QSize>
#include <QPixmap>
#include <QTimer>

#include "graph.h"

class GraphWidget : public QWidget
{
    Q_OBJECT

public:
    explicit GraphWidget(QWidget *parent = nullptr);

    void setGraph(const Graph &graph);
    void startPathSelection();
    bool saveAdjacencyMatrix(const QString &fileName) const;

public slots:
    void startCarAnimation();

signals:
    void startVertexSelected(int vertex);
    void pathComputed(int start, int end, double distance, bool found);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private slots:
    void onCarTimerTick();

private:
    enum class SelectionMode { None, WaitingForStart, WaitingForEnd };

    Graph m_graph;

    QVector<QPointF> m_relativePositions;
    QVector<int> m_shortestPath;
    double m_vertexRadiusRatio;

    int m_draggedVertex;

    SelectionMode m_selectionMode;
    int m_pathStart;
    int m_pathEnd;

    QPixmap m_carPixmap;
    QTimer *m_carTimer;
    QVector<QPointF> m_carPathPoints;
    int m_carSegmentIndex;
    double m_carSegmentT;
    double m_carAngleDegrees;
    double m_carSpeed;
    bool m_carActive;

    QSize currentWindowSize() const { return QSize(width(), height()); }

    void calculatePositions();
    void recomputePath();

    QPointF toScreen(const QPointF &relative) const;
    QPointF toRelative(const QPointF &screen) const;
    QVector<QPointF> screenPositions() const;
    double vertexRadius() const;

    int vertexAt(const QPointF &screenPoint) const;
    bool isPathEdge(int u, int v) const;
};

#endif // GRAPHWIDGET_H
