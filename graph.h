#ifndef GRAPH_H
#define GRAPH_H

#include <QString>
#include <QVector>
#include <QPointF>

struct Edge
{
    int from;
    int to;
};

class Graph
{
public:
    Graph();

    bool loadFromFile(const QString &fileName);
    bool saveAdjacencyMatrix(const QString &fileName, const QVector<QPointF> &positions) const;

    int vertexCount() const { return m_vertexCount; }
    int edgeCount() const { return m_edges.size(); }

    const QVector<Edge> &edges() const { return m_edges; }

    QVector<int> dijkstra(int start, int end, const QVector<QPointF> &positions, double &totalDistance) const;

private:
    int m_vertexCount;
    QVector<Edge> m_edges;

    double edgeLength(const Edge &edge, const QVector<QPointF> &positions) const;
};

#endif // GRAPH_H
