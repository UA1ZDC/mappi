#include <stdlib.h>
#include <iostream>

#include <qtextcodec.h>

#include <cross-commons/app/paths.h>
#include <cross-commons/debug/tlog.h>

#include <commons/textproto/tprototext.h>

#include <meteo/commons/proto/document_service.pb.h>
#include <meteo/commons/global/weatherloader.h>

#include <meteo/commons/settings/settings.h>
#include <meteo/commons/global/global.h>
#include <meteo/commons/proto/map_document.pb.h>

#include "mapclient.h"

using commons::ArgParser;
using commons::Arg;

namespace {



std::string paramhelp( const Arg& arg, const QString& descr )
{
  QString str = QObject::tr("    %1")
                .arg( arg.help() ).leftJustified(40);
  return (str+descr).toStdString();
}

void usage()
{
  std::cout << QObject::tr("Программа формирования карт погоды.").toStdString() << '\n';
  std::cout
      << QObject::tr("Использование: ").toStdString()
      << qApp->applicationName().toStdString() << '\n'
      << QObject::tr("Параметры:").toStdString()
      << '\n';

  std::cout << paramhelp( kJob,         QObject::tr("Наименование задания, которую требуется выполнить") ) << '\n';
  std::cout << paramhelp( kList,        QObject::tr("Список заданий, которые можно выполнить") ) << '\n';
  std::cout << paramhelp( kAvai,        QObject::tr("Список доступных документов") ) << '\n';
  std::cout << paramhelp( kDate,        QObject::tr("Срок, за который будет построена карта") ) << '\n';
  std::cout << paramhelp( kBegin,       QObject::tr("Начальный срок, за который будут построены карты") ) << '\n';
  std::cout << paramhelp( kEnd,         QObject::tr("Конечный срок, за который будут построены карты") ) << '\n';
  std::cout << paramhelp( kMap,         QObject::tr("Наименование карты, которую необходимо построить") ) << '\n';
  std::cout << paramhelp( kFormat,      QObject::tr("Формат карты (jpeg, bmp, png, sxf)") ) << '\n';
  std::cout << paramhelp( kHour,        QObject::tr("Срок прогноза,ч") ) << '\n';
  std::cout << paramhelp( kCenterParam,      QObject::tr("Номер центра прогнозирования") ) << '\n';
  std::cout << paramhelp( kModel,       QObject::tr("Номер модели, используемой центром прогнозирования") ) << '\n';
  std::cout << paramhelp( kPath,        QObject::tr("Имя файла для сохранения построенной карты") ) << '\n';
  std::cout << paramhelp( kMapCnt,      QObject::tr("Географические координаты центра карты в фомате ШИРxДОЛГ, например 60.1x30.2") ) << '\n';
  std::cout << paramhelp( kScale,       QObject::tr("Масштаб карты в диапазоне 1-20") ) << '\n';
  std::cout << paramhelp( kProj,        QObject::tr("Проекция карты (stereo, merkat)") ) << '\n';
  std::cout << paramhelp( kSize,        QObject::tr("Размер документа в пикселах (ШИРИНАxВЫСОТА)") ) << '\n';
  std::cout << paramhelp( kHelp,        QObject::tr("Эта справка") ) << '\n';
}

}

void jobList( const QMap< QString, meteo::map::proto::Job >& jobs )
{
  QMapIterator< QString, meteo::map::proto::Job > it(jobs);
  int i = 1;
  while ( true == it.hasNext() ) {
    it.next();
    const meteo::map::proto::Job& job = it.value();
    std::cout << '\t' << i << ". " << job.name() << '\n';
    ++i;
  }
}

int appMain( int argc, char* argv[] )
{
  TAPPLICATION_NAME("meteo");
  meteo::global::setLogHandler();

  QCoreApplication* app = new QCoreApplication( argc, argv);
  gSettings(meteo::global::Settings::instance());
  if ( false == meteo::global::Settings::instance()->load() ) {
    error_log << QObject::tr("Не удалось загрузить настройки.");
    return EXIT_FAILURE;
  }

  ArgParser* options = ArgParser::instance();
  if ( false == options->parse( argc, argv ) ) {
    usage();
    delete app;
    return EXIT_FAILURE;
  }

  if ( true == options->installed(kHelp) ) {
    usage();
    delete app;
    return EXIT_SUCCESS;
  }
  if ( true == options->installed(kAvai) ) {
    meteo::rpc::Channel* ch = meteo::global::serviceChannel( meteo::settings::proto::kMap );
    if ( nullptr == ch ) {
      error_log << QObject::tr("Не удалось установить соединение с картографическим сервисом");
      return EXIT_FAILURE;
    }
    meteo::map::Client client;
    client.getAvailableDocuments(ch);
    delete ch;
    delete app;
    return EXIT_SUCCESS;
  }
  const QMap< QString, meteo::map::proto::Job >& jobs = meteo::map::WeatherLoader::instance()->mapjobs();
  meteo::map::Client client;
  //  bool ok = false;
  //  client.load(&ok);
  //  if ( !ok ) {
  //    error_log << "Не удалось загрузить необходимые данные";
  //    return EXIT_FAILURE;
  //  }
  QMap< QString, meteo::map::proto::Job >& docJobs = client.docjobs_;

  if ( true == options->installed(kList) ) {
    std::cout << QObject::tr("Список доступных заданий для формализованных документов:").toStdString() << '\n';
    jobList(docJobs);
    std::cout << QObject::tr("Список доступных заданий для картографических документов:").toStdString() << '\n';
    jobList(jobs);
    delete app;
    return EXIT_SUCCESS;
  }

  bool dJob = false;
  if ( true == options->installed(kJob) ) {
    QString jobname = options->at(kJob).value();
    if ( true == docJobs.keys().contains(jobname) )
    {
      dJob = true;
      meteo::map::proto::Job need2beDone= docJobs.value(jobname);
      docJobs.clear();
      docJobs.insert(jobname, need2beDone);
    }
    else {
      docJobs.clear();
    }
  }
  QMapIterator< QString, meteo::map::proto::Job > it(docJobs);
  while (true == it.hasNext())
  {
    it.next();
    const meteo::map::proto::Job& job = it.value();

    if(false == job.enabled())
    {
      warning_log << QObject::tr("Внимание! Задание помечено как неактивное");
      continue;
    }

    auto list = client.termsForJob(job);
    auto formals = meteo::map::WeatherLoader::instance()->formals();
    auto name = QString::fromStdString(job.formal().name());
    meteo::map::proto::Map map;
    if ( true == formals.contains(name) ) {
      map = formals.value(name);
    }
    //map.add_station(job.formal().stnumber());
    map.add_station()->CopyFrom(job.formal().station());
    for ( const auto& dt : list ) {
      map.set_datetime(dt.toString(Qt::ISODate).toStdString());
      auto locjob = job;
      map.set_name( locjob.map_id() );
      map.set_job_name( job.name() );
      locjob.mutable_document()->set_doctype(map.document().doctype());

      locjob.mutable_map()->CopyFrom(map);
      if (false == client.processJob(locjob))
      {
        delete app;
        return EXIT_FAILURE;
      }
    }
  }
  if (dJob) {
    delete app;
    return EXIT_SUCCESS;
  }

  auto cmdmaps = client.getMapsFromCommandLine();
  auto cmdit = cmdmaps.begin();
  while  ( cmdit != cmdmaps.end() ) {

    auto job = cmdit.key();
    if(true == job.enabled())
    {
      for ( auto map : cmdit.value() )
      {
        debug_log << map.Utf8DebugString();

        map.set_job_title(job.title());
        map.set_name( job.map_id() );
        map.set_job_name( job.name() );
        job.mutable_map()->CopyFrom(map);
        if ( false == client.processJob(job) )
        {
          //delete app;
          //return EXIT_FAILURE;
        }
      }
    } else {
      warning_log << QObject::tr("Внимание! Задание помечено как неактивное");
    }
    ++cmdit;
  }
  delete app;
  return EXIT_SUCCESS;
}
