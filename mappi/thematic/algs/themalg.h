#ifndef MAPPI_THEMATIC_ALGS_THEMALG_H
#define MAPPI_THEMATIC_ALGS_THEMALG_H

#include <cross-commons/debug/tlog.h>

#include <mappi/proto/thematic.pb.h>

#include <qmap.h>
#include <qsharedpointer.h>
#include <qimage.h>

namespace mappi {
  namespace to {

    class Channel;
    class DataStore;

    class ThemAlg {
    public:
      ThemAlg(QSharedPointer<to::DataStore>& ds);
      virtual ~ThemAlg();

      enum {
      	    Type = conf::kThemUnk
      };
      virtual conf::ThemType type() const { return conf::kThemUnk; }
      
      void clear() { _ch.clear(); }

      //обработка сеанса
      virtual bool process(const QDateTime& start, const QString& satname, QList<conf::InstrumentType> instruments);
      virtual bool process(const QDateTime& start, const QString& satname, conf::InstrumentType instrument);

      virtual bool process(conf::InstrumentType instr) = 0;
      
      void normalize(int min, int max);



    protected:
      virtual bool readConfig(QList<conf::InstrumentType>* valid) = 0; //!< чтение конфиг файла
      bool readConfig(const QString& filename, ::google::protobuf::Message* conf, QList<conf::InstrumentType>* valid);
      virtual bool channelsList(conf::InstrumentType instr, QList<uint8_t>* chl) = 0;      //!< список номеров каналов необходимых для обработки
      
      bool readChannelData(uint8_t number, const QString& fileName); //!< чтение файла
      
      const QMap<uint8_t, QSharedPointer<Channel>>& channels() { return _ch; }
      
      QSharedPointer<DataStore> store() { return  _ds; }
      void clearChannels() { _ch.clear(); }

      void getSavePath(const QDateTime& start, const QString& satname, QString* path, QString* templName);
      
      bool saveData(const QString& );
      bool saveImage(const QString& name, const QSharedPointer<Channel>& channel, QImage::Format format);

      QVector<uchar> _data;
      QVector<QRgb> _palette;

    private:
      QList<conf::InstrumentType> _validInstr; //!< приборы для которых есть настройка для обработки
      
      QMap<uint8_t, QSharedPointer<Channel>> _ch;
      QSharedPointer<DataStore> _ds;
    };

  }
}

#endif
