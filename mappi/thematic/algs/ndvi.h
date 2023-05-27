#ifndef MAPPI_THEMATIC_ALGS_NDVI_H
#define MAPPI_THEMATIC_ALGS_NDVI_H


#include <mappi/thematic/algs/themalg.h>
#include <mappi/proto/thematic.pb.h>
#include <qvector.h>

namespace mappi {
  namespace to {

    class Ndvi: public ThemAlg {
    public:
      Ndvi(QSharedPointer<to::DataStore>& ds);
      ~Ndvi();
      enum class Color { kRed, kGreen, kBlue };

      enum {
      	    Type = conf::kNdvi
      };
      virtual conf::ThemType type() const { return conf::kNdvi; }

      bool process(conf::InstrumentType instr);
      
    protected:
      bool readConfig(QList<conf::InstrumentType>* valid); //!< чтение конфиг файла
      //! список псевдонимов каналов необходимых для обработки
      bool channelsList(conf::InstrumentType instr, QList<uint8_t>* chl); 
      
    private:
      //      QStringList _chAlias;

      conf::NdviConf _conf;
    };

  }
}


#endif
  
