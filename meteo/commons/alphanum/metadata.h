#ifndef METEO_ALPHANUM_METADATA_H
#define METEO_ALPHANUM_METADATA_H


#include <cross-commons/singleton/tsingleton.h>
#include <meteo/commons/proto/alphanum.pb.h>
#include <meteo/commons/global/global.h>

#include <qhash.h>
#include <qvector.h>
#include <qstringlist.h>
#include <qdom.h>
#include <qmap.h>

class MeteoElement;

class QString;
class QByteArray;
class TMeteoData;


//#define PARSE_DEBUG_LOG //подробный вывод всех шагов
#define PARSE_LOG  //основные ошибки раскодирования

namespace meteo {
  namespace anc {

    class DbMeta;

    //! Формат данных
    enum DecoderFormat {
      kUnkFormat   = 0,
      kWmoFormat   = 1,		//!< Буквенно-цифровом код ВМО
      kTableFormat = 2,		//!< Данные в виде таблицы
      kGphFormat   = 3,		//!< Геофизика (заголовок + таблица)
      kIonfoTlg    = 4,		//!< Какие-то ИОНФО были не в ВМО формате. пока так
      kCliwareFormat = 5        //!< Из БД ВНИИГМИ-МЦД 
    };

    class TMetaData {
    public:
      TMetaData();
      ~TMetaData();

      int load(const QString& confFile, const QList<int>& decodeTypes);
      int load(const QList<QString>& usedForms, const QList<int>& decodeTypes);
      int loadConf(const QStringList& confFiles, const QList<int>& decodeTypes);
      bool isCodeLoaded(int num);
      void setDbMeta(anc::DbMeta* meta) { _dbmeta = meta; }

      const QHash<QString,int>& forms() { return _forms; }
      
      QList<QRegExp> headers(int num)  { return _headers.values(num); }
      QList<int> uniqHeaderKeys() { return _headers.uniqueKeys(); }

      const QMultiHash<int,QString>& subgroups() { return _subgroups; }

      QDomDocument domDecoder(int num);
      DecoderFormat format(const QString& type);
      DecoderFormat format(int typeNum);

      const MeteoElement* dataElements() { return _dataElements; }
      void setMeteoData(TMeteoData* data);

      const meteo::anc::DecodingTypes& decodingTypes() { return _conf; }

      int bufrcode(const QString& text);
      QString currentCountry(int category, const QString& station);

    private:
      bool loadXmlCodeforms(const QString &filename, const QList<int>& decodeTypes);
      bool loadXmlDecoders(const QList<QString>& usedForms);

    private:
      bool _isLoaded;

      QHash<QString, int> _forms; //! тип (название файла), порядковый номер
      QMultiHash<int, QString> _subgroups; //! группа в составных сводках (гелио), порядковый номер (соотв. _forms)
      QHash<int, DecoderFormat> _format;
      QMultiHash<int, QRegExp> _headers;
      QHash<int, QDomDocument> _domDecoders; //!< xml для всех сводок (FM12, FM13, FM35, ...)

      MeteoElement* _dataElements = nullptr; //!< для преобразования метеовеличин из строкового представления

      meteo::anc::DecodingTypes _conf; //!< Типы данных и названия для отображения состояния


      anc::DbMeta* _dbmeta = nullptr; //!< Доступ к инфе в БД
    };

    typedef TSingleton<TMetaData> MetaData;

  } //anc
} //meteo

#endif
