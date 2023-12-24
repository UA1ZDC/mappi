#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QFile>
#include <QMessageBox>
#include <QFileDialog>
#include <QTableWidget>
#include <QList>

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
    void move(bool up);
    QList<QTableWidgetItem *> takeRow(int row);
    void setRow(int row, const QList<QTableWidgetItem *> &rowItems);
    void updateButtons();
private slots:
    void on_btnAdd_clicked();
    void on_btnUp_clicked();
    void on_btnDown_clicked();
    void on_btnDelete_clicked();
    void on_actionSave_triggered();
    void on_actionAdd_triggered();
    void on_actionClear_triggered();
    void on_tableWidget_itemSelectionChanged();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
