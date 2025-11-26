#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>
#include <QMutex>
#include <atomic>
#include "collatz.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_exitButton_clicked();
    void on_startButton_clicked();
    void on_stopButton_clicked();
    void on_threadsSlider_sliderMoved(int value);

private:
    Ui::MainWindow *ui;

    CollatzSolver solver_;
    QThread* workerThread_ = nullptr;
    std::atomic<bool> stopRequested_;
    QMutex mutex_; // если понадобится синхронизация UI
};

#endif // MAINWINDOW_H
