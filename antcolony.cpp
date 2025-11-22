#include "antcolony.h"
#include <QDebug>

AntColony::AntColony(int numVertices, int numAnts, double alpha, double beta,
                     double rho, double Q, int maxIterations)
    : numVertices(numVertices), numAnts(numAnts), alpha(alpha), beta(beta),
    rho(rho), Q(Q), maxIterations(maxIterations), currentIteration(0),
    bestCost(std::numeric_limits<double>::max())
{
    // Инициализация генератора случайных чисел
    std::random_device rd;
    rng.seed(rd());

    // Инициализация муравьёв
    ants.resize(numAnts, Ant(numVertices));

    // Инициализация матрицы индексов рёбер
    edgeIndices.resize(numVertices, std::vector<int>(numVertices, -1));
}

void AntColony::generateRandomGraph(int width, int height) {
    vertices.clear();
    edges.clear();

    std::uniform_real_distribution<double> distX(50, width - 50);
    std::uniform_real_distribution<double> distY(50, height - 50);
    std::uniform_real_distribution<double> distCost(10.0, 100.0);

    // Генерация вершин со случайными координатами и стоимостями
    for (int i = 0; i < numVertices; ++i) {
        QPointF position(distX(rng), distY(rng));
        double visitCost = distCost(rng);
        vertices.emplace_back(i, position, visitCost);
    }

    // Инициализация рёбер
    initializeEdges();
}

void AntColony::initializeEdges() {
    edges.clear();

    // Создание полного графа (все вершины соединены между собой)
    int edgeIndex = 0;
    for (int i = 0; i < numVertices; ++i) {
        for (int j = 0; j < numVertices; ++j) {
            if (i != j) {
                double distance = getDistance(i, j);
                edges.emplace_back(i, j, distance);
                edgeIndices[i][j] = edgeIndex++;
            }
        }
    }
}

void AntColony::reset() {
    currentIteration = 0;
    bestCost = std::numeric_limits<double>::max();
    bestRoute.clear();

    // Сброс феромонов
    for (auto& edge : edges) {
        edge.pheromone = 1.0;
    }
}

void AntColony::runIteration() {
    if (currentIteration >= maxIterations) {
        emit algorithmFinished();
        return;
    }

    // Каждый муравей строит маршрут
    for (int i = 0; i < numAnts; ++i) {
        // Случайная стартовая вершина
        std::uniform_int_distribution<int> distStart(0, numVertices - 1);
        int startVertex = distStart(rng);

        ants[i].reset(startVertex);
        constructAntSolution(ants[i]);

        // Обновление лучшего решения
        if (ants[i].totalCost < bestCost) {
            bestCost = ants[i].totalCost;
            bestRoute = ants[i].route;
        }
    }

    // Обновление феромонов
    updatePheromones();

    currentIteration++;
    emit iterationCompleted(currentIteration, bestCost);

    if (currentIteration >= maxIterations) {
        emit algorithmFinished();
    }
}

void AntColony::constructAntSolution(Ant& ant) {
    // Построение маршрута для одного муравья
    while (ant.route.size() < static_cast<size_t>(numVertices)) {
        int nextVertex = selectNextVertex(ant);

        // Добавление стоимости ребра
        double edgeDistance = getDistance(ant.currentVertex, nextVertex);
        ant.totalCost += edgeDistance;

        // Переход к следующей вершине
        ant.currentVertex = nextVertex;
        ant.route.push_back(nextVertex);
        ant.visited[nextVertex] = true;

        // Добавление стоимости посещения вершины
        ant.totalCost += vertices[nextVertex].visitCost;
    }

    // Возврат к стартовой вершине
    ant.totalCost += getDistance(ant.currentVertex, ant.route[0]);
}

int AntColony::selectNextVertex(const Ant& ant) {
    std::vector<int> unvisited;
    std::vector<double> probabilities;
    double sumProbabilities = 0.0;

    // Находим непосещённые вершины и вычисляем вероятности
    for (int i = 0; i < numVertices; ++i) {
        if (!ant.visited[i]) {
            unvisited.push_back(i);

            // Получаем уровень феромона и расстояние
            double pheromone = getPheromone(ant.currentVertex, i);
            double distance = getDistance(ant.currentVertex, i);
            double vertexCost = vertices[i].visitCost;

            // Эвристическая информация (обратная величина общей стоимости)
            double eta = 1.0 / (distance + vertexCost);

            // Вероятность выбора вершины: τ^α * η^β
            double probability = std::pow(pheromone, alpha) * std::pow(eta, beta);
            probabilities.push_back(probability);
            sumProbabilities += probability;
        }
    }

    if (unvisited.empty()) {
        return -1;
    }

    // Нормализация вероятностей
    for (double& prob : probabilities) {
        prob /= sumProbabilities;
    }

    // Выбор следующей вершины методом рулетки
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    double random = dist(rng);
    double cumulative = 0.0;

    for (size_t i = 0; i < unvisited.size(); ++i) {
        cumulative += probabilities[i];
        if (random <= cumulative) {
            return unvisited[i];
        }
    }

    return unvisited.back();
}

void AntColony::updatePheromones() {
    // Испарение феромонов
    evaporatePheromones();

    // Откладывание феромонов каждым муравьём
    for (const Ant& ant : ants) {
        depositPheromones(ant);
    }
}

void AntColony::evaporatePheromones() {
    for (Edge& edge : edges) {
        edge.pheromone *= (1.0 - rho);

        // Минимальный уровень феромона
        if (edge.pheromone < 0.01) {
            edge.pheromone = 0.01;
        }
    }
}

void AntColony::depositPheromones(const Ant& ant) {
    double deltaPheromone = Q / ant.totalCost;

    // Откладывание феромонов на все рёбра маршрута
    for (size_t i = 0; i < ant.route.size(); ++i) {
        int from = ant.route[i];
        int to = ant.route[(i + 1) % ant.route.size()];

        int edgeIdx = getEdgeIndex(from, to);
        if (edgeIdx != -1) {
            edges[edgeIdx].pheromone += deltaPheromone;
        }
    }
}

double AntColony::getDistance(int v1, int v2) const {
    const QPointF& p1 = vertices[v1].position;
    const QPointF& p2 = vertices[v2].position;

    double dx = p1.x() - p2.x();
    double dy = p1.y() - p2.y();

    return std::sqrt(dx * dx + dy * dy);
}

int AntColony::getEdgeIndex(int from, int to) const {
    return edgeIndices[from][to];
}

double AntColony::getPheromone(int from, int to) const {
    int edgeIdx = getEdgeIndex(from, to);
    if (edgeIdx != -1) {
        return edges[edgeIdx].pheromone;
    }
    return 0.0;
}

double AntColony::calculateRouteCost(const std::vector<int>& route) {
    double cost = 0.0;

    for (size_t i = 0; i < route.size(); ++i) {
        int from = route[i];
        int to = route[(i + 1) % route.size()];

        cost += getDistance(from, to);
        cost += vertices[to].visitCost;
    }

    return cost;
}
