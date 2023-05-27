#include "talphanum.h"
#include "gphdecoder.h"

#include <commons/meteo_data/tmeteodescr.h>

#include <meteo/commons/control/tmeteocontrol.h>
#include <meteo/commons/alphanum/stationmsg.h>
#include <meteo/commons/alphanum/wmomsg.h>
#include <meteo/commons/alphanum/cliwaremsg.h>
#include <meteo/commons/alphanum/metadata.h>

#include <meteo/commons/proto/state.pb.h>
#include <meteo/commons/proto/surface_service.pb.h>
#include <meteo/commons/proto/sprinf.pb.h>

#include <commons/mathtools/mnmath.h>

#include <qdatetime.h>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

using namespace meteo;

//телеграмма в формате вмо
WmoAlphanum::WmoAlphanum():
  _isFragment(false)
{
  _wm = new meteo::anc::WmoMsg;

  _stat.set(surf::kUnknownDataType, anc::kReceivedTlg, meteo::app::OperationState_WARN,
            QObject::tr("Принято телеграмм"));

  const meteo::anc::DecodingTypes& conf = anc::MetaData::instance()->decodingTypes();
  if (conf.IsInitialized()) {
    QList<anc::StatType> stype;
    stype << anc::kDecoded << anc::kReceived;

    for (int idx = 0; idx < conf.names_size(); idx++) {
      _stat.set(conf.names(idx).type(), stype, meteo::app::OperationState_WARN,
        QObject::tr("%1").arg(QString::fromUtf8(conf.names(idx).name().c_str())));
    }
  }

  _stat.set(surf::kUnknownDataType, anc::kReceived, meteo::app::OperationState_WARN,
            QObject::tr("Другие. Принято"));

  // _stat.set(surf::kUnknownDataType, anc::kEmpty, meteo::app::OperationState_NORM,
  // 	    QObject::tr("Пустых сводок"));
}

WmoAlphanum::~WmoAlphanum()
{
  delete _wm;
  _wm = nullptr;
}

//! Раскодирование, используется при подписке
//return - 0 - если нет ни одной сводки или они все пустые, -1 - ни одна не раскодирована, или число раскодированных сводок
int WmoAlphanum::decode(const QMap<QString, QString>& type, const QByteArray& ba)
{
  _stat.add(surf::kUnknownDataType, anc::kReceivedTlg, meteo::app::OperationState_NORM);
  int cnt = _wm->parse(type, ba, headerDt());

  if (0 == cnt) {
    _stat.add(surf::kUnknownDataType, anc::kReceived, meteo::app::OperationState_NORM);
    return 0;
  }

  int parsedCnt = 0;

  while(_wm->hasNext()) {
    if (_wm->decodeNext() ) {
      TMeteoData* data = _wm->data();
      if (nullptr != data && data->count() != 0) {
        ++parsedCnt;
        _code = _wm->code();
        //fillCoord(data, _ctrl);
        _stat.add(surf::DataType(_wm->category()), anc::kDecoded, meteo::app::OperationState_NORM);

        QList<TMeteoData*> fr = _wm->split();

	_isFragment = false;
	_current = data;
	dataReady();
	for (int i = 0; i < fr.size(); i++) {
	  _current = fr[i];
	  _isFragment = true;
	  dataReady();
	  if (fr[i] != 0) {
	    delete fr[i];
	    fr[i] = 0;
	  }
	}
      } else {
        _stat.add(surf::kUnknownDataType, anc::kEmpty, meteo::app::OperationState_NORM);
      }
    }
    _stat.add(surf::DataType(_wm->category() == -1 ? surf::kUnknownDataType : _wm->category()), anc::kReceived, meteo::app::OperationState_NORM);
  }

  return parsedCnt == 0 ? -1 : parsedCnt; //если 0, значит все сводки ошибочные
}

//! Раскодирование, используется при запросе
// int WmoAlphanum::decode(const QByteArray& ba, QList<QByteArray>* result, QString* error, const QDateTime& dt)
// {
//   const QMap<QString, QString> type;
//   int cnt = _wm->parse(type, ba, dt);
//   if (cnt == 0) {
//     *error = QObject::tr("Пустое сообщение");
//   }

//   while(_wm->hasNext()) {
//     if (_wm->decodeNext() ) {
//       TMeteoData* data = _wm->data();
//       if (0 != data && data->count() != 0) {
// 	_code = _wm->code();
// 	fillCoord(data);

// 	uint mask = control::LIMIT_CTRL | control::CONSISTENCY_CTRL;
// 	if (!TMeteoControl::instance()->control(mask, data)) {
// 	  *error = QObject::tr("Ошибка контроля");
// 	}
// 	QByteArray buf;
// 	(*data) >> buf;
// 	result->append(buf);
//       } else {
// 	*error = QObject::tr("Пустое сообщение");
//       }
//     }
//   }

//   return 0;
// }

QDateTime WmoAlphanum::dateTime()
{
  return TMeteoDescriptor::instance()->dateTime(*_current);
}

bool WmoAlphanum::setCodec(const QByteArray& name)
{
  if (QTextCodec::availableCodecs().contains(name)) {
    error_log << QObject::tr("Указанный кодек не поддерживается");
    return false;
  }
  _wm->setCodec(QTextCodec::codecForName(name));
  return true;
}

//Заполнение координат для стационарной станции при их отсутствии
// void WmoAlphanum::fillCoord(TMeteoData* data, rpc::TController* ctrl)
// {
//   if (data == 0) return;
//   if (0 == ctrl) return;

//   if (!data->hasParam(TMeteoDescriptor::instance()->descriptor("station_index")) ||
//       (data->hasParam(TMeteoDescriptor::instance()->descriptor("La")) &&
//        data->hasParam(TMeteoDescriptor::instance()->descriptor("Lo"))) ) {
//     return;
//   }

//   int index = data->getValue(TMeteoDescriptor::instance()->descriptor("station_index"), -1);
//   if (index == -1) return;
//   int dataType = data->getValue(TMeteoDescriptor::instance()->descriptor("category"), -1);
//   if (dataType == -1) return;

// //  bool ok;
// //  const rpc::Address& addr = meteo::global::serviceAddress( meteo::settings::proto::kSprinf, &ok );
// //  if (addr.host().isEmpty() || !ok) return;

//   meteo::sprinf::MultiStatementRequest request;
//   request.add_index(index);
//   request.add_data_type(dataType);

//   meteo::sprinf::Stations* response = ctrl->remoteCall(&meteo::sprinf::SprinfService::GetStations, request, 10000);
//   if (response != 0 && response->result() == true) {
//     if (response->station_size() != 0 && response->station(0).has_position()) {
//       const meteo::sprinf::GeoPoint& pnt = response->station(0).position();
//       if (pnt.has_height_meters()) {
// 	data->setCoord(MnMath::rad2deg(pnt.lat_radian()), MnMath::M180To180(MnMath::rad2deg(pnt.lon_radian())), pnt.height_meters());
//       } else {
// 	data->setCoord(MnMath::rad2deg(pnt.lat_radian()), MnMath::M180To180(MnMath::rad2deg(pnt.lon_radian())));
//       }
//     }
//   }
//   if (0 != response) {
//     delete response;
//   }

// }

//Заполнение координат для стационарной станции при их отсутствии
void WmoAlphanum::fillCoord(TMeteoData* data)
{
  if (data == 0) return;

  if (!data->hasParam(TMeteoDescriptor::instance()->descriptor("station_index")) ||
      (data->hasParam(TMeteoDescriptor::instance()->descriptor("La")) &&
       data->hasParam(TMeteoDescriptor::instance()->descriptor("Lo"))) ) {
    return;
  }

  int index = data->getValue(TMeteoDescriptor::instance()->descriptor("station_index"), -1);
  if (index == -1) return;
  int dataType = data->getValue(TMeteoDescriptor::instance()->descriptor("category"), -1);
  if (dataType == -1) return;

//  bool ok;
//  const rpc::Address& addr = meteo::global::serviceAddress( meteo::settings::proto::kSprinf, &ok );
//  if (addr.host().isEmpty() || !ok) return;

//  rpc::TController* ctrl = new rpc::TController(rpc::ServiceLocation::instance()->serviceClient(addr));
//  rpc::TController* ctrl = meteo::global::serviceController( meteo::settings::proto::kSprinf );
//  if (0 == ctrl) return;
  rpc::Channel* ch = meteo::global::serviceChannel( meteo::settings::proto::kSprinf );
  if (0 == ch) return;

  meteo::sprinf::MultiStatementRequest request;
  request.add_index(index);
  request.add_data_type(dataType);

  meteo::sprinf::Stations* response = ch->remoteCall(&meteo::sprinf::SprinfService::GetStations, request, 10000);
  if (response != 0 && response->result() == true) {
    if (response->station_size() != 0 && response->station(0).has_position()) {
      const meteo::GeoPointPb& pnt = response->station(0).position();
      if (pnt.has_height_meters()) {
        data->setCoord(MnMath::rad2deg(pnt.lat_radian()), MnMath::M180To180(MnMath::rad2deg(pnt.lon_radian())), pnt.height_meters());
      } else {
        data->setCoord(MnMath::rad2deg(pnt.lat_radian()), MnMath::M180To180(MnMath::rad2deg(pnt.lon_radian())));
      }
    }
  }
  if (0 != response) {
    delete response;
  }

  delete ch;
}

//----------

GphAlphanum::GphAlphanum()
{
  _gm = new meteo::anc::GphDecoder(QTextCodec::codecForLocale());

  _stat.set(surf::kUnknownDataType, anc::kReceivedTlg, meteo::app::OperationState_WARN,
            QObject::tr("Принято файлов"));

  const meteo::anc::DecodingTypes& conf = anc::MetaData::instance()->decodingTypes();
  if (conf.IsInitialized()) {
    for (int idx = 0; idx < conf.names_size(); idx++) {
      _stat.set(conf.names(idx).type(), anc::kReceived, meteo::app::OperationState_WARN,
                QObject::tr("%1. Принято").arg(QString::fromStdString(conf.names(idx).name())));
      // _stat.set(conf.names(idx).type(), anc::kDecoded, meteo::app::OperationState_WARN,
      // 		QObject::tr("%1. Раскодировано").arg(QString::fromStdString(conf.names(idx).name())));
      _stat.set(conf.names(idx).type(), anc::kExtract, meteo::app::OperationState_WARN,
                QObject::tr("%1. Извлечено карт").arg(QString::fromStdString(conf.names(idx).name())));
    }
  }

  _stat.set(surf::kUnknownDataType, anc::kReceived, meteo::app::OperationState_WARN,
            QObject::tr("Другие. Принято"));
}

GphAlphanum::~GphAlphanum()
{
  delete _gm;
  _gm = nullptr;    
}


int GphAlphanum::decode(const QMap<QString, QString>& type, const QByteArray& ba)
{
  _current = 0;
  _stat.add(surf::kUnknownDataType, anc::kReceivedTlg, meteo::app::OperationState_NORM);
  int cnt = _gm->parse(type, ba, headerDt());
  if (cnt != 0) {
    _stat.add(surf::DataType(_gm->category() == -1 ? surf::kUnknownDataType : _gm->category()), anc::kReceived, meteo::app::OperationState_NORM);
    _code = _gm->code();
  } else {
    _stat.add(surf::kUnknownDataType, anc::kReceived, meteo::app::OperationState_NORM);
    _code = QString();
  }

  QList<int> keys = _gm->mapKeys();
  for (int idx = 0; idx < keys.size(); idx++) {
    _stat.add(surf::DataType(_gm->category()), anc::kExtract, meteo::app::OperationState_NORM);
    _current = keys.at(idx);
    dataReady();
  }

  return 0;
}

const TMeteoData* GphAlphanum::header()
{
  if (nullptr != _gm) {
    return _gm->header();
  }

  return nullptr;
}

const QByteArray* GphAlphanum::data()
{
  if (nullptr != _gm) {
    return _gm->data(_current);
  }

  return nullptr;
}

const QDateTime* GphAlphanum::epoch()
{
   if (nullptr != _gm) {
    return _gm->epoch(_current);
  }

  return nullptr;
}

bool GphAlphanum::setCodec(const QByteArray& name)
{
  if (QTextCodec::availableCodecs().contains(name)) {
    error_log << QObject::tr("Указанный кодек не поддерживается");
    return false;
  }
  _gm->setCodec(QTextCodec::codecForName(name));
  return true;
}

//----------

//данные с имитатора в байконуре

StationAlphanum::StationAlphanum()
{
  _sm = new meteo::anc::StationMsg;

  _stat.set(surf::kUnknownDataType, anc::kReceivedTlg, meteo::app::OperationState_WARN,
            QObject::tr("Принято телеграмм"));
  _stat.set(surf::kUnknownDataType, anc::kReceived, meteo::app::OperationState_WARN,
            QObject::tr("Принято сводок"));
  _stat.set(surf::kUnknownDataType, anc::kDecoded, meteo::app::OperationState_WARN,
            QObject::tr("Раскодировано сводок"));
}

StationAlphanum::~StationAlphanum()
{
  delete _sm;
  _sm = nullptr;  
}

int StationAlphanum::decode(const QMap<QString, QString>& type, const QByteArray& ba)
{
  _stat.add(surf::kUnknownDataType, anc::kReceived, meteo::app::OperationState_NORM);
  bool ok = _sm->decode(type, ba);
  //    var(ok);
  if (ok) {
    _code = _sm->code();
    _current = _sm->data();
    dataReady();
    _stat.add(surf::kUnknownDataType, anc::kDecoded, meteo::app::OperationState_NORM);
  }
  return 0;
}

QDateTime StationAlphanum::dateTime()
{
  return TMeteoDescriptor::instance()->dateTime(*_current);
}

bool StationAlphanum::setCodec(const QByteArray& name)
{
  if (QTextCodec::availableCodecs().contains(name)) {
    error_log << QObject::tr("Указанный кодек не поддерживается");
    return false;
  }
  _sm->setCodec(QTextCodec::codecForName(name));
  return true;
}


//--------------------------------

CliwareAlphanum::CliwareAlphanum()
{
  _cm = new meteo::anc::CliwareMsg;
  
  _stat.set(surf::kUnknownDataType, anc::kReceivedTlg, meteo::app::OperationState_WARN,
            QObject::tr("Принято телеграмм"));

  //TODO

  const meteo::anc::DecodingTypes& conf = anc::MetaData::instance()->decodingTypes();
  if (conf.IsInitialized()) {
    QList<anc::StatType> stype;
    stype << anc::kDecoded << anc::kReceived;

    for (int idx = 0; idx < conf.names_size(); idx++) {
      _stat.set(conf.names(idx).type(), stype, meteo::app::OperationState_WARN,
        QObject::tr("%1").arg(QString::fromUtf8(conf.names(idx).name().c_str())));
    }
  }
  

  _stat.set(surf::kUnknownDataType, anc::kReceived, meteo::app::OperationState_WARN,
            QObject::tr("Другие. Принято"));
}

CliwareAlphanum::~CliwareAlphanum()
{
  delete _cm;
  _cm = nullptr;
}

//! Раскодирование, используется при подписке
//return - 0 - если нет ни одной сводки или они все пустые, -1 - ни одна не раскодирована, или число раскодированных сводок
int CliwareAlphanum::decode(const QMap<QString, QString>& type, const QByteArray& ba)
{
  _stat.add(surf::kUnknownDataType, anc::kReceivedTlg, meteo::app::OperationState_NORM);
  
  QJsonDocument json = QJsonDocument::fromJson(ba);
  if (json.isEmpty()) {
    _stat.add(surf::kUnknownDataType, anc::kReceived, meteo::app::OperationState_NORM);
    return 0;
  }

  int cnt = _cm->parse(type, json);
  if (0 == cnt) {
    _stat.add(surf::kUnknownDataType, anc::kReceived, meteo::app::OperationState_NORM);
    return 0;
  }
  
  int parsedCnt = 0;
  while(_cm->hasNext()) {
    if (_cm->decodeNext() ) {
      TMeteoData* data = _cm->data();
      if (nullptr != data && data->count() != 0) {
	++parsedCnt;
	_code = _cm->code();
	_current = data;
	dataReady();
	_stat.add(surf::DataType(_cm->category()), anc::kDecoded, meteo::app::OperationState_NORM);
      } else {
	_stat.add(surf::kUnknownDataType, anc::kEmpty, meteo::app::OperationState_NORM);
      }
    } else {
      _stat.add(surf::DataType(_cm->category() == -1 ? surf::kUnknownDataType : _cm->category()), anc::kReceived, meteo::app::OperationState_NORM);
    }
  }
    
  return parsedCnt == 0 ? -1 : parsedCnt;
}

QDateTime CliwareAlphanum::dateTime()
{
  return TMeteoDescriptor::instance()->dateTime(*_current);
}

