#include "checktask.h"

#include <qfile.h>
#include <qelapsedtimer.h>

#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/qobjectdeleter.h>

#include <sql/nspgbase/tsqlquery.h>

#include <mappi/global/global.h>

namespace mappi {
namespace cleaner {

void CheckTask::run()
{
//  NS_PGBase* db = new NS_PGBase(global::dbConfTelegram());
//  QObjectDeleter deleter(db);

//  int dayOffset = 0;
//  while ( true ) {
//    // проверяем наличие телеграмм подлежащих проверке
//    QString checkSql = "SELECT id FROM telegrams "
//                       "WHERE message_path NOT LIKE 'tar:%' "
//                       "AND converted_dt < TIMESTAMP 'today' - interval '%1 days' LIMIT 1";
//    checkSql = checkSql.arg(dayOffset);
//    TSqlQuery q(db);
//    if ( !q.exec(checkSql) ) {
//      error_log << tr("Ошибка при взаимодействии с БД");
//      debug_log << q.errorMessage();
//      debug_log << q.query();
//      return;
//    }
//    if ( q.size() == 0 ) {
//      break;
//    }

//    QElapsedTimer timer;
//    timer.start();

//    QString selectSql = "SELECT id,message_path FROM telegrams "
//                        "WHERE message_path NOT LIKE 'tar:%' "
//                        "AND converted_dt >= TIMESTAMP 'today' - interval '%1 days' "
//                        "AND converted_dt < TIMESTAMP 'today' - interval '%2 days' ";
//    selectSql = selectSql.arg(dayOffset+1).arg(dayOffset);
//    if ( !q.exec(selectSql) ) {
//      error_log << tr("Ошибка при взаимодействии с БД");
//      debug_log << q.errorMessage();
//      debug_log << q.query();
//      return;
//    }
//    if ( q.size() == 0 ) {
//      dayOffset += 1;
//      continue;
//    }

//    debug_log << "select query:" << double(timer.restart())/1000 << "sec.";

//    QStringList badIds;
//    for ( int i=0,isz=q.size(); i<isz; ++i ) {
//      if ( !QFile::exists(q.value(i,"message_path")) ) {
//        badIds << q.value(i,"id");
//        debug_log << q.value(i,"message_path");
//      }
//    }
//    if ( badIds.size() == 0 ) {
//      dayOffset += 1;
//      continue;
//    }

//    debug_log << "found" << badIds.size() << "bad telegrams (" << double(timer.restart())/1000 << "sec.)";

//    QString deleteSql = "DELETE FROM telegrams WHERE id IN (%1)";
//    for ( int removed=0,sz=badIds.size(); badIds.size()>0; ) {
//      QStringList ids;
//      for ( int n=0; n<5000 && badIds.size()>0; ++n ) {
//        ids << badIds.takeFirst();
//      }

//      TSqlQuery deleteQ(db);
//      if ( !deleteQ.exec(deleteSql.arg(ids.join(","))) ) {
//        error_log << tr("Ошибка при взаимодействии с БД");
//        debug_log << deleteQ.errorMessage();
//        debug_log << deleteQ.query();
//        return;
//      }

//      removed += ids.size();
//      debug_log << "delete" << removed << "of" << sz << double(timer.elapsed() + 1)/1000 << "sec.";
//    }

//    dayOffset += 1;
//  }
}

} // cleaner
} // mappi
