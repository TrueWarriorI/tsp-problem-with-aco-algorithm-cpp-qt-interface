#ifndef ANTCOLONY_H
#define ANTCOLONY_H

#include <vector>
#include <random>
#include <QPointF>
#include <QObject>
#include <cmath>
#include <limits>
#include <algorithm>

// Структура вершины графа
struct Vertex {
    int id;                  // Идентификатор вершины
    QPointF position;        // Координаты на плоскости
    double visitCost;        // Стоимость посещения вершины

    Vertex(int i, QPointF pos, double cost)
        : id(i), position(pos), visitCost(cost) {}
};

// Структура ребра графа
struct Edge {
    int from;               // Начальная вершина
    int to;                 // Конечная вершина
    double distance;        // Расстояние между вершинами
    double pheromone;       // Уровень феромона на ребре

    Edge(int f, int t, double dist)
        : from(f), to(t), distance(dist), pheromone(1.0) {}
};

// Класс для представления муравья
class Ant {
public:
    std::vector<int> route;           // Маршрут муравья
    std::vector<bool> visited;        // Посещённые вершины
    double totalCost;                 // Общая стоимость маршрута
    int currentVertex;                // Текущая вершина

    Ant(int numVertices) : visited(numVertices, false), totalCost(0.0), currentVertex(-1) {}

    void reset(int startVertex) {
        route.clear();
        std::fill(visited.begin(), visited.end(), false);
        totalCost = 0.0;
        currentVertex = startVertex;
        route.push_back(startVertex);
        visited[startVertex] = true;
    }
};

// Основной класс алгоритма муравьиной колонии
class AntColony : public QObject {
    Q_OBJECT

public:
    // Конструктор
    AntColony(int numVertices, int numAnts, double alpha, double beta,
              double rho, double Q, int maxIterations);

    // Генерация случайного графа
    void generateRandomGraph(int width, int height);

    // Запуск одной итерации алгоритма
    void runIteration();

    // Сброс алгоритма
    void reset();

    // Геттеры
    const std::vector<Vertex>& getVertices() const { return vertices; }
    const std::vector<Edge>& getEdges() const { return edges; }
    const std::vector<int>& getBestRoute() const { return bestRoute; }
    double getBestCost() const { return bestCost; }
    int getCurrentIteration() const { return currentIteration; }
    int getMaxIterations() const { return maxIterations; }
    const std::vector<Ant>& getAnts() const { return ants; }

    // Получение матрицы феромонов (для визуализации)
    double getPheromone(int from, int to) const;

signals:
    void iterationCompleted(int iteration, double bestCost);
    void algorithmFinished();

private:
    // Параметры алгоритма
    int numVertices;          // Количество вершин
    int numAnts;              // Количество муравьёв
    double alpha;             // Влияние феромона
    double beta;              // Влияние эвристической информации
    double rho;               // Коэффициент испарения феромона
    double Q;                 // Константа для обновления феромона
    int maxIterations;        // Максимальное количество итераций
    int currentIteration;     // Текущая итерация

    // Структуры данных графа
    std::vector<Vertex> vertices;      // Вершины графа
    std::vector<Edge> edges;           // Рёбра графа
    std::vector<std::vector<int>> edgeIndices; // Индексы рёбер для быстрого доступа

    // Муравьиная колония
    std::vector<Ant> ants;            // Муравьи
    std::vector<int> bestRoute;       // Лучший найденный маршрут
    double bestCost;                  // Стоимость лучшего маршрута

    // Генератор случайных чисел
    std::mt19937 rng;

    // Вспомогательные методы
    void initializeEdges();
    void constructAntSolution(Ant& ant);
    int selectNextVertex(const Ant& ant);
    double calculateRouteCost(const std::vector<int>& route);
    void updatePheromones();
    void evaporatePheromones();
    void depositPheromones(const Ant& ant);
    double getDistance(int v1, int v2) const;
    int getEdgeIndex(int from, int to) const;
};

#endif // ANTCOLONY_H
