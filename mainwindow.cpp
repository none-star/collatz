#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QToolTip>
#include <QCursor>
#include <thread>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    stopRequested_ = false;

    ui->stopButton->setEnabled(false);

    uint hw = std::thread::hardware_concurrency();

    ui->threadsSlider->setMaximum(hw);
    ui->threadsSlider->setValue(hw);
    ui->threadsMaxLabel->setText(QString::number(hw));
}

MainWindow::~MainWindow()
{
    if (workerThread_)
    {
        solver_.request_stop();
        if (workerThread_->isRunning())
        {
            workerThread_->quit();
            workerThread_->wait();
        }

        delete workerThread_;
    }
    delete ui;
}


void MainWindow::on_exitButton_clicked()
{
    close();
}

void MainWindow::on_startButton_clicked()
{
    ui->startButton->setEnabled(false);
    ui->stopButton->setEnabled(true);
    ui->resTextEdit->clear();
    stopRequested_ = false;

    if (workerThread_)
    {
        if (workerThread_->isRunning())
        {
            workerThread_->wait();
        }
        delete workerThread_;
        workerThread_ = nullptr;
    }

    uint64_t limit = static_cast<uint64_t>(ui->numberSpinBox->value());
    unsigned numThreads = static_cast<unsigned>(ui->threadsSlider->value());

    workerThread_ = QThread::create([this, limit, numThreads]()
    {

        solver_.compute(limit, numThreads);

        QMetaObject::invokeMethod(this, [this]()
        {
            ui->resTextEdit->appendPlainText(QString::fromStdString(solver_.message()));
            ui->startButton->setEnabled(true);
            ui->stopButton->setEnabled(false);
        });
    });
    workerThread_->start();
}

void MainWindow::on_stopButton_clicked()
{
    solver_.request_stop();
    stopRequested_ = true;
    ui->resTextEdit->appendPlainText("Stop requested...");
    ui->stopButton->setEnabled(false);
}

void MainWindow::on_threadsSlider_sliderMoved(int position)
{
    QToolTip::showText(QCursor::pos(), QString::number(position), ui->threadsSlider);
}


