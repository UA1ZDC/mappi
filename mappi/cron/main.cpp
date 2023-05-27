#include <meteo/commons/cron/appmain.h>

int main(int argc,  char** argv)
{
  return appMain(argc, argv);
}



//#include "taskgenerate.h"
// #include "runapp.h"

// #include <qfile.h>
// #include <qdir.h>
// #include <qcoreapplication.h>

// #include <cross-commons/debug/tlog.h>
// #include <cross-commons/app/paths.h>

// #include <meteo/commons/planner/planner.h>
// #include <commons/textproto/tprototext.h>
// #include <mappi/global/global.h>
// #include <mappi/settings/tmeteosettings.h>

// #include <mappi/proto/cron.pb.h>

// namespace {

// void loadRunApps( meteo::Planner* planner, const mappi::cron::Settings& opt )
// {
//   for ( int i = 0, isz = opt.run_app_size(); i < isz; ++i ) {
//     const mappi::cron::RunApp& appconf = opt.run_app(i);
//     meteo::mappi::RunApp* task = new meteo::mappi::RunApp;
//     if ( true == appconf.has_timesheet() ) {
//       task->setTimesheet(
//           meteo::Timesheet::fromString( QString::fromUtf8( appconf.timesheet().c_str() ) )
//           );
//     }
//     if ( true == appconf.has_name() ) {
//       task->setName( QObject::tr( appconf.name().c_str() ) );
//     }
//     if ( false == appconf.has_path() ) {
//       error_log << QObject::tr("Не указан путь к приложению. Конфигурация запуска приложения %1")
//         .arg( QObject::tr( appconf.Utf8DebugString().c_str() ) );
//       continue;
//     }
//     task->setPath( QObject::tr( appconf.path().c_str() ) );
//     for ( int j = 0,jsz = appconf.arg_size(); j < jsz; ++j ) {
//       const mappi::cron::RunApp::Arg& arg = appconf.arg(j);
//       if ( false == arg.has_key() ) {
//         warning_log << QObject::tr("НЕ задано имя параметра в параметрах запуска приложения %1")
//           .arg( QObject::tr( appconf.Utf8DebugString().c_str() ) );
//         continue;
//       }
//       if ( false == arg.has_value() ) {
//         task->addArg( QObject::tr(arg.key().c_str()) );
//       }
//       else {
//         task->addArg( QObject::tr(arg.key().c_str()), QObject::trUtf8(arg.value().c_str()) );
//       }
//     }
//     planner->addTask(task);
//   }
// }

// bool loadConfigPart( mappi::cron::Settings* part, const QString& filename )
// {
//   QFile file(filename);
//   if ( false == file.exists() ) {
//     error_log << QObject::tr("Отсутствует конфигурационный файл '%1'.").arg(file.fileName());
//     return false;
//   }
//   if ( false == file.open(QIODevice::ReadOnly) ) {
//     error_log << QObject::tr("Ошибка при открытии файла конфигурации '%1'").arg(file.fileName());
//     debug_log << file.errorString();
//     return false;
//   }
//   QString text = QString::fromUtf8(file.readAll());

//   if ( false == TProtoText::fillProto(text, part) ) {
//     error_log << QObject::tr("Ошибка при обработке конфигурационного файла '%1'").arg(file.fileName());
//     return false;
//   }
//   return true;
// }

// }

// int main(int argc,  char** argv)
// {
//   TAPPLICATION_NAME("meteo");
// //  TLog::setMsgHandler(&TLog::clearStdOut);
//   // QTextCodec::setCodecForCStrings(QTextCodec::codecForLocale());

//   try {
//     QCoreApplication app(argc, argv);
//     meteo::mappi::TMeteoSettings::instance()->load();
//     //TLog::setMsgHandler(meteo::global::logoutToDb);

//     QDir dir( MnCommon::etcPath() + "cron.d" );
//     if ( false ==  dir.exists() ) {
//       error_log << QObject::tr("Отсутствует директория с конфигурационными файлами '%1'.").arg(dir.dirName());
//       return EXIT_FAILURE;
//     }

//     mappi::cron::Settings opt;

//     QStringList conflist = dir.entryList( QStringList("*.conf"));
//     for ( int i = 0, sz = conflist.size(); i < sz; ++i ) {
//       mappi::cron::Settings part;
//       QString filename = dir.absoluteFilePath( conflist[i] );
//       if ( false  == loadConfigPart( &part, filename ) ) {
//         warning_log << QObject::tr("Не удалось обработать файл %1")
//           .arg(filename);
//         continue;
//       }
//       opt.MergeFrom(part);
//     }

//     meteo::Planner* planner = new meteo::Planner;

//     loadRunApps( planner, opt );

//     QTimer::singleShot( 100, planner, SLOT(slotProcessTasks()) ); // hack to fast start

//     app.exec();

//     delete planner;
//   }
//   catch(const std::bad_alloc& ) {
//     critical_log << QObject::tr("Недостаточно памяти для работы программы");
//     return EXIT_FAILURE;
//   }

//   return EXIT_SUCCESS;
// }
