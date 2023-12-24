#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QFile>
#include <QMessageBox>
#include <QFileDialog>
#include <sql/dbi/dbi.h>

#define RECEPTION_FILE MnCommon::etcPath("mappi") + "reception.conf"

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
  bool loadSessions();
  bool loadGeoSats();
private slots:
  void slotTypeChanged();
  void slotShowMapInfo();

private:
    Ui::MainWindow *ui;
    std::unique_ptr<meteo::Dbi> db_;
    QStringList geoSats_;
};

#endif // MAINWINDOW_H
