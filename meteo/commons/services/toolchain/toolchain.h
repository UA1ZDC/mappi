#ifndef METEO_TOOLCHAIN_H
#define METEO_TOOLCHAIN_H

#include <stdlib.h>
#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/paths.h>
#include <cross-commons/app/options.h>
#include <meteo/commons/settings/settings.h>
#include <meteo/commons/proto/field.pb.h>

#include <meteo/commons/rpc/rpc.h>
#include <meteo/commons/proto/sprinf.pb.h>
#include <meteo/commons/proto/field.pb.h>

#include <meteo/commons/proto/surface.pb.h>
#include <commons/meteo_data/tmeteodescr.h>


namespace meteo {
namespace tools {

/**
 * Класс работы с данными из консоли
 * получения погоды за дату, сохранения грибов и т.д.
 */
class Toolchain
{

public:
  Toolchain();
  ~Toolchain();

/**
 * Переключения режима отладки
 * @return [description]
 */
// отправляем сообщение в дебаг, если надо
Toolchain&         debugMSG( QString txt );
Toolchain&         setDebugOn();
Toolchain&         setDebugOff();


//
// подключаемся к сервису
//
// подключаемся к произвольному сервису
Toolchain&         connect(meteo::settings::proto::ServiceCode srv);
// подключение к сервису исходников
Toolchain&         connectToSrcSRV();
// подключение к сервису полей
Toolchain&         connectToFieldSRV();
// подключение к сервису данных
Toolchain&         connectToSprinfSRV();
// подключение к сервису прогнозов
Toolchain&         connectToForecastSRV();

Toolchain&         disconnect();

// Проверяем есть ли ошикбки?
bool                      checkError();
// получаем сообщение об ошибке
QString                   getErrorMsg();


private:

  QDateTime*     req_date_;

  bool isDebug_            = false;

  // сообщение об ошибке
  QString *errormsg_       = nullptr;

  // коннектор к сервисам
  meteo::rpc::Channel* ctrl_       = nullptr;
};

}
}

#endif
