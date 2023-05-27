#include "ui_smallant.h"
#include "smallantwidget.h"

#include <mappi/device/antenna/antprogon.h>

#include <cross-commons/app/paths.h>
#include <cross-commons/debug/tlog.h>
#include <commons/textproto/tprototext.h>
#include <commons/textproto/pbtools.h>

#include <meteo/commons/global/common.h>
#include <mappi/settings/mappisettings.h>
#include <mappi/global/global.h>
#include <mappi/device/antenna/suzhetant.h>
#include <mappi/device/antenna/cafant.h>

#include <unistd.h>
#include <math.h>
#include <qdatetime.h>
#include <qbitmap.h>
#include <qdir.h>
#include <qlocale.h>
#include <qmenu.h>
#include <qaction.h>
#include <qevent.h>
#include <qpainter.h>

#define RECEPTION_FILE MnCommon::etcPath("mappi") + "reception.conf"
#define ANTCIRCLE_FILE MnCommon::sharePath("mappi") + "antcircle.png"

#define BAD_ANGLE 9999
// #define TRACK_POINT_STEP 0.5
// #define SCHEDULE_PATH           MnCommon::varPath("mappi") + "schedule.txt"

using namespace mappi;
using namespace receive;


SmallAntWidget::SmallAntWidget(QWidget* parent /*= 0*/, Qt::WindowFlags fl /*= 0*/):
  QWidget(parent, fl),
  _ui(0),
  _ant(0),
  _progon(0),
  _azCur(BAD_ANGLE),
  _elCur(BAD_ANGLE),
  _azDest(BAD_ANGLE),
  _elDest(BAD_ANGLE)
{
  _ui = new Ui::SmallAntWidget;
  _ui->setupUi(this);
  _ui->powerButton->setIcon(QIcon(":/mappi/icons/connect.png"));
  _ui->powerLabel->setPixmap(QIcon(":/mappi/icons/ledgrey.png").pixmap(QSize(16,16)));
  _ui->powerLabel->setToolTip(QObject::tr("Питание не подано"));
  _ui->currBtn->setIcon(QIcon(":mappi/icons/refresh.gif"));
  _ui->currBtn->setEnabled(false);

  _ui->runBtn->setIcon(QIcon(":/mappi/icons/run_proc.png"));
  _ui->setButton->setIcon(QIcon(":/mappi/icons/run_proc.png"));
  _ui->setButton->setToolTip(" Установить целевое положение");

  _ui->azInc->setIcon(QIcon(":/mappi/icons/right-arrow.png"));
  _ui->azDec->setIcon(QIcon(":/mappi/icons/left-arrow.png"));
  _ui->elInc->setIcon(QIcon(":/mappi/icons/arrow-up.png"));
  _ui->elDec->setIcon(QIcon(":/mappi/icons/arrow-down.png"));

  // _ant = new SmallAnt;
  _ant = new CafAnt;
  _progon = new AntProgon(_ant->dsaMax(), _ant->dseMax(), _ant->azMin(),
			  _ant->azMax(),  _ant->elMin(),  _ant->elMax() - 5);
  _timer = new QTimer(this);
  _stopTimer = new QTimer(this);
  // _nextPositionTimer = new QTimer(this);
  _coordsTimer = new QTimer(this);
  // _updateTimer = new QTimer(this);

  setCircle();
  setCirclePoint();

  // _ui->azDest->installEventFilter(this);
  // _ui->elDest->installEventFilter(this);
  // _ui->azStep->installEventFilter(this);
  // _ui->elStep->installEventFilter(this);
  // _ui->azCur->installEventFilter(this);
  // _ui->elCur->installEventFilter(this);
  // _ui->port->installEventFilter(this);
  
  connect(_ui->port, SIGNAL(currentIndexChanged(const QString&)), SLOT(setPort(const QString&)));
  connect(_ui->powerButton, SIGNAL(clicked(bool)), SLOT(powerChanged(bool)));
  connect(_ui->currBtn, SIGNAL(clicked()), SLOT(getCurrentCoords()));

  connect(_ui->runBtn, SIGNAL(clicked(bool)), SLOT(setRun(bool)));
  connect(_stopTimer, SIGNAL(timeout()), SLOT(slotStopTimer()));
  connect(_timer, SIGNAL(timeout()), SLOT(setNextValue()));
  connect(_coordsTimer, SIGNAL(timeout()), SLOT(slotOnCoordsTimer()));
  // connect(_updateTimer, SIGNAL(timeout()),SLOT(slotUpdate()));
  // connect(_nextPositionTimer, SIGNAL(timeout()), SLOT(slotNextPosition()));
  // connect(_ui->showCorr, SIGNAL(clicked(bool)),SLOT(slotShowCorr(bool)));
  // connect(_ui->setCorrButton, SIGNAL(clicked()), SLOT(slotSetCorr()));
  connect(_ui->setButton, SIGNAL(clicked(bool)), SLOT(slotSetPosition(bool)) );
  connect(_ui->azInc, SIGNAL(clicked(bool)), SLOT(slotAzInc()));
  connect(_ui->azDec, SIGNAL(clicked(bool)), SLOT(slotAzDec()));
  connect(_ui->elInc, SIGNAL(clicked(bool)), SLOT(slotElInc()));
  connect(_ui->elDec, SIGNAL(clicked(bool)), SLOT(slotElDec()));
  connect(_ui->travelBtn, SIGNAL(clicked(bool)), SLOT(slotSetTravel()));
  connect(_ui->workBtn, SIGNAL(clicked(bool)), SLOT(slotSetWork()));
  connect(_ui->azDest, SIGNAL(valueChanged(double)), SLOT(setCirclePoint()));
  connect(_ui->elDest, SIGNAL(valueChanged(double)), SLOT(setCirclePoint()));
  connect(_ui->azCur, SIGNAL(textChanged(const QString &)), SLOT(setCirclePoint()));
  connect(_ui->elCur, SIGNAL(textChanged(const QString &)), SLOT(setCirclePoint()));

  // _updateTimer->setInterval(1000);
  // _updateTimer->start();

  enableButtons(false);
  _ui->elDest->setEnabled(true);

  QFile file(RECEPTION_FILE);
  if ( !file.open(QIODevice::ReadOnly) ) {
    error_log << QObject::tr("Ошибка при загрузке настроек '%1'").arg(RECEPTION_FILE);
  }

  QString text = QString::fromUtf8(file.readAll());
  file.close();

  if ( !TProtoText::fillProto(text, &_conf) ) {
    error_log << QObject::tr("Ошибка в структуре файла настроек '%1'").arg(RECEPTION_FILE);
  }
  _ui->port->setCurrentIndex(_ui->port->findText(QString::fromStdString(_conf.antenna().port())));
  _ant->setZeroAzimut(_conf.antenna().azimut_corr());
  //debug_log << _conf.antenna().port() << _conf.antenna().azimut_corr();

  // grabKeyboard();
}

SmallAntWidget::~SmallAntWidget()
{
  delete _ui;
  _ui = 0;

  delete _ant;
  _ant = 0;

  delete _progon;
  _progon = 0;

  delete _stopTimer;
  _stopTimer = 0;

  delete _timer;
  _timer = 0;

  delete _coordsTimer;
  _coordsTimer = 0;

  // delete _updateTimer;
  // _updateTimer = 0;

  // delete _nextPositionTimer;
  // _nextPositionTimer = 0;
  delete _pix;
  _pix = 0;

}

void SmallAntWidget::setCircle()
{
  _pix = new QPixmap();
 _ui->pict->installEventFilter(this);
   
  bool ok = _pix->load(ANTCIRCLE_FILE);
  if (!ok) {
    error_log << QObject::tr("Не найден файл %1").arg(ANTCIRCLE_FILE);
    return;
  }

  _scale_az = _pix->width() / float(_ui->pict->width());
  _scale_el = _pix->width() / float(_ui->pict->width());
  // debug_log << _pix->width() / float(_ui->pict->width()) << _pix->height() / float(_ui->pict->height());
  
  _ui->pict->setScaledContents(true);
  _ui->pict->setPixmap(*_pix);
}

bool SmallAntWidget::eventFilter(QObject* obj, QEvent* ev)
{
  if (_ui->currBtn->isEnabled() &&
      _curTask == CurrentTask::kNone &&
      ev->type() == QEvent::MouseButtonRelease &&
      obj == _ui->pict) {
    QMouseEvent* mev  = static_cast<QMouseEvent*>(ev);
    if (nullptr != mev) {
      pointToAngles(mev->x(), mev->y());
      slotSetPosition(true);
      return true;
    }
  }
/*  if (ev->type() == QEvent::KeyPress) {
    QKeyEvent* kev = static_cast<QKeyEvent*>(ev);
    if (nullptr != kev) {
      return checkKeyEvent(kev);
    }
  }
*/
  return false;
}

void SmallAntWidget::setCirclePoint()
{
  float xd, yd, xc, yc;
  anglesToPoint(_ui->azDest->value(), _ui->elDest->value(), &xd, &yd);
  if (_azCur !=  BAD_ANGLE && _elCur != BAD_ANGLE) {
    anglesToPoint(_azCur, _elCur, &xc, &yc);
  }

  //debug_log << _elCur << _ui->elCur->text() << _ui->elDest->value() << yd << yc;

  QPixmap lpix(*_pix); //TODO иногда лишнее перерисовывание
  QPainter painter(&lpix);
  painter.setBrush(Qt::SolidPattern);
  if (_curTask != receive::kProgon) {
    painter.setPen(Qt::blue);
    painter.drawEllipse(QPoint(xd, yd), 5, 5);
  }
  if (_azCur !=  BAD_ANGLE && _elCur != BAD_ANGLE) {
    painter.setPen(Qt::green);
    painter.drawEllipse(QPoint(xc, yc), 5, 5);
  }
  _ui->pict->setPixmap(lpix);
}

void SmallAntWidget::pointToAngles(int xl, int yl)
{
  // debug_log << "xl" << xl << "yl" << yl;
  
  float xc = _ui->pict->width() / 2.0;
  float yc = _ui->pict->height() / 2.0;
  
  //к единичному кругу
  float x = (xl - xc);
  float y = _ui->pict->height() - yl - yc;

  x = x;
  y = y;
  // debug_log << "xy" << x << y;
  
  double az = MnMath::M0To360(MnMath::rad2deg(atan2(y, x)));
  double r = sqrt(x*x + y*y) * _ant->elMax() / yc;

  //debug_log << "angl" << az << r << _ant->elMax();
  
  if (r > _ant->elMax()) {
    return;
  }

  if (az > _ant->azMax()) {
    az -= 360;
  }
  
  if (az < _ant->azMax() && az > _ant->azMin() + 360) {
    if (_ui->azDest->value() < 0) {
      az -= 360;
    }
  }
  
  // QPixmap lpix(*_pix);
  // QPainter painter(&lpix);
  // painter.setPen(Qt::blue);
  // painter.setBrush(Qt::SolidPattern);
  // painter.drawEllipse(QPoint(xl * _scale_az, yl * _scale_el), 5, 5);
  // _ui->pict->setPixmap(lpix);
  
  if (!qFuzzyCompare(_ui->azDest->value(), az)) {
    _ui->azDest->setValue(az);
  }
  if (!qFuzzyCompare(_ui->elDest->value(), r)) {
    _ui->elDest->setValue(r);
  }
  
   // anglesToPoint(az, r);
}

void SmallAntWidget::anglesToPoint(double az, double elsrc, float* xres, float* yres)
{
  float xc = _ui->pict->width() / 2.0;
  float yc = _ui->pict->height() / 2.0;
  
  az = MnMath::M0To360(az);
  double el = elsrc * yc / _ant->elMax();
  if (el < 0) {
    el = 0;
  }
  
  float x = cos(MnMath::deg2rad(az)) * el;
  float y = sin(MnMath::deg2rad(az)) * el;
  
  //  debug_log << "-xy" << x << y;
  
  float xl = x + xc;
  float yl = _ui->pict->height() - y - yc;
  
  *xres = xl * _scale_az;
  *yres = yl * _scale_el;
    
  //debug_log << "pnt" << xl << yl;  
}

void SmallAntWidget::slotStopTimer()
{
  float angle1, angle2;
  _ant->stop(&angle1, &angle2);
  _curTask = CurrentTask::kNone;
  slotOnCoordsTimer();
  _coordsTimer->stop();
  _stopTimer->stop();
}

// void SmallAntWidget::slotShowCorr(bool isSet)
// {
//   if (isSet) {
//     _ui->corrSpinBox->setVisible(true);
//     _ui->setCorrButton->setVisible(true);
//     double zeroAz = _ant->zeroAzimut();
//     _ui->corrSpinBox->setValue(zeroAz);
//     _ui->showCorr->setToolTip("Скрыть угол коррекции");
//   }
//   else {
//     _ui->corrSpinBox->setVisible(false);
//     _ui->setCorrButton->setVisible(false);
//     _ui->showCorr->setToolTip("Показать угол коррекции");
//   }
// }

// void SmallAntWidget::slotSetCorr() {
//   _ant->setZeroAzimut(_ui->corrSpinBox->value());
//   _ui->corrSpinBox->setValue(_ant->zeroAzimut());
// }

bool SmallAntWidget::checkAnt()
{
  if (!_ant->isInit()) {
    powerChanged(false);
    _ui->log->setText(QObject::tr("Ошибка антенны"));
    error_log << QObject::tr("Ошибка антенны");
    return false;
  }
  
  return true;
}

void SmallAntWidget::slotSetPosition(bool isSet)
{
  disableRunButtons();
  _ui->setButton->setEnabled(true);
  
  if ( isSet ) {

    if (!checkAnt()) {
      return;
    }
    
    getCurrentCoords();
    if (_ui->azDest->value() < _ant->azMax()) {
      _azDest = _ui->azDest->value();
    }
    else {
      _azDest = _ui->azDest->value() - 360;
    }
    _elDest = _ui->elDest->value();

    if ( std::abs(_elDest - _elCur) < 0.5 && std::abs(_azDest - _azCur) < 0.5 ) {
      _curTask = CurrentTask::kNone;
      _ui->setButton->setChecked(false);
      enableButtons(true);
    } else {

      _ui->setButton->setIcon(QIcon(":/mappi/icons/stop_proc.png"));
      _ui->setButton->setToolTip("Стоп");
      _coordsTimer->start(100);
      _stopTimer->start(50000);
      bool ok = false;
      
      // if (std::abs(_elDest - _elCur) < 0.5) {
      // 	_curTask = CurrentTask::kAzimutFix;
      // 	ok = _ant->setAzimut(_azDest);
      // }
      // else if (std::abs(_azDest - _azCur) < 0.5) {
      // 	_curTask = CurrentTask::kElevationFix;
      // 	ok = _ant->setElevat(_elDest);
      // }
      // else
	{
	_curTask = CurrentTask::kSetPosition;
	ok = _ant->setPosition(_azDest, _elDest, false);
      }
      if (!ok) {
        error_log << QObject::tr("Ошибка установки антенны");
	_ui->setButton->setToolTip(" Установить целевое положение");
	_ui->setButton->setIcon(QIcon(":/mappi/icons/run_proc.png"));
	_ui->setButton->setChecked(false);
	enableButtons(true);
	_curTask = CurrentTask::kNone;
      }      
    }
  }
  else {
    if (_ant->stop(&_azCur, &_elCur)) {
      //_azCur = MnMath::M0To360(_azCur); //
      _ui->azCur->setText(locale().toString(_azCur, 'f', 1));
      _ui->elCur->setText(locale().toString(_elCur, 'f', 1));
    }
    _coordsTimer->stop();
    _stopTimer->stop();
    _ui->setButton->setIcon(QIcon(":/mappi/icons/run_proc.png"));
    _ui->setButton->setToolTip(" Установить целевое положение");
    enableButtons(true);
    _curTask = CurrentTask::kNone;
  }
}

void SmallAntWidget::disableRunButtons(bool azStepEn /*= false*/, bool elStepEn /*= false*/)
{
  bool enable = false;
  
  _ui->runBtn->setEnabled(enable);
  _ui->setButton->setEnabled(enable);

  _ui->azInc->setEnabled(azStepEn);
  _ui->azDec->setEnabled(azStepEn);
 
  _ui->elInc->setEnabled(elStepEn);
  _ui->elDec->setEnabled(elStepEn);
    
  _ui->workBtn->setEnabled(enable);
  _ui->travelBtn->setEnabled(enable);
}

void SmallAntWidget::enableButtons(bool enable)
{
  _ui->currBtn->setEnabled(enable);
  _ui->runBtn->setEnabled(enable);
  // _ui->followButton->setEnabled(enable);
  _ui->setButton->setEnabled(enable);
  _ui->azInc->setEnabled(enable);
  _ui->azDec->setEnabled(enable);
  _ui->elInc->setEnabled(enable);
  _ui->elDec->setEnabled(enable);
  _ui->workBtn->setEnabled(enable);
  _ui->travelBtn->setEnabled(enable);
}

void SmallAntWidget::setPort(const QString& port)
{
  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
  if (_ant->openPort(port)) {
    _ui->powerButton->setEnabled(true);
  } else {
    _ui->powerButton->setEnabled(false);
    enableButtons(false);
  }
  QApplication::restoreOverrideCursor();
}

void SmallAntWidget::powerChanged(bool state)
{
  if ( state ) {
    _ui->log->setText(QObject::tr("Регенерация"));
    _ant->openPort(_ui->port->currentText());
    if ( false == _ant->setPower(true) ) {
      _ui->log->setText(QObject::tr("Ошибка подключения"));
      error_log << "Ошибка подключения";
    }
    getCurrentCoords();
    //getCurrentCoords();
    if ( _azCur == BAD_ANGLE || _elCur == BAD_ANGLE) {
      _ui->powerLabel->setPixmap(QIcon(":/mappi/icons/ledred.png").pixmap(QSize(16,16)));
      _ui->powerLabel->setToolTip(QObject::tr("Получен некорректный ответ от антенны"));
      return;
    }
    _ui->log->clear();
    _azDest = _azCur;
    _elDest = _elCur;
    _ui->azDest->setValue(_azDest);
    _ui->elDest->setValue(_elDest);
    _ui->powerLabel->setPixmap(QIcon(":/mappi/icons/ledgreen.png").pixmap(QSize(16,16)));
    _ui->powerLabel->setToolTip(QObject::tr("Питание подано"));
    _ui->port->setEnabled(false);
    enableButtons(true);
  } else {
    // if ( false == _ant->setPower(true) ) {
    //   error_log << "Ошибка подключения";
    // }

    if (_curTask != CurrentTask::kNone) {
      slotStopTimer();
    }

    _ant->setPower(false);
    _ant->closePort();
    _ui->azCur->clear();
    _azCur = BAD_ANGLE;
    _azDest = BAD_ANGLE;
    _ui->elCur->clear();
    _elCur = BAD_ANGLE;
    _elDest = BAD_ANGLE;
    _ui->powerLabel->setPixmap(QIcon(":/mappi/icons/ledgrey.png").pixmap(QSize(16,16)));
    _ui->powerLabel->setToolTip(QObject::tr("Питание не подано"));
    // _ui->followButton->setChecked(false);
    // slotFollow(false);
    _ui->port->setEnabled(true);
    enableButtons(false);
  }
}

void SmallAntWidget::getCurrentCoords()
{
  bool ok = false;
  float azimut;
  float eleval;
  ok = _ant->currentAngles(&azimut, &eleval);

  if (ok) {
    QString text = locale().toString(azimut, 'f', 1);
    _ui->azCur->setText(text);
    _azCur = azimut;
    text = locale().toString(eleval, 'f', 1);
    _ui->elCur->setText(text);
    _elCur = eleval;
  } else {
    _ui->azCur->clear();
    _azCur = BAD_ANGLE;
    _ui->elCur->clear();
    _elCur = BAD_ANGLE;
  }
}


// void SmallAntWidget::setAzimut(bool isSet)
// {
//   getCurrentCoords();
//   _elDest = _elCur;
//   _coordsTimer->start(100);
//   bool setAz = isSet;
//   if (isSet) {
//     if (!_ant->setAzimut(_azDest)) {
//       setAz = false;
//       _ui->setButton->setChecked(false);
//       _curTask = CurrentTask::kNone;
//     }
//     _curTask = CurrentTask::kAzimutFix;
//   } else {
//     float angle = 0;
//     if (_ant->stopAzimut(&angle)) {
//       QString text;
//       text = locale().toString(angle, 'f', 1);
//       _ui->azCur->setText(text);
//       _coordsTimer->stop();
//       _curTask = CurrentTask::kNone;
//     }
//   }

//   if (!setAz) {
//     _ui->setButton->setIcon(QIcon(":/mappi/icons/run_proc.png"));
//     _ui->setButton->setToolTip(" Установить целевое положение");
//     _ui->runBtn->setEnabled(true);
//   }
// }

// void SmallAntWidget::setElevation()
// {
//   // getCurrentCoords();
//   // _azDest = _azCur;
//   _coordsTimer->start(100);
//   // bool setEl = isSet;
//   // if (isSet) {
//     _ui->setButton->setIcon(QIcon(":/mappi/icons/stop_proc.png"));
//     _ui->setButton->setToolTip("Стоп");
//     _ui->runBtn->setEnabled(false);
//     _elDest = _ui->elDest->value();
//     if (!_ant->setElevat(_elDest)) {
//       setEl = false;
//       _ui->setButton->setChecked(false);
//       _curTask = CurrentTask::kNone;
//     }
//     _curTask = CurrentTask::kElevationFix;
//     //}  else {
//   //   float angle = 0;
//   //   if (_ant->stopElevat(&angle)) {
//   //     QString text;
//   //     text = locale().toString(angle, 'f', 1);
//   //     _ui->elCur->setText(text);
//   //     _elCur = angle;
//   //     _coordsTimer->stop();
//   //     _curTask = CurrentTask::kNone;
//   //   }
//   // }

//   // if (!setEl) {
//   //   _ui->setButton->setIcon(QIcon(":/mappi/icons/run_proc.png"));
//   //   _ui->setButton->setToolTip(" Установить целевое положение");
//   //   _ui->runBtn->setEnabled(true);
//   // }
// }

void SmallAntWidget::setRun(bool isSet)
{
  getCurrentCoords();
  
  if (_azCur == BAD_ANGLE ||
      _elCur == BAD_ANGLE) {
    _ui->runBtn->setChecked(false);
    error_log << QObject::tr("Не определены текущие координаты");
    _ui->log->setText(QObject::tr("Не определены текущие координаты"));
    return;
  }
  _ui->log->clear();

  if (isSet) {
    disableRunButtons();
    _ui->azDest->setEnabled(false);
    _ui->elDest->setEnabled(false);
    _ui->runBtn->setEnabled(true);
    _ui->runBtn->setIcon(QIcon(":/mappi/icons/stop_proc.png"));
    _curTask = receive::kProgon;
    setCirclePoint();

    _progon->setStartPosition(_azCur, _elCur);
    _progon->setDSA(_ui->azStep->value());
    _progon->setDSE(_ui->elStep->value());
    _timer->start(500);

    setNextValue();
  } else {
    _ui->azDest->setEnabled(true);
    _ui->elDest->setEnabled(true);
    _timer->stop();
    float azimut = 0;
    float elev = 0;
    if (_ant->stop(&azimut, &elev)) {
      QString text = locale().toString(azimut, 'f', 1);
      _ui->azCur->setText(text);
      _azCur = azimut;
      text = locale().toString(elev, 'f', 1);
      _ui->elCur->setText(text);
      _elCur = elev;
    }

    
    _ui->runBtn->setIcon(QIcon(":/mappi/icons/run_proc.png"));
    _curTask = mappi::receive::kNone;
    setCirclePoint();
    enableButtons(true);
  }
}

void SmallAntWidget::setNextValue()
{
  float az = _progon->azimut();
  float el = _progon->elevat();

  float nextAz = _progon->nextAzimut();
  float nextEl = _progon->nextElevat();

  debug_log << "** Next " << nextAz << nextEl;

  if (!_ant->movePosition(nextAz, nextEl, &_azCur, &_elCur)) {
    if (!checkAnt()) {
      return;
    }
  }

  QString text;
  text = locale().toString(_azCur, 'f', 1);
  _ui->azCur->setText(text);
  text = locale().toString(_elCur, 'f', 1);
  _ui->elCur->setText(text);

  if (fabs(az - _azCur) > 1 ||
      fabs(el - _elCur) > 1) {
    warning_log << QObject::tr("Ошибка позиционирования. Азимут = %1, ожидаемый = %2. Угол места = %3, ожидаемый = %4")
                   .arg(_azCur).arg(az).arg(_elCur).arg(el);
  }

}

void SmallAntWidget::slotOnCoordsTimer()
{
  getCurrentCoords();
  float azDest = MnMath::M0To360(_azDest);
  float azCur =  MnMath::M0To360(_azCur);

  if ((azDest > 355 && azCur < 5) ||
      (azDest < 5 && azCur > 355) ||
      (azCur > 355 && _curTask == CurrentTask::kWorkPos)
      ) {
    azDest =  MnMath::M180To180(azDest);
    azCur =  MnMath::M180To180(azCur);  
  }
  
  debug_log << "destination: " << azDest << _elDest << " current: " << azCur << _elCur << "task" << _curTask;
  switch(_curTask)
  {
  case CurrentTask::kAzimutFix:
    if (std::abs(azDest - azCur) < 0.5) {
      _coordsTimer->stop();
      _stopTimer->stop();
      _ui->setButton->setIcon(QIcon(":/mappi/icons/run_proc.png"));
      _ui->setButton->setToolTip(" Установить целевое положение");
      _ui->setButton->setChecked(false);
      enableButtons(true);
      //_azDest = azCur;
      _curTask = CurrentTask::kNone;
    }
    break;
  case CurrentTask::kElevationFix:
    if ( std::abs(_elDest - _elCur) < 0.5 ) {
      _coordsTimer->stop();
      _stopTimer->stop();
      _ui->setButton->setIcon(QIcon(":/mappi/icons/run_proc.png"));
      _ui->setButton->setToolTip(" Установить целевое положение");
      _ui->setButton->setChecked(false);
      enableButtons(true);
      _elDest = _elCur;
      _curTask = CurrentTask::kNone;
    }
    break;
  case CurrentTask::kSetPosition:
    if ( std::abs(_elDest - _elCur) < 0.5 && std::abs(azDest - azCur) < 0.5 ) {
      _coordsTimer->stop();
      _stopTimer->stop();
      _elDest = _elCur;
      _azDest = azCur;
      if (_azDest > _ant->azMax()) {
	_azDest -= 360;
      }
      _ui->setButton->setIcon(QIcon(":/mappi/icons/run_proc.png"));
      _ui->setButton->setToolTip(" Установить целевое положение");
      _ui->setButton->setChecked(false);
      enableButtons(true);
      _curTask = CurrentTask::kNone;
    }
    break;

  case CurrentTask::kAzimutInc:
  case CurrentTask::kAzimutDec:
    if (std::abs(azDest - azCur) < 0.5) {
      _coordsTimer->stop();
      enableButtons(true);
      //_azDest = azCur;
      _curTask = CurrentTask::kNone;
    }
    break;
  case CurrentTask::kElevationInc:
  case CurrentTask::kElevationDec:
    if (std::abs(_elDest - _elCur) < 0.5) {
      _coordsTimer->stop();
      enableButtons(true);
      _elDest = _elCur;
      _curTask = CurrentTask::kNone;
    }
    break;
  case CurrentTask::kTravelPos: {
    uint32_t stat;
    bool ok = _ant->status(&stat);
    if (ok && (stat & mappi::receive::kElMin) != 0) {
      _coordsTimer->stop();
      _stopTimer->stop();
      enableButtons(true);
      // _elDest = _elCur;
      _curTask = CurrentTask::kNone;
    }
  }
    break;
  case CurrentTask::kWorkPos:
    if ( std::abs(7 - _elCur) < 0.5 && std::abs(0 - azCur) < 0.5 ) {
      _coordsTimer->stop();
      _stopTimer->stop();
      enableButtons(true);
      // _elDest = _elCur;
      _curTask = CurrentTask::kNone;
    }

    break;
  case CurrentTask::kNone:
    _ui->setButton->setIcon(QIcon(":/mappi/icons/run_proc.png"));
    _ui->setButton->setToolTip(" Установить целевое положение");
    _ui->setButton->setChecked(false);
    enableButtons(true);
    break;
  default:
    break;
  }
}

void SmallAntWidget::slotAzInc()
{
  float step = 5;

  disableRunButtons(false, true);
  if (!checkAnt()) {
    return;
  }
  
  getCurrentCoords();
  if (_azCur >= _ant->azMax()) {
    enableButtons(true);
    return;
  }
 
  _elDest = _elCur;
  _azDest = _azCur + step;
  if (_azDest > _ant->azMax()) {
    _azDest = _ant->azMax();
  }

  _ui->azDest->setValue(_azDest);
  _ui->elDest->setValue(_elDest);
  
  if (!_ant->setAzimut(_azDest)) {
    enableButtons(true);
    return;
  }
  _curTask = CurrentTask::kAzimutInc;

  _coordsTimer->start(100);
  _stopTimer->start(50000);
}

void SmallAntWidget::slotAzDec()
{
  float step = 5;

  disableRunButtons(false, true);

  if (!checkAnt()) {
    return;
  }
  
  getCurrentCoords();
  if (_azCur <= _ant->azMin() || _azCur == BAD_ANGLE) {
    enableButtons(true);
    return;
  }
  
  _elDest = _elCur;
  _azDest = _azCur - step;
  if (_azDest < _ant->azMin()) {
    _azDest = _ant->azMin();
  }

  _ui->azDest->setValue(_azDest);
  _ui->elDest->setValue(_elDest);
  
  if (!_ant->setAzimut(_azDest)) {
    enableButtons(true);
    return;
  }
  _curTask = CurrentTask::kAzimutInc;

  _coordsTimer->start(100);
  _stopTimer->start(50000);
}

void SmallAntWidget::slotElInc()
{
  float step = 5;

  disableRunButtons(true, false);

  if (!checkAnt()) {
    return;
  }
  
  getCurrentCoords();
  if (_elCur >= _ant->elMax()) {
    enableButtons(true);
    return;    
  }
  
  _azDest = _azCur;
  _elDest = _elCur + step;
  if (_elDest > _ant->elMax()) {
    _elDest = _ant->elMax();
  }

  _ui->azDest->setValue(_azDest);
  _ui->elDest->setValue(_elDest);
  
  if (!_ant->setElevat(_elDest)) {
    enableButtons(true);
    return;
  }
  _curTask = CurrentTask::kElevationInc;

  _coordsTimer->start(100);
  _stopTimer->start(50000);
}

void SmallAntWidget::slotElDec()
{
  float step = 5;

  disableRunButtons(false, true);

  if (!checkAnt()) {
    return;
  }
  
  getCurrentCoords();
  if (_elCur <= _ant->elMin() || _elCur == BAD_ANGLE) {
    enableButtons(true);
    return;
  }
  
  _azDest = _azCur;
  _elDest = _elCur - step;
  if (_elDest < _ant->elMin()) {
    _elDest = _ant->elMin();
  }

  _ui->azDest->setValue(_azDest);
  _ui->elDest->setValue(_elDest);

  if (!_ant->setElevat(_elDest)) {
    enableButtons(true);
    return;
  }
  _curTask = CurrentTask::kElevationDec;

  _coordsTimer->start(100);
  _stopTimer->start(50000);
}

void SmallAntWidget::slotSetTravel()
{
  disableRunButtons();

  if (!checkAnt()) {
    return;
  }
  
  if (!_ant->setTravelPosition()) {
    enableButtons(true);
    return;
  }
  _curTask = CurrentTask::kTravelPos;
  
  _coordsTimer->start(100);
  _stopTimer->start(50000);
}

void SmallAntWidget::slotSetWork()
{
  debug_log << "not realised";
  // disableRunButtons();

  // if (!checkAnt()) {
  //   return;
  // }
  
  // if (!_ant->setWorkPosition()) {
  //   enableButtons(true);
  //   return;
  // }
  // _curTask = CurrentTask::kWorkPos;
  
  // _coordsTimer->start(100);
  // _stopTimer->start(50000);
}

bool SmallAntWidget::checkKeyEvent(QKeyEvent *event)
{
  bool fl = false;
  if (_ui->elInc->isEnabled()) {
    if (event->key() == Qt::Key_Up) {
      fl = true;
      slotElInc();
    } else if (event->key() == Qt::Key_Down) {
      fl = true;
      slotElDec();
    }
  } 
  if (_ui->azInc->isEnabled()) {
    if (event->key() == Qt::Key_Left) {
      fl = true;
      slotAzDec();
    } else if (event->key() == Qt::Key_Right) {
      fl = true;
      slotAzInc();
    }
  }

  return fl;
}

void SmallAntWidget::keyPressEvent(QKeyEvent *event)
{
  bool fl = checkKeyEvent(event);
  
  if (!fl) {
    QWidget::keyPressEvent(event);
  }
}
