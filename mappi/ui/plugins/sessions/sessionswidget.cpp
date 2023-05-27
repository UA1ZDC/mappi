#include "sessionswidget.h"
#include "ui_sessionswidget.h"
#include "satellitetrackerscene.h"
#include "receiverimageview.h"
#include "receiverimagescene.h"

#include <qdialog.h>
#include <qbitmap.h>
#include <qsettings.h>
#include <qlistview.h>
#include <qstringlistmodel.h>
#include <qbuttongroup.h>

#include <meteo/commons/global/global.h>
#include <meteo/commons/proto/appconf.pb.h>

#include <mappi/settings/mappisettings.h>

#include <mappi/ui/scheduletable/scheduletable.h>

static const int kTimeoutConnect  = 30000; //!< Таймаут соединения
static const int kTimeoutRequest  = 10000; //!< Таймаут запроса

static const QString kSettings = QDir::homePath() + "/.meteo/mappi/sessionswidget.ini";

static const QString kReceiverApplication = QObject::tr("mappi.session.manager");

namespace mappi {

SessionsWidget::SessionsWidget(QWidget *parent)
  : QWidget(parent)
  , ui_(new Ui::SessionsWidget)
  , satelliteTracker_(nullptr)
  , receiverImageView_(new ReceiverImageView(this))
  , receiverImage_(new ReceiverImageScene(receiverImageView_))
  , scheduleTable_(new ScheduleTable(this))
  , errorDialog_(new QDialog(this, Qt::Dialog))
  , mapModeGroup_(new QButtonGroup(this))
  , showMap_(true)
  , showErrorList_(false)
  , loadedSettings_(false)
  , errorList_(new QListView(this))
  , errorsModel_(new QStringListModel(this))
  , chAntenna_(nullptr)
  // , ctrlReceiver_(nullptr)
  // , replyReceiver_(new conf::DeviceStateReply())
  , chAppManager_(nullptr)
  // , ctrlAppManager_(nullptr)
  // , replyAppManager_(new meteo::app::AppState_Proc())
  , near_(nullptr)
  , updateTimer_(new QTimer(this))
{
  ui_->setupUi(this);
  //Сокрытие неиспользуемых UI элементов
  {
    ui_->logBtn->setVisible(false);
    ui_->iReceiverGbox->setVisible(false);
    //ui_->iAntennaGbox->setVisible(false);
  }
  //Диалоговое окно журнала
  {
    QVBoxLayout* layout = new QVBoxLayout(errorDialog_);
    layout->addWidget(errorList_);
    errorDialog_->setLayout(layout);
    errorDialog_->setWindowTitle(QObject::tr("Ошибки"));
    connect(errorDialog_, &QDialog::finished,    this, &SessionsWidget::slotToggleShowErrorList);

    errorList_->setBackgroundRole(QPalette::Background);
    errorList_->setModel(errorsModel_);
    errorList_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    connect(ui_->logBtn,  &QPushButton::clicked, this, &SessionsWidget::slotToggleShowErrorList);
  }
  //Таблица расписания
  {
    ui_->iSessionVbox->insertWidget(2, scheduleTable_);
    scheduleTable_->show();
    //connect(scheduleTable_, &ScheduleTable::changeNear,    this, &SessionsWidget::slotChangedNear);
    //connect(scheduleTable_, &ScheduleTable::changeSession, this, &SessionsWidget::slotChangedSession);
  }
  //Карта
  {
    satelliteTracker_ = new SatelliteTrackerScene(ui_->satelliteTrackerMap);
    ui_->satelliteTrackerMap->setScene(satelliteTracker_);
    connect(ui_->asideBtn, &QPushButton::clicked, this, &SessionsWidget::slotToggleVisibleMap);

    mapModeGroup_->addButton(ui_->orbitaRbtn, static_cast<int>(SatelliteTrackerScene::MapType::kOrbita));
    mapModeGroup_->addButton(ui_->receiverRbtn, static_cast<int>(SatelliteTrackerScene::MapType::kReceiver));
    connect(mapModeGroup_, static_cast<void(QButtonGroup::*)(int, bool)>(&QButtonGroup::buttonToggled),
            this, &SessionsWidget::slotToggleMapMode);
  }
  //Приём данных
  {
    receiverImageView_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    receiverImageView_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    receiverImageView_->setInteractive(false);
    receiverImageView_->setFocusPolicy(Qt::NoFocus);

    ui_->iChannelsVbox->insertWidget(1, receiverImageView_);
    receiverImageView_->setScene(receiverImage_);

    connect(ui_->mapBtn,    &QPushButton::clicked, [&]() {
      receiverImage_->slotToggleMap(!receiverImage_->showLayers());
      ui_->mapBtn->setIcon(getEyeIcon(receiverImage_->showLayers()));
    });

    connect(receiverImage_, &ReceiverImageScene::sigResize,        this, &SessionsWidget::slotResizeImage);

    connect(receiverImage_, &ReceiverImageScene::readySatChannels, this, &SessionsWidget::slotSetChannels);

    connect(ui_->receiveCtrlBtn, &QPushButton::clicked, this, &SessionsWidget::slotClickReceiverCtrl);

    connect(ui_->channelsCbox, static_cast<void (QComboBox::*)(const QString&)>(&QComboBox::currentIndexChanged),
            receiverImage_, &ReceiverImageScene::slotSwitchChannel);
  }
  loadSettings();
  ui_->mapBtn->setIcon(getEyeIcon(receiverImage_->showLayers()));
  init();
  //Таймер
  {
    connect(updateTimer_, &QTimer::timeout, this, &SessionsWidget::slotTimeout);
    updateTimer_->setInterval(1000);
    updateTimer_->start();
  }
}

SessionsWidget::~SessionsWidget()
{
  saveSettings();
  deleteAppManagerChannel();
  //deleteReceiverChannel();
  deleteAntennaChannel();

  //  delete replyReceiver_;
  delete ui_;
}

void SessionsWidget::init()
{
  const auto reception = ::mappi::inter::Settings::instance()->reception();
  satelliteTracker_->setConf(reception);
  near_ = std::make_shared<schedule::Session>();
  getSessions();
  // scheduleTable_->init();
  ui_->dataTimeLbl->setText(currentDt().toString("dd-MM-yyyy  hh:mm:ss"));

  if(false == loadedSettings_) {
    ui_->receiverRbtn->setChecked(true);
  }
  slotInitAppManagerChannel();
  //slotInitReceiverChannel();
  slotInitAntennaChannel();
  refreshReceiverControls();
}

void SessionsWidget::loadSettings()
{
  if(false == QFile::exists(kSettings)) return;

  QSettings settings(kSettings, QSettings::IniFormat);

  if(settings.value("map/type", true).toBool()) {
    ui_->orbitaRbtn->setChecked(true);
  }
  else {
    ui_->receiverRbtn->setChecked(true);
  }
  receiverImage_->slotToggleMap(settings.value("map/showLayers", true).toBool());

  errorDialog_->restoreGeometry(settings.value("dialog/error").toByteArray());
  loadedSettings_ = true;
}

void SessionsWidget::saveSettings() const
{
  QSettings settings(kSettings, QSettings::IniFormat);
  settings.setValue("map/type", ui_->orbitaRbtn->isChecked());
  settings.setValue("dialog/error", errorDialog_->saveGeometry());
  settings.setValue("map/showLayers", receiverImage_->showLayers());
}


/*!
 * \brief Обработать виджет перед показом
 * \param e Событие показа
 */
void SessionsWidget::showEvent(QShowEvent *e)
{
  resizeMap(ui_->satelliteTrackerMap->size());
  slotResizeImage(receiverImageView_->size());
  QWidget::showEvent(e);
}

void SessionsWidget::resizeEvent( QResizeEvent* e )
{
  QWidget::resizeEvent(e);
  resizeMap(ui_->satelliteTrackerMap->size());
  slotResizeImage(receiverImageView_->size());
}

/*! \brief Измененить размер окна с картой */
void SessionsWidget::resizeMap(const QSize& size)
{
  if (nullptr != ui_->satelliteTrackerMap->scene()) {
    // На 4 меньше, чтобы вся рамка влезла...
    QSize sz(size.width() - 4, size.height() - 4);
    satelliteTracker_->resize(sz);
  }
}

/*! \brief Слот изменения размера окна с принятым изображением(вызов инициируется сценой ReceivImageScene) */
void SessionsWidget::slotResizeImage(const QSize& size)
{
  Q_UNUSED(size);
  if(nullptr != receiverImageView_->scene()) {
    receiverImage_->resize(receiverImageView_->size());
  }
}

void SessionsWidget::slotTimeout()
{
  ui_->dataTimeLbl->setText(currentDt().toString("dd-MM-yyyy  hh:mm:ss"));

  if(nullptr != near_ && false == near_->data().isDefault()) {
    QDateTime curTime = currentDtUtc();
    if(near_->data().aos > curTime) {
      //ui_->nearGbox->setTitle(QObject::tr("Ближайшая сессия"));
      ui_->nearGbox->setText(QObject::tr("Ближайшая сессия"));
      ui_->nearTimerTitleLbl->setText(QObject::tr("До начала:"));
      ui_->nearTimerLbl->setText(QTime::fromMSecsSinceStartOfDay(curTime.secsTo(near_->data().aos) * 1000).toString("hh:mm:ss"));
    }
    else {
      ui_->nearGbox->setText(QObject::tr("Текущая сессия"));
      ui_->nearTimerTitleLbl->setText(QObject::tr("До окончания:"));
      qint64 secs_to = curTime.secsTo(near_->data().los);
      if(secs_to < 1){
        if(nullptr != scheduleTable_){
          scheduleTable_->sheckShedule();
        }
        getSessions();
      }
      ui_->nearTimerLbl->setText(QTime::fromMSecsSinceStartOfDay(secs_to* 1000).toString("hh:mm:ss"));
    }
  }
}

void SessionsWidget::slotChangedNear(std::shared_ptr<schedule::Session> near)
{
  near_ = near;
  if(nullptr != near_) {
    ui_->nearSatLbl->setText(near_->data().satellite);
  }
  else
  {
    ui_->nearGbox->setText(QObject::tr("Ближайшая сессия"));
    ui_->nearSatLbl->setText("");
    ui_->nearTimerTitleLbl->setText(QObject::tr("До начала:"));
    ui_->nearTimerLbl->setText("");
  }
  receiverImage_->removeTemp();
  debug_log<<"slotChangedNear";
}

void SessionsWidget::slotChangedSession(std::shared_ptr<schedule::Session> session, const conf::TleItem &tle)
{
  conf::Instrument instrument;
  if(::mappi::inter::Settings::instance()->radiometer(session->data().satellite, &instrument))
  {
    MnSat::STLEParams stle;
    stle.satName = tle.title().c_str();
    stle.firstString = tle.line_1().c_str();
    stle.secondString = tle.line_2().c_str();
    //меняем сеанс в сценах траектории и приема изображения
    if(nullptr != satelliteTracker_) {
      satelliteTracker_->setSession(session, instrument, timeOffset_, stle);
    }
    if(nullptr != receiverImage_) {
      receiverImage_->setSession(session, instrument, currentDtUtc(), stle);
    }
  }
}

/*!
 * \brief Заполнение комбобокса списком каналов
 * \param channels список каналов
 */
void SessionsWidget::slotSetChannels(const QStringList& channels)
{
  //  var(channels);
  QString currentChannel = ui_->channelsCbox->currentText();
  ui_->channelsCbox->clear();
  if(false == channels.isEmpty())
  {
    auto chans = channels;
    //qSort(chans); TODO нельзя, т.к. сортировка как строк и '1' будет позже '10'
    ui_->channelsCbox->insertItems(0, chans);

    if(channels.contains(currentChannel)) {
      ui_->channelsCbox->setCurrentText(currentChannel);
    }
    else {
      ui_->channelsCbox->setCurrentIndex(0);
    }
  }
}


void SessionsWidget::deleteAntennaChannel()
{
  if(nullptr != chAntenna_) {
    disconnect(chAntenna_, 0, this, 0);
    chAntenna_->deleteLater();
    chAntenna_ = nullptr;
  }
}
/*
void SessionsWidget::deleteReceiverChannel()
{
  debug_log<<"deleteReceiverChannel";
  if(nullptr != chReceiver_) {
    disconnect(chReceiver_, 0, this, 0);
    chReceiver_->deleteLater();
    chReceiver_ = nullptr;
  }
}

void SessionsWidget::slotInitReceiverChannel()
{
  //deleteReceiverChannel();
  //slotConnectToReceiver();
}
*/

void SessionsWidget::slotInitAntennaChannel()
{
  deleteAntennaChannel();
  slotConnectToAntenna();
}

void SessionsWidget::slotConnectToAntenna()
{
  if(nullptr == chAntenna_)
  {
    chAntenna_ = meteo::global::serviceChannel(meteo::settings::proto::kAntenna);
    if(nullptr == chAntenna_) {
      QTimer::singleShot(kTimeoutConnect, this, &SessionsWidget::slotConnectToAntenna);
      setMsgFromAntenna( "Нет данных" );
      return;
    }
    connect(chAntenna_, &meteo::rpc::Channel::disconnected, this, &SessionsWidget::slotDisconnectedFromAntenna);
  }

  if(false == chAntenna_->isConnected())
  {
    // if(false == chReceiver_->connect()) {
    deleteAntennaChannel();
    if(nullptr == chAppManager_) {
      QTimer::singleShot(kTimeoutConnect, this, &SessionsWidget::slotConnectToAntenna);
      setMsgFromAntenna( "Нет данных" );
    }
    return;
    //  }
  }
  subscribe();
}

void SessionsWidget::slotDisconnectedFromAntenna()
{
  deleteAntennaChannel();
  if(nullptr == chAppManager_) {
    QTimer::singleShot(kTimeoutConnect, this, &SessionsWidget::slotConnectToAntenna);
  }
  setMsgFromAntenna( "Нет данных" );
  ui_->azimutSendLbl->setText("");
  ui_->azimutTryLbl->setText("");
  ui_->elevationSendLbl->setText("");
  ui_->elevationTryLbl->setText("");
}

bool SessionsWidget::subscribe()
{
  if(nullptr == chAntenna_) return false;
  if(false == chAntenna_->isConnected()) return false;

  //ctrlReceiver_ = new rpc::TController(chReceiver_);

  Dummy request;
  //request.set_subscr(mappi::conf::SubscrType::kAntennaSubscr);

  bool ok = chAntenna_->subscribe(&mappi::conf::AntennaService::Subscribe, request,
                                  this, &SessionsWidget::callbackAntenna);
  if(false == ok) {
    error_log << QObject::tr("Не удалось подписаться на получение информации о состоянии антенны");
    QTimer::singleShot(kTimeoutConnect, this, &SessionsWidget::slotInitAntennaChannel);
  }
  else {
    setMsgFromAntenna( "Нет данных" );

    receiverImage_->subscribe(chAntenna_);
  }
  return ok;
}

void SessionsWidget::callbackAntenna(conf::AntennaResponse *reply)
{
  /* required float dst = 1;       //!< целевое значение угла, градусы
  required float self = 2;      //!< текущее значение угла, градусы
  required int32 state = 3;     //!< состояние привода
  */
  if(reply->has_azimut() && reply->azimut().has_dst()) {
    ui_->azimutSendLbl->setText(QString::number(MnMath::angle0to360(reply->azimut().dst()), 'f', 1) + "°");
  }

  if(reply->has_azimut() && reply->azimut().has_self()) {
    ui_->azimutTryLbl->setText(QString::number(MnMath::angle0to360(reply->azimut().self()), 'f', 1) + "°");
  }

  if(reply->has_elevat()&&reply->elevat().has_dst()) {
    ui_->elevationSendLbl->setText(QString::number(MnMath::angle0to360(reply->elevat().dst()), 'f', 1) + "°");
  }

  if(reply->has_elevat()&&reply->elevat().has_self()) {
    ui_->elevationTryLbl->setText(QString::number(MnMath::angle0to360(reply->elevat().self()), 'f', 1) + "°");
  }
  QString state;

  if(reply->has_state()) {
    switch (reply->state()) {
      case kIdlingState://!< ожидание приёма, холостой ход
        state = "ожидание приёма, холостой ход";
      break;
      case kWarmingState://!< подготовка к приёму, "прогрев"
        state = "подготовка к приёму,(прогрев)";
      break;
      case kActiveState://!< приём сеанса
        state = "приём сеанса";
      break;
      case kSkippingState://!< пассивное ожидание завершения приёма
        state = "пассивное ожидание завершения приёма";
      break;
      case kServiceState://!< сервисный режим
        state = "сервисный режим";
      break;
      case kFindState://!< поиск сеанса для приёма
        state = "поиск сеанса для приёма";
      break;
      case kPreWaitState://!< предварительное ожидание начала приёма
        state = "предварительное ожидание начала приёма";
      break;
      case kSyncState://!< синхронизация
        state = "синхронизация";
      break;
    }

  } else {
    state = "не определено";
  }

  setMsgFromAntenna( state );

  /* debug_log << reply->Utf8DebugString();
  if (reply->has_satellite() && reply->has_revol()) {
    if (reply->has_time_offset()) {
      timeOffset_ = reply->time_offset();
    }*/

  //    auto session = scheduleTable_->getSession(QString::fromStdString(reply->satellite()),
  //					      reply->revol(), reply->time_offset());

  //    slotChangedNear(std::make_shared<schedule::Session>(session));
  //    slotChangedSession(session);

}


void SessionsWidget::deleteAppManagerChannel()
{
  if(nullptr != chAppManager_) {
    QObject::disconnect(chAppManager_, 0, this, 0);
    chAppManager_->deleteLater();
    chAppManager_ = nullptr;
  }

  // if (nullptr != ctrlAppManager_) {
  //   ctrlAppManager_->deleteLater();
  //   ctrlAppManager_ = nullptr;
  // }
}

void SessionsWidget::slotInitAppManagerChannel() {
  deleteAppManagerChannel();
  slotConnectToAppManager();
}

void SessionsWidget::slotConnectToAppManager()
{
  if(nullptr == chAppManager_) {
    chAppManager_ = meteo::global::serviceChannel(meteo::settings::proto::kDiagnostic);
    if(nullptr == chAppManager_) {
      QTimer::singleShot(kTimeoutConnect, this, &SessionsWidget::slotConnectToAppManager);
      return;
    }
    QObject::connect(chAppManager_, &meteo::rpc::Channel::disconnected, this, &SessionsWidget::slotDisconnectedFromAppManager);
  }

  if(false == chAppManager_->isConnected()) {
    //  if(false == chAppManager_->connect()) {
    deleteAppManagerChannel();
    QTimer::singleShot(kTimeoutConnect, this, &SessionsWidget::slotConnectToAppManager);
    return;
    //  }
  }
  subscribeAppManager();
  receiverRunning_ = getReceiverServiceRunning();
}

void SessionsWidget::slotDisconnectedFromAppManager()
{
  deleteAppManagerChannel();
  QTimer::singleShot(kTimeoutConnect, this, &SessionsWidget::slotConnectToAppManager);
}

bool SessionsWidget::subscribeAppManager()
{
  if(nullptr == chAppManager_) return false;
  if(false == chAppManager_->isConnected()) return false;

  // ctrlAppManager_ = new rpc::TController(chAppManager_);

  bool ok = chAppManager_->subscribe(&meteo::app::ControlService::ProcChanged, meteo::app::Dummy(),
                                     this, &SessionsWidget::callbackAppManager);
  if(false == ok) {
    error_log << QObject::tr("Не удалось подписаться на получение информации о состоянии приемника");
    QTimer::singleShot(kTimeoutConnect, this, &SessionsWidget::slotInitAppManagerChannel);
  }
  return ok;
}

void SessionsWidget::callbackAppManager(meteo::app::AppState_Proc* reply)
{
  if(reply->has_app()) {
    if(reply->app().has_id() &&
       QString::fromStdString(reply->app().id()) == kReceiverApplication) {
      if(reply->has_state()) {
        receiverRunning_ = reply->state() == meteo::app::AppState_ProcState_PROC_RUNNING;
        if(receiverRunning_) {
          if(nullptr == chAntenna_) {
            QTimer::singleShot(1000, this, &SessionsWidget::slotConnectToAntenna);
          }
        }
        refreshReceiverControls();
      }
    }
  }
}


void SessionsWidget::refreshReceiverControls()
{
  if(receiverRunning_) {
    ui_->receiveCtrlBtn->setIcon(QPixmap(":/mappi/icons/stop_proc.png"));
  }
  else {
    ui_->receiveCtrlBtn->setIcon(QPixmap(":/mappi/icons/run_proc.png"));
  }
}

void SessionsWidget::slotClickReceiverCtrl() const
{
  receiverRunning_ ? stopReceiverApp() : startReceiverApp();
}

void SessionsWidget::startReceiverApp() // static
{
  auto* ch = meteo::global::serviceChannel(meteo::settings::proto::kDiagnostic);
  if(nullptr != ch)
  {
    auto* processes = ch->remoteCall(&meteo::app::ControlService::GetProcList, meteo::app::Dummy(), kTimeoutRequest, true);
    if(nullptr != processes)
    {
      for(const auto& process : processes->procs()) {
        if(QString::fromStdString(process.app().id()) == kReceiverApplication) {
          if(process.state() == meteo::app::AppState_ProcState_PROC_STOPPED) {
            meteo::app::AppState_Proc request;
            request.set_id(process.id());
            request.mutable_app()->CopyFrom(process.app());
            request.set_state(meteo::app::AppState_ProcState_PROC_RUNNING);
            auto* processes = ch->remoteCall(&meteo::app::ControlService::StartProc, request, kTimeoutRequest, true);
            if(nullptr != processes) {
              delete processes;
            }
            delete ch;

          }
          break;
        }
      }
      delete processes;
    }
  }
}

void SessionsWidget::stopReceiverApp()  // static
{
  auto* ch = meteo::global::serviceChannel(meteo::settings::proto::kDiagnostic);
  var(ch);
  if(nullptr != ch)
  {
    auto* processes = ch->remoteCall(&meteo::app::ControlService::GetProcList, meteo::app::Dummy(), kTimeoutRequest, true);
    var(processes);
    if(nullptr != processes)
    {
      for(const auto& process : processes->procs()) {
        if(QString::fromStdString(process.app().id()) == kReceiverApplication) {
          if(process.state() == meteo::app::AppState_ProcState_PROC_RUNNING) {
            meteo::app::AppState_Proc request;
            request.set_id(process.id());
            request.mutable_app()->CopyFrom(process.app());
            request.set_state(meteo::app::AppState_ProcState_PROC_STOPPED);
            auto* processes = ch->remoteCall(&meteo::app::ControlService::StopProc, request, kTimeoutRequest, true);
            if(nullptr != processes) {
              delete processes;
            }
            delete ch;

          }
          break;
        }
      }
      delete processes;
    }
  }
}

bool SessionsWidget::getReceiverServiceRunning() //static
{
  bool running = true;
  auto* ch = meteo::global::serviceChannel(meteo::settings::proto::kDiagnostic);
  if(nullptr != ch)
  {
    auto* processes = ch->remoteCall(&meteo::app::ControlService::GetProcList, meteo::app::Dummy(), kTimeoutRequest, true);
    if(nullptr != processes)
    {
      for(const auto& process : processes->procs()) {
        if(QString::fromStdString(process.app().id()) == kReceiverApplication) {
          running = process.state() == meteo::app::AppState_ProcState_PROC_RUNNING;
          break;
        }
      }
      delete processes;
    }
    delete ch;
  }
  return running;
}

/*!
 * \brief Изменить режим отображения карты сессии
 * \param index Индекс кнопки в порядке добавления (0 - stereoBtn, 1 - mercatBtn)
 * \param state Состояние кнопки
 */
void SessionsWidget::slotToggleMapMode(int index, bool state)
{
  if(false == state) return;
  switch (index) {
    case SatelliteTrackerScene::MapType::kOrbita:
      satelliteTracker_->changeMapMode(SatelliteTrackerScene::MapType::kOrbita);
    break;
    case SatelliteTrackerScene::MapType::kReceiver:
      satelliteTracker_->changeMapMode(SatelliteTrackerScene::MapType::kReceiver);
    break;
    default:
    break;
  }
}

/*! Спрятать/показать карту */
void SessionsWidget::slotToggleVisibleMap()
{
  showMap_ = !showMap_;
  ui_->iSessionsGbox->setVisible(showMap_);
  ui_->asideBtn->setText(showMap_ ? QObject::tr("<<") : QObject::tr(">>"));
}

/*! Скрыть/показать список ошибок */
void SessionsWidget::slotToggleShowErrorList()
{
  showErrorList_ = !showErrorList_;
  errorDialog_->setVisible(showErrorList_);
}

//!< Создать обычный / серый глаз
QPixmap SessionsWidget::getEyeIcon(bool active) // static
{
  QPixmap eye = QPixmap(":mappi/icons/eye.png");
  if(false == active) {
    QPixmap pix(eye.size());
    pix.fill(Qt::gray);
    pix.setMask(eye.createMaskFromColor(Qt::transparent));
    return pix;
  }
  return eye;
}

/*!
 * \brief SessionsWidget::addMsgToLog Добавить сообщение в журнала
 * \param message
 */
void SessionsWidget::addMsgToLog(const QString& message)
{
  errorsModel_->insertRow(errorsModel_->rowCount());
  auto index = errorsModel_->index(errorsModel_->rowCount() - 1);
  errorsModel_->setData(index, currentDt().toString("dd.MM.yyyy HH:mm:ss ") + message);
}

void SessionsWidget::setMsgFromReceiver(const QString& message)
{
  ui_->receiverMsgLbl->setText(message);
}

void SessionsWidget::setMsgFromAntenna(const QString& message)
{
  ui_->antennaMsgLbl->setText(message);
}


bool SessionsWidget::getSessions()
{
  auto* ch = meteo::global::serviceChannel(meteo::settings::proto::kSchedule);

  if (ch == nullptr) {
    warning_log << QObject::tr("%1 недоступен")
                   .arg(meteo::global::serviceTitle(meteo::settings::proto::kSchedule));
    return false;
  }
  conf::Session session;
  //! сеанс
  conf::SessionResponse *resp = nullptr;

  resp = ch->remoteCall(&conf::ScheduleService::NextSession, session, 10000, true);
  QSharedPointer<conf::SessionResponse> next_shedule_(resp);
  resp = ch->remoteCall(&conf::ScheduleService::NearSession, Dummy(), 10000, true);
  QSharedPointer<conf::SessionResponse> near_shedule_(resp);

  if (near_shedule_.isNull()||next_shedule_.isNull()) {
    error_log << QObject::tr("Сервис вернул пустой ответ");
    return false;
  }

  if (near_shedule_->result() == false || next_shedule_->result() == false) {
    if (resp->has_comment())
      error_log << resp->comment().c_str();
    return false;
  }
  std::shared_ptr<schedule::Session> near;
  near = std::make_shared<schedule::Session>();
  conf::TleItem tle;
  schedule::Session::fromProto(resp->session(), near.get());
  slotChangedNear(near);
  slotChangedSession(near, resp->tle());
  return true;
}

} //mappi
