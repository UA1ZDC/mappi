#ifndef METEO_ALPHANUM_GPHDECODER_H
#define METEO_ALPHANUM_GPHDECODER_H

#include <meteo/commons/alphanum/regexpdecode.h>

namespace meteo {
  namespace anc {

    enum GphMapType {
      kTec = 0,   // полное электронное содержание
      kRms = 1,   // СКО
      kHeight = 2 // ?
    };

    //! раскодировщик геофизических сводок (заголовок + данные)
    class GphDecoder : public RegExpDecode {
    public:

      GphDecoder(QTextCodec* codec);
      ~GphDecoder();

      void setCodec(QTextCodec* codec) { _codec = codec; }
      int parse(const QMap<QString, QString>& type, const QByteArray& ba, const QDateTime& dt);

      int count() { return _epochs.size(); }
      QList<int> mapKeys() { return _epochs.keys(); }
      const TMeteoData* header() { return RegExpDecode::data(); }
      const QByteArray* data(int num) { return _maps.contains(num) ? &_maps[num] : 0; }
      const QDateTime*  epoch(int num) { return _epochs.contains(num) ? &_epochs[num] : 0; }

    private:
      int  findType(const QByteArray &report, int* start);
      bool readMaps(const QByteArray& ba, float expon);
      QByteArray parseMapData(int type, float exp, const QByteArray& ba);

      bool decode(int code, const QByteArray& report) {
	return RegExpDecode::decode(code, report);
      }
      bool decode(const QByteArray &report, const QDomDocument &decoder);

    private:
      QTextCodec* _codec;
      QString _idPtkpp;

      QMap<int, QByteArray> _maps; //!< Номер карты, данные
      QMap<int, QDateTime> _epochs; //!< Номер карты, эпоха

    };

  } //anc
} //meteo

#endif
