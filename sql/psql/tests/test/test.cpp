#include <qapplication.h>
#include <qtextcodec.h>

#include <cross-commons/app/paths.h>
#include <cross-commons/debug/tlog.h>

#include <sql/nspgbase/ns_pgbase.h>
#include <sql/nspgbase/tsqlquery.h>

void query()
{
  NS_PGBase* db = new NS_PGBase("10.10.11.20", "db_obanal", "postgres", "");
  info_log << "connect:" << db->Connect();

  QString sql("SELECT cname FROM center_info LIMIT 1");
  TSqlQuery q(db);
  info_log << "exec:" << q.exec(sql);

  for ( int i=0,isz=q.size(); i<isz; ++i ) {
    info_log << "result" << i << "-" << q.value(i, 0);
  }
}

int main( int argc, char* argv[] )
{
  TAPPLICATION_NAME("test");

  QApplication app( argc, argv, false );
  QTextCodec::setCodecForCStrings( QTextCodec::codecForLocale() );

  TLog::setMsgHandler(TLog::clearStdOut);

  query();

  return EXIT_SUCCESS;
}
