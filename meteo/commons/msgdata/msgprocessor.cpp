#include "msgprocessor.h"

#include <qtimer.h>

#include <cross-commons/debug/tlog.h>


#include <meteo/commons/msgparser/common.h>
#include <meteo/commons/msgparser/tlgfuncs.h>
#include <meteo/commons/msgparser/tlgparser.h>

#include "msgqueue.h"
#include "types.h"

namespace meteo {

MsgProcessor::MsgProcessor(QObject* parent)
  : QObject(parent)
{
}

void MsgProcessor::slotInit()
{
  if ( nullptr == timer_ ) {
    timer_ = new QTimer(this);
    timer_->setInterval(500);
    connect( timer_, &QTimer::timeout, this, &MsgProcessor::slotRun );
  }

  run_ = false;
}

void MsgProcessor::slotRun()
{
  if ( nullptr == queue_ ) { return; }

  if ( run_ ) { return; }
  run_ = true;

  MsgQueueItem item = queue_->head(kProcess);
  while ( item.isValid() )
  {
    const tlg::MessageNew& msg = item.msg();

    tlg::MsgType t = tlg::msgtype(msg);
    switch ( t ) {
      case tlg::kRequest: {
        if ( !processHmiRequest(msg) ) {
          // error
        }
      } break;
      case tlg::kData: {
        processHmiData(msg);
      } break;
      case tlg::kConfirm: {
        processConfirmMsg(msg);
      } break;
      default: {
        not_impl;
        var(t);
      }
    }

    if ( false == item.nextStage() ) { emit msgProcessed(item.id()); }

    item = queue_->head(kProcess);
  }

  run_ = false;
}

bool MsgProcessor::processHmiRequest(const tlg::MessageNew& msg)
{
  Q_UNUSED(msg);
  not_impl;
  return false;
//  QString tmp = QString::fromUtf8(msg.msg().data(), msg.msg().size());
//  QStringList lines = tmp.split("\r\r\n", QString::SkipEmptyParts);
//
//  if ( lines.size() == 0 ) {
//    error_log << tr("Пустое сообщение.");
//    return false;
//  }
//
//  QString hostCCCC = lines.takeFirst().trimmed();
//  if ( hostCCCC.size() != 4 ) {
//    error_log << tr("Первая строка сообщения должна содержать идентификатор центра.");
//    return false;
//  }
//
//  foreach ( const QString& line, lines )
//  {
//    if ( !line.startsWith("AHD") ) {
//      warning_log << tr("Некорректное значение '%1'.").arg(line);
//      continue;
//    }
//
//    nosql::NoSql db;
//    if ( db.connect() ) {
//      return false;
//    }
//
//    bson_t* query = bson_new();
//    bson_t* opts = bson_new();
//
//    if ( !queryFromAhd(line, query, opts) ) {
//      warning_log << tr("Не удалось распарсить сокращенный заголовок для составления запроса.");
//      continue;
//    }
//
//    QList<tlg::MsgDocument*> list;
//    bool ok = db.find("telegramsdb", "msg", query, opts, &list);
//
//    bson_destroy(query);
//    bson_destroy(opts);
//
//    if ( !ok ) {
//      qDeleteAll(list);
//      return false;
//    }
//
////    none_log << tr("Найдено %1 телеграмм для заголовка '%2'.").arg(q.size()).arg(line);
//
//    QByteArray raw;
//
//    raw += 001;
//    raw += "\r\r\n999";
//    raw += "\r\r\nBIDA01 " + hostCCCC.toUtf8() + " " + QDateTime::currentDateTimeUtc().toString("ddhhmm").toUtf8();
//    raw += "\r\r\n" + wmoId_.toUtf8();
//    QString ahd = line;
//    ahd.remove("AHD ");
//
//    TlgParser parser(QByteArray(0));
//
//    if ( list.isEmpty() ) {
//        // Указание о том, что сообщение не найдено.
//        raw += "\r\r\nNIL " + ahd.toUtf8();
//    }
//    else {
//      // Запрошенное сообщение.
//      QString filePath = QString::fromStdString(list.first()->path());
//      QByteArray ba = readTelegram(filePath);
//      if ( ba.isNull() ) {
//        error_log << tr("Не удалось загрузить телеграмму '%1'").arg(filePath);
//
//        raw += "\r\r\nERR " + ahd.toUtf8();
//      }
//      else {
//        parser.setData(ba);
//
//        tlg::MessageNew msg;
//        if ( !parser.parseNextMessage(&msg) ) {
//          error_log << tr("Не удалось распарсить телеграмму '%1'").arg(filePath);
//          raw += "\r\r\nERR " + ahd.toUtf8();
//        }
//        else {
//          raw += "\r\r\n";
//          raw += QByteArray::fromRawData(msg.header().data().data(), msg.header().data().size());
//          raw += QByteArray::fromRawData(msg.msg().data(), msg.msg().size());
//        }
//      }
//
//      qDeleteAll(list);
//      list.clear();
//    }
//
//    raw += "\r\r\n";
//    raw += "\r\r\n";
//    raw += 003;
//
//    parser.setData(raw);
//    tlg::MessageNew msg;
//    if ( !parser.parseNextMessage(&msg) ) {
//      error_log << tr("Сообщение составлено некорректно.");
//      var(raw);
//      continue;
//    }
//
//    msg.mutable_metainfo()->set_converted_dt(QDateTime::currentDateTimeUtc().toString(Qt::ISODate).toUtf8());
//
//    MsgQueueItem item = queue_->append(msg, kNone);
//    item.appendStage(kRoute);
//    item.nextStage();
//  }
//
  return true;
}

bool MsgProcessor::processHmiData(const tlg::MessageNew& msg)
{
  QByteArray ba = QByteArray(msg.msg().data(), msg.msg().size());

  QString cccc = tlg::creator(msg);
  if ( cccc.isEmpty() ) {
    error_log << tr("Отсутствует информаци о составители сообщения.");
    return false;
  }

  ba.remove(0, 7);

  if ( ba.startsWith("ERR ") ) {
    var(ba);
    return true;
  }
  if ( ba.startsWith("NIL ") ) {
    var(ba);
    return true;
  }

  ba.prepend("\r\r\n000\r\r\n");
  ba.prepend(001);
  ba.append("\r\r\n");
  ba.append(003);

  TlgParser parser(ba);

  tlg::MessageNew submsg;
  if ( !parser.parseNextMessage(&submsg) ) {
    error_log << tr("Невозможно распарсить полученое сообщение.");
    return false;
  }

  submsg.mutable_metainfo()->set_converted_dt(msg.metainfo().converted_dt());

  /*MsgQueueItem item = */queue_->append(submsg, kReceive);

  return true;
}

bool MsgProcessor::processConfirmMsg(const tlg::MessageNew& msg)
{
  QByteArray ba = QByteArray(msg.msg().data(), msg.msg().size());

  assert_log( ba.startsWith("CONFIRM MD5 ") );

  ba.remove(0, 12);

  assert_log( ba.size() >= 32 );

  not_impl;

//  TSqlQuery q(global::dbTelegram());

//  QString sql = "SELECT id FROM telegrams_addr WHERE md5='%1' LIMIT 1";

//  QString confMd5 = ba.left(32);
//  if ( !q.exec(sql.arg(confMd5)) ) {
//    error_log << msglog::kDbRequestFailed;
//    return false;
//  }

//  if ( q.size() == 0 ) {
//    error_log << tr("Телеграмма-шторм отсутствует в БД.");
//    var(msg.Utf8DebugString());
//    var(q.query());
//    return false;
//  }

//  quint64 stormId = q.value(0,0).toULongLong();

//  QString md5 = tlg::md5hash(msg).toHex();
//  if ( !q.exec(sql.arg(md5)) ) {
//    error_log << msglog::kDbRequestFailed;
//    return false;
//  }

//  if ( q.size() == 0 ) {
//    error_log << tr("Телеграмма-подтверждение отсутствует в БД.");
//    var(q.query());
//    return false;
//  }

//  quint64 confId = q.value(0,0).toULongLong();

//  sql = "UPDATE telegrams_addr SET confirm_id='%1' WHERE id='%2';";
//  if ( !q.exec(sql.arg(stormId).arg(confId)) ) {
//    error_log << msglog::kDbRequestFailed;
//    return false;
//  }
//  if ( !q.exec(sql.arg(confId).arg(stormId)) ) {
//    error_log << msglog::kDbRequestFailed;
//    return false;
//  }

  return true;
}

bool MsgProcessor::queryFromAhd(const QString& ahd, bson_t* q, bson_t* opts) const
{
  //  Формат заголовков при запросе ГМИ в ВМО-формате.
  //  AHD TTAAii CCCC YYGGgg (BBB)=     <--
  //  AHD TTAAii CCCC YY//// (BB/)=     <-- когда BB = RR/CC/AA
  //  AHD TTAAii CCCC YY//// (P//)=     <--
  //  AHD TTAAii CCCC ////// =          <-- самое последнее сообщение за последние 24 часа
  //  --
  //  YY//// - самое последнее сообщение за день YY

  QRegExp rx("\\s*AHD ([A-Z]{2})([A-Z]{2})([0-9]{2}) ([A-Z]{4}) ([0-9/]{6})\\s?([A-Z/]{3})?=\\s*");

  if ( -1 == rx.indexIn(ahd) ) { return false; }

  QString tt = rx.capturedTexts()[1];
  QString aa = rx.capturedTexts()[2];
  QString ii = rx.capturedTexts()[3];
  QString cccc = rx.capturedTexts()[4];
  QString yygggg = rx.capturedTexts()[5];
  QString bbb = rx.capturedTexts()[6];

  if ( tt.size() != 2 ) { return false; }
  if ( aa.size() != 2 ) { return false; }


  if ( yygggg.contains("/") ) {
    if ( yygggg.right(4) != "////" && yygggg != "//////" ) {
      error_log << tr("Некорректно заполнена группа YYGGgg.");
      return false;
    }
  }

  if ( bbb.contains("/") ) {
    if ( "P//" == bbb ) {
    }
    else if ( bbb.right(1) == "/" && (bbb.left(2) == "RR" || bbb.left(2) == "CC" || bbb.left(2) == "AA") ) {
    }
    else {
      error_log << tr("Некорректно заполнена группа BBB.");
      return false;
    }
  }

  BSON_APPEND_UTF8(q, "t1", tt.left(1).toUtf8().constData());
  BSON_APPEND_UTF8(q, "t2", tt.mid(1,1).toUtf8().constData());
  BSON_APPEND_UTF8(q, "a1", aa.left(1).toUtf8().constData());
  BSON_APPEND_UTF8(q, "a2", aa.mid(1,1).toUtf8().constData());
  BSON_APPEND_UTF8(q, "cccc", cccc.toUtf8().constData());
  BSON_APPEND_INT32(q, "ii", ii.toInt());

  bool order = false;

  if ( yygggg.contains("/") ) {
    yygggg.remove("/");

    if ( yygggg.isEmpty() ) {
      QDateTime now = QDateTime::currentDateTimeUtc();
      QDateTime now_24 = now.addDays(-1);

      bson_t* b = BCON_NEW("$gte", BCON_DATE_TIME(now_24.toMSecsSinceEpoch()), "$lte", BCON_DATE_TIME(now.toMSecsSinceEpoch()));
      BSON_APPEND_DOCUMENT(q, "msg_dt", b);
      bson_destroy(b);

      order = true;
    }
    else if ( yygggg.size() == 2 ) {
      QDateTime beg = QDateTime::currentDateTimeUtc();
      beg.setTime(QTime(0,0));
      QDate d = beg.date();
      if ( d.day() < yygggg.toInt() ) {
        d = d.addMonths(-1);
      }
      d.setDate(d.year(), d.month(), yygggg.toInt());
      beg.setDate(d);

      if ( !beg.isValid() ) {
        error_log << tr("Не удалось расчитать период выборки данных.");
        var(yygggg);
        bson_destroy(q);
        return false;
      }

      QDateTime end = beg;
      end.setTime(QTime(23,59,59));

      bson_t* b = BCON_NEW("$gte", BCON_DATE_TIME(beg.toMSecsSinceEpoch()), "$lte", BCON_DATE_TIME(end.toMSecsSinceEpoch()));
      BSON_APPEND_DOCUMENT(q, "msg_dt", b);
      bson_destroy(b);
    }
    else {
      yygggg.remove("/");

      bson_t* b = BCON_NEW("$regexp", BCON_UTF8(QString("^" + yygggg + ".*").toUtf8().constData()));
      BSON_APPEND_DOCUMENT(q, "yygggg", b);
      bson_destroy(b);
    }
  }
  else {
    BSON_APPEND_UTF8(q, "yygggg", yygggg.toUtf8().constData());
  }

  if ( bbb.contains("/") ) {
    bbb.remove("/");

    bson_t* b = BCON_NEW("$regexp", BCON_UTF8(QString("^" + bbb + ".*").toUtf8().constData()));
    BSON_APPEND_DOCUMENT(q, "bbb", b);
    bson_destroy(b);
  }
  else if ( !bbb.isEmpty() ) {
    BSON_APPEND_UTF8(q, "bbb", bbb.toUtf8().constData());
  }

  if ( order ) {
    bson_t* b = BCON_NEW("dt", BCON_INT32(-1));
    BSON_APPEND_DOCUMENT(opts, "sort", b);
    bson_destroy(b);
  }
  BSON_APPEND_INT32(opts, "limit", 1);

  return true;
}

} // meteo
