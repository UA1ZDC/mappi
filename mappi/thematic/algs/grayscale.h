#ifndef MAPPI_THEMATIC_ALGS_GRAYSCALE_H
#define MAPPI_THEMATIC_ALGS_GRAYSCALE_H

#include <mappi/thematic/algs/themalg.h>
#include <mappi/proto/thematic.pb.h>
#include <qvector.h>

namespace mappi {
  namespace to {

    class GrayScale: public ThemAlg {
    public:
      GrayScale(QSharedPointer<to::DataStore>& ds);
      ~GrayScale();

      enum {
      	    Type = conf::kGrayScale
      };
      virtual conf::ThemType type() const { return conf::kGrayScale; }

      bool process(conf::InstrumentType ) { return false; }
      bool process(const QDateTime& start, const QString& satname, conf::InstrumentType instrument);

    protected:
      bool readConfig(QList<conf::InstrumentType>* valid); //!< чтение конфиг файла
      //! список псевдонимов каналов необходимых для обработки
      bool channelsList(conf::InstrumentType instr, QList<uint8_t>* chl);

   private:
      bool processChannel(const QSharedPointer<Channel>& ch);
      
    private:
      conf::GrayscaleConf _conf;
    };

  }
}

#endif
