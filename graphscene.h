#ifndef GRAPHSCENE_H
#define GRAPHSCENE_H

#include <QGraphicsScene>
#include <QGraphicsEllipseItem>
#include <QGraphicsLineItem>
#include <QGraphicsTextItem>
#include <QPen>
#include <QBrush>
#include "antcolony.h"

class GraphScene : public QGraphicsScene {
    Q_OBJECT

public:
    explicit GraphScene(QObject* parent = nullptr);

    // Отрисовка графа
    void drawGraph(AntColony* colony, bool showAllEdges = true, bool showBestRoute = false);

    // Очистка сцены
    void clearGraph();

private:
    // Вспомогательные методы отрисовки
    void drawVertices(const std::vector<Vertex>& vertices);
    void drawAllEdges(AntColony* colony);
    void drawBestRoute(AntColony* colony);
    void drawVertex(const Vertex& vertex);
    void drawEdge(const QPointF& p1, const QPointF& p2, double pheromone, bool isBest = false);

    // Получение цвета на основе уровня феромона
    QColor getPheromoneColor(double pheromone, double maxPheromone);

    // Списки графических элементов
    std::vector<QGraphicsEllipseItem*> vertexItems;
    std::vector<QGraphicsLineItem*> edgeItems;
    std::vector<QGraphicsTextItem*> textItems;
};

#endif // GRAPHSCENE_H
