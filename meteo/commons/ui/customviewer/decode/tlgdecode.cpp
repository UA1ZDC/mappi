#include "tlgdecode.h"

#include <commons/meteo_data/meteo_data.h>
#include <meteo/commons/proto/sprinf.pb.h>
#include <meteo/commons/proto/surface_service.pb.h>
#include <meteo/commons/rpc/rpc.h>
#include <meteo/commons/proto/decoders.pb.h>
#include <meteo/commons/proto/tgribformat.pb.h>

#include <meteo/commons/alphanum/talphanum.h>
#include <meteo/commons/alphanum/wmomsg.h>
#include <meteo/commons/alphanum/metadata.h>
#include <meteo/commons/bufr/tbufr.h>
#include <meteo/commons/grib/decoder/tgribdecode.h>
#include <meteo/commons/grib/iface/tgribcalc.h>
#include <meteo/commons/control/tmeteocontrol.h>
#include <meteo/commons/alphanum/gphdecoder.h>

#include <qbytearray.h>
#include <qlist.h>
#include <qstring.h>

namespace {
  int syncCallTimeoutMsec() { return 10000; }
}

namespace meteo {
namespace internal {

bool getDecodedAlphanum(const QByteArray& tlg,
			const QDateTime& dt, QList<TMeteoData>* result, const QString& codec )
{
  bool ok = false;

  if (nullptr == result) {
    return false;
  }
  
  QMap<QString, QString> rt;
  meteo::anc::WmoMsg wm;
  wm.setCodec(QTextCodec::codecForName(codec.toUtf8()));
  wm.parse(rt, tlg, dt);
  while(wm.hasNext()) {
    bool oneok = wm.decodeNext();
    if (oneok) {
      TMeteoData* data = wm.data();
      //data->printData();
      WmoAlphanum::fillCoord(data);
      TMeteoControl::instance()->control(control::LIMIT_CTRL | control::CONSISTENCY_CTRL, data);
      result->append(*data);
      ok |= true;
    } else {
      ok |= false;
    }
  }


  return ok;
}

bool getDecodedBufr(const QByteArray& tlg, const QDateTime& dt, QList<TMeteoData>* data, const QString& codec )
{
  bool ok = true;

  if (nullptr == data) return false;

  QString error;
  TBufr bufr;
  bufr.setCodec(QTextCodec::codecForName(codec.toUtf8()));
  bufr.decode(tlg, data, &error, dt);
  if (0 == data->size()) {
    ok = false;
  }

  return ok;
}


const QString getStationName(meteo::rpc::Channel* ch, int stationIndex, int dataType, QString* errorMessage /* = 0*/)
{
  if ( 0 == ch ) {
    QObject::tr("Нулевой указатель");
    return QString();
  }
  if (errorMessage != 0) {
    errorMessage->clear();
  }

  QString result;

  sprinf::MultiStatementRequest req;
  req.add_index(stationIndex);
  req.add_data_type(dataType);

  sprinf::Stations* res = ch->remoteCall(&sprinf::SprinfService::GetStations, req, ::syncCallTimeoutMsec());
  if (res != 0) {
    for (int i = 0, sz = res->station_size(); i < sz; ++i) {
      if (res->station(i).name().has_rus() == true) {
        result = QString::fromStdString(res->station(i).name().rus());
      }
      else if (res->station(i).name().has_international() == true) {
        result = QString::fromStdString(res->station(i).name().international());
      }
      else if (res->station(i).name().has_short_() == true) {
        result = QString::fromStdString(res->station(i).name().short_());
      }
    }
    if (res->error_message().empty() == false) {
      if (errorMessage != 0) {
        *errorMessage = QString::fromStdString(res->error_message());
      }
    }
  }
  else {
    if (errorMessage != 0) {
      *errorMessage = QString::fromUtf8("Ошибка. Не удалось получить ответ от сервиса");
    }
  }
  delete res;
  return result;
}


const QList<grib::TGribData> getDecodedGrib(const QByteArray& tlg)
{
  QList<grib::TGribData> result;

  TDecodeGrib dec;
  dec.decode(tlg);
  if( 0 == dec.grib() ){
    return result;
  }
  grib::TGribData simple;
  grib::copyIdentificataion(*(dec.grib()), &simple);
  result.append(simple);

  return result;
}

const QList<grib::TGribData> getDecodedGrib(rpc::Channel* ch, const QByteArray& tlg, QString* errorMessage)
{
  if ( 0 == ch ) {
    QObject::tr("Нулевой указатель");
    return QList<grib::TGribData>();
  }
  if (errorMessage != 0) {
    errorMessage->clear();
  }
  QList<grib::TGribData> result;
  if( tlg.isEmpty() ){
    error_log << QString::fromUtf8("Ошибка. Пустой запрос");
    return result;
  }
  meteo::decoders::DecodeRequest req;
  req.set_tlg(tlg, tlg.size());
  meteo::decoders::DecodedMsg* resp =
      ch->remoteCall(&meteo::decoders::DecodersRpc::GetDecodedMsg, req, ::syncCallTimeoutMsec() );
  if (resp != 0) {
    for (int i = 0, sz = resp->data_size(); i < sz; ++i) {
      grib::TGribData grib;
      grib.ParseFromString(resp->data(i));
      //var(grib.DebugString());
      result.append(grib);
    }
    if (resp->error().empty() == false) {
      if (errorMessage != 0) {
        *errorMessage = QString::fromStdString(resp->error());
      }
    }
  }
  else {
    if (errorMessage != 0) {
      *errorMessage = QString::fromUtf8("Ошибка. Не удалось получить ответ от сервиса");
    }
  }
  delete resp;
  return result;
}

bool getDecodedIonex(const QByteArray& tlg, QList<TMeteoData>* result, const QString& /*codec*/)
{
  meteo::anc::GphDecoder gm(QTextCodec::codecForLocale());
  QMap<QString, QString> type;
  gm.parse(type, tlg, QDateTime());
  const TMeteoData* data = gm.header();
  if( 0 == data ){
    return false;
  }
  QList<int> keys = gm.mapKeys();
  QString dt;
  for (int idx = 0; idx < keys.size(); idx++) {
    TMeteoData* md = &const_cast<TMeteoData*>(data)->addChild();
    const QDateTime* epoch = gm.epoch(keys.at(idx));
    if( 0 != epoch ){
      dt = epoch->toString(Qt::ISODate);
    }
    md->add(descr_t(0004), QObject::tr("Дата: %1 Номер карты: %2").arg(dt).arg(keys.at(idx)), 0, control::NO_CONTROL);
  }
  result->append(*data);
  return true;
}

}
}
