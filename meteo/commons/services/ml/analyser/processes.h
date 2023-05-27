#ifndef METEO_ML_ANALYSER_PROCESSES_H
#define METEO_ML_ANALYSER_PROCESSES_H

#include <stdlib.h>
#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/paths.h>
#include <cross-commons/app/options.h>
#include <meteo/commons/settings/settings.h>
#include <meteo/commons/proto/field.pb.h>
#include <meteo/commons/proto/forecast.pb.h>
#include <meteo/commons/proto/process.pb.h>

#include <meteo/commons/rpc/rpc.h>
#include <meteo/commons/services/obanal/tobanalservice.h>
#include <meteo/commons/proto/sprinf.pb.h>
#include <meteo/commons/proto/field.pb.h>

#include <meteo/commons/proto/surface.pb.h>
#include <commons/textproto/pbtools.h>
#include <commons/meteo_data/meteo_data.h>
#include <commons/meteo_data/complexmeteo.h>
#include <commons/meteo_data/tmeteodescr.h>
#include <commons/mathtools/mnmath.h>
#include <meteo/commons/services/obanal/tobanalservice.h>
#include <meteo/commons/services/obanal/tobanaldb.h>
#include <commons/obanal/func_obanal.h>

#include <commons/geobasis/geodata.h>
//#include <meteo/commons/services/fieldata/fieldservicedb.h>

namespace meteo {
namespace ml {

/**
 * Класс работы с очередями и процессами выполняемыми постепенно
 */
class Processes
{

public:
  Processes();
  ~Processes();

// загружаем незавершенные процессы
Processes&        loadProcess();
// сохраняем  процессы
Processes&        saveProcess();
// процессим процесс
Processes&        process();


/**
 * Переключения режима отладки
 * @return [description]
 */
// отправляем сообщение в дебаг, если надо
Processes&         debugMSG( QString txt );
Processes&         setDebugOn();
Processes&         setDebugOff();

// считываем файл из базы
Processes&         getFile( QByteArray& data );
// сохраняеем файл в базу
Processes&         saveFile(QByteArray data);

// подключаемся к произвольному сервису
Processes&         connect(meteo::settings::proto::ServiceCode srv);
// подключение к сервису исходников
Processes&         connectToSrcSRV();

Processes&         disconnect();

// Проверяем есть ли ошикбки?
bool                      checkError();
// получаем сообщение об ошибке
QString                   getErrorMsg();
// очищаем данные (подготавливаем)
Processes&         clearData();

// рассчитываем шаги, на основе информации о заблаговременности
// days_before - сколько дней назад отсчитываем. текущая дата - days_before
Processes&        calcStepFromDate( int days_before );

//
//  ========   SETTERS   =========
//
Processes& setProcessType(QString process){ this->processParams->set_processtype(process.toStdString()); return *this; }
Processes& setFileName(QString filename)  { this->processParams->set_filename(filename.toStdString()); return *this; }
//                 добавляем станции в лист
Processes& addStation(QString st)  { this->processParams->add_stantions(st.toStdString()); return *this; }
//                 добавляем станции, по которым будем проводить расчеты
Processes& addStationPdc(QString st)  { this->processParams->add_stantionspdc(st.toStdString()); return *this; }
// добавляем параметр которых хотим получить
Processes& addParams(QString st)  { this->processParams->add_params(st.toStdString()); return *this; }
// добавляем параметры на которых будем считать
Processes& addParamsPdc(QString st)  { this->processParams->add_paramspdc(st.toStdString()); return *this; }
//                 записываем последнее значение
Processes& setLastVal(QString st){ this->processParams->set_lastval( st.toStdString() );return *this; };

//
//  ========   GETTERS   =========
//
// получение переменной
QString        getProcessId();
// получение переменной
QString        getFileId();
// получение переменной
int            getNumberOfExec();
// получение переменной
bool           getIsFinished();
// получение переменной
QString        getError();
// получение переменной
QString        getProcessType();
QString        getLastVal(){ return QString::fromStdString(this->processParams->lastval()); };
int            getProgress();

// гененируем имя файла
// если файл загружен - то возвращаем то имя, оторое уже есть
QString        getFileName();

// станции, по которым будем прогнозировать (те, которые нужны)
QList<QString> getStations();
// станции, на данных которых будем обучать модель
QList<QString> getStationsPdc();

// метеопараметры, по которым будем прогнозировать (те, которые нужны)
QList<int> getParams();
// метеопараметры, на данных которых будем обучать модель
QList<int> getParamsPdc();

private:

  QDateTime*     req_date;

  // описание самого процесса
  meteo::forecast::ProcessParams* processParams= nullptr;

  // коллекция, в которой будем хранить обучающиеся модели
  QString        collection = "mlmodel";

  bool isDebug            = false;

  // сообщение об ошибке
  QString *errormsg       = nullptr;

  // коннектор к сервисам
  meteo::rpc::Channel* ctrl       = nullptr;
  // класс работы с базой данных полей
};

}
}

#endif
