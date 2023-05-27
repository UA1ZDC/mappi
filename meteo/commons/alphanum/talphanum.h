#ifndef METEO_COMMONS_ALPHANUM_TALPHANUM_H
#define METEO_COMMONS_ALPHANUM_TALPHANUM_H

#include <meteo/commons/rpc/rpc.h>
#include <meteo/commons/services/decoders/decservice.h>
#include <meteo/commons/services/decoders/tservicestat.h>
#include <meteo/commons/alphanum/metadata.h>

/*!
  \file   talphanum.h
  \brief  Раскодирование данных в буквенно-цифровом формате
  class WmoAlphanum      обработка телеграмм в формате вмо
  class StationAlphanum  данные с имитатора в байконуре 
  class GphAlphanum      обработка файлов с геофизическими данными
*/


class TDecodeData;
class QDateTime;
class TMeteoData;

namespace meteo {
  namespace anc {
    class StationMsg;
    class WmoMsg;
    class GphDecoder;
    class CliwareMsg;
  }
}

namespace meteo {
  namespace app {
    class AppState_OperationStatus;
  }

  namespace anc {
    //! Состояние раскодировщика БЦК
    enum StatType {
      kReqUnknown = -1,
      kReceivedTlg = 0, //!< Принято телеграмм
      kReceived    = 1,	//!< Количество полученных сводок
      kDecoded     = 2,	//!< Количество раскодированных
      kEmpty       = 3, //!< Пустые сводки
      kExtract     = 4, //!< Извлечено карт
    };
  }

}

//!телеграмма в формате вмо
class WmoAlphanum {
public:
  WmoAlphanum();
  virtual ~WmoAlphanum();

  virtual int decode(const QMap<QString, QString>& type, const QByteArray& ba); 
  //  virtual int decode(const QByteArray& ba, QList<QByteArray>* result, QString* error, const QDateTime& dt = QDateTime()); 
  virtual int decode(const QString&) {return 0;}
  virtual void dataReady()  {}
  // virtual const rpc::Address& sprinfAddress() { return _adr; }
  virtual QDateTime headerDt() { return QDateTime(); }
  virtual int32_t corNumber() { return 0; }

  TMeteoData* data() { return _current; }
  QString     code() { return _code; }
  QDateTime dateTime();
  bool isFragment()  { return _isFragment; }

  const service::Status<meteo::surf::DataType, meteo::anc::StatType>& status() const { return _stat; }

  bool setCodec(const QByteArray& name);

//  void fillCoord(TMeteoData* data, rpc::TController* ctrl);
  static void fillCoord(TMeteoData* data);

private:
  QString _code;		//!< Название кодовой формы текущей сводки
  TMeteoData* _current = nullptr;		//!< Текущая раскодированная сводка
  bool _isFragment;		//!< true - фрагмент сводки (напр, TREND в METAR)
  meteo::anc::WmoMsg* _wm = nullptr;	//!< Парсер сводок

  service::Status<meteo::surf::DataType, meteo::anc::StatType> _stat; //!< Состояние раскодирования (статистика)
};

//!геофизические текстовые
class GphAlphanum {
public:
  GphAlphanum();
  virtual ~GphAlphanum();

  virtual int decode(const QMap<QString, QString>& type, const QByteArray& ba); 
  //  virtual int decode(const QString&) {return 0;}
  // virtual int decode(const QByteArray& , QList<QByteArray>* , 
  // 		     QString* error, const QDateTime&  = QDateTime())  { *error = "Not realised";  return 0; }
  virtual void dataReady() {}
  virtual QDateTime headerDt() { return QDateTime(); }
  virtual int32_t corNumber() { return 0; }

  const TMeteoData* header();
  const QByteArray* data();
  const QDateTime*  epoch();
  
  QString     code() { return _code; }

  const service::Status<meteo::surf::DataType, meteo::anc::StatType>& status() const { return _stat; }

  bool setCodec(const QByteArray& name);
private:
  QString _code;
  int _current = 0;
  meteo::anc::GphDecoder* _gm = nullptr;
  
  service::Status<meteo::surf::DataType, meteo::anc::StatType> _stat;
};

//данные с имитатора в байконуре 
class StationAlphanum {
public:
  StationAlphanum();
  virtual ~StationAlphanum();

  virtual int decode(const QMap<QString, QString>& type, const QByteArray& ba); 
  virtual int decode(const QString&) {return 0;}
  // virtual int decode(const QByteArray& /*ba*/, 
  // 		     QList<QByteArray>* /*result*/, QString* error) { *error = "Not realised";  return 0; }
  virtual void dataReady() {};

  TMeteoData* data() { return _current; }
  QString     code() { return _code; }
  QDateTime dateTime();

  const service::Status<meteo::surf::DataType, meteo::anc::StatType>& status() const { return _stat; }

  bool setCodec(const QByteArray& name);
private:
  QString _code;
  TMeteoData* _current = nullptr;
  meteo::anc::StationMsg* _sm = nullptr;
  
  service::Status<meteo::surf::DataType, meteo::anc::StatType> _stat;
};

//данные ВНИИГМИ-МЦД 
class CliwareAlphanum {
public:
 CliwareAlphanum();
  virtual ~CliwareAlphanum();

  virtual int decode(const QMap<QString, QString>& type, const QByteArray& ba); 
  virtual int decode(const QString&) {return 0;}

  virtual void dataReady() {};

  TMeteoData* data() { return _current; }
  QString     code() { return _code; }
  QDateTime dateTime();

  const service::Status<meteo::surf::DataType, meteo::anc::StatType>& status() const { return _stat; }

private:
  QString _code;
  TMeteoData* _current = nullptr;
  meteo::anc::CliwareMsg* _cm = nullptr;
  
  service::Status<meteo::surf::DataType, meteo::anc::StatType> _stat;
};

#endif
