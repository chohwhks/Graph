#include "graph.h"

#include <QFile>
#include <QTextStream>
#include <algorithm>
#include <cmath>
#include <limits>
#include <queue>
#include <utility>

Graph::Graph()
    : m_vertexCount(0)
{
}

bool Graph::loadFromFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;

    QTextStream in(&file);

    int n = 0;
    in >> n;
    if (n <= 0)
        return false;

    int m = 0;
    in >> m;
    if (m < 0)
        return false;

    m_vertexCount = n;
    m_edges.clear();

    for (int i = 0; i < m; ++i)
    {
        int from, to;
        in >> from >> to;

        Edge edge;
        edge.from = from;
        edge.to = to;

        m_edges.append(edge);
    }

    file.close();
    return true;
}

double Graph::edgeLength(const Edge &edge, const QVector<QPointF> &positions) const
{
    double dx = positions[edge.to].x() - positions[edge.from].x();
    double dy = positions[edge.to].y() - positions[edge.from].y();
    return std::sqrt(dx * dx + dy * dy);
}

bool Graph::saveAdjacencyMatrix(const QString &fileName, const QVector<QPointF> &positions) const
{
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;

    QVector<QVector<double>> matrix(m_vertexCount, QVector<double>(m_vertexCount, -1.0));

    for (int i = 0; i < m_vertexCount; ++i)
        matrix[i][i] = 0.0;

    for (const Edge &edge : m_edges)
    {
        double length = edgeLength(edge, positions);
        matrix[edge.from][edge.to] = length;
        matrix[edge.to][edge.from] = length;
    }

    QTextStream out(&file);

    for (int i = 0; i < m_vertexCount; ++i)
    {
        for (int j = 0; j < m_vertexCount; ++j)
        {
            out << QString::number(matrix[i][j], 'f', 2);
            if (j != m_vertexCount - 1)
                out << " ";
        }
        out << "\n";
    }

    file.close();
    return true;
}

QVector<int> Graph::dijkstra(int start, int end, const QVector<QPointF> &positions, double &totalDistance) const
{
    const double INF = std::numeric_limits<double>::infinity();

    QVector<double> dist(m_vertexCount, INF);
    QVector<int> prev(m_vertexCount, -1);
    QVector<bool> visited(m_vertexCount, false);

    QVector<QVector<std::pair<int, double>>> adjacency(m_vertexCount);

    for (const Edge &edge : m_edges)
    {
        double length = edgeLength(edge, positions);
        adjacency[edge.from].append({edge.to, length});
        adjacency[edge.to].append({edge.from, length});
    }

    using PQItem = std::pair<double, int>;
    std::priority_queue<PQItem, std::vector<PQItem>, std::greater<PQItem>> pq;

    dist[start] = 0.0;
    pq.push({0.0, start});

    while (!pq.empty())
    {
        double currentDist = pq.top().first;
        int current = pq.top().second;
        pq.pop();

        if (visited[current])
            continue;

        visited[current] = true;

        if (current == end)
            break;

        for (const auto &neighborPair : adjacency[current])
        {
            int neighbor = neighborPair.first;
            double weight = neighborPair.second;

            if (visited[neighbor])
                continue;

            double newDist = currentDist + weight;
            if (newDist < dist[neighbor])
            {
                dist[neighbor] = newDist;
                prev[neighbor] = current;
                pq.push({newDist, neighbor});
            }
        }
    }

    QVector<int> path;

    if (dist[end] == INF)
    {
        totalDistance = -1.0;
        return path;
    }

    for (int v = end; v != -1; v = prev[v])
        path.append(v);

    std::reverse(path.begin(), path.end());

    totalDistance = dist[end];
    return path;
}
