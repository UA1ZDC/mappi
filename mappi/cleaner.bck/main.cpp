#include "archivetask.h"
#include "removetask.h"
#include "checktask.h"
#include "dbtask.h"
#include "cleartabletask.h"
#include "cleartabledirtask.h"

#include <qdir.h>
#include <qfile.h>
#include <qtimer.h>
#include <qtextcodec.h>
#include <qcoreapplication.h>

#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/targ.h>
#include <cross-commons/app/paths.h>

#include <commons/textproto/tprototext.h>

#include <meteo/commons/planner/planner.h>

#include <mappi/global/global.h>
#include <mappi/proto/cleaner.pb.h>
#include <mappi/settings/tmeteosettings.h>

#include "appstatus.h"


const QStringList kPathOpt = QStringList() << "p" << "path";


//void out(tlog::Priority priority, const QString& facility, const QString& fileName, int line, const QString& msg)
//{
//  QString m = msg;
//  meteo::PlannerTask* task = qobject_cast<meteo::PlannerTask*>(QThread::currentThread());
//  if ( 0 != task ) {
//    m.prepend(QString("#%1 ").arg(task->id(), 2, 10, QChar('0')));
//  }

////  ::meteo::ukaz::logoutToDb(priority, facility, fileName, line, m);
//}

//!
bool loadConfig(mappi::cleaner::Tasks* part, const QString& filename )
{
  QFile file(filename);
  if ( false == file.exists() ) {
    error_log << QObject::tr("Отсутствует конфигурационный файл '%1'.").arg(file.fileName());
    return false;
  }
  if ( false == file.open(QIODevice::ReadOnly) ) {
    error_log << QObject::tr("Ошибка при открытии файла конфигурации '%1'").arg(file.fileName());
    debug_log << file.errorString();
    return false;
  }
  QString text = QString::fromUtf8(file.readAll());

  if ( false == TProtoText::fillProto(text, part) ) {
    error_log << QObject::tr("Ошибка при обработке конфигурационного файла '%1'").arg(file.fileName());
    return false;
  }
  return true;
}


int main(int argc, char** argv)
{
  try {
    TAPPLICATION_NAME("mappi");

    QCoreApplication app(argc, argv);

    ::meteo::gGlobalObj(new ::mappi::MappiGlobal);
    if( !::meteo::mappi::TMeteoSettings::instance()->load() ){
      return EXIT_FAILURE;
    }

//    TLog::setMsgHandler( &out );

    TArg args(argc,argv);

    QDir dir(args.value(kPathOpt, MnCommon::etcPath() + "cleaner.d"));
    if ( false ==  dir.exists() ) {
      error_log << QObject::tr("Отсутствует директория с конфигурационными файлами '%1'.").arg(dir.dirName());
      return EXIT_FAILURE;
    }

    mappi::cleaner::Tasks tasks;

    QStringList conflist = dir.entryList(QStringList("*.conf"));
    for ( int i = 0, sz = conflist.size(); i < sz; ++i ) {
      mappi::cleaner::Tasks part;
      QString filename = dir.absoluteFilePath( conflist[i] );
      if ( false  == loadConfig(&part, filename) ) {
        warning_log << QObject::tr("Не удалось обработать файл %1").arg(filename);
        continue;
      }
      tasks.MergeFrom(part);
    }

    meteo::Planner* planner = new meteo::Planner;

    for ( int i=0,isz=tasks.archive_size(); i<isz; ++i ) {
      meteo::Timesheet ts;

      if ( tasks.archive(i).has_archive_timesheet() ) {
        ts = meteo::Timesheet::fromString(QString::fromUtf8(tasks.archive(i).archive_timesheet().c_str()));
        mappi::cleaner::ArchiveTask* arch = new mappi::cleaner::ArchiveTask;
        arch->setName(QObject::tr("Архивирование телеграмм"));
        arch->setTimesheet(ts);
        arch->setPath(QString::fromUtf8(tasks.archive(i).path().c_str()));
        if ( ts.isValid() ) {
          planner->addTask(arch);
        }
        error_log_if( !ts.isValid() ) << QObject::tr("Шаблон расписания некорректен (archive)");
      }

      if ( tasks.archive(i).has_remove_timesheet() ) {
        ts = meteo::Timesheet::fromString(QString::fromUtf8(tasks.archive(i).remove_timesheet().c_str()));
        mappi::cleaner::RemoveTask* remove = new mappi::cleaner::RemoveTask;
        remove->setName(QObject::tr("Удаление устаревших записей"));
        remove->setTimesheet(ts);
        remove->setArchivePath(QString::fromUtf8(tasks.archive(i).path().c_str()));
        remove->setTelegramsPath(QString::fromUtf8(tasks.archive(i).telegrams_path().c_str()));
        remove->setQuery(QString::fromUtf8(tasks.archive(i).query().c_str()));
        if ( ts.isValid() ) {
          planner->addTask(remove);
        }
        error_log_if( !ts.isValid() ) << QObject::tr("Шаблон расписания некорректен (archive)");
      }

      if ( tasks.archive(i).has_check_timesheet() ) {
        ts = meteo::Timesheet::fromString(QString::fromUtf8(tasks.archive(i).check_timesheet().c_str()));
        mappi::cleaner::CheckTask* check = new mappi::cleaner::CheckTask;
        check->setName(QObject::tr("Проверка целостности"));
        check->setTimesheet(ts);
        if ( ts.isValid() ) {
          planner->addTask(check);
        }
        error_log_if( !ts.isValid() ) << QObject::tr("Шаблон расписания некорректен (archive)");
      }
    }

    for ( int i=0,isz=tasks.db_task_size(); i<isz; ++i ) {
      meteo::Timesheet ts = meteo::Timesheet::fromString(QString::fromUtf8(tasks.db_task(i).timesheet().c_str()));
      mappi::cleaner::DbTask* db = new mappi::cleaner::DbTask;
      QString subname = QString::fromUtf8(tasks.db_task(i).name().c_str());
      db->setName(QObject::tr("Обслуживание БД (%1)").arg(subname));
      db->setTimesheet(ts);
      db->setConnectionName(QString::fromUtf8(tasks.db_task(i).connection_name().c_str()));
      for ( int j=0,jsz=tasks.db_task(i).query_size(); j<jsz; ++j ) {
        QString q = QString::fromUtf8(tasks.db_task(i).query(j).c_str());
        db->addQuery(q);
      }
      for ( int j=0,jsz=tasks.db_task(i).reindex_table_size(); j<jsz; ++j ) {
        QString q = QString::fromUtf8(tasks.db_task(i).reindex_table(j).c_str());
        db->addReindexQuery(q);
      }
      for ( int j=0,jsz=tasks.db_task(i).vacuum_table_size(); j<jsz; ++j ) {
        QString q = QString::fromUtf8(tasks.db_task(i).vacuum_table(j).c_str());
        db->addVacuumQuery(q);
      }
      for ( int j=0,jsz=tasks.db_task(i).analyze_table_size(); j<jsz; ++j ) {
        QString q = QString::fromUtf8(tasks.db_task(i).analyze_table(j).c_str());
        db->addAnalyzeQuery(q);
      }
      if ( ts.isValid() ) {
        planner->addTask(db);
      }
      error_log_if( !ts.isValid() ) << QObject::tr("Шаблон расписания некорректен (db_task)");
    }

    for ( int i=0,isz=tasks.clear_tbl_dir_size(); i<isz; ++i ) {
      const mappi::cleaner::ClearTableDir& opt = tasks.clear_tbl_dir(i);

      meteo::Timesheet ts = meteo::Timesheet::fromString(QString::fromUtf8(opt.timesheet().c_str()));
      error_log_if( !ts.isValid() ) << QObject::tr("Шаблон расписания некорректен (clear_tbl_dir)");

      mappi::cleaner::ClearTableDirTask* task = new mappi::cleaner::ClearTableDirTask;

      QString subname = QString::fromUtf8(opt.name().c_str());
      task->setName(QObject::tr("Обслуживание (%1)").arg(subname));
      task->setTimesheet(ts);
      task->setConnectionName(QString::fromUtf8(opt.connection_name().c_str()));
      task->setDtFormat(QString::fromUtf8(opt.dt_format().c_str()));
      task->setMacLevel(opt.mac());
      for ( int j=0,jsz=opt.path_size(); j<jsz; ++j ) {
        task->addPath(QString::fromUtf8(opt.path(j).c_str()));
      }
      for ( int j=0,jsz=opt.rm_file_size(); j<jsz; ++j ) {
        task->addFileTemplate(QString::fromStdString(opt.rm_file(j)));
      }

      for ( int j=0,jsz=opt.query_size(); j<jsz; ++j ) {
        QString q = QString::fromUtf8(opt.query(j).c_str());
        task->addQuery(q);
      }

      if ( ts.isValid() ) {
        planner->addTask(task);
      }
    }

    for ( int i=0,isz=tasks.clear_task_size(); i<isz; ++i ) {
      meteo::Timesheet ts = meteo::Timesheet::fromString(QString::fromUtf8(tasks.clear_task(i).timesheet().c_str()));
      mappi::cleaner::ClearTableTask* task = new mappi::cleaner::ClearTableTask;
      QString tableName = QString::fromUtf8(tasks.clear_task(i).table_name().c_str());
      task->setName(QObject::tr("Очистка таблицы (%1)").arg(tableName));
      task->setTimesheet(ts);
      task->setConnectionName(QString::fromUtf8(tasks.clear_task(i).connection_name().c_str()));
      task->setTableName(tableName);
      task->setDateTimeFieldName(QString::fromUtf8(tasks.clear_task(i).dt_field_name().c_str()));
      task->setPathFieldName(QString::fromUtf8(tasks.clear_task(i).path_field_name().c_str()));
      if ( ts.isValid() ) {
        planner->addTask(task);
      }
      error_log_if( !ts.isValid() ) << QObject::tr("Шаблон расписания некорректен (clear_task)");
    }

    mappi::cleaner::AppStatus status(planner);
    status.setInterval(5*1000);
    status.slotSend();

    QTimer::singleShot( 100, planner, SLOT(slotProcessTasks()) ); // hack for fast start

    app.exec();

    delete planner;
  }
  catch(const std::bad_alloc& ) {
    critical_log << QObject::tr("Недостаточно памяти для работы приложения");
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
