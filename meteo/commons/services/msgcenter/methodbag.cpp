#include "methodbag.h"
#include "msgcenter.h"

#include <qelapsedtimer.h>

#include <sql/dbi/dbi.h>
#include <sql/dbi/dbiquery.h>
#include <sql/dbi/dbientry.h>
#include <sql/dbi/gridfs.h>
#include <sql/dbi/gridfile.h>
#include <meteo/commons/msgparser/tlgparser.h>
#include <meteo/commons/global/global.h>

#include <meteo/commons/alphanum/wmomsg.h>
#include <meteo/commons/alphanum/metadata.h>
#include <commons/meteo_data/tmeteodescr.h>

namespace meteo {

MethodBag::MethodBag( MsgCenter* s )
  : service_(s)
{
}

MethodBag::~MethodBag()
{
}

void MethodBag::makeQuery( const msgcenter::GetTelegramRequest& r, DbiQuery* query ) const
{
  QList<qint64> ids;
  for ( int i = 0, isz = r.ptkpp_id_size(); i < isz; ++i ) {
    ids << r.ptkpp_id(i);
  }
  if ( !ids.isEmpty() ) {
    query->arg("id",ids);
  }

  QStringList ts1;
  for ( int i = 0, isz = r.t1_size(); i < isz; ++i ) {
    ts1 << QString::fromStdString(r.t1(i));
  }
  if ( false == ts1.isEmpty() ) {
    query->arg("t1",ts1);
  }

  QStringList ts2;
  for ( int i = 0, isz = r.t2_size(); i < isz; ++i ) {
    ts2 << QString::fromStdString(r.t2(i));
  }
  if ( false == ts2.isEmpty() ) {
    query->arg("t2",ts2);
  }

  QStringList as1;
  for ( int i = 0, isz = r.a1_size(); i < isz; ++i ) {
    as1 << QString::fromStdString(r.a1(i));
  }
  if ( false == as1.isEmpty() ) {
    query->arg("a1",as1);
  }

  QStringList as2;
  for ( int i = 0, isz = r.a2_size(); i < isz; ++i ) {
    as2 << QString::fromStdString(r.a2(i));
  }
  if ( false == as2.isEmpty() ) {
    query->arg("a2",as2);
  }

  QStringList ccccs;
  for ( int i = 0, isz = r.cccc_size(); i < isz; ++i ) {
    ccccs << QString::fromStdString(r.cccc(i));
  }
  if ( false == ccccs.isEmpty() ) {
    query->arg("cccc",ccccs);
  }

  QStringList bbbs;
  for ( int i = 0, isz = r.bbb_size(); i < isz; ++i ) {
    bbbs << QString::fromStdString(r.bbb(i));
  }
  if ( false == bbbs.isEmpty() ) {
    query->arg("bbbb",bbbs);
  }

  QList<qint32> iis;
  for ( int i = 0, isz = r.ii_size(); i < isz; ++i ) {
    iis << r.ii(i);
  }
  if ( false == iis.isEmpty() ) {
    query->arg("ii",iis);
  }

  QStringList yyggggs;
  for ( int i = 0, isz = r.yygggg_size(); i < isz; ++i ) {
    yyggggs << QString::fromStdString(r.yygggg(i));
  }
  if ( false == yyggggs.isEmpty() ) {
    query->arg("yygggg",yyggggs);
  }

  QStringList msgtypes;
  for ( int i = 0, isz = r.msgtype_size(); i < isz; ++i ) {
    msgtypes << QString::fromStdString(r.msgtype(i));
  }
  if ( false == msgtypes.isEmpty() ) {
    query->arg("type",msgtypes);
  }

  if ( r.has_decoded() ) {
    query->arg("decoded",r.decoded());
  }

  if ( r.has_msg_beg_dt() && r.has_msg_end_dt() ) {
    QDateTime beg = QDateTime::fromString(QString::fromStdString(r.msg_beg_dt()), Qt::ISODate);
    QDateTime end = QDateTime::fromString(QString::fromStdString(r.msg_end_dt()), Qt::ISODate);
    query->arg("start_msgdt",beg);
    query->arg("end_msgdt",end);
  }

  if ( r.has_beg_dt() && r.has_end_dt() ) {
    QDateTime beg = QDateTime::fromString(QString::fromStdString(r.beg_dt()), Qt::ISODate);
    QDateTime end = QDateTime::fromString(QString::fromStdString(r.end_dt()), Qt::ISODate);
    query->arg("start_dt",beg);
    query->arg("end_dt",end);
  }

  if ( r.has_sort_by_id() ) {
    query->arg("sid",qBound(-1, r.sort_by_id(), 1));
  }

  if ( r.has_limit() ) {
    query->arg("limit",r.limit());
  }

  if ( r.has_skip() ) {
    query->arg("skip",r.skip());
  }
  if ( r.has_global_file_id() ) {
    query->arg("globalfile_id",r.global_file_id());
  }
}

void MethodBag::fillMessage( const DbiEntry& doc, tlg::MessageNew* mes ) const
{
  mes->mutable_metainfo()->set_id(doc.valueInt64("_id"));
  if ( doc.hasField("t1") ) { mes->mutable_header()->set_t1(doc.valueString("t1").toStdString()); }
  if ( doc.hasField("t2") ) { mes->mutable_header()->set_t2(doc.valueString("t2").toStdString()); }
  if ( doc.hasField("a1") ) { mes->mutable_header()->set_a1(doc.valueString("a1").toStdString()); }
  if ( doc.hasField("a2") ) { mes->mutable_header()->set_a2(doc.valueString("a2").toStdString()); }
  if ( doc.hasField("ii") ) { mes->mutable_header()->set_ii(doc.valueInt32("ii")); }
  if ( doc.hasField("cccc") ) { mes->mutable_header()->set_cccc(doc.valueString("cccc").toStdString()); }
  if ( doc.hasField("dddd") ) {
    QString dddd = doc.valueString("dddd");
    if ( !dddd.isEmpty() ) {
      mes->mutable_header()->mutable_gms()->set_dddd(dddd.toStdString());
    }
  }
  if ( doc.hasField("yygggg") ) { mes->mutable_header()->set_yygggg(doc.valueString("yygggg").toStdString()); }
  if ( doc.hasField("bbb") ) { mes->mutable_header()->set_bbb(doc.valueString("bbb").toStdString()); }
  if ( doc.hasField("external") ) { mes->mutable_metainfo()->set_external(doc.valueBool("external")); }
  if ( doc.hasField("file_id") ) { mes->mutable_metainfo()->set_file_id(doc.valueOid("file_id").toStdString()); }
  if ( doc.hasField("type") ) { mes->set_msgtype(doc.valueString("type").toStdString()); }
  if ( doc.hasField("msg_dt") ) {
    QDateTime dt = doc.valueDt("msg_dt");
    mes->mutable_metainfo()->set_converted_dt(dt.toString(Qt::ISODate).toStdString());
  }
}

void MethodBag::GetUndecodedFaxTelegram( const msgcenter::GetUndecodedFaxTelegramRequest* req,
                                         msgcenter::GetTelegramResponse* resp )
{
  std::unique_ptr<Dbi> db(meteo::global::dbTelegram());
  if ( nullptr == db ) {
    error_log << QObject::tr("Не удалось создать клиент БД");
    return;
  }
  auto query = db->queryptrByName("get_undecoded_fax_tlg");
  if ( nullptr == query ) {
    error_log << QObject::tr("Не удалось создать клиент БД");
    return;
  }

  GridFs gridfs;
  if ( false == gridfs.connect( db->params() ) ) {
    error_log << gridfs.lastError();
    return;
  }
  gridfs.use( db->params().name(), "fs" );

  if ( 0 < req->msgtype_size() ) {
    query->arg("type",req->msgtype());
  }

  if ( req->has_beg_dt() && req->has_end_dt() ) {
    QDateTime beg = QDateTime::fromString(QString::fromStdString(req->beg_dt()), Qt::ISODate);
    QDateTime end = QDateTime::fromString(QString::fromStdString(req->end_dt()), Qt::ISODate);
    query->arg("start_dt",beg);
    query->arg("end_dt",end);
  }

  if ( req->has_limit() ) {
    query->arg("limit",req->limit());
  }
  QString error;
 // QElapsedTimer timer; timer.start();

  if ( false == query->execInit( &error ) ) {
    resp->set_error( error.toStdString() );
    return;
  }
 // debug_log << QObject::tr("execInit за %1 мсек.").arg(timer.restart());

  while ( true == query->next() ) {
    const DbiEntry& doc = query->entry();
    tlg::MessageNew* mes = resp->add_msg();
    QString qq = doc.valueOid("file_id");
    GridFile file = gridfs.findOneById(qq);
   // debug_log << QObject::tr("findOneById за %1 мсек.").arg(timer.restart());

    if ( true == file.hasFile() ) {
      bool ok = false;
      QByteArray raw = file.readAll(&ok);
      if ( true == ok ) {
        mes->set_msg(std::string( raw.data(), raw.size() ));
      }
    }
  //  debug_log << QObject::tr("readAll за %1 мсек.").arg(timer.restart());

    fillMessage( doc, mes );
  }

}

void MethodBag::GetTelegram( const msgcenter::GetTelegramRequest* req, msgcenter::GetTelegramResponse* resp )
{
  std::unique_ptr<Dbi> db(meteo::global::dbTelegram());
  if ( nullptr == db ) {
    error_log << QObject::tr("Не удалось создать клиент БД");
    return;
  }
  auto query = db->queryptrByName("get_tlg");
  if ( nullptr == query ) {
    error_log << QObject::tr("Не удалось создать клиент БД");
    return;
  }

  GridFs gridfs;
  if ( false == gridfs.connect( db->params() ) ) {
    error_log << gridfs.lastError();
    return;
  }
  gridfs.use( db->params().name(), "fs" );

  makeQuery( *req, query.get()  );

  QString error;
  if ( false == query->execInit( &error ) ) {
    resp->set_error( error.toStdString() );
    return;
  }
  QByteArray arr;
  TlgParser parser(arr);
  //debug_log << query->query();
  while ( true == query->next() ) {
    const DbiEntry& doc = query->entry();

    tlg::MessageNew* mes = resp->add_msg();

    if ( false == req->onlyheader() ) {
      QString qq = doc.valueOid("file_id");
      GridFile file = gridfs.findOneById(qq);
      if ( true == file.hasFile() ) {
        bool ok = false;
        QByteArray raw = file.readAll(&ok);
        if ( true == ok ) {
          parser.setData(raw);
          if ( false == parser.parseNextMessage(mes) ) {
            error_log << QObject::tr("Не удалось загрузить телеграмму с идентификатором %1.").arg(doc.valueInt64("_id"));
          }
        }
      }
    }

    fillMessage( doc, mes );
  }
  //debug_log << "end";

}

void MethodBag::SetDecode( const msgcenter::DecodeRequest* req, msgcenter::Dummy* resp )
{
  Q_UNUSED(resp);
  std::unique_ptr<Dbi> db(meteo::global::dbTelegram());
  if ( nullptr == db ) {
    error_log << QObject::tr("Нет подключения к БД телеграмм!");
    return;
  }
  auto query = db->queryptrByName("update_tlg_set_decoded");
  if ( nullptr == query ) {
    error_log << QObject::tr("Не удалось создать запрос '%1'").arg("update_tlg_set_decoded");
    return;
  }

  query->arg( "id", req->id() );
  query->arg( "decoded", req->state() );

  if ( false == query->exec() ) {
    error_log << QObject::tr("Не удлаось выполнить запрос = %1").arg( query->query() );
    return;
  }

  const DbiEntry& result = query->result();
  int ok = qRound(result.valueDouble("ok"));
  int n = qRound(result.valueDouble("n"));
  int nModified = qRound(result.valueDouble("nModified"));
  if ( 1 != ok ){
    debug_log << QObject::tr("Не удалось обновить телеграмму в соответствии с запросом = %1").arg( query->query() );
  }
  if ( n != req->id_size() || nModified != req->id_size() ){
    warning_log << QObject::tr("Выбрано для обновления %1 телеграмм, обновлено %2 телеграмм, ожидалось %3")
                   .arg(n)
                   .arg(nModified)
                   .arg(req->id_size());
  }
}
void MethodBag::GetReport( const msgcenter::GetReportRequest* req, msgcenter::GetReportResponse* resp )
{
  if (!req->has_station() || req->station().empty()) {
    resp->set_comment(QObject::tr("Не указана станция").toStdString());
    resp->set_result(false);
    return;
  }

  msgcenter::GetTelegramRequest  tlgreq;
  msgcenter::GetTelegramResponse tlgResp;
  tlgreq.set_onlyheader(false);
  for (auto id : req->tlg_id()) {
    tlgreq.add_ptkpp_id(id);
  }
  
  GetTelegram(&tlgreq, &tlgResp);
  if (tlgResp.has_error()) {
    resp->set_comment(tlgResp.error());
    resp->set_result(false);
    return;
  }

  QMap<QString, QString> rt;
  QDateTime dt;
  QList<int> types;
  types << meteo::anc::kWmoFormat << meteo::anc::kIonfoTlg;
  meteo::anc::MetaData::instance()->load(MnCommon::etcPath() + "alphanum.conf", types);
  
  //  var(req->Utf8DebugString());
  //  var(tlgResp.Utf8DebugString());

  for (auto tlg : tlgResp.msg()) {
    if (tlg.msgtype() != "alphanum") {
      msgcenter::OneReport* one = resp->add_report();
      one->set_tlg_id(tlg.metainfo().id());
      one->set_report(QObject::tr("Бинарные данные").toStdString());
    }
    else {
      QByteArray ba(tlg.msg().data(), tlg.msg().size());
      meteo::anc::WmoMsg wm;
      wm.setCodec(QTextCodec::codecForName("Windows-1251"));
      const tlg::Header& head = tlg.header();
      if (head.t1() == "o" && head.t2() == "b" &&
          head.a1() == "m" && head.a2() == "n" &&
          head.ii() == 91) {
        rt.insert("ttaaii", "obmn91");
      }
      wm.parse(rt, ba, dt);
      bool exists = false;

      while(wm.hasNext()) {
        bool ok = wm.decodeNext();
        if (!ok) continue;

        TMeteoData* data = wm.data();
        if (0 != data) {

          bool match = false;
          QString station = TMeteoDescriptor::instance()->stationIdentificator(*data);
          if (station.isEmpty()) {
            station = TMeteoDescriptor::instance()->stationName(*data);
          }
          //var(station);
          if (station == QString::fromStdString(req->station())) {
            match = true;
          } else {
            QString stationReq = QString::fromStdString(req->station_name());
            station = station.toUpper().replace(QRegExp("\\W"), "").replace(QObject::tr("Ё"), QObject::tr("Е"));
            stationReq = stationReq.toUpper().replace(QRegExp("\\W"), "").replace(QObject::tr("Ё"), QObject::tr("Е"));
            match = (station == stationReq);
          }
          if (match) {
            msgcenter::OneReport* one = resp->add_report();
            one->set_tlg_id(tlg.metainfo().id());
            one->set_report(wm.current().toStdString());
            exists = true;
            //debug_log << "station exist" << station << wm.current();
          }
        }
      }
      
      if (!exists) {
        msgcenter::OneReport* one = resp->add_report();
        one->set_tlg_id(tlg.metainfo().id());
        one->set_report(QObject::tr("Станция не найдена").toStdString());
      }
    }
  }

  resp->set_result(true);
}

}


