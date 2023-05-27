#include "scheduletable.h"

#include "ui_scheduletable.h"

#include <meteo/commons/global/global.h>

namespace mappi
{
static const int kTimeoutConnect  = 30000; //!< Таймаут соединения
//static const int kTimeoutRequest  = 10000; //!< Таймаут запроса


ScheduleTable::ScheduleTable(QWidget* parent /*=*/) :
  QWidget(parent),
  ui_(new Ui::ScheduleTable)
{
  ui_->setupUi(this);
  getShedule();
}

ScheduleTable::~ScheduleTable()
{
}

void ScheduleTable::sheckShedule()
{
    getShedule();
  }


void ScheduleTable::deleteShedulerChannel()
{
  if(nullptr != chSheduler_) {
    disconnect(chSheduler_, 0, this, 0);
    chSheduler_->deleteLater();
    chSheduler_ = nullptr;
  }
}

void ScheduleTable::slotDisconnectedFromSheduler()
{
  deleteShedulerChannel();
  QTimer::singleShot(kTimeoutConnect, this, &ScheduleTable::slotConnectToSheduler);
}

void ScheduleTable::slotConnectToSheduler()
{
  if(nullptr == chSheduler_)
  {
    chSheduler_ = meteo::global::serviceChannel(meteo::settings::proto::kAntenna);
    if(nullptr == chSheduler_) {
      QTimer::singleShot(kTimeoutConnect, this, &ScheduleTable::slotConnectToSheduler);
      return;
    }
  }

  if(false == chSheduler_->isConnected())
  {
    deleteShedulerChannel();
    QTimer::singleShot(kTimeoutConnect, this, &ScheduleTable::slotConnectToSheduler);
    return;
  }
  connect(chSheduler_, &meteo::rpc::Channel::disconnected, this, &ScheduleTable::slotDisconnectedFromSheduler);

  // subscribe();
}

bool ScheduleTable::getShedule()
{
  //mappi::schedule::Context *ctx
  auto* ch = meteo::global::serviceChannel(meteo::settings::proto::kSchedule);
  if (ch == nullptr) {
    warning_log << QObject::tr("%1 недоступен")
                   .arg(meteo::global::serviceTitle(meteo::settings::proto::kSchedule));
    return false;
  }
  //   schedule::Session;
  conf::Session session;
  //! сеанс
  conf::ScheduleResponse *resp = nullptr;
  resp = ch->remoteCall(&conf::ScheduleService::CurrentSchedule, Dummy(), 10000, true);  // текущее расписаниe
  QSharedPointer<conf::ScheduleResponse> shedule_(resp);
  if (shedule_.isNull()) {
    error_log << QObject::tr("Сервис вернул пустой ответ");
    return false;
  }

  if (shedule_->result() == false ) {
    if (resp->has_comment())
      error_log << resp->comment().c_str();
    return false;
  }
  for(const auto &session: resp->list()){
    schedule::Session line;
    schedule::Session::fromProto(session, &line);
    insertRow(line);
  }
  ui_->scheduleTbl->sortItems(kAos);

  return true;
  //Session::fromProto(resp->session(), &ctx->session);
}

void ScheduleTable::insertRow(const schedule::Session &item)
{
  if(nullptr == ui_->scheduleTbl) return;
  int row = ui_->scheduleTbl->rowCount();
  ui_->scheduleTbl->insertRow(row);
  for(auto cel : { kAos,kLos,  kName, kNapravl, kVitokNum, kElevatMax, kConfl}) {
    ui_->scheduleTbl->setItem(row, cel, createCell(cel, item));
  }
}

QTableWidgetItem* ScheduleTable::createCell(Column col, const schedule::Session &sitem)
{
  QTableWidgetItem* item = new QTableWidgetItem();
  fillCell(item, col, sitem);
  return item;
}

void ScheduleTable::fillCell(QTableWidgetItem *item, Column col, const schedule::Session &sitem)
{
  //static const QPixmap kEnableThem    = QPixmap(":/mappi/icons/restart_green.png");
  //static const QPixmap kDisableThem   = QPixmap(":/mappi/icons/restart_red.png");
  /*
  session->data_.conflState = other.confl_state();
  */
  switch (col) {
    case kAos:
     // item->setText(sitem.data().aos.toString("hh:mm:ss dd-MM-yyyy"));
      item->setData(Qt::DisplayRole, sitem.data().aos);
      //item->setToolTip(them.enabled() ? QObject::tr("Выполнять") : QObject::tr("Пропускать"));
    break;

    case kLos:
      item->setData(Qt::DisplayRole, sitem.data().los);
      //item->setText(sitem.data().los.toString("hh:mm:ss dd-MM-yyyy"));
    break;
    case kName:
      item->setText(sitem.data().satellite);
    break;
    case kNapravl:
    {
      QString vitok = "не определен";
      switch (sitem.data().direction) {
        case conf::kDescending://!< нисходящее
          vitok = "нисходящий";
        break;
        case conf::kAscending://!< восходящий
          vitok = "восходящий";
        break;
        default:
        break;
      }
      item->setText(vitok);
      break;
    }
    case kElevatMax:
      item->setText(QString::number(MnMath::rad2deg(sitem.data().elevatMax)));
    break;
    case kVitokNum:
      item->setText(QString::number(sitem.data().revol));
    break;

    case kConfl:
    {
      QString confl = "не определен";
      switch (sitem.data().conflState) {
        case conf::kNormalState://!< не конфликтует
          confl = "нет";
        break;
        case conf::kSelfState://!< конфликт решён в пользу этого спутника
          confl = "решён";
        break;
        case conf::kOtherState://!<  конфликт решён в пользу другого спутника
          confl = "решён в пользу другого спутника ";
        break;
        case conf::kUserSelfState://!< конфликт решён пользователем в пользу этого спутника
          confl = "решён пользователем";
        break;
        case conf::kUserOtherState://!< конфликт решён пользователем в пользу другого спутника
          confl = "решён пользователем в пользу другого спутника";
        break;
        default:  //    kUnkState = -1;       //!< неизвестно

        break;
      }
      item->setText(confl);
      break;
    }

    default: break;
  }
}


}
