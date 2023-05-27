#ifndef METEO_COMMONS_ALPHANUM_STATIONMSG_H
#define METEO_COMMONS_ALPHANUM_STATIONMSG_H

//! Раскодирование данных станции
/*!
  \file   stationmsg.h
  \brief  Раскодирование результатов измерения станции. 
  Сообщение соотоит из заголовка и тела. 
  Формат заголовка: 
    <Дата/время в ISO> <Идентификатор станции> <Кодовая форма>\r\n
  Тело представляет собой непосредственно сводку, для раскодирования которой 
  применяются регулярные выражения,  описанные в соответствующем файле.
*/

#include <qdatetime.h>

template<class key, class value > class QMap;
class TMeteoData;

namespace meteo {
  namespace anc {
    class WmoDecoder;
    class TableDecoder;
  }
}

namespace meteo {
namespace anc {

  
  class StationMsg {
  public:
    StationMsg();
    virtual ~StationMsg();
    
    bool decode(const QMap<QString, QString>& type, const QByteArray& ba);

    QString code() { return _code; }
    TMeteoData* data() { return _data; }

    void setCodec(QTextCodec* c) { _codec = c; }
  protected:
    virtual void dataReady() {}

  private:
    bool parseHeader(const QByteArray& ba, int* headerSize);
    bool decodeWmo(const QByteArray& ba);
    bool decodeTable(const QByteArray& ba);

  private:

    QDateTime _dt;    //!< Дата/время формирования сообщения
    int _typeNum; //!< Номер кодовой формы
    QString _station; //!< Идентификатор станции
    int _stationIdx; //!< Индекс станции
    int _diffUtc; //!< Разница времени в телеграмме с UTC 

    QString _code;
    TMeteoData* _data;

    WmoDecoder* _wmoDecoder;
    TableDecoder* _tableDecoder;

    QTextCodec* _codec; //!< Кодек для преобразования символов
  };
    

}
}


#endif
