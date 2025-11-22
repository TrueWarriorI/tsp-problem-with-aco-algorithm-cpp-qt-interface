#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsView>
#include <QPushButton>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QTimer>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QCheckBox>
#include <QSlider>
#include "antcolony.h"
#include "graphscene.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onGenerateGraph();
    void onStartAlgorithm();
    void onStopAlgorithm();
    void onResetAlgorithm();
    void onIterationStep();
    void onIterationCompleted(int iteration, double bestCost);
    void onAlgorithmFinished();
    void onSpeedChanged(int value);

private:
    void setupUI();
    void setupConnections();
    void updateStatistics();
    void updateVisualization();

    // UI элементы
    QGraphicsView* graphicsView;
    GraphScene* scene;

    // Параметры алгоритма
    QSpinBox* spinVertices;
    QSpinBox* spinAnts;
    QSpinBox* spinIterations;
    QDoubleSpinBox* spinAlpha;
    QDoubleSpinBox* spinBeta;
    QDoubleSpinBox* spinRho;
    QDoubleSpinBox* spinQ;

    // Кнопки управления
    QPushButton* btnGenerate;
    QPushButton* btnStart;
    QPushButton* btnStop;
    QPushButton* btnReset;

    // Опции отображения
    QCheckBox* checkShowAllEdges;
    QCheckBox* checkShowBestRoute;
    QSlider* sliderSpeed;

    // Статистика
    QLabel* labelIteration;
    QLabel* labelBestCost;
    QLabel* labelStatus;

    // Алгоритм и таймер
    AntColony* colony;
    QTimer* timer;
    bool isRunning;
};

#endif // MAINWINDOW_H
