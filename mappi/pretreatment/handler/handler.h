#ifndef MAPPI_PRETREATMENT_HANDLER_PRETRHANDLER_H
#define MAPPI_PRETREATMENT_HANDLER_PRETRHANDLER_H

#include <mappi/proto/satellite.pb.h>
#include <mappi/global/streamheader.h>
#include <mappi/pretreatment/formats/satpipeline.h>

#include <QString>
#include <QDateTime>
#include <QFile>
#include <stdint.h>

namespace TSatPretr {
  //! Опции запуска предварительной обработки
  struct PretrOpt {
    int year;  //!< Год приёма данных
    int month; //!< Месяц приёма данных
    int day;   //!< День приёма данных
    mappi::conf::ByteSwap swap;//!< Порядок байтов(иногда в Raw16 четные и нечтные байты нужно поменять местами)
    mappi::conf::RateMode mode;//!< Режим приёма
    bool headerExist; //!< Наличие доп. заголовка
    bool oldHeaderExist; //!< Наличие доп. заголовка старого формата
    bool manchester;     //!< Необходимо снятие манчестера
    bool deframer;       //!< Необходим побитовый поиск синхропоследовательности
    bool invert;   //!< Инвертированные биты в потоке
    QString satName;     //!< Название спутника
    QString siteName;     //!< Название приемной станции
    QString pipelineName;   //!< Название пайплайна
    QString json_params;  //!< Дополнительные параметры пайплайна
  };
}

using namespace TSatPretr;

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
      Handler(){};
      ~Handler(){};

      enum HeaderType {
        kNoHeader      = 0, //!< нет заголовка
	      kPreOldHeader  = 1, //!< сюжетовский заголовко в начале файла
	      kPostOldHeader = 2, //!< сюжетовский заголовко в конце файла
	      kMappiHeader   = 3  //!< наш заголовок
      };
      
      const meteo::global::StreamHeader& header() { return _header; }

      void setFile(const QString& fileName)   { _fileName = fileName; }
      void setRecvMode(const conf::RateMode& mode)  { _mode = mode; }
      void setDataLevel(const mappi::conf::DataLevel level) { _level = level; }
      void setPipeline(const QString &pipelineName) { _pipeline.setName(pipelineName); }
      void setHeaderType(const HeaderType type)     { _headerType = type; }
      void setTle(const MnSat::TLEParams& tle) { _header.tle = tle; }
      void setSatName(const QString& satName) { _header.satellite = satName; }
      void setDateTime(const QDateTime& dt)   { _header.start = dt; _header.stop = dt;}
      void setSite(const QString& name)       { _header.site = name; }
      void setPath(const QString& path) { _path = path + "pretreatment"; }
      void setNameSuffix(const QString& suffix) { _suffix = suffix; }
      void setSwap(const conf::ByteSwap swap) { _swap = swap; }

      bool process(SaveNotify* notify);
      void parseStream(const QString &fileName, const QString &weatherFile, const QString &path, PretrOpt &opt);

      void dateFromOpt(TSatPretr::PretrOpt &opt, QDate &date);
      void clear();
    private:
      bool readHeader();
      bool setupPipeline(SaveNotify* notify);

    private:
      SatPipeline _pipeline; //!< Обработчик данных заданного формата

      conf::RateMode _mode = conf::kUnkRate; //!< Режим приёма
      conf::ByteSwap _swap = conf::kNoSwap; //!< Порядок байтов(иногда в Raw16 четные и нечтные байты нужно поменять местами)
      QString _fileName;    //!< Файл с данными
      meteo::global::StreamHeader _header; //!< мета-информация потока данных

      HeaderType _headerType = kNoHeader; 
      mappi::conf::DataLevel _level = mappi::conf::kUnkLevel;

      QString _tempFileName;  //<! Временный файл, если нужено удалить хедер или поменять байты
      QString _path; //!< Путь для сохранения
      QString _suffix; //!< Добавление к имени файла (для уникальности имени при сохранении одних данных из разных источников)
    };

  }
}

#endif
