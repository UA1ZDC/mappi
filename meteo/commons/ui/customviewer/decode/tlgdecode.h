#ifndef METEO_NOVOST_UI_MSGVIEWER_DECODE_TLGDECODE_H
#define METEO_NOVOST_UI_MSGVIEWER_DECODE_TLGDECODE_H

#include <qglobal.h>

template<typename T> class QList;

class QString;
class QDateTime;

namespace meteo {
  namespace rpc {
    class Channel;
  }
}

class TMeteoData;
namespace grib {
  class TGribData;
}

namespace meteo {
namespace internal {

//const QList<TMeteoData> getDecodedContent(rpc::TController* ctrl, u_int64_t msgid, QString* errorMessage = 0);
// const QList<TMeteoData> getDecodedContent(rpc::TController* ctrl, const QByteArray& tlg,
//                                           const QString& dt, QString* errorMessage = 0);
bool getDecodedAlphanum(const QByteArray& tlg, const QDateTime& dt, QList<TMeteoData>* data, const QString& codec );
bool getDecodedIonex(const QByteArray& tlg, QList<TMeteoData>* data, const QString& codec);
bool getDecodedBufr(const QByteArray& tlg, const QDateTime& dt, QList<TMeteoData>* data, const QString& codec );
const QList<grib::TGribData> getDecodedGrib(const QByteArray& tlg);
  //const QList<grib::TGribData> getDecodedGrib(rpc::TController* ctrl, const QByteArray& tlg, QString* errorMessage = 0);
const QString getStationName(meteo::rpc::Channel* ch, int stationIndexes, int dataType, QString* errorMessage = 0);

}
}

#endif
