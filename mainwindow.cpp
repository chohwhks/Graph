#include "mainwindow.h"

#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QStatusBar>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    QWidget *central = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(central);

    QHBoxLayout *buttonsLayout = new QHBoxLayout();

    m_openButton = new QPushButton("Открыть файл графа", this);
    m_saveMatrixButton = new QPushButton("Сохранить матрицу смежности", this);
    m_dijkstraButton = new QPushButton("Найти кратчайший путь", this);
    m_carButton = new QPushButton("Машинка", this);

    buttonsLayout->addWidget(m_openButton);
    buttonsLayout->addWidget(m_saveMatrixButton);
    buttonsLayout->addWidget(m_dijkstraButton);
    buttonsLayout->addWidget(m_carButton);

    m_graphWidget = new GraphWidget(this);

    mainLayout->addLayout(buttonsLayout);
    mainLayout->addWidget(m_graphWidget);

    setCentralWidget(central);
    setWindowTitle("Отрисовка графа");

    m_saveMatrixButton->setEnabled(false);
    m_dijkstraButton->setEnabled(false);
    m_carButton->setEnabled(false);

    connect(m_openButton, &QPushButton::clicked, this, &MainWindow::openGraphFile);
    connect(m_saveMatrixButton, &QPushButton::clicked, this, &MainWindow::saveMatrixFile);
    connect(m_dijkstraButton, &QPushButton::clicked, this, &MainWindow::runDijkstra);
    connect(m_carButton, &QPushButton::clicked, m_graphWidget, &GraphWidget::startCarAnimation);

    connect(m_graphWidget, &GraphWidget::startVertexSelected, this, &MainWindow::onStartVertexSelected);
    connect(m_graphWidget, &GraphWidget::pathComputed, this, &MainWindow::onPathComputed);
}

void MainWindow::openGraphFile()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Выберите файл графа",
                                                    QString(), "Текстовые файлы (*.txt)");
    if (fileName.isEmpty())
        return;

    if (!m_graph.loadFromFile(fileName))
    {
        QMessageBox::warning(this, "Ошибка", "Не удалось прочитать файл графа");
        return;
    }

    m_graphWidget->setGraph(m_graph);

    m_saveMatrixButton->setEnabled(true);
    m_dijkstraButton->setEnabled(true);
    m_carButton->setEnabled(false);

    statusBar()->showMessage("Граф загружен");
}

void MainWindow::saveMatrixFile()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Сохранить матрицу смежности",
                                                    "matrix.txt", "Текстовые файлы (*.txt)");
    if (fileName.isEmpty())
        return;

    if (m_graphWidget->saveAdjacencyMatrix(fileName))
        QMessageBox::information(this, "Готово", "Матрица смежности сохранена");
    else
        QMessageBox::warning(this, "Ошибка", "Не удалось сохранить файл");
}

void MainWindow::runDijkstra()
{
    m_carButton->setEnabled(false);
    m_graphWidget->startPathSelection();
    statusBar()->showMessage("Кликните на первую вершину");
}

void MainWindow::onStartVertexSelected(int vertex)
{
    statusBar()->showMessage(QString("Начальная вершина: %1. Кликните на конечную вершину").arg(vertex));
}

void MainWindow::onPathComputed(int start, int end, double distance, bool found)
{
    if (!found)
    {
        m_carButton->setEnabled(false);
        statusBar()->showMessage(QString("Путь между вершинами %1 и %2 не существует").arg(start).arg(end));
        return;
    }

    m_carButton->setEnabled(true);
    statusBar()->showMessage(QString("Кратчайший путь %1 -> %2: длина %3")
                                 .arg(start)
                                 .arg(end)
                                 .arg(distance, 0, 'f', 1));
}
