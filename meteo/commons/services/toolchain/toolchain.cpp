#include "toolchain.h"
#include <meteo/commons/proto/field.pb.h>
#include <meteo/commons/global/global.h>

namespace meteo {
namespace tools {

Toolchain::Toolchain()
{
  
}

Toolchain::~Toolchain()
{
 // delete db_;
}

/**
 * Отправляем сообщение в дебаг, если надо
 * @param  txt [description]
 * @return     [description]
 */
Toolchain&  Toolchain::debugMSG( QString txt ){
  if ( isDebug_ ){
    debug_log << txt;
  }
  return *this;
}


/**
 * Включение/выключение режима отладки
 * @return [description]
 */
Toolchain&  Toolchain::setDebugOn(){
  isDebug_ = true;
  return *this;
}

/**
 * Включение/выключение режима отладки
 * @return [description]
 */
Toolchain&  Toolchain::setDebugOff(){
  isDebug_ = false;
  return *this;
}


/**
 * подключаемся к сервису
 */
Toolchain& Toolchain::connect( meteo::settings::proto::ServiceCode srv ){
  // создаем коннектор
  ctrl_ = meteo::global::serviceChannel( srv );
  if(0 == ctrl_) {
    error_log << "Не удалось подключиться к сервису";
  }
  return *this;
}


/**
 * подключаемся к сервису сухопутному
 * @return [description]
 */
Toolchain& Toolchain::connectToSrcSRV() {
  connect(meteo::settings::proto::kSrcData);
  return *this;
}

/**
 * подключаемся к сервису полей
 * @return [description]
 */
Toolchain& Toolchain::connectToFieldSRV() {
  connect(meteo::settings::proto::kField);
  return *this;
}

/**
 * подключаемся к сервису полей
 * @return [description]
 */
Toolchain& Toolchain::connectToSprinfSRV() {
  connect(meteo::settings::proto::kSprinf);
  return *this;
}


/**
 * подключаемся к сервису полей
 * @return [description]
 */
Toolchain& Toolchain::connectToForecastSRV() {
  connect(meteo::settings::proto::kForecastData);
  return *this;
}


/**
 * отключаемся от сервисов
 * @return [description]
 */
Toolchain& Toolchain::disconnect() {
  delete ctrl_;
  return *this;
}



/**
 * Проверяем есть ли ошибки
 * @return [description]
 */
bool Toolchain::checkError(){
  if ( errormsg_->isEmpty() ){
    return false;
  }else{
    return true;
  }
}

/**
 * получаем текст ошибки
 * @return [description]
 */
QString Toolchain::getErrorMsg(){
  return *errormsg_;
}



} // namespace tools
} //namespace meteo
