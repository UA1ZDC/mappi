#include "ctrl.h"

#include <qmutex.h>
#include <qtimer.h>

#include <cross-commons/debug/tlog.h>
#include <sat-commons/satellite/satellite.h>

namespace mappi {
namespace receive {

static constexpr int kCheckTimeInterval = 5000; // мс

Ctrl::Ctrl(const QString& app)
  : editMutex_(new QMutex())
  , timer_(new QTimer(this))
  , app_(app)
{
  connect(timer_, &QTimer::timeout,  this, &Ctrl::adjustFreq);
  connect(this,   &Ctrl::starSignal, this, &Ctrl::startControl);
  connect(this,   &Ctrl::stopSignal, this, &Ctrl::stopControl);
}

Ctrl::~Ctrl()
{
  if (pyapp_->state() == QProcess::Running) {
    pyapp_->terminate();
    pyapp_->waitForFinished(3000);
  }
  delete editMutex_;
  delete satellite_;
}

bool Ctrl::setAdjustFreq(const Coords::GeoCoord& site)
{
  if(false == editMutex_->tryLock(1000)) return false;
  adjustFreq_ = true;
  site_ = site;
  editMutex_->unlock();
  return true;
}

bool Ctrl::init(const MnSat::STLEParams& stle, float freq, float gain, float rate)
{
  if(false == editMutex_->tryLock(1000)) return false;
  pyapp_ = new QProcess(this);
  satellite_ = new Satellite();
  freq_ = freq;
  gain_ = gain;
  rate_ = rate;
  editMutex_->unlock();
  satellite_->readTLE(stle);
  return true;
}

bool Ctrl::isWorking() { return working_; }

bool Ctrl::start()
{
  if(false == editMutex_->tryLock(1000)) return false;
  Q_EMIT(starSignal());
  return true;
}
bool Ctrl::startControl()
{ 
  if (pyapp_->state() == QProcess::Running) {
    pyapp_->terminate();
    pyapp_->waitForFinished();
  }

  if (pyapp_->state() != QProcess::Running)
  {
    QStringList args;
    args  << QString("-f %1").arg(freq_)
          << QString("-r %1").arg(rate_)
          << QString("-g %1").arg(gain_)
          << QString("-n %1").arg(satellite_->name());

    var(args);
    // var(_app);
    pyapp_->start(app_, args);

    if (false == pyapp_->waitForStarted()) {
      error_log << QObject::tr("Ошибка запуска (%1)").arg(pyapp_->errorString());
      editMutex_->unlock();
      return false;
    }
  }
  working_ = true;
  connect(pyapp_, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), this, &Ctrl::appFinished);
  
  if (adjustFreq_)
  {
    timer_->start(kCheckTimeInterval);
  }

  connect(pyapp_, &QProcess::readyReadStandardOutput, this, &Ctrl::readOutput);
  connect(pyapp_, &QProcess::readyReadStandardError,  this, &Ctrl::readOutput);

  return true;
}


void Ctrl::stop()
{
  Q_EMIT(stopSignal());
}
void Ctrl::stopControl()
{
  timer_->stop();

  if (pyapp_->state() == QProcess::Running)
  {
    pyapp_->write("quit\n"); //TODO в заглушке это не обрабатывается, в реальном тоже закомментировано
    
    if (!pyapp_->waitForFinished(3000)) {
      pyapp_->terminate();
      pyapp_->waitForFinished(3000);
    }
  }
  working_ = false;
  editMutex_->unlock();
  Q_EMIT(finished());
}

//! Подстройка частоты
void Ctrl::adjustFreq()
{
  if(pyapp_->state() != QProcess::Running) {
    return;
  }

  QDateTime dt = QDateTime::currentDateTimeUtc();
  Coords::TopoCoord topo;
  bool ok = satellite_->getPosition(dt, site_, &topo);
  if (!ok)
  {
    QObject::tr("Ошибка получения радиальной скорости спутника");
    timer_->stop();
    return;
  }

  //f = fс + fq; fq = V / lambda; lambda = c / fс; = > f = fс - V * fс / c = fc * (1 - V/c);
  //fс - частота исходная, V - радиальная скорость, lambda - длина волны
  static const uint64_t kc = 299792458; //! скорость света
  float freq = freq_ * (1 - topo.rangeRate * 1000/kc);

  
  QString cmd = QString("freq = %1\n").arg(freq);
  pyapp_->write(cmd.toUtf8());

  // debug_log << "freq=" << (_freq/1e6) << "freq_n=" << (freq /1e6)
  //           << "dr" << topo.rangeRate;
}

void Ctrl::readOutput()
{
  QString err = pyapp_->readAllStandardError();
  var(err);

 //  QRegExp rx("Ctrl Error:([\\w\\s]+)\n");

 //  int pos = 0;
  
 //  while ((pos = rx.indexIn(err, pos)) != -1) {
 //    error_log << QObject::tr("Ошибка изменения параметров приёма: %1").arg(rx.cap(1).trimmed());
 //    pos += rx.matchedLength();
 // }


  QString out = pyapp_->readAllStandardOutput();
  // var(out);
  // return;


  QRegExp rxo("Ctrl Output:([\\w\\s]+)\n");

  int pos = 0;
  while ((pos = rxo.indexIn(out, pos)) != -1) {
    debug_log << QObject::tr("%1").arg(rxo.cap(1).trimmed());
    pos += rxo.matchedLength();
  }

}

void Ctrl::appFinished(int code, QProcess::ExitStatus stat)
{
  debug_log << "receiver finished code" << code << "status" << stat;
  working_ = false;
  editMutex_->unlock();
  Q_EMIT(finished());
}

} // receive
} // mappi
