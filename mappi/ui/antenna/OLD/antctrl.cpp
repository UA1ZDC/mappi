#include "ui_antctrl.h"
#include "antctrl.h"

//#include <mappi/device/antenna/antprogon.h>

#include <cross-commons/app/paths.h>
#include <cross-commons/debug/tlog.h>
#include <commons/textproto/tprototext.h>
#include <commons/textproto/pbtools.h>

#include <meteo/commons/global/common.h>
#include <mappi/settings/mappisettings.h>
#include <mappi/global/global.h>
//#include <mappi/device/antenna/prbl-ant.h>
//#include <mappi/device/antenna/antfollow.h>

#include <unistd.h>
#include <math.h>
#include <qdatetime.h>
#include <qbitmap.h>
#include <qdir.h>
#include <qlocale.h>
#include <qmenu.h>
#include <qaction.h>

#define RECEPTION_FILE MnCommon::etcPath("mappi") + "reception.conf"

#define BAD_ANGLE 9999
#define TRACK_POINT_STEP 0.5
#define SCHEDULE_PATH           MnCommon::varPath("mappi") + "schedule.txt"

using namespace mappi;
using namespace receive;


AntCtrl::AntCtrl(QWidget* parent /*= 0*/, Qt::WindowFlags fl /*= 0*/):
  QWidget(parent, fl),
  _ui(0),
  //_ant(0),
  //_progon(0),
  _scheduleTable(new ScheduleTable(this)),
  _azCur(BAD_ANGLE),
  _elCur(BAD_ANGLE),
  _azDest(BAD_ANGLE),
  _elDest(BAD_ANGLE),
  _conf(::mappi::inter::Settings::instance()->reception())
{
  _ui = new Ui::AntCtrl;
  _ui->setupUi(this);
  _ui->powerButton->setIcon(QIcon(":/mappi/icons/connect.png"));
  _ui->powerLabel->setPixmap(QIcon(":/mappi/icons/ledgrey.png").pixmap(QSize(16,16)));
  _ui->powerLabel->setToolTip(QObject::tr("Питание не подано"));
  _ui->currBtn->setIcon(QIcon(":mappi/icons/refresh.gif"));
  _ui->currBtn->setEnabled(false);

  _ui->showCorr->setIcon(QIcon(":mappi/icons/settings.png"));
  _ui->runBtn->setIcon(QIcon(":/mappi/icons/run_proc.png"));
  _ui->showTableButton->setIcon(QIcon(":mappi/icons/go-back2.png"));
  _ui->setButton->setIcon(QIcon(":/mappi/icons/run_proc.png"));
  _ui->setButton->setToolTip(" Установить целевое положение");
  _ui->corrSpinBox->setVisible(false);
  _ui->setCorrButton->setVisible(false);

  _ui->tableLayout->insertWidget(1,_scheduleTable);


  QFile file(RECEPTION_FILE);
  if ( !file.open(QIODevice::ReadOnly) ) {
    error_log << QObject::tr("Ошибка при загрузке настроек '%1'").arg(RECEPTION_FILE);
  }
  QString text = QString::fromUtf8(file.readAll());
  file.close();

  if (TProtoText::fillProto(text, &_conffollow)) {
    //_ant = AntFollow::createAntenna(_conffollow.antenna().type());
  } else {
    error_log << QObject::tr("Ошибка в структуре файла настроек '%1'").arg(RECEPTION_FILE);
  }

  //_ant->setZeroAzimut(_conffollow.antenna().azimut_correct());
/*
  _progon = new AntProgon(_ant->dsaMax(), _ant->dseMax(), _ant->azMin(),
			  _ant->azMax(),  _ant->elMin(),  _ant->elMax());
  _track = new MnDevice::MAntCoord(_ant);
  _track->setAngleType(MnDevice::DEGREE);
*/
  _timer = new QTimer(this);
  _stopTimer = new QTimer(this);
  _nextPositionTimer = new QTimer(this);
  _coordsTimer = new QTimer(this);
  _updateTimer = new QTimer(this);

  _ui->sessionGbox->setDisabled(true);
  _ui->showTableButton->setVisible(true);
  _ui->showTableButton->setToolTip(" Скрыть расписание");
  _ui->splitter->setStretchFactor(0,0);
  _ui->splitter->setStretchFactor(1,1);

  connect(_ui->port, SIGNAL(currentIndexChanged(const QString&)), SLOT(setPort(const QString&)));
  connect(_ui->powerButton, SIGNAL(clicked(bool)), SLOT(powerChanged(bool)));
  connect(_ui->currBtn, SIGNAL(clicked()), SLOT(getCurrentCoords()));

  connect(_ui->followButton, SIGNAL(clicked(bool)),SLOT(slotFollow(bool)));
  connect(_ui->showTableButton, SIGNAL(clicked()), SLOT(slotVisibleTableBox()));

  connect(_ui->runBtn, SIGNAL(clicked(bool)), SLOT(setRun(bool)));
  connect(_stopTimer, SIGNAL(timeout()), SLOT(slotStopTimer()));
  connect(_timer, SIGNAL(timeout()), SLOT(setNextValue()));
  connect(_coordsTimer, SIGNAL(timeout()), SLOT(slotOnCoordsTimer()));
  connect(_updateTimer, SIGNAL(timeout()),SLOT(slotUpdate()));
  connect(_nextPositionTimer, SIGNAL(timeout()), SLOT(slotNextPosition()));
  connect(_ui->showCorr, SIGNAL(clicked(bool)),SLOT(slotShowCorr(bool)));
  connect(_ui->setCorrButton, SIGNAL(clicked()), SLOT(slotSetCorr()));
  connect(_ui->setButton, SIGNAL(clicked(bool)), SLOT(slotSetPosition(bool)) );

  //connect(_scheduleTable, &ScheduleTable::changeNear, this, &AntCtrl::slotChangedNear);
  //_scheduleTable->init();

  _updateTimer->setInterval(1000);
  _updateTimer->start();

  enableButtons(false);
  _ui->elDest->setEnabled(true);

  _ui->port->setCurrentIndex(_ui->port->findText(QString::fromStdString(_conf.antenna().serial_port().tty())));

  double zeroAz = 0;//_ant->zeroAzimut();
  _ui->corrSpinBox->setValue(zeroAz);
}

AntCtrl::~AntCtrl()
{
  delete _ui;
  _ui = 0;

//  delete _ant;
//  _ant = 0;

//  delete _progon;
//  _progon = 0;

  delete _stopTimer;
  _stopTimer = 0;

  delete _timer;
  _timer = 0;

  delete _coordsTimer;
  _coordsTimer = 0;

  delete _updateTimer;
  _updateTimer = 0;

  delete _nextPositionTimer;
  _nextPositionTimer = 0;

//  delete _track;
//  _track = 0;
}


void AntCtrl::slotStopTimer()
{
  _curTask = CurrentTask::kNone;
  slotOnCoordsTimer();
  _coordsTimer->stop();
  _stopTimer->stop();
}

void AntCtrl::slotShowCorr(bool isSet)
{
  if (isSet) {
    _ui->corrSpinBox->setVisible(true);
    _ui->setCorrButton->setVisible(true);
    double zeroAz = 0;//_ant->zeroAzimut();
    _ui->corrSpinBox->setValue(zeroAz);
    _ui->showCorr->setToolTip("Скрыть угол коррекции");
  }
  else {
    _ui->corrSpinBox->setVisible(false);
    _ui->setCorrButton->setVisible(false);
    _ui->showCorr->setToolTip("Показать угол коррекции");
  }
}

void AntCtrl::slotSetCorr() {
  //_ant->setZeroAzimut(_ui->corrSpinBox->value());
  _ui->corrSpinBox->setValue(0/*_ant->zeroAzimut()*/);
}

void AntCtrl::slotSetPosition(bool isSet)
{
  getCurrentCoords();
  Q_UNUSED(isSet)
/*
  if (_ui->azDest->value() < _ant->azMax()) {
    _azDest = _ui->azDest->value();
  }
  else {
    _azDest = _ui->azDest->value() - 360;
  }
  _elDest = _ui->elDest->value();
  if ( isSet ) {
    if ( std::abs(_elDest - _elCur) < 0.5 && std::abs(_azDest - _azCur) < 0.5 ) {
      _curTask = CurrentTask::kNone;
      _ui->setButton->setChecked(false);
    }
    else if (std::abs(_elDest - _elCur) < 0.5) {
      setAzimut(isSet);
      _curTask = CurrentTask::kAzimutFix;
    }
    else if (std::abs(_azDest - _azCur) < 0.5) {
      _curTask = CurrentTask::kElevationFix;
      setElevation(isSet);
    }
    else {
      bool ok = _ant->setPosition(_azDest, _elDest, false);
      if (!ok) {
        error_log << QObject::tr("Ошибка установки антенны в начальное положение");
      }
      _ui->setButton->setIcon(QIcon(":/mappi/icons/stop_proc.png"));
      _ui->setButton->setToolTip("Стоп");
      _coordsTimer->start(100);
      _stopTimer->start(50000);
      _curTask = CurrentTask::kSetPosition;
    }
  }
  else{
    setAzimut(isSet);
    setElevation(isSet);
    float angle = 0;
    if (_ant->stop(&angle, &angle)) {
      getCurrentCoords();
    }
    _coordsTimer->stop();
    _curTask = CurrentTask::kNone;
  }
*/
}


void AntCtrl::enableButtons(bool enable)
{
  _ui->currBtn->setEnabled(enable);
  _ui->runBtn->setEnabled(enable);
  _ui->followButton->setEnabled(enable);
  _ui->setButton->setEnabled(enable);
}

void AntCtrl::setPort(const QString& port)
{
  Q_UNUSED(port)
/*  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
  if (_ant->openPort(port)) {
    _ui->powerButton->setEnabled(true);
  } else {
    _ui->powerButton->setEnabled(false);
    enableButtons(false);
  }
  QApplication::restoreOverrideCursor();
*/
}

void AntCtrl::powerChanged(bool state)
{
  if ( state ) {
//    _ant->openPort(_ui->port->currentText());
//    if ( false == _ant->setPower(true) ) {
//      error_log << "Ошибка подключения";
//    }

    getCurrentCoords();
    getCurrentCoords();
    if ( _azCur == BAD_ANGLE || _elCur == BAD_ANGLE) {
      _ui->powerLabel->setPixmap(QIcon(":/mappi/icons/ledred.png").pixmap(QSize(16,16)));
      _ui->powerLabel->setToolTip(QObject::tr("Получен некорректный ответ от антенны"));
      return;
    }
    _azDest = _azCur;
    _elDest = _elCur;
    _ui->azDest->setValue(_azDest);
    _ui->elDest->setValue(_elDest);
    _ui->powerLabel->setPixmap(QIcon(":/mappi/icons/ledgreen.png").pixmap(QSize(16,16)));
    _ui->powerLabel->setToolTip(QObject::tr("Питание подано"));
    _ui->port->setEnabled(false);
    enableButtons(true);
  } else {
//    if ( false == _ant->setPower(true) ) {
//      error_log << "Ошибка подключения";
//    }
//    _ant->setPower(false);
//    _ant->closePort();
    _ui->azCur->clear();
    _azCur = BAD_ANGLE;
    _azDest = BAD_ANGLE;
    _ui->elCur->clear();
    _elCur = BAD_ANGLE;
    _elDest = BAD_ANGLE;
    _ui->powerLabel->setPixmap(QIcon(":/mappi/icons/ledgrey.png").pixmap(QSize(16,16)));
    _ui->powerLabel->setToolTip(QObject::tr("Питание не подано"));
    _ui->followButton->setChecked(false);
    slotFollow(false);
    enableButtons(false);
    _ui->port->setEnabled(true);
  }
}

void AntCtrl::getCurrentCoords()
{
  trc;
  float angle = 0;

  debug_log << "azimut";
  bool ok = false;
  for (int idx = 0; idx < 5; idx++) {
//    ok = _ant->currentAzimut(&angle);
    if (ok) break;
  }

  if (ok) {
    if ( angle > 360 ) { angle -= 360; }
    if ( angle < 0 ) { angle += 360; }
    QString text;
    text = locale().toString(angle, 'f', 1);
    _ui->azCur->setText(text);
    _azCur = angle;
  } else {
    _ui->azCur->clear();
    _azCur = BAD_ANGLE;
  }

  ok = false;
  debug_log << "elevation";
  for (int idx = 0; idx < 5; idx++) {
//    ok = _ant->currentElevat(&angle);
    if (ok) break;
  }

  if (ok) {
    if ( angle > 360 ) { angle -= 360; }
    if ( angle < 0 ) { angle += 360; }
    QString text;
    text = locale().toString(angle, 'f', 1);
    _ui->elCur->setText(text);
    _elCur = angle;
  } else {
    _ui->elCur->clear();
    _elCur = BAD_ANGLE;
  }
}


void AntCtrl::setAzimut(bool isSet)
{
  getCurrentCoords();
  _elDest = _elCur;
  _coordsTimer->start(100);
  bool setAz = isSet;
  if (isSet) {
    _ui->runBtn->setEnabled(false);
    _ui->setButton->setIcon(QIcon(":/mappi/icons/stop_proc.png"));
    _ui->setButton->setToolTip("Стоп");
/*
    if (_ui->azDest->value() < _ant->azMax()) {
      _azDest = _ui->azDest->value();
    }
    else {
      _azDest = _ui->azDest->value() - 360;
    }
*/
/*
    if (!_ant->setAzimut(_azDest)) {
      setAz = false;
      _ui->setButton->setChecked(false);
      _curTask = CurrentTask::kNone;
    }
*/
    _curTask = CurrentTask::kAzimutFix;
  } else {
    float angle = 0;
    if (true/*_ant->stop(&angle, &angle)*/) {
      QString text;
      text = locale().toString(angle, 'f', 1);
      _ui->azCur->setText(text);
      _coordsTimer->stop();
      _curTask = CurrentTask::kNone;
    }
  }

  if (!setAz) {
    _ui->setButton->setIcon(QIcon(":/mappi/icons/run_proc.png"));
    _ui->setButton->setToolTip(" Установить целевое положение");
    _ui->runBtn->setEnabled(true);
  }
}

void AntCtrl::setElevation(bool isSet)
{
  getCurrentCoords();
  _azDest = _azCur;
  _coordsTimer->start(100);
  bool setEl = isSet;
  if (isSet) {
    _ui->setButton->setIcon(QIcon(":/mappi/icons/stop_proc.png"));
    _ui->setButton->setToolTip("Стоп");
    _ui->runBtn->setEnabled(false);
    _elDest = _ui->elDest->value();
/*
    if (!_ant->setElevat(_elDest)) {
      setEl = false;
      _ui->setButton->setChecked(false);
      _curTask = CurrentTask::kNone;
    }
*/
    _curTask = CurrentTask::kElevationFix;
  } else {
//    float angle = 0;
/*
    if (_ant->stop(&angle, &angle)) {
      QString text;
      text = locale().toString(angle, 'f', 1);
      _ui->elCur->setText(text);
      _elCur = angle;
      _coordsTimer->stop();
      _curTask = CurrentTask::kNone;
    }
*/
  }

  if (!setEl) {
    _ui->setButton->setIcon(QIcon(":/mappi/icons/run_proc.png"));
    _ui->setButton->setToolTip(" Установить целевое положение");
    _ui->runBtn->setEnabled(true);
  }
}

void AntCtrl::setRun(bool isSet)
{
  getCurrentCoords();
  if (_azCur == BAD_ANGLE ||
      _elCur == BAD_ANGLE) {
    _ui->runBtn->setChecked(false);
    error_log << QObject::tr("Не определены текущие координаты");
    return;
  }

  if (isSet) {
    _ui->runBtn->setIcon(QIcon(":/mappi/icons/stop_proc.png"));
    _ui->setButton->setEnabled(false);

    //_progon->setStartPosition(_azCur, _elCur);
    getCurrentCoords();
    //_progon->setDSA(_ui->azStep->value());
    //_progon->setDSE(_ui->elStep->value());
    _timer->start(500);

    setNextValue();
  } else {
    _timer->stop();
    //float azimut = 0, elev = 0;
/*
    if (_ant->stop(&azimut, &elev)) {
      QString text = locale().toString(azimut, 'f', 1);
      _ui->azCur->setText(text);
      _azCur = azimut;

      text = locale().toString(elev, 'f', 1);
      _ui->elCur->setText(text);
      _elCur = elev;
    }
*/
    _ui->runBtn->setIcon(QIcon(":/mappi/icons/run_proc.png"));
    _ui->setButton->setEnabled(true);
  }
}

void AntCtrl::setNextValue()
{
  float az = 0;//_progon->azimut();
  float el = 0;//_progon->elevat();

  float nextAz = 0;//_progon->nextAzimut();
  float nextEl = 0;//_progon->nextElevat();

  debug_log << "** Next " << nextAz << nextEl;
/*
  if (!_ant->moveAzimut(nextAz, &_azCur)) {
    setRun(false);
    _ui->runBtn->setChecked(false);
    return;
  }
*/
/*
  if (!_ant->moveElevat(nextEl, &_elCur)) {
    setRun(false);
    _ui->runBtn->setChecked(false);
    return;
  }
*/
  QString text;
  float angle = _azCur;
  if (angle < 0)   { angle += 360;}
  if (angle > 360) { angle -= 360;}
  text = locale().toString(angle, 'f', 1);
  _ui->azCur->setText(text);
  text = locale().toString(_elCur, 'f', 1);
  _ui->elCur->setText(text);

  if (fabs(az - _azCur) > 1 ||
      fabs(el - _elCur) > 1) {
    warning_log << QObject::tr("Ошибка позиционирования. Азимут = %1, ожидаемый = %2. Угол места = %3, ожидаемый = %4")
                   .arg(_azCur).arg(az).arg(_elCur).arg(el);
  }

}

void AntCtrl::slotOnCoordsTimer()
{
  getCurrentCoords();
  float azDest =_azDest ;
  if (azDest < 0) { azDest +=360; }
  debug_log << "destination: " << azDest << _elDest << " current: " << _azCur << _elCur;
  switch(_curTask)
  {
  case CurrentTask::kAzimutFix:
    if (std::abs(azDest - _azCur) < 0.5) {
      _coordsTimer->stop();
      enableButtons(true);
      _ui->setButton->setIcon(QIcon(":/mappi/icons/run_proc.png"));
      _ui->setButton->setToolTip(" Установить целевое положение");
      _ui->setButton->setChecked(false);
      //_azDest = _azCur;
      _curTask = CurrentTask::kNone;
    }
    break;
  case CurrentTask::kElevationFix:
    if ( std::abs(_elDest - _elCur) < 0.5 ) {
      _coordsTimer->stop();
      enableButtons(true);
      _ui->setButton->setIcon(QIcon(":/mappi/icons/run_proc.png"));
      _ui->setButton->setToolTip(" Установить целевое положение");
      _ui->setButton->setChecked(false);
      _elDest = _elCur;
      _curTask = CurrentTask::kNone;
    }
    break;
  case CurrentTask::kRun:
    break;
  case CurrentTask::kSetPosition:
    if ( std::abs(_elDest - _elCur) < 0.5 && std::abs(_azDest - _azCur) < 0.5 ) {
      _coordsTimer->stop();
      _elDest = _elCur;
      _azDest = _azCur;
      _ui->setButton->setIcon(QIcon(":/mappi/icons/run_proc.png"));
      _ui->setButton->setToolTip(" Установить целевое положение");
      _ui->setButton->setChecked(false);
      _curTask = CurrentTask::kNone;
    }
    break;
  case CurrentTask::kNone:
    _ui->setButton->setIcon(QIcon(":/mappi/icons/run_proc.png"));
    _ui->setButton->setToolTip(" Установить целевое положение");
    _ui->setButton->setChecked(false);
    break;
  default:
    break;
  }
}


void AntCtrl::followForSatellite()
{

  var((*_near).data().toString());
  _ui->recvSatLbl->setText((*_near).data().satellite);
  setStartPosition(*_near, false); //TODO проверка установки антенны. но тестировать без антенны, тогда не получится
  getCurrentCoords();

  int timeout = QDateTime::currentDateTimeUtc().secsTo((*_near).data().aos);
  if ( timeout <= 0 ) {
    _nextPositionTimer->start(5);
  }
  else {
    _nextPositionTimer->start(timeout * 1000);
  }
}


bool AntCtrl::setStartPosition(const schedule::Session& session, bool wait)
{
  Q_UNUSED(wait)
  //_ant->setPower(true);

  if (!getTrackSatellite(session)) {
    error_log.msgBox().operator <<("Ошибка получения траектории. Проверьте TLE файл");
    slotFollow(false);
    _ui->followButton->setChecked(false);
    return false;
  }

  //_trackIdx = 0;
//  _azDest = _track->startPointAzimut(_ant->zeroAzimut());
//  _elDest = (*_track)[_trackIdx].elevat();
//  ++_trackIdx;
//  debug_log << "start pos = " << _track->dt(0).toString("hh:mm:ss.zzz") << _azDest << _elDest << "azCorr=" << _ant->zeroAzimut();

bool ok = true;//_ant->setPosition(_azDest, _elDest, wait);
//if (!ok) {
//  error_log << QObject::tr("Ошибка установки антенны в начальное положение");
//}
  _coordsTimer->start(100);
  _stopTimer->start(50000);
  _curTask = CurrentTask::kSetPosition;
  return ok;
}



void AntCtrl::setNextPosition()
{
  _nextPositionTimer->stop();
//  int64_t mtcount = _track -> count();

  QDateTime cur = QDateTime::currentDateTimeUtc();
/*
  while (_trackIdx < mtcount &&
         cur.msecsTo(_track->dt(_trackIdx)) < TRACK_POINT_STEP*500) {
    ++_trackIdx;
  }
*/
/*
  if ( _trackIdx < mtcount ) {
    MnDevice::MAntPoint point = ( *_track )[ _trackIdx ];
    _ant->moveAzimut(point.azimut(), &_azCur);
    _ant->moveElevat(point.elevat(), &_elCur);

    if (_trackIdx % 10 == 0) {
    
      debug_log << _trackIdx << "(" << mtcount << ")"
                << _track->dt(_trackIdx - 1).toString("hh:mm:ss.zzz") << _azDest<< _elDest
                << QDateTime::currentDateTimeUtc().toString("hh:mm:ss.zzz") << _azCur << _elCur << "|" << fabs(_azDest  - _azCur) << fabs(_elDest - _elCur);
    }

    _elDest = point.elevat();
    _azDest = point.azimut();

    ++_trackIdx;
    _nextPositionTimer->start(TRACK_POINT_STEP * 1000);
    //emit positionChanged();
  }
  else {
    followForSatellite();
  }
*/
}

bool AntCtrl::getTrackSatellite(const schedule::Session& session)
{
  Satellite satellite;
  QDateTime timeCompare = QDateTime::currentDateTimeUtc();
  _tle = meteo::global::findWeatherFile(pbtools::toQString(_conf.file_storage().tle()), timeCompare);
  debug_log << _tle;
  bool is_OK = satellite.readTLE(session.data().satellite, _tle);
  if ( is_OK ) {
    QDateTime dt = QDateTime::currentDateTime();
    dt = QDateTime::currentDateTimeUtc();
    QList< MnSat::TrackTopoPoint > track;
    is_OK = satellite.trajectory(( session.data().aos < dt ? dt : session.data().aos ),
                                 session.data().los,
                                 TRACK_POINT_STEP,
                                 _stationCoord,
                                 track);

    if ( is_OK ) {
//      _track->setTrack( track );
    } else {
      error_log << QObject::tr( "Траектория спутника не определена" );
    }
  }

  return is_OK;
}

void AntCtrl::slotFollow(bool isSet)
{
  if (true == isSet) {
    _ui->sessionGbox->setDisabled(false);
    _ui->setButton->setDisabled(true);
    if (_ui->setButton->isChecked()) {
      _ui->setButton->setChecked(false);
      _ui->setButton->setIcon(QIcon(":/mappi/icons/run_proc.png"));
      _ui->setButton->setToolTip("Установить целевое положение");
    }
    _ui->runBtn->setDisabled(true);
    QFile file(RECEPTION_FILE);
    if ( !file.open(QIODevice::ReadOnly) ) {
      error_log << QObject::tr("Ошибка при загрузке настроек '%1'").arg(RECEPTION_FILE);
    }

    QString text = QString::fromUtf8(file.readAll());
    file.close();

    if ( !TProtoText::fillProto(text, &_conffollow) ) {
      error_log << QObject::tr("Ошибка в структуре файла настроек '%1'").arg(RECEPTION_FILE);
    }

    _stationCoord =  meteo::GeoPoint(_conffollow.site().point().lat_radian(),
				     _conffollow.site().point().lon_radian(),
				     _conffollow.site().point().height_meters());

    followForSatellite();
    //_updateTimer->start();
  }
  else {
    _ui->sessionGbox->setDisabled(true);
    _nextPositionTimer->stop();
    //_updateTimer->stop();
    _ui->runBtn->setDisabled(false);
    _ui->setButton->setDisabled(false);
  }
}


void AntCtrl::slotNextPosition()
{
  setNextPosition();
}

void AntCtrl::slotUpdate()
{
  if(nullptr != _near) {
    //Сеанс еще идет/
    if ((*_near).stage() == schedule::Session::PROCEED)
    {
      _ui->sessionTimerLbl->setText(QTime::fromMSecsSinceStartOfDay(_near->secsToLos() * 1000).toString("hh:mm:ss"));
      _ui->sessionGbox->setTitle("Текущая сессия");
      _ui->sessionTimerTitleLbl->setText("До окончания: ");
    }
    //Сеанс еще не начался
    else {
      _ui->sessionGbox->setTitle("Ближайшая сессия");
      _ui->sessionTimerTitleLbl->setText("До начала: ");
      _ui->sessionTimerLbl->setText(QTime::fromMSecsSinceStartOfDay((*_near).secsToAos()*1000).toString("hh:mm:ss"));
    }
    if (_ui->followButton->isChecked()) {
      _ui->elDest->setValue(_elDest);
      float value =_azDest;
      if (value < 0) { value += 360; }
      if (value > 360) { value -= 360; }
      _ui->azDest->setValue(value);
    }
    QString text;
    text = locale().toString(_elCur, 'f', 1);
    _ui->elCur->setText(text);
    float value = _azCur;
    if (value < 0) { value += 360; }
    if (value > 360) { value -= 360; }
    text = locale().toString(value, 'f', 1);
    _ui->azCur->setText(text);
  }
}


void AntCtrl::slotChangedNear(std::shared_ptr<schedule::Session>  near)
{
  if(nullptr != near) {
    _near = near;
    _ui->recvSatLbl->setText((*_near).data().satellite);
  }
  else
  {
    _ui->sessionGbox->setTitle(QObject::tr("Ближайшая сессия"));
    _ui->recvSatLbl->setText("");
    _ui->sessionTimerTitleLbl->setText(QObject::tr("До начала:"));
    _ui->sessionTimerLbl->setText("");
  }
}



void AntCtrl::slotVisibleTableBox()
{
  if( true == _ui->tableBox->isVisible()) {
    _ui->tableBox->setVisible(false);
    _ui->showTableButton->setToolTip("Показать расписание");
    _ui->showTableButton->setIcon(QIcon(":mappi/icons/go-next.png"));
  }
  else {
    _ui->tableBox->setVisible(true);
    _ui->showTableButton->setToolTip("Скрыть расписание");
    _ui->showTableButton->setIcon(QIcon(":mappi/icons/go-back2.png"));
  }
}
