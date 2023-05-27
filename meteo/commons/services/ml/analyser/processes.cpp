#include "processes.h"
#include <meteo/commons/global/global.h>


namespace meteo {
namespace ml {

Processes::Processes()
{
  // дату для запроса данных
  req_date = new QDateTime(QDateTime::currentDateTimeUtc());

  // класс работы с базой прогностических полей

  processParams = new meteo::forecast::ProcessParams();

  // инициализируем переменные
  clearData();
}

Processes::~Processes()
{
  delete req_date;
  delete processParams;
}


/**
 * функция загрузки процессов из базы
 * @return [description]
 */
Processes&        Processes::loadProcess(){

  TObanalDb* obanal_db;
  obanal_db = new TObanalDb();

  if ( false == obanal_db->loadProcess( *processParams ) ){
    debugMSG("Неудалось загрузить данные по очередности процессов из базы");
    errormsg->append("\nНеудалось загрузить данные по очередности процессов из базы");
  };

  delete obanal_db;

  return *this;
}


/**
 * функция сохранения процесса в базу
 * @return [description]
 */
Processes&        Processes::saveProcess(){

  TObanalDb* obanal_db;
  obanal_db = new TObanalDb();

  processParams->set_filename( getFileName().toStdString() );

  if ( false == obanal_db->saveProcess( *processParams ) ){
    debugMSG("Неудалось сохранить данные по очередности процессов в базу");
    errormsg->append("\nНеудалось сохранить данные по очередности процессов в базу");
  };

  delete obanal_db;

  return *this;
}



/**
 * запускаем процесс дальше
 * @return [description]
 */
Processes&        Processes::process(){

  if ( !processParams->isfinished()  ){

    // увеличиваем количество запусков процесса
    processParams->set_numberofexec( processParams->numberofexec()+1 );

    // прогрессим
    processParams->set_progress( processParams->progress()+processParams->step_percent() );

    // если джостигли пределеа
    // завершаем процесс
    if ( processParams->progress() >=100 ){
      processParams->set_progress(100);
      processParams->set_isfinished(true);
    }
  }

  return *this;
}

/**
 * получаем имя файла
 * если файл не загружен, то генерируем имя
 * @return [description]
 */
QString                   Processes::getFileName(){
  QString filename;
  if ( !processParams->has_filename() ||
       QString::fromStdString(processParams->filename()).length()==0 ){


    if ( processParams->stantions_size()>0 ){
      filename.append( QString::fromStdString(processParams->processtype()) )
              .append( "_" )
              .append( QString::fromStdString(processParams->stantions(0)) )
              .append( "_" )
              .append( QDateTime::currentDateTimeUtc().toString("dd.MM.yyyy_hh:mm:ss.z") );

      processParams->set_filename( filename.toStdString() );
    }

  }else{
    filename = QString::fromStdString(processParams->filename());
  }
  return filename;
}


/**
 * считываем файл из базы
 * @param  data [description]
 * @return      [description]
 */
Processes&        Processes::getFile( QByteArray& data ){
  //
  // должно быть заполнено поле fileId
  //
  TObanalDb* obanal_db;
  obanal_db = new TObanalDb();

  if ( false == obanal_db->readFileByName( data, getFileName() , collection ) ){
    debugMSG( QString("Неудалось считать файл из базы с айди %1").arg( getFileName() ) );
    errormsg->append( QString("\nНеудалось считать файл из базы с айди %1").arg( getFileName() ) );
  };

  delete obanal_db;

  return *this;
}

/**
 * Сохраняем файл в базу
 * @param  data [description]
 * @return      [description]
 */
Processes&        Processes::saveFile(QByteArray data){
  TObanalDb* obanal_db;
  obanal_db = new TObanalDb();

  if ( false == obanal_db->saveBinaryToFile( collection, getFileName() , data ) ){
    debugMSG( QString("Неудалось сохранить файл в базу с именем %1").arg( getFileName() ) );
    errormsg->append( QString("\nНеудалось сохранить файл в базу с именем %1").arg( getFileName() ) );
  };

  delete obanal_db;
  return *this;
}

/**
 *
 *  ===    HELP FUNCTIONS    ====
 *
 */


/**
 * получение значения fileId
 * @return [description]
 */
QString        Processes::getFileId(){
  return QString::fromStdString(processParams->fileid());
}

/**
 * получение значения numberOfExec
 * @return [description]
 */
int            Processes::getNumberOfExec(){
  return processParams->numberofexec();
}

/**
 * получение значения isFinished
 * @return [description]
 */
bool           Processes::getIsFinished(){
  return processParams->isfinished();
}

/**
 * получение значения error
 * @return [description]
 */
QString        Processes::getError(){
  return QString::fromStdString(processParams->error());
}

/**
 * получение значения processType
 * @return [description]
 */
QString        Processes::getProcessType(){
  return QString::fromStdString(processParams->processtype());
}

/**
 * получение значения stationList
 * @return [description]
 */
QList<QString>        Processes::getStations(){
  QList<QString> st;

  for ( int i=0; i<processParams->stantions_size(); i++  ){
    st.append( QString::fromStdString(processParams->stantions(i)) );
  }
  return st;
}

/**
 * получение список станций, по которым будем прогнозировать и обучать модель
 * @return [description]
 */
QList<QString>        Processes::getStationsPdc(){
  QList<QString> st;

  for ( int i=0; i<processParams->stantionspdc_size(); i++  ){
    st.append( QString::fromStdString(processParams->stantionspdc(i)) );
  }
  return st;
}


/**
 * получение значения stationList
 * @return [description]
 */
QList<int>        Processes::getParams(){
  QList<int> st;

  for ( int i=0; i<processParams->params_size(); i++  ){
    st.append( QString::fromStdString(processParams->params(i)).toInt() );
  }
  return st;
}

/**
 * получение список метеопараметров, по которым будем прогнозировать и обучать модель
 * @return [description]
 */
QList<int>        Processes::getParamsPdc(){
  QList<int> st;

  for ( int i=0; i<processParams->paramspdc_size(); i++  ){
    st.append( QString::fromStdString(processParams->paramspdc(i)).toInt() );
  }
  return st;
}

/**
 * Очищаем данные, чтобы работать дальше
 * параметры не трогаем
 * @return [description]
 */
Processes& Processes::clearData(){

  errormsg     = new QString();
  processParams->clear_filename();
  processParams->clear_processid();
  processParams->clear_fileid();
  processParams->clear_numberofexec();
  processParams->clear_isfinished();
  processParams->clear_error();
  processParams->clear_processtype();
  processParams->clear_lastval();
  processParams->clear_progress();
  processParams->clear_stantions();
  processParams->clear_stantionspdc();
  return *this;
}

/**
 * Проверяем есть ли ошибки
 * @return [description]
 */
bool Processes::checkError(){
  if ( errormsg->isEmpty() ){
    return false;
  }else{
    return true;
  }
}

/**
 * получаем текст ошибки
 * @return [description]
 */
QString Processes::getErrorMsg(){
  return *errormsg;
}


/**
 * // рассчитываем шаги, на основе информации о заблаговременности
      days_before - сколько дней назад отсчитываем. текущая дата - days_before
 * @param  days_before [description]
 * @return             [description]
 */
Processes& Processes::calcStepFromDate(int days_before){
  //минимальное количество итерайий для шага
  int min_step_iterations = 90;

  debugMSG("Пробуем рассчитать количество итерайи процесса...");

  if ( !processParams->has_step() || processParams->step()==0 ){
    // задаем время
    QDateTime dt = QDateTime::currentDateTimeUtc().addDays(-days_before);
    // ставим нули
    dt.setTime( QTime(0,0,0) );
    // задаем последнее значение
    debugMSG( QString("Задаем последнее значение даты %1").arg(dt.toString(Qt::ISODate)) );
    processParams->set_lastval( dt.toString(Qt::ISODate).toStdString() );
    // считаем скольо всего шагов
    // например, 90 дней всего, с чагом в 3 часа
    // получается 90 * (24 / 3) итераций
    int iterations;
    // если шаг по часам не задан - задем его 12
    // главное чтоб не 0
    if ( !processParams->has_hour_step() || processParams->hour_step() == 0 ){
      processParams->set_hour_step(12);
    }

    // смотрим сколько итраций
    iterations = days_before * ( 24 / processParams->hour_step() );

    debugMSG( QString("Всего итераций %1").arg(iterations) );

    // за раз меньше 90 не имеет смысла брать даже
    // если итераций меньше - отработаем за один шаг
    if (iterations < 100){
      processParams->set_step(1)  ;
      processParams->set_step_percent(100);
      debugMSG( QString("Заданы шаги %1 и %2").arg(processParams->step() )
                          .arg(processParams->step_percent() ) );
    }else{
      processParams->set_step( (iterations/min_step_iterations) +1 );
      processParams->set_step_percent( 100/processParams->step()  );
      debugMSG( QString("Заданы шаги %1 и %2").arg(processParams->step() )
                          .arg(processParams->step_percent() ) );
    }
  }
  return *this;
}
/**
 * подключаемся к сервису
 */
Processes& Processes::connect( meteo::settings::proto::ServiceCode srv ){
  // создаем коннектор
  ctrl = meteo::global::serviceChannel( srv );
  if(0 == ctrl) {
    error_log << "Не удалось подключиться к сервису";
  }
  return *this;
}


/**
 * подключаемся к сервису сухопутному
 * @return [description]
 */
Processes& Processes::connectToSrcSRV() {
  connect(meteo::settings::proto::kSrcData);
  return *this;
}


/**
 * отключаемся от сервисов
 * @return [description]
 */
Processes& Processes::disconnect() {
  delete ctrl;
  return *this;
}


/**
 * Отправляем сообщение в дебаг, если надо
 * @param  txt [description]
 * @return     [description]
 */
Processes&  Processes::debugMSG( QString txt ){
  if ( isDebug ){
    error_log << txt;
  }
  return *this;
}




/**
 * Включение/выключение режима отладки
 * @return [description]
 */
Processes&  Processes::setDebugOn(){
  isDebug = true;
  return *this;
}

/**
 * Включение/выключение режима отладки
 * @return [description]
 */
Processes&  Processes::setDebugOff(){
  isDebug = false;
  return *this;
}



} // namespace ml
} //namespace meteo
