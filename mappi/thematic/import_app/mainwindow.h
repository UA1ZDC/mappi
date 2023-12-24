#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QFile>
#include <QMessageBox>
#include <QFileDialog>

#include "import.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    void updatePicture();
    void currentIndexChanged(mappi::Import::ColorChannel color, int currentIndex);
private slots:
    void on_btnOpen_clicked();
    void on_btnSave_clicked();
    void on_cmbRed_currentIndexChanged(int currentIndex);
    void on_cmbGreen_currentIndexChanged(int currentIndex);
    void on_cmbBlue_currentIndexChanged(int currentIndex);
    void on_cmbAlpha_currentIndexChanged(int currentIndex);
    void on_cmbFormat_currentIndexChanged(int currentIndex);

private:
    Ui::MainWindow *ui;
    mappi::Import::GDALImporter importer;
};

#endif // MAINWINDOW_H
