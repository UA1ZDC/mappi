#include "sateliteloaderparamswatcher.h"

#include <qobject.h>
#include <meteo/commons/proto/appconf.pb.h>
#include <meteo/commons/global/global.h>
#include <QProcessEnvironment>

template<> meteo::internal::ParamsHandler* meteo::ParamsHandler::_instance = nullptr;

namespace meteo {

namespace internal {

ParamsHandler::ParamsHandler()
{
  notifyTimer_.setInterval(timeout);
  QObject::connect(&notifyTimer_, &QTimer::timeout, this, &ParamsHandler::sendStatsToAppmanager);
  notifyTimer_.start();
}

void ParamsHandler::incParam(const QObject* source, IntegerParam param, int value)
{
  mutex_.lock();

  if ( value != 0 ) {
    StatsHandler* handler = tryGetHandler(source);

    int oldValue = handler->intergerParams_.value(param, 0);
    handler->intergerParams_.insert(param, oldValue + value);
    updated_ = true;
  }
  mutex_.unlock();
}

void ParamsHandler::setBooleanParam(const QObject* source, BooleanParams param, bool value)
{
  mutex_.lock();
  StatsHandler* handler = tryGetHandler(source);

  if ( value != handler->booleanParams_.value(param) || false == handler->booleanParams_.contains(param) ){
    handler->booleanParams_.insert(param, value);
    updated_ = true;
  }
  mutex_.unlock();
}

ParamsHandler::StatsHandler* ParamsHandler::tryGetHandler(const QObject* source) {
  const QString& objectName = source->objectName();
  StatsHandler* handler = handlers_.value(objectName, nullptr);
  if ( nullptr == handler ){
    handler = new StatsHandler();
    handlers_.insert(objectName, handler);
  }
  return handler;
}

void ParamsHandler::fillStateProto(meteo::app::OperationStatus* state)
{
  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
  if ( false == env.contains("procid") ) {
    error_log << QObject::tr("Не удалось получить идентифкатор процесса. Отправка статистики невозможна");
    return;
  }
  auto id = env.value("procid").toLongLong();
  state->set_id(id);
  meteo::app::OperationState totalState = meteo::app::OperationState::OperationState_NORM;

  for ( const QString& decoderName: handlers_.keys() ) {    

    StatsHandler* handler = handlers_[decoderName];

    for ( IntegerParam param: handler->intergerParams_.keys() ){
      int value = handler->intergerParams_.value(param, 0);
      auto valueStr = QString::number(value);
      QString title;
      ::meteo::app::OperationState paramState = ::meteo::app::OperationState::OperationState_NORM;

      switch(param){
      case kTlgReceived:{
        title = QObject::tr("Получено из канала подписки");
        break;
      }
      case kTlgReloaded: {
        title =  QObject::tr("Загружено не раскодированных");
        break;
      }
      case kTlgDecoded:{
        title = QObject::tr("Раскодировано");
        break;
      }
      case kDocumentsBuilded: {
        title = QObject::tr("Собрано");
        break;
      }
      case kTlgDroppedByTimeout:{
        title = QObject::tr("Отброшено из-за слишком длительного ожидания");
        if ( value != 0 ) {
          paramState = ::meteo::app::OperationState::OperationState_WARN;
        }
        break;
      }
      case kTlgPraseError:{
        title = QObject::tr("Раскодированных с ошибками");
        if ( value != 0 ){
          paramState = ::meteo::app::OperationState::OperationState_ERROR;
        }
        break;
      }
      case kSegmentDupCount:{
        title = QObject::tr("Количество принятых дублирующихся сегментов");
        if ( value != 0 ){
          paramState = ::meteo::app::OperationState::OperationState_WARN;
        }
      }
      }
      auto paramProto = state->add_param();
      paramProto->set_title(title.toStdString());
      paramProto->set_value(valueStr.toStdString());
      paramProto->set_state(paramState);
    }

    for ( BooleanParams param: handler->booleanParams_.keys() ){
      int value = handler->booleanParams_.value(param, false);
      QString title;
      QString valueStr;
      ::meteo::app::OperationState paramState = ::meteo::app::OperationState::OperationState_NORM;

      switch (param) {
      case kRpcChannelAlive:{
        title = QObject::tr("Соединение с ЦКС по каналу вызова удаленных процедур");
        if ( true == value ) {
          valueStr = QObject::tr("Установлено");
        }
        else {
          valueStr = QObject::tr("Разорвано");
          paramState = meteo::app::OperationState::OperationState_ERROR;
          totalState = meteo::app::OperationState::OperationState_ERROR;
        }
        break;
      }
      case kSubcribeChannelAlive: {
        title = QObject::tr("Подписка на новые телеграммы");
        if ( true == value ){
          valueStr = QObject::tr("Оформлена");
        }
        else {
          valueStr = QObject::tr("Ожидание подключения");
          paramState = meteo::app::OperationState::OperationState_ERROR;
          totalState = meteo::app::OperationState::OperationState_ERROR;
        }
        break;
      }
      }
      auto paramProto = state->add_param();
      paramProto->set_title(title.toStdString());
      paramProto->set_value(valueStr.toStdString());
      paramProto->set_state(paramState);
    }
  }
  state->set_state(totalState);
}

void ParamsHandler::sendStatsToAppmanager() {
  if ( false == updated_ ){
    return;
  }
  mutex_.lock();
  app::OperationStatus state;
  fillStateProto(&state);
  mutex_.unlock();

  auto channel = std::unique_ptr<rpc::Channel>(global::serviceChannel(settings::proto::kDiagnostic));
  if ( nullptr == channel ){
    error_log << meteo::msglog::kServiceConnectFailedSimple.arg(global::serviceTimeout(settings::proto::kDiagnostic));
    return;
  }
  auto dummy = std::unique_ptr<app::Dummy>(channel->remoteCall(&app::ControlService::SetOperaionStatus, state, 1000 ));
  if ( nullptr == dummy ){
    error_log << QObject::tr("Ошибка при отправке статуса процесса.");
    return;
  }
  this->updated_ = false;
}


}

}
