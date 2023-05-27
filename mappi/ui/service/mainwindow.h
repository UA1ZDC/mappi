#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "include/settingsdialog.h"
#include "include/console.h"
#include "include/mytimer.h"

#include "vkaprotocol.h"
#include "plot.h"

#include <qwt_plot.h>
#include <qwt_plot_grid.h>

#include <qwt_legend.h>

#include <qwt_plot_curve.h>
#include <qwt_symbol.h>

#include <qwt_plot_magnifier.h>

#include <qwt_plot_panner.h>

#include <qwt_plot_picker.h>
#include <qwt_picker_machine.h>
#include <qwt_system_clock.h>

#include <QMainWindow>
#include <QLabel>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();


private slots:
    void writeSettings();
    void timerAction();

private:
    Ui::MainWindow *ui = nullptr;

//signals :
    void recv(const mappi::antenna::Drive& az,const mappi::antenna::Drive& el);
   // void initQwt();
    void initActionsConnections();
    void showStatusMessage(QStatusBar * pstatusBar, const QString &message);

    void handleError(QSerialPort::SerialPortError error);
    void closeSerialPort();
    void openSerialPort();

    void readData();
    void writeData(const QByteArray &data);

    void replaceTab(QTabWidget * tabs, int index,
                    QWidget * replacement, const QString & label = QString());

    void terminalTabFill();


    void splitData();

    SettingsDialog * m_settings = nullptr;
    QSerialPort * m_serial = nullptr;
    Console * m_console = nullptr;
    myTimer * m_timer = nullptr;

    mappi::antenna::Protocol* protocol_=nullptr;

    QByteArray buf_rx_;

    QPolygonF points_;

    Plot *d_plot_az = nullptr;
    Plot *d_plot_el = nullptr;
    Plot *d_delta_plot_az = nullptr;
    Plot *d_delta_plot_el = nullptr;

    int i_ =0;
};
#endif // MAINWINDOW_H
