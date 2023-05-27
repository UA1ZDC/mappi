#include "main.h"
#include "dbthread.h"

#include <quuid.h>
#include <qtextcodec.h>
#include <qelapsedtimer.h>
#include <qcoreapplication.h>

#include <cross-commons/app/targ.h>
#include <cross-commons/app/helpformatter.h>

#include <cross-commons/debug/tlog.h>

#include <sql/nspgbase/ns_pgbase.h>

#define HELP(opt, text)  (QStringList() << opt << QObject::tr(text))


const QStringList kHelpOpt = QStringList() << "?" << "help";
const QStringList kHostOpt = QStringList() << "h" << "host";
const QStringList kNameOpt = QStringList() << "d" << "db";
const QStringList kManyOpt = QStringList() << "m" << "many";
const QStringList kThreadOpt = QStringList() << "T" << "threads";
const QStringList kNRunOpt = QStringList() << "n" << "run";

const HelpFormatter kHelp = HelpFormatter()
  << HELP(kHelpOpt, "Эта справка.")
  << HELP(kHostOpt, "Адрес для подключения к БД (по умолчанию localhost)")
  << HELP(kNameOpt, "Имя БД, в которой будет создана временная таблица для тестирования.")
  << HELP(kManyOpt, "Создать для каждого потока отдельный экземпляр NS_PGBase.")
  << HELP(kThreadOpt, "Количество потоков.")
  << HELP(kNRunOpt, "Количество последовательных запусков теста.")
     ;


int main( int argc, char** argv )
{
//  TLog::setMsgHandler(TLog::clearStdOut);
  QTextCodec::setCodecForCStrings( QTextCodec::codecForLocale() );
  QCoreApplication app(argc, argv);

  TArg args(argc, argv);

  if( args.contains(kHelpOpt) ){
    kHelp.print();
    return EXIT_SUCCESS;
  }

  int nThreads = args.value(kThreadOpt, "3").toInt();
  none_log << QObject::tr("Количество потоков: %1").arg(nThreads);

  for ( int i=0; i<args.value(kNRunOpt, "1").toInt(); ++i ) {
    QString host = args.value(kHostOpt, "localhost");
    QString name = args.value(kNameOpt, "");
    if ( name.isEmpty() ) {
      error_log << QObject::tr("Необходимо указать имя БД.");
      return EXIT_FAILURE;
    }

    QString uuid = "tbl" + QUuid::createUuid().toString().replace(QRegExp("[{}]"), "").replace("-", "_");

    ConnectProp prop(host, name, "postgres", "");

    // create tbl
    {
      NS_PGBase db;
      if ( !db.Connect(prop) ) {
        error_log << QObject::tr("Ошибка при подключении к БД.");
        return EXIT_FAILURE;
      }

      TSqlQuery q(&db);
      if ( !q.exec(QString("CREATE TABLE %1 (data TEXT);").arg(uuid)) ) {
        error_log << QObject::tr("Ошибка при создании тестовой таблицы.");
        return EXIT_FAILURE;
      }
    }

    NS_PGBase* db = 0;

    if ( !args.contains(kManyOpt) ) {
      db = new NS_PGBase;
    }

    MainTest test(nThreads);

    QList<DbThread*> threads;
    for ( int i=0; i<nThreads; ++i ) {
      threads << new DbThread(db, prop, uuid);
      threads[i]->start();

      QObject::connect( threads[i], SIGNAL(started()), threads[i], SLOT(slotStartTest()) );
      QObject::connect( threads[i], SIGNAL(finished()), &test, SLOT(slotThreadFinished()) );
    }

    QElapsedTimer total;
    total.start();

    app.exec();

    {
      none_log << QObject::tr("Время выполнения: %1 сек.").arg(double(total.elapsed()) / 1000);
    }

    // remove test table
    {
      NS_PGBase db;
      if ( !db.Connect(prop) ) {
        error_log << QObject::tr("Ошибка при подключении к БД.");
        return EXIT_FAILURE;
      }

      TSqlQuery q(&db);
      if ( !q.exec(QString("DROP TABLE %1;").arg(uuid)) ) {
        error_log << QObject::tr("Ошибка при удалении тестовой таблицы.");
        return EXIT_FAILURE;
      }
    }
  }

  return EXIT_SUCCESS;
}

