#include "graphwidget.h"

#include <QPainter>
#include <QMouseEvent>
#include <QCoreApplication>
#include <cmath>

GraphWidget::GraphWidget(QWidget *parent)
    : QWidget(parent)
    , m_vertexRadiusRatio(0.05)
    , m_draggedVertex(-1)
    , m_selectionMode(SelectionMode::None)
    , m_pathStart(-1)
    , m_pathEnd(-1)
    , m_carSegmentIndex(0)
    , m_carSegmentT(0.0)
    , m_carAngleDegrees(0.0)
    , m_carSpeed(4.0)
    , m_carActive(false)
{
    setMinimumSize(300, 300);

    m_carPixmap.load(QCoreApplication::applicationDirPath() + "/car1.png");

    m_carTimer = new QTimer(this);
    connect(m_carTimer, &QTimer::timeout, this, &GraphWidget::onCarTimerTick);
}

void GraphWidget::setGraph(const Graph &graph)
{
    m_graph = graph;
    m_shortestPath.clear();
    m_selectionMode = SelectionMode::None;
    m_pathStart = -1;
    m_pathEnd = -1;
    m_carActive = false;
    m_carTimer->stop();
    calculatePositions();
    update();
}

void GraphWidget::startPathSelection()
{
    m_shortestPath.clear();
    m_pathStart = -1;
    m_pathEnd = -1;
    m_carActive = false;
    m_carTimer->stop();
    m_selectionMode = SelectionMode::WaitingForStart;
    update();
}

bool GraphWidget::saveAdjacencyMatrix(const QString &fileName) const
{
    return m_graph.saveAdjacencyMatrix(fileName, screenPositions());
}

QPointF GraphWidget::toScreen(const QPointF &relative) const
{
    QSize size = currentWindowSize();
    return QPointF(relative.x() * size.width(), relative.y() * size.height());
}

QPointF GraphWidget::toRelative(const QPointF &screen) const
{
    QSize size = currentWindowSize();
    return QPointF(screen.x() / size.width(), screen.y() / size.height());
}

QVector<QPointF> GraphWidget::screenPositions() const
{
    QVector<QPointF> result;
    result.reserve(m_relativePositions.size());

    for (const QPointF &relative : m_relativePositions)
        result.append(toScreen(relative));

    return result;
}

double GraphWidget::vertexRadius() const
{
    QSize size = currentWindowSize();
    return m_vertexRadiusRatio * std::min(size.width(), size.height());
}

void GraphWidget::calculatePositions()
{
    int n = m_graph.vertexCount();
    m_relativePositions.clear();

    if (n <= 0)
        return;

    m_relativePositions.resize(n);

    QSize size = currentWindowSize();
    int windowWidth = size.width();
    int windowHeight = size.height();

    QPointF center(windowWidth / 2.0, windowHeight / 2.0);

    double layoutRadius = std::min(windowWidth, windowHeight) / 2.0 - 40.0;
    if (layoutRadius < 10)
        layoutRadius = 10;

    double chordLength = 2.0 * layoutRadius * std::sin(M_PI / n);
    double radiusPixels = chordLength / 2.0 * 0.7;

    if (radiusPixels > 22.0)
        radiusPixels = 22.0;
    if (radiusPixels < 6.0)
        radiusPixels = 6.0;

    m_vertexRadiusRatio = radiusPixels / std::min(windowWidth, windowHeight);

    double angleStep = 2.0 * M_PI / n;

    for (int i = 0; i < n; ++i)
    {
        double angle = M_PI + i * angleStep;

        double x = center.x() + layoutRadius * std::cos(angle);
        double y = center.y() + layoutRadius * std::sin(angle);

        m_relativePositions[i] = toRelative(QPointF(x, y));
    }
}

int GraphWidget::vertexAt(const QPointF &screenPoint) const
{
    double radius = vertexRadius();

    for (int i = 0; i < m_relativePositions.size(); ++i)
    {
        QPointF center = toScreen(m_relativePositions[i]);

        double dx = screenPoint.x() - center.x();
        double dy = screenPoint.y() - center.y();
        double distance = std::sqrt(dx * dx + dy * dy);

        if (distance <= radius)
            return i;
    }
    return -1;
}

bool GraphWidget::isPathEdge(int u, int v) const
{
    for (int i = 0; i + 1 < m_shortestPath.size(); ++i)
    {
        int a = m_shortestPath[i];
        int b = m_shortestPath[i + 1];

        if ((a == u && b == v) || (a == v && b == u))
            return true;
    }
    return false;
}

void GraphWidget::recomputePath()
{
    if (m_pathStart == -1 || m_pathEnd == -1)
        return;

    double totalDistance = 0.0;
    m_shortestPath = m_graph.dijkstra(m_pathStart, m_pathEnd, screenPositions(), totalDistance);

    emit pathComputed(m_pathStart, m_pathEnd, totalDistance, !m_shortestPath.isEmpty());
}

void GraphWidget::startCarAnimation()
{
    if (m_shortestPath.size() < 2)
        return;

    m_carPathPoints.clear();
    for (int vertex : m_shortestPath)
        m_carPathPoints.append(toScreen(m_relativePositions[vertex]));

    m_carSegmentIndex = 0;
    m_carSegmentT = 0.0;
    m_carAngleDegrees = 0.0;
    m_carActive = true;

    m_carTimer->start(30);
}

void GraphWidget::onCarTimerTick()
{
    if (m_carSegmentIndex >= m_carPathPoints.size() - 1)
    {
        m_carTimer->stop();
        m_carActive = false;
        update();
        return;
    }

    QPointF p1 = m_carPathPoints[m_carSegmentIndex];
    QPointF p2 = m_carPathPoints[m_carSegmentIndex + 1];

    double dx = p2.x() - p1.x();
    double dy = p2.y() - p1.y();
    double segmentLength = std::sqrt(dx * dx + dy * dy);

    m_carAngleDegrees = std::atan2(dy, dx) * 180.0 / M_PI;

    if (segmentLength < 1.0)
    {
        m_carSegmentIndex++;
        m_carSegmentT = 0.0;
        update();
        return;
    }

    m_carSegmentT += m_carSpeed / segmentLength;

    if (m_carSegmentT >= 1.0)
    {
        m_carSegmentT = 0.0;
        m_carSegmentIndex++;
    }

    update();
}

void GraphWidget::mousePressEvent(QMouseEvent *event)
{
    int idx = vertexAt(event->pos());

    if (m_selectionMode == SelectionMode::WaitingForStart)
    {
        if (idx != -1)
        {
            m_pathStart = idx;
            m_selectionMode = SelectionMode::WaitingForEnd;
            emit startVertexSelected(idx);
            update();
        }
        return;
    }

    if (m_selectionMode == SelectionMode::WaitingForEnd)
    {
        if (idx != -1 && idx != m_pathStart)
        {
            m_pathEnd = idx;
            m_selectionMode = SelectionMode::None;
            recomputePath();
            update();
        }
        return;
    }

    m_draggedVertex = idx;
}

void GraphWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (m_draggedVertex == -1)
        return;

    m_relativePositions[m_draggedVertex] = toRelative(event->pos());

    if (m_pathStart != -1 && m_pathEnd != -1)
        recomputePath();

    update();
}

void GraphWidget::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    m_draggedVertex = -1;
}

void GraphWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    painter.fillRect(rect(), Qt::white);

    if (m_graph.vertexCount() == 0)
        return;

    double radius = vertexRadius();

    for (const Edge &edge : m_graph.edges())
    {
        QPointF p1 = toScreen(m_relativePositions[edge.from]);
        QPointF p2 = toScreen(m_relativePositions[edge.to]);

        bool highlighted = isPathEdge(edge.from, edge.to);

        QPen pen;
        if (highlighted)
        {
            pen.setColor(Qt::green);
            pen.setWidth(4);
        }
        else
        {
            pen.setColor(Qt::black);
            pen.setWidth(2);
        }

        painter.setPen(pen);
        painter.drawLine(p1, p2);
    }

    for (int i = 0; i < m_graph.vertexCount(); ++i)
    {
        QPointF center = toScreen(m_relativePositions[i]);

        bool onPath = m_shortestPath.contains(i);
        bool isEndpoint = (i == m_pathStart || i == m_pathEnd);

        QColor fillColor(220, 220, 255);
        if (onPath)
            fillColor = QColor(200, 255, 200);
        else if (isEndpoint)
            fillColor = QColor(255, 255, 150);

        QPen pen(Qt::black);
        pen.setWidth((onPath || isEndpoint) ? 3 : 2);
        painter.setPen(pen);
        painter.setBrush(fillColor);

        painter.drawEllipse(center, radius, radius);

        QRectF textRect(center.x() - radius, center.y() - radius,
                        radius * 2, radius * 2);

        painter.setPen(Qt::black);
        painter.drawText(textRect, Qt::AlignCenter, QString::number(i));
    }

    if (m_carActive && m_carSegmentIndex < m_carPathPoints.size() - 1)
    {
        QPointF p1 = m_carPathPoints[m_carSegmentIndex];
        QPointF p2 = m_carPathPoints[m_carSegmentIndex + 1];

        QPointF carPos = p1 + (p2 - p1) * m_carSegmentT;

        int carWidth = 100;
        int carHeight = 100;

        painter.save();
        painter.translate(carPos);
        painter.rotate(m_carAngleDegrees);

        if (!m_carPixmap.isNull())
        {
            painter.drawPixmap(-carWidth / 2, -carHeight / 2, carWidth, carHeight, m_carPixmap);
        }
        else
        {
            QPolygonF triangle;
            triangle << QPointF(carWidth / 2.0, 0)
                     << QPointF(-carWidth / 2.0, -carHeight / 2.0)
                     << QPointF(-carWidth / 2.0, carHeight / 2.0);

            painter.setPen(Qt::black);
            painter.setBrush(Qt::red);
            painter.drawPolygon(triangle);
        }

        painter.restore();
    }
}
