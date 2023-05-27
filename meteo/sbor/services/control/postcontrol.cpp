#include "postcontrol.h"

#include <commons/meteo_data/tmeteodescr.h>

#include <meteo/commons/global/global.h>
#include <meteo/commons/proto/surface_service.pb.h>
#include <meteo/commons/control/tmeteocontrol.h>
#include <meteo/commons/primarydb/ancdb.h>

#include <cross-commons/debug/tlog.h>

#include <sql/dbi/dbi.h>
#include <sql/dbi/dbiquery.h>
#include <sql/dbi/dbientry.h>


using namespace meteo;

PostControl::PostControl()
{
  init();
}

PostControl::~PostControl()
{
  delete db_;
  db_ = nullptr;
}

void PostControl::init()
{
  db_ = meteo::global::dbMeteo();
  if (!db_->connect()) {
      error_log << QObject::tr("Ошибка установления соединения с БД");
    }
}

//! контроль по времени
bool PostControl::timeControl(const QDateTime& dtStart, const QDateTime& dtEnd)
{
  debug_log << dtStart << dtEnd;

  QTime ttt; ttt.start();

  QMap<PostControl::DataKey, TMeteoData> md;

  bool ok = timeGetData(dtStart, dtEnd, &md);
  if (!ok) return false;

  QStringList descrsList;
  descrsList << "T" << "Td" << "p";
  QString updParam;

  int cnt = 0;
  QMap<PostControl::DataKey, TMeteoData>::iterator it = md.begin();
  while (it != md.end()) {
      for (auto name : descrsList) {
          if (it.value().hasParam(name)) {

              TMeteoData prev;
              ok = timePrevData(name, it.key(), &prev);
              if (ok) {
                  timeControl(&it.value(), &prev);
                  updParam += createUpdateParam(it.value());
                }
            }
        }
      // debug_log << it.key().station << it.key().dt;
      // it.value().printData();
      ++it;
      ++cnt;
    }

  //var(cnt);

  if (!updParam.isEmpty()) {
      updParam.remove(-1, 1);
      timeControlUpdate(updParam);
    }

  //debug_log << "time" << ttt.elapsed() << "cnt" << cnt;

  return true;
}

//!запрос данных для контроля
bool PostControl::timeGetData(const QDateTime& dtStart, const QDateTime& dtEnd,
                              QMap<PostControl::DataKey, TMeteoData>* md)
{
  if (nullptr == md) {
      return false;
    }

  if ( nullptr == db_ ) { return false; }
  auto query = db_->queryptrByName("control_time_req");
  if(nullptr == query) {return false;}

  query->arg("start_dt",dtStart);
  query->arg("end_dt",dtEnd);

  QString error;
  if(false == query->execInit( &error))
    {
     // error_log << error;
      return false;
    }

  while (query->next()) {
      const DbiEntry& doc = query->entry();
      QString station = doc.valueString("station");
      int type = doc.valueInt32("station_type");
      QDateTime dt = doc.valueDt("dt");
      PostControl::DataKey key(station, type, dt);

      QString oid = doc.valueOid("_id");
      QString descrname = doc.valueString("descrname");
      float value = doc.valueDouble("value");
      control::QualityControl quality = control::QualityControl(doc.valueInt32("quality"));
      ctrlfl_t ctrlfl = ctrlfl_t(doc.valueInt32("control"));

      if (!md->contains(key)) {
          md->insert(key, TMeteoData());
        }

      TMeteoData& cur = (*md)[key];
      TMeteoParam mp("", value, quality, ctrlfl, oid);
      cur.add(descrname, mp);
      cur.setDateTime(dt);
      cur.set("level_type", TMeteoParam("", 1, control::RIGHT));
    }

  return true;
}

//! запрос предыдущих данных для контроля по времени
bool PostControl::timePrevData(const QString& descr, const PostControl::DataKey& key, TMeteoData* md)
{
  if (nullptr == md) {
      return false;
    }

  if ( nullptr == db_ ) { return false; }
  auto query = db_->queryptrByName("control_time_prev");
  if(nullptr == query) {return false;}

  if (descr == "p") {
      QDateTime end = key.dt.addSecs(-2*60*60 - 50*60);
      QDateTime start = key.dt.addSecs(-6*60*60 - 5*60);
      QStringList descrs("p");
      descrs << "P" << "P0";
      query->arg("start_dt",start);
      query->arg("end_dt",end);
      query->arg("station",key.station);
      query->arg("station_type",key.type);
      query->arg("descrname",descrs);
    } else {
      QDateTime end = key.dt.addSecs(-30*60);
      QDateTime start = end.addDays(-1);
      query->arg("start_dt",start);
      query->arg("end_dt",end);
      query->arg("station",key.station);
      query->arg("station_type",key.type);
      query->arg("descrname",QStringList(descr));
    }
  QString error;
  if(false == query->execInit( &error))
    {
      //error_log << error;
      return false;
    }


  if (!query->next()) {
      //debug_log << QObject::tr("Нет предыдущего значения параметра %1. Запрос = %2").arg(descr).arg(query.query());
      return false;
    }

  const DbiEntry& doc = query->entry();

  //QDateTime dt = doc.valueDt("dt");
  QString descrname = doc.valueString("descrname");
  float value = doc.valueDouble("value");
  control::QualityControl quality = control::QualityControl(doc.valueInt32("quality"));
  QDateTime dt_end = doc.valueDt("dt_end");

  TMeteoParam mp("", value, quality);
  md->add(descrname, mp);
  md->setDateTime(dt_end);
  md->set("level_type", TMeteoParam("", 1, control::RIGHT));

  return true;
}

//! контроль текущих данных по предыдущим
bool PostControl::timeControl(TMeteoData* cur, TMeteoData* prev)
{
  if (!TMeteoControl::instance()->control(control::TIME_CTRL, cur, prev)) {
      error_log << QObject::tr("Ошибка контроля");
      return false;
    }
  return true;
}

QString PostControl::createUpdateParam(const TMeteoData& md)
{

  if ( nullptr == db_ ) { return QString(); }

  meteodescr::TMeteoDescriptor* d = TMeteoDescriptor::instance();
  QString upd;

  for (auto it = md.data().cbegin(); it != md.data().cend(); ++it) {
      if (d->isIdentDescr(it.key()))
        { continue; }
      QMapIterator<int, TMeteoParam> itp(it.value());
      while (itp.hasNext()) {
          itp.next();
          const TMeteoParam& param = itp.value();
          if (param.description().isEmpty() || 0 == (param.controlFlag() & control::TIME_CTRL)) {
              continue;
            }

          QString name = d->name(it.key());
          if (d->isAddParent(name)) {
              name = d->findAdditional(it.key(), itp.key(), param.value());
            }
          auto query = db_->queryptrByName("update_quality_many");
          if(nullptr == query) {return QString();}
          query->argOid("id",param.description());
          query->arg("descrname",name);
          query->arg("quality",param.quality());
          query->arg("control",param.controlFlag());

          upd += query->query() + ",";
        }
    }
  return upd;
}

bool PostControl::timeControlUpdate(const QString& updparam)
{
  if ( nullptr == db_ ) { return false; }
  auto query = db_->queryptrByName("update_meteoparam");
  if(nullptr == query) {return false;}
  query->argJson("param",updparam);
  bool ok = query->exec();
  if (!ok) {
      error_log << QObject::tr("Не удалось выполнить запрос = %1").arg(query->query());
      return false;
    }

  return true;
}
