#include "antennawidget.h"
#include "ui_antennawidget.h"
#include <cross-commons/app/paths.h>

#define CONF_FILE (MnCommon::etcPath("mappi") + "reception.conf")


namespace mappi {

namespace antenna {

static const int HEALTHCHECK_TIMER = 2000;  // 2 sec


// class CursorGuard
class CursorGuard {
  Q_DISABLE_COPY(CursorGuard)

public :
  CursorGuard()  { QApplication::setOverrideCursor(Qt::WaitCursor); };
  ~CursorGuard() { QApplication::restoreOverrideCursor(); };
};


// class AntennaWidget
AntennaWidget::AntennaWidget(QWidget* parent /*=*/, Qt::WindowFlags f /*=*/) :
    QWidget(parent, f),
  ui_(new Ui::AntennaWidget()),
  groupScript_(new QButtonGroup(this)),
  timer_(new QTimer(this)),
  navigator_(new Navigator(this)),
  client_(new ClientStub(this)),
  currentDSA_(0),
  currentDSE_(0),
  currentAzimut_(0),
  currentElevat_(0)
{
  ui_->setupUi(this);

  ui_->horizontalLayout_2->addWidget(navigator_);
  ui_->fControl->setEnabled(false);

  groupScript_->addButton(ui_->tbPosition);
  groupScript_->addButton(ui_->tbMonkeyTest);

  QObject::connect(ui_->tbServiceMode, &QToolButton::toggled, this, &AntennaWidget::serviceMode);
  QObject::connect(ui_->tbPosition, &QToolButton::toggled, this, &AntennaWidget::positionActivated);
  QObject::connect(ui_->tbMonkeyTest, &QToolButton::toggled, this, &AntennaWidget::monkeyTestActivated);
  QObject::connect(ui_->tbStart, &QToolButton::clicked, this, &AntennaWidget::start);
  QObject::connect(ui_->tbStop, &QToolButton::clicked, this, &AntennaWidget::stop);

  QObject::connect(client_, &ClientStub::snapshot, this, &AntennaWidget::update);

  QObject::connect(navigator_, &Navigator::keepMoving, ui_->tbStart, &QToolButton::click);
  QObject::connect(navigator_, &Navigator::stop, ui_->tbStop, &QToolButton::click);
  QObject::connect(navigator_, &Navigator::newPosition, this, [this](float azimut, float elevat) {
    ui_->dsbPointAz->setValue(azimut);
    ui_->dsbPointEn->setValue(elevat);

    ui_->tbStart->click();
  });

  QObject::connect(timer_, &QTimer::timeout, this, &AntennaWidget::healthCheck);

  // TODO надо конфигурацию брать уже готовой
  Configuration conf;
  if (conf.load(CONF_FILE) == false)
    return ;

  debug_log << conf.toString();
  profile_ = conf.profile;

  if (profile_.hasFeedhornControl) {
    QObject::connect(ui_->tbFeedhornTurnOn, &QToolButton::toggled, client_, &ClientStub::feedhornPower);
    QObject::connect(ui_->tbOutput_1, &QToolButton::toggled, this, [this](bool checked) {
      client_->feedhornOutput(1, checked);
    });

    QObject::connect(ui_->tbOutput_2, &QToolButton::toggled, this, [this](bool checked) {
      client_->feedhornOutput(2, checked);
    });

    QObject::connect(ui_->tbOutput_3, &QToolButton::toggled, this, [this](bool checked) {
      client_->feedhornOutput(3, checked);
    });

    QObject::connect(ui_->tbOutput_4, &QToolButton::toggled, this, [this](bool checked) {
      client_->feedhornOutput(4, checked);
    });
  } else
    ui_->fFeedhorn->setVisible(false);

  navigator_->setScope(conf.azimutCorrect, profile_);
}

AntennaWidget::~AntennaWidget()
{
  timer_->stop();
  serviceMode(false);

  delete ui_;
  ui_ = nullptr;
}

void AntennaWidget::closeEvent(QCloseEvent* event)
{
  Q_UNUSED(event)

  timer_->stop();
  client_->unSubscribe();

  serviceMode(false);
}

void AntennaWidget::showEvent(QShowEvent* event)
{
  Q_UNUSED(event)

  timer_->start(HEALTHCHECK_TIMER);

  CursorGuard guard;
  if (client_->subscribe() == false) {
    setStatus(QObject::tr("Нет соедиенения"));
    setError(QObject::tr("%1 недоступен")
      .arg(global::serviceTitle(settings::proto::kAntenna)));
  }
}

void AntennaWidget::setError(const QString& text)
{
  ui_->lError->setText(text);
}

void AntennaWidget::setStatus(const QString& text)
{
  ui_->lState->setText(text);
}

void AntennaWidget::setCurrentPos(float azimut, float elevat)
{
  ui_->leCurrentAz->setText(QString::number(azimut, 'f', 2));
  ui_->leCurrentEn->setText(QString::number(elevat, 'f', 2));
}

void AntennaWidget::serviceMode(bool checked)
{
  if (checked)
    navigator_->ready();
  else
    navigator_->waiting();

  client_->serviceMode(checked);

  ui_->fControl->setEnabled(checked);
  ui_->dsbPointAz->setValue(ui_->leCurrentAz->text().toFloat());
  ui_->dsbPointEn->setValue(ui_->leCurrentEn->text().toFloat());
}

void AntennaWidget::positionActivated(bool checked)
{
  ui_->tbStop->click();
  navigator_->ready();

  currentDSA_ = ui_->dsbPointAz->value();
  ui_->dsbPointAz->setMinimum(profile_.azimut.min);
  ui_->dsbPointAz->setMaximum(profile_.azimut.max);
  ui_->dsbPointAz->setValue(currentAzimut_);

  currentDSE_= ui_->dsbPointEn->value();
  ui_->dsbPointEn->setMinimum(profile_.elevat.min);
  ui_->dsbPointEn->setMaximum(profile_.elevat.max);
  ui_->dsbPointEn->setValue(currentElevat_);
}

void AntennaWidget::monkeyTestActivated(bool checked)
{
  ui_->tbStop->click();
  navigator_->waiting();

  currentAzimut_ = ui_->dsbPointAz->value();
  ui_->dsbPointAz->setMinimum(0);
  ui_->dsbPointAz->setMaximum(profile_.dsa);
  ui_->dsbPointAz->setValue(currentDSA_);

  currentElevat_ = ui_->dsbPointEn->value();
  ui_->dsbPointEn->setMinimum(0);
  ui_->dsbPointEn->setMaximum(profile_.dse);
  ui_->dsbPointEn->setValue(currentDSE_);
}

void AntennaWidget::start()
{
  float azimut = ui_->dsbPointAz->value();
  float elevat = ui_->dsbPointEn->value();

  debug_log << QString("AntennaWidget::start(%1, %2)").arg(azimut).arg(elevat);
  if (ui_->tbPosition->isChecked()) {
    client_->setPosition(azimut, elevat);
    return ;
  }

  if (ui_->tbMonkeyTest->isChecked())
    client_->monkeyTest(azimut, elevat);
}

void AntennaWidget::stop()
{
  debug_log << QString("AntennaWidget::stop");
  client_->stop();
}

void AntennaWidget::healthCheck()
{
  if (client_->healthCheck() == false) {
    setStatus(QObject::tr("Нет соедиенения"));
    setError(QObject::tr("%1 недоступен")
       .arg(global::serviceTitle(settings::proto::kAntenna)));

    return ;
  }

  client_->subscribe();
}

void AntennaWidget::update(conf::AntennaResponse* resp)
{
  timer_->start();

  float azimut = resp->azimut().self();
  float elevat = resp->elevat().self();
  debug_log << QString("Got response %1 (%2, %3)").arg(resp->state()).arg(azimut).arg(elevat);
  navigator_->setCurrentPositon(azimut, elevat);
  setCurrentPos(azimut, elevat);

  switch (resp->state()) {
    case kIdlingState :   setStatus(QObject::tr("Ожидание"));   break ;
    case kWarmingState :  setStatus(QObject::tr("Подготовка")); break ;
    case kActiveState :   setStatus(QObject::tr("Приём"));      break ;
    case kSkippingState : setStatus(QObject::tr("Пропуск"));    break ;

    case kServiceState :
      ui_->tbServiceMode->setChecked(true);
      setStatus(QObject::tr("Сервисный режим"));
      break ;

    default :
      setStatus("???");
  }

  if (profile_.hasFeedhornControl) {
    ui_->tbFeedhornTurnOn->setChecked(resp->feedhorn().power_on());
    ui_->tbOutput_1->setChecked(resp->feedhorn().output_1());
    ui_->tbOutput_2->setChecked(resp->feedhorn().output_2());
    ui_->tbOutput_3->setChecked(resp->feedhorn().output_3());
    ui_->tbOutput_4->setChecked(resp->feedhorn().output_4());
  }

  if ((static_cast<Drive::state_t>(resp->azimut().state()) == Drive::UNKNOWN) ||
        (static_cast<Drive::state_t>(resp->elevat().state()) == Drive::UNKNOWN)) {

      setError(QObject::tr("Нет доступа к приводам, транспортный уровень отключен"));
      return ;
  }

  QString driveState = "";
  switch (resp->azimut().state()) {
    case Drive::OK :                driveState = QObject::tr("азимут\\привод исправен");              break ;
    case Drive::FAIL :              driveState = QObject::tr("азимут\\ошибка");                       break ;
    case Drive::SENSOR_ANGLE_FAIL : driveState = QObject::tr("азимут\\ошибка энкодера(датчик угла)"); break ;
    case Drive::CRC_NOT_VALID :     driveState = QObject::tr("азимут\\не совпадает crc");             break ;

    default :
      break ;
  }

  switch (resp->elevat().state()) {
    case Drive::OK :                driveState += QObject::tr(" угол места\\привод исправен");              break ;
    case Drive::FAIL :              driveState += QObject::tr(" угол места\\ошибка");                       break ;
    case Drive::SENSOR_ANGLE_FAIL : driveState += QObject::tr(" угол места\\ошибка энкодера(датчик угла)"); break ;
    case Drive::CRC_NOT_VALID :     driveState += QObject::tr(" угол места\\не совпадает crc");             break ;

    default :
      break ;
  }

  setError(driveState);
}

}

}
