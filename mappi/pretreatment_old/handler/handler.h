#ifndef MAPPI_PRETREATMENT_HANDLER_PRETRHANDLER_H
#define MAPPI_PRETREATMENT_HANDLER_PRETRHANDLER_H

#include <mappi/proto/satellite.pb.h>
#include <mappi/global/streamheader.h>


#include <qstring.h>
#include <qdatetime.h>
#include <stdint.h>

class QFile;

namespace MnSat {
  class TLEParams;
}

namespace mappi {
  class SaveNotify;
  
  namespace po {

    struct StreamHeader;
    class Frame;

    //! предварительная обработка спутниковых данных
    class Handler {
    public:
      Handler();
      ~Handler();

      enum HeaderType {
        kNoHeader      = 0, //!< нет заголовка
	kPreOldHeader  = 1, //!< сюжетовский заголовко в начале файла
	kPostOldHeader = 2, //!< сюжетовский заголовко в конце файла
	kMappiHeader   = 3  //!< наш заголовок
      };
      
      const meteo::global::StreamHeader& header() { return _header; }

      void setTle(const MnSat::TLEParams& tle) { _header.tle = tle; }
      void setFile(const QString& fileName)   { _fileName = fileName; }
      void setRecvMode(conf::RateMode& mode)  { _mode = mode; }
      void setSatName(const QString& satName) { _header.satellite = satName; }
      void setDateTime(const QDateTime& dt)   { _header.start = dt; }
      void setHeaderType(HeaderType type)     { _headerType = type; }
      void setDataLevel(mappi::conf::DataLevel level) { _level = level; }
      void setSite(const QString& name)       { _header.site = name; }
      
      void clear();
      bool process(SaveNotify* notify);
      bool processFileAll(QFile& file, uint32_t dataSize);
      bool processFileParts(QFile& file, uint32_t dataSize);
      // bool processL();
      

      // bool setupRT(const QString& satName, mappi::conf::RateMode mode, const MnSat::TLEParams& tle);
      // bool processRT(const QDateTime& dt, const QByteArray& data, QByteArray* result);

      void setPath(const QString& path) { _path = path + "/pretreatment/"; }
      void setNameSuffix(const QString& suffix) { _suffix = suffix; }
      
    private:
      bool readHeader(QFile* file, uint64_t* dataSize);
      bool setupFormat(SaveNotify* notify);      

    private:
      Frame* _format = nullptr; //!< Обработчик данных заданного формата

      conf::RateMode _mode = conf::kUnkRate; //!< Режим приёма
      QString _fileName;    //!< Файл с данными
      meteo::global::StreamHeader _header; //!< мета-информация потока данных
      uint64_t _oneRead = 0; //!< Число Мб, считываемых за раз ( 0 - весь файл )

      HeaderType _headerType = kNoHeader; 
      mappi::conf::DataLevel _level = mappi::conf::kUnkLevel;

      QString _path;
      QString _suffix; //!< Добавление к имени файла (для уникальности имени при сохранении одних данных из разных источников)

    };

  }
}

#endif
