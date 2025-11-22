#include "graphscene.h"
#include <QDebug>
#include <algorithm>

GraphScene::GraphScene(QObject* parent) : QGraphicsScene(parent) {
    setBackgroundBrush(QBrush(QColor(250, 250, 250)));
}

void GraphScene::clearGraph() {
    clear();
    vertexItems.clear();
    edgeItems.clear();
    textItems.clear();
}

void GraphScene::drawGraph(AntColony* colony, bool showAllEdges, bool showBestRoute) {
    clearGraph();

    if (!colony) return;

    // Отрисовка рёбер (сначала, чтобы они были под вершинами)
    if (showAllEdges) {
        drawAllEdges(colony);
    }

    if (showBestRoute && !colony->getBestRoute().empty()) {
        drawBestRoute(colony);
    }

    // Отрисовка вершин
    drawVertices(colony->getVertices());
}

void GraphScene::drawVertices(const std::vector<Vertex>& vertices) {
    for (const auto& vertex : vertices) {
        drawVertex(vertex);
    }
}

void GraphScene::drawVertex(const Vertex& vertex) {
    double radius = 25;

    // Круг вершины
    QGraphicsEllipseItem* circle = addEllipse(
        vertex.position.x() - radius,
        vertex.position.y() - radius,
        2 * radius, 2 * radius,
        QPen(QColor(50, 50, 150), 2),
        QBrush(QColor(100, 150, 255))
        );
    vertexItems.push_back(circle);

    // Номер вершины
    QGraphicsTextItem* idText = addText(QString::number(vertex.id));
    idText->setDefaultTextColor(Qt::white);
    idText->setFont(QFont("Arial", 10, QFont::Bold));
    QRectF textRect = idText->boundingRect();
    idText->setPos(
        vertex.position.x() - textRect.width() / 2,
        vertex.position.y() - textRect.height() / 2 - 5
        );
    textItems.push_back(idText);

    // Стоимость посещения
    QGraphicsTextItem* costText = addText(QString("$%1").arg(vertex.visitCost, 0, 'f', 1));
    costText->setDefaultTextColor(Qt::white);
    costText->setFont(QFont("Arial", 8));
    QRectF costRect = costText->boundingRect();
    costText->setPos(
        vertex.position.x() - costRect.width() / 2,
        vertex.position.y() - costRect.height() / 2 + 5
        );
    textItems.push_back(costText);
}

void GraphScene::drawAllEdges(AntColony* colony) {
    const auto& edges = colony->getEdges();
    const auto& vertices = colony->getVertices();

    // Находим максимальный уровень феромона для нормализации
    double maxPheromone = 1.0;
    for (const auto& edge : edges) {
        maxPheromone = std::max(maxPheromone, edge.pheromone);
    }

    // Отрисовка всех рёбер с учётом феромонов
    for (const auto& edge : edges) {
        const QPointF& p1 = vertices[edge.from].position;
        const QPointF& p2 = vertices[edge.to].position;

        drawEdge(p1, p2, edge.pheromone / maxPheromone, false);
    }
}

void GraphScene::drawBestRoute(AntColony* colony) {
    const auto& bestRoute = colony->getBestRoute();
    const auto& vertices = colony->getVertices();

    if (bestRoute.size() < 2) return;

    // Отрисовка лучшего маршрута
    for (size_t i = 0; i < bestRoute.size(); ++i) {
        int from = bestRoute[i];
        int to = bestRoute[(i + 1) % bestRoute.size()];

        const QPointF& p1 = vertices[from].position;
        const QPointF& p2 = vertices[to].position;

        drawEdge(p1, p2, 1.0, true);
    }
}

void GraphScene::drawEdge(const QPointF& p1, const QPointF& p2, double normalizedPheromone, bool isBest) {
    if (isBest) {
        // Лучший маршрут - толстая зелёная линия
        QGraphicsLineItem* line = addLine(
            p1.x(), p1.y(), p2.x(), p2.y(),
            QPen(QColor(0, 200, 0), 4, Qt::SolidLine)
            );
        line->setZValue(10);
        edgeItems.push_back(line);

        // Стрелка направления
        double angle = std::atan2(p2.y() - p1.y(), p2.x() - p1.x());
        double arrowSize = 15;
        QPointF midPoint = (p1 + p2) / 2;

        QPointF arrowP1 = midPoint - QPointF(
                              arrowSize * std::cos(angle - M_PI / 6),
                              arrowSize * std::sin(angle - M_PI / 6)
                              );
        QPointF arrowP2 = midPoint - QPointF(
                              arrowSize * std::cos(angle + M_PI / 6),
                              arrowSize * std::sin(angle + M_PI / 6)
                              );

        addLine(midPoint.x(), midPoint.y(), arrowP1.x(), arrowP1.y(),
                QPen(QColor(0, 200, 0), 3));
        addLine(midPoint.x(), midPoint.y(), arrowP2.x(), arrowP2.y(),
                QPen(QColor(0, 200, 0), 3));
    } else {
        // Обычное ребро с цветом, зависящим от феромона
        QColor color = getPheromoneColor(normalizedPheromone, 1.0);
        double width = 1.0 + normalizedPheromone * 2.0;

        QGraphicsLineItem* line = addLine(
            p1.x(), p1.y(), p2.x(), p2.y(),
            QPen(color, width, Qt::SolidLine)
            );
        line->setZValue(0);
        line->setOpacity(0.3 + normalizedPheromone * 0.4);
        edgeItems.push_back(line);
    }
}

QColor GraphScene::getPheromoneColor(double pheromone, double maxPheromone) {
    // Градиент от синего (мало феромона) к красному (много феромона)
    double normalized = pheromone / maxPheromone;

    int r = static_cast<int>(normalized * 255);
    int b = static_cast<int>((1.0 - normalized) * 255);
    int g = 100;

    return QColor(r, g, b);
}
