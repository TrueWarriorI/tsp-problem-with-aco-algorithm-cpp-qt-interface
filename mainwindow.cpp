#include "mainwindow.h"
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), colony(nullptr), isRunning(false)
{
    setupUI();
    setupConnections();

    // Инициализация таймера
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::onIterationStep);
}

MainWindow::~MainWindow() {
    if (colony) {
        delete colony;
    }
}

void MainWindow::setupUI() {
    // Центральный виджет
    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QHBoxLayout* mainLayout = new QHBoxLayout(centralWidget);

    // === Левая панель с параметрами ===
    QVBoxLayout* leftLayout = new QVBoxLayout();

    // Группа параметров графа
    QGroupBox* graphGroup = new QGroupBox("Параметры графа");
    QVBoxLayout* graphLayout = new QVBoxLayout();

    QHBoxLayout* verticesLayout = new QHBoxLayout();
    verticesLayout->addWidget(new QLabel("Количество вершин:"));
    spinVertices = new QSpinBox();
    spinVertices->setRange(5, 50);
    spinVertices->setValue(10);
    verticesLayout->addWidget(spinVertices);
    graphLayout->addLayout(verticesLayout);

    btnGenerate = new QPushButton("Сгенерировать граф");
    graphLayout->addWidget(btnGenerate);

    graphGroup->setLayout(graphLayout);
    leftLayout->addWidget(graphGroup);

    // Группа параметров алгоритма
    QGroupBox* algoGroup = new QGroupBox("Параметры алгоритма");
    QVBoxLayout* algoLayout = new QVBoxLayout();

    QHBoxLayout* antsLayout = new QHBoxLayout();
    antsLayout->addWidget(new QLabel("Количество муравьёв:"));
    spinAnts = new QSpinBox();
    spinAnts->setRange(5, 100);
    spinAnts->setValue(20);
    antsLayout->addWidget(spinAnts);
    algoLayout->addLayout(antsLayout);

    QHBoxLayout* iterLayout = new QHBoxLayout();
    iterLayout->addWidget(new QLabel("Количество итераций:"));
    spinIterations = new QSpinBox();
    spinIterations->setRange(10, 1000);
    spinIterations->setValue(100);
    iterLayout->addWidget(spinIterations);
    algoLayout->addLayout(iterLayout);

    QHBoxLayout* alphaLayout = new QHBoxLayout();
    alphaLayout->addWidget(new QLabel("Alpha (влияние феромона):"));
    spinAlpha = new QDoubleSpinBox();
    spinAlpha->setRange(0.1, 5.0);
    spinAlpha->setValue(1.0);
    spinAlpha->setSingleStep(0.1);
    alphaLayout->addWidget(spinAlpha);
    algoLayout->addLayout(alphaLayout);

    QHBoxLayout* betaLayout = new QHBoxLayout();
    betaLayout->addWidget(new QLabel("Beta (влияние эвристики):"));
    spinBeta = new QDoubleSpinBox();
    spinBeta->setRange(0.1, 10.0);
    spinBeta->setValue(2.0);
    spinBeta->setSingleStep(0.1);
    betaLayout->addWidget(spinBeta);
    algoLayout->addLayout(betaLayout);

    QHBoxLayout* rhoLayout = new QHBoxLayout();
    rhoLayout->addWidget(new QLabel("Rho (испарение феромона):"));
    spinRho = new QDoubleSpinBox();
    spinRho->setRange(0.01, 0.99);
    spinRho->setValue(0.5);
    spinRho->setSingleStep(0.05);
    rhoLayout->addWidget(spinRho);
    algoLayout->addLayout(rhoLayout);

    QHBoxLayout* qLayout = new QHBoxLayout();
    qLayout->addWidget(new QLabel("Q (константа феромона):"));
    spinQ = new QDoubleSpinBox();
    spinQ->setRange(1.0, 1000.0);
    spinQ->setValue(100.0);
    spinQ->setSingleStep(10.0);
    qLayout->addWidget(spinQ);
    algoLayout->addLayout(qLayout);

    algoGroup->setLayout(algoLayout);
    leftLayout->addWidget(algoGroup);

    // Группа управления
    QGroupBox* controlGroup = new QGroupBox("Управление");
    QVBoxLayout* controlLayout = new QVBoxLayout();

    btnStart = new QPushButton("Запустить алгоритм");
    btnStart->setEnabled(false);
    btnStart->setStyleSheet("QPushButton { background-color: #4CAF50; color: white; font-weight: bold; padding: 8px; }");
    controlLayout->addWidget(btnStart);

    btnStop = new QPushButton("Остановить");
    btnStop->setEnabled(false);
    btnStop->setStyleSheet("QPushButton { background-color: #f44336; color: white; font-weight: bold; padding: 8px; }");
    controlLayout->addWidget(btnStop);

    btnReset = new QPushButton("Сбросить");
    btnReset->setEnabled(false);
    controlLayout->addWidget(btnReset);

    controlGroup->setLayout(controlLayout);
    leftLayout->addWidget(controlGroup);

    // Группа скорости анимации
    QGroupBox* speedGroup = new QGroupBox("Скорость анимации");
    QVBoxLayout* speedLayout = new QVBoxLayout();

    QHBoxLayout* sliderLayout = new QHBoxLayout();
    sliderLayout->addWidget(new QLabel("Медленно"));
    sliderSpeed = new QSlider(Qt::Horizontal);
    sliderSpeed->setRange(1, 10);
    sliderSpeed->setValue(5);
    sliderLayout->addWidget(sliderSpeed);
    sliderLayout->addWidget(new QLabel("Быстро"));
    speedLayout->addLayout(sliderLayout);

    speedGroup->setLayout(speedLayout);
    leftLayout->addWidget(speedGroup);

    // Группа опций отображения
    QGroupBox* displayGroup = new QGroupBox("Опции отображения");
    QVBoxLayout* displayLayout = new QVBoxLayout();

    checkShowAllEdges = new QCheckBox("Показать все рёбра (феромоны)");
    checkShowAllEdges->setChecked(true);
    displayLayout->addWidget(checkShowAllEdges);

    checkShowBestRoute = new QCheckBox("Показать лучший маршрут");
    checkShowBestRoute->setChecked(true);
    displayLayout->addWidget(checkShowBestRoute);

    displayGroup->setLayout(displayLayout);
    leftLayout->addWidget(displayGroup);

    // Группа статистики
    QGroupBox* statsGroup = new QGroupBox("Статистика");
    QVBoxLayout* statsLayout = new QVBoxLayout();

    labelIteration = new QLabel("Итерация: 0 / 0");
    labelIteration->setStyleSheet("font-weight: bold; font-size: 12px;");
    statsLayout->addWidget(labelIteration);

    labelBestCost = new QLabel("Лучшая стоимость: -");
    labelBestCost->setStyleSheet("font-weight: bold; font-size: 12px; color: #4CAF50;");
    statsLayout->addWidget(labelBestCost);

    labelStatus = new QLabel("Статус: Ожидание");
    labelStatus->setStyleSheet("font-size: 11px; color: #666;");
    statsLayout->addWidget(labelStatus);

    statsGroup->setLayout(statsLayout);
    leftLayout->addWidget(statsGroup);

    leftLayout->addStretch();

    // === Правая панель с визуализацией ===
    QVBoxLayout* rightLayout = new QVBoxLayout();

    QLabel* titleLabel = new QLabel("Визуализация графа и маршрута");
    titleLabel->setStyleSheet("font-size: 16px; font-weight: bold; padding: 10px;");
    titleLabel->setAlignment(Qt::AlignCenter);
    rightLayout->addWidget(titleLabel);

    // Графическая сцена
    scene = new GraphScene(this);
    graphicsView = new QGraphicsView(scene);
    graphicsView->setRenderHint(QPainter::Antialiasing);
    graphicsView->setMinimumSize(800, 600);
    rightLayout->addWidget(graphicsView);

    // Легенда
    QGroupBox* legendGroup = new QGroupBox("Легенда");
    QHBoxLayout* legendLayout = new QHBoxLayout();

    QLabel* legendVertex = new QLabel("● Вершина (номер / стоимость)");
    legendVertex->setStyleSheet("color: #6496FF; font-weight: bold;");
    legendLayout->addWidget(legendVertex);

    QLabel* legendPheromone = new QLabel("━ Феромон (синий=мало, красный=много)");
    legendLayout->addWidget(legendPheromone);

    QLabel* legendBest = new QLabel("━ Лучший маршрут");
    legendBest->setStyleSheet("color: #00C800; font-weight: bold;");
    legendLayout->addWidget(legendBest);

    legendGroup->setLayout(legendLayout);
    rightLayout->addWidget(legendGroup);

    // Добавление панелей в основной layout
    mainLayout->addLayout(leftLayout, 1);
    mainLayout->addLayout(rightLayout, 3);
}

void MainWindow::setupConnections() {
    connect(btnGenerate, &QPushButton::clicked, this, &MainWindow::onGenerateGraph);
    connect(btnStart, &QPushButton::clicked, this, &MainWindow::onStartAlgorithm);
    connect(btnStop, &QPushButton::clicked, this, &MainWindow::onStopAlgorithm);
    connect(btnReset, &QPushButton::clicked, this, &MainWindow::onResetAlgorithm);
    connect(sliderSpeed, &QSlider::valueChanged, this, &MainWindow::onSpeedChanged);

    connect(checkShowAllEdges, &QCheckBox::stateChanged, [this]() {
        updateVisualization();
    });

    connect(checkShowBestRoute, &QCheckBox::stateChanged, [this]() {
        updateVisualization();
    });
}

void MainWindow::onGenerateGraph() {
    // Создание нового алгоритма с заданными параметрами
    if (colony) {
        delete colony;
    }

    int numVertices = spinVertices->value();
    int numAnts = spinAnts->value();
    double alpha = spinAlpha->value();
    double beta = spinBeta->value();
    double rho = spinRho->value();
    double Q = spinQ->value();
    int maxIterations = spinIterations->value();

    colony = new AntColony(numVertices, numAnts, alpha, beta, rho, Q, maxIterations);

    // Подключение сигналов
    connect(colony, &AntColony::iterationCompleted, this, &MainWindow::onIterationCompleted);
    connect(colony, &AntColony::algorithmFinished, this, &MainWindow::onAlgorithmFinished);

    // Генерация графа
    QRect viewRect = graphicsView->viewport()->rect();
    colony->generateRandomGraph(viewRect.width() - 100, viewRect.height() - 100);

    // Обновление UI
    btnStart->setEnabled(true);
    btnReset->setEnabled(true);
    labelStatus->setText("Статус: Граф сгенерирован");

    updateStatistics();
    updateVisualization();

    QMessageBox::information(this, "Успех",
                             QString("Граф с %1 вершинами успешно сгенерирован!").arg(numVertices));
}

void MainWindow::onStartAlgorithm() {
    if (!colony) {
        QMessageBox::warning(this, "Ошибка", "Сначала сгенерируйте граф!");
        return;
    }

    isRunning = true;
    btnStart->setEnabled(false);
    btnStop->setEnabled(true);
    btnGenerate->setEnabled(false);
    labelStatus->setText("Статус: Алгоритм работает...");

    // Запуск таймера
    int interval = 1100 - sliderSpeed->value() * 100; // От 1000 мс до 100 мс
    timer->start(interval);
}

void MainWindow::onStopAlgorithm() {
    isRunning = false;
    timer->stop();

    btnStart->setEnabled(true);
    btnStop->setEnabled(false);
    btnGenerate->setEnabled(true);
    labelStatus->setText("Статус: Остановлено");
}

void MainWindow::onResetAlgorithm() {
    if (!colony) return;

    timer->stop();
    isRunning = false;

    colony->reset();

    btnStart->setEnabled(true);
    btnStop->setEnabled(false);
    btnGenerate->setEnabled(true);

    updateStatistics();
    updateVisualization();

    labelStatus->setText("Статус: Алгоритм сброшен");
}

void MainWindow::onIterationStep() {
    if (!colony || !isRunning) {
        timer->stop();
        return;
    }

    colony->runIteration();
}

void MainWindow::onIterationCompleted(int iteration, double bestCost) {
    updateStatistics();
    updateVisualization();

    labelStatus->setText(QString("Статус: Итерация %1 завершена").arg(iteration));
}

void MainWindow::onAlgorithmFinished() {
    timer->stop();
    isRunning = false;

    btnStart->setEnabled(false);
    btnStop->setEnabled(false);
    btnGenerate->setEnabled(true);

    labelStatus->setText("Статус: Алгоритм завершён!");

    QMessageBox::information(this, "Завершено",
                             QString("Алгоритм завершён!\n\nЛучшая найденная стоимость: %1\nИтераций: %2")
                                 .arg(colony->getBestCost(), 0, 'f', 2)
                                 .arg(colony->getCurrentIteration()));
}

void MainWindow::onSpeedChanged(int value) {
    if (timer->isActive()) {
        int interval = 1100 - value * 100;
        timer->setInterval(interval);
    }
}

void MainWindow::updateStatistics() {
    if (!colony) return;

    labelIteration->setText(QString("Итерация: %1 / %2")
                                .arg(colony->getCurrentIteration())
                                .arg(colony->getMaxIterations()));

    if (colony->getBestCost() < std::numeric_limits<double>::max()) {
        labelBestCost->setText(QString("Лучшая стоимость: %1")
                                   .arg(colony->getBestCost(), 0, 'f', 2));
    } else {
        labelBestCost->setText("Лучшая стоимость: -");
    }
}

void MainWindow::updateVisualization() {
    if (!colony) return;

    bool showAllEdges = checkShowAllEdges->isChecked();
    bool showBestRoute = checkShowBestRoute->isChecked();

    scene->drawGraph(colony, showAllEdges, showBestRoute);
    graphicsView->fitInView(scene->sceneRect(), Qt::KeepAspectRatio);
}
