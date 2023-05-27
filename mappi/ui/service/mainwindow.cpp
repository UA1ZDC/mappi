#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "include/console.h"
#include "include/mytimer.h"

#include "circularbuffer.h"
#include <qwt_plot_canvas.h>

#include <QMessageBox>
#include <QSplitter>
#include <QGraphicsView>
#include <QWidget>
#include <QGroupBox>

#include <QDebug>

static const int MAX_BUFFER_SIZE = 8192;  // bytes
static const int PERIOD_DRAW = 10; //мсек
static const int KOL_POINT = 5000;  // количество точек на графике с интервалом в PERIOD_DRAW

MainWindow::MainWindow(QWidget *parent)
  : QMainWindow(parent)
  , ui(new Ui::MainWindow)
  , m_settings(new SettingsDialog(this))
  , m_serial(new QSerialPort(this))
  , m_console(new Console())
  , m_timer(new myTimer(this))
{
  ui->setupUi(this);

  initActionsConnections();

  m_console->setEnabled(false);
  m_console->setLocalEchoEnabled(true);

  terminalTabFill();

  showStatusMessage(ui->statusbar, tr("Disconnected"));

  connect(m_serial, &QSerialPort::readyRead, this, &MainWindow::readData);
  connect(m_console, &Console::getData, this, &MainWindow::writeData);


  double period = PERIOD_DRAW; //мсек
  double numpoint = KOL_POINT;
  d_plot_az = new Plot(this, -270, 270,period,numpoint);
  d_plot_el = new Plot(this, 0, 110,period,numpoint);

  d_delta_plot_az = new Plot(this, -4, 4,period,numpoint);
  d_delta_plot_el = new Plot(this, -4, 4,period,numpoint);

  ui->tabWidget_2->addTab(d_plot_az,QObject::tr("Азимут"));
  ui->tabWidget_2->addTab(d_plot_el,QObject::tr("Угол места"));
  ui->tabWidget_2->addTab(d_delta_plot_az,QObject::tr("Азимут (дельта)"));
  ui->tabWidget_2->addTab(d_delta_plot_el,QObject::tr("Угол места (дельта)"));

  //m_timer->setInterval(500);
  //m_timer->start();
}

MainWindow::~MainWindow()
{
  delete protocol_;
  delete d_plot_el;
  delete d_plot_az;
  delete d_delta_plot_az;
  delete d_delta_plot_el;

  delete ui;
  delete m_console;
  delete m_serial;
  delete m_settings;
  delete m_timer;
}

void MainWindow::initActionsConnections()
{
  connect(ui->actionSerialSettings, &QAction::triggered, m_settings, &SettingsDialog::show);
  connect(m_settings, &SettingsDialog::settingsApplied, ui->actionSerialConnect, &QAction::setEnabled);
  connect(m_settings, &SettingsDialog::settingsApplied, this, &MainWindow::writeSettings);
  connect(ui->actionConsoleClear, &QAction::triggered, m_console, &Console::clear);

  connect(ui->actionSerialConnect, &QAction::triggered, this, &MainWindow::openSerialPort);
  connect(ui->actionSerialDisconnect, &QAction::triggered, this, &MainWindow::closeSerialPort);

  connect(m_serial, &QSerialPort::errorOccurred, this, &MainWindow::handleError);
  //connect(m_timer, SIGNAL(&myTimer::timeout), m_timer, SLOT(&myTimer::timerAction));
  connect(m_timer, &myTimer::timeout, this, &MainWindow::timerAction);

  connect(ui->actionTimerStart, &QAction::triggered, m_timer, &myTimer::timerStart);
}

void MainWindow::showStatusMessage(QStatusBar * statusBar, const QString &message)
{
  statusBar->QStatusBar::showMessage(message);
}

void MainWindow::writeSettings()
{
  QSettings settings("MySoft", "terminal2");
  SettingsDialog::Settings _t_settings = m_settings->settings();

  settings.beginGroup("SerialSettings");
  settings.setValue("name", _t_settings.name);
  settings.setValue("baudRate", _t_settings.baudRate);
  settings.setValue("dataBits", _t_settings.dataBits);
  settings.setValue("parity", _t_settings.parity);
  settings.setValue("stopBits", _t_settings.stopBits);
  settings.setValue("flowControl", _t_settings.flowControl);
  settings.setValue("localEchoEnabled", _t_settings.localEchoEnabled);
  settings.endGroup();
}

void MainWindow::handleError(QSerialPort::SerialPortError error)
{
  if (error == QSerialPort::ResourceError) {
    QMessageBox::critical(this, tr("Critical Error"), m_serial->errorString());
    closeSerialPort();
  }
}

void MainWindow::closeSerialPort()
{
  if (m_serial->isOpen())
    m_serial->close();
  //m_console->setEnabled(false);
  ui->actionSerialConnect->setEnabled(true);
  ui->actionSerialDisconnect->setEnabled(false);
  ui->actionSerialSettings->setEnabled(true);
  showStatusMessage(ui->statusbar, tr("Disconnected"));
}

void MainWindow::openSerialPort()
{
  const SettingsDialog::Settings p = m_settings->settings();
   m_serial->setPortName(p.name);

  //m_serial->setPortName("/home/gotur/dev/ttyNET0");

  m_serial->setBaudRate(p.baudRate);
  m_serial->setDataBits(p.dataBits);
  m_serial->setParity(p.parity);
  m_serial->setStopBits(p.stopBits);
  m_serial->setFlowControl(p.flowControl);
  if (m_serial->open(QIODevice::ReadWrite)) {
    m_console->setEnabled(true);
    m_console->setLocalEchoEnabled(p.localEchoEnabled);
    ui->actionSerialConnect->setEnabled(false);
    ui->actionSerialDisconnect->setEnabled(true);
    ui->actionSerialSettings->setEnabled(false);
    showStatusMessage(ui->statusbar, tr("Connected to %1 : %2, %3, %4, %5, %6")
                      .arg(p.name).arg(p.stringBaudRate).arg(p.stringDataBits)
                      .arg(p.stringParity).arg(p.stringStopBits).arg(p.stringFlowControl));
  } else {
    QMessageBox::critical(this, tr("Error"), m_serial->errorString());
    showStatusMessage(ui->statusbar, tr("Open error"));
  }

  m_console->clear();
  m_console->printPromt();
  protocol_ = new mappi::antenna::VkaProtocol;

}


void MainWindow::splitData( )
{

  int mtu = protocol_->mtu();
  while (mtu <= buf_rx_.size()) {
    if (protocol_->split(buf_rx_) &&
        protocol_->split(buf_rx_, mtu)) {
      QByteArray pack = buf_rx_.left(mtu);
      // debug_log << "RX:" << (protocol_->isText() ? pack : pack.toHex());
      QString error;
      mappi::antenna::Drive azimut_;
      mappi::antenna::Drive elevat_;
      if (protocol_->unpack(pack, &azimut_, &elevat_, &error) == false)
      {
        qDebug() << error;
      } else {
        //qDebug() << azimut_.self<< elevat_.self;
        recv(azimut_, elevat_);
      }
      buf_rx_.remove(0, mtu);

      continue ;
    }

    buf_rx_.remove(0, 1);
  }

  if (MAX_BUFFER_SIZE < buf_rx_.size()) {
    qDebug() << buf_rx_.toHex();
    qDebug() << QObject::tr("RX: переполнение буфера, буфер будет очищен");
    buf_rx_.clear();
  }
}


void MainWindow::recv(const mappi::antenna::Drive& az,const mappi::antenna::Drive& el )
{
  d_plot_az->addPoint(az.self);
  d_plot_el->addPoint(el.self);

  d_delta_plot_az->addPoint(az.dst);
  d_delta_plot_el->addPoint(el.dst);


}

void MainWindow::readData()
{
  const QByteArray data = m_serial->readAll();
  buf_rx_.append(data);
  m_console->putData(data);

  splitData();

}

void MainWindow::writeData(const QByteArray &data)
{
  m_serial->write(data);
}

void MainWindow::timerAction()
{
  /*if(m_serial->isOpen())
  {
    m_serial->write("hello world from my little app!\n");
  }*/
 //d_plot_az->addPoint(100.*sin(i_*0.1));
// d_plot_el->addPoint(10.*cos(i_*0.1));
 //i_++;
}

void MainWindow::replaceTab(QTabWidget * tabs, int index,
                            QWidget * replacement, const QString & label)
{
  Q_ASSERT(tabs && tabs->count() > index);
  tabs->removeTab(index);
  if (replacement) tabs->insertTab(index, replacement, label);

  tabs->setCurrentIndex(index);
}

void MainWindow::terminalTabFill()
{
  ui->splitter->replaceWidget(0, m_console);
  m_console->show();
}




