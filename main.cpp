#include "mainwindow.h"
#include <QApplication>
#include <QLocale>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Установка локали для корректного отображения чисел
    QLocale::setDefault(QLocale::C);

    MainWindow w;
    w.setWindowTitle("Задача коммивояжёра - Алгоритм муравьиной колонии");
    w.resize(1400, 800);
    w.show();

    return a.exec();
}
