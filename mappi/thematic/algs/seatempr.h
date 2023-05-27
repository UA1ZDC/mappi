#pragma once

#include <mappi/thematic/algs/themalg.h>
#include <mappi/proto/thematic.pb.h>
#include <qvector.h>

namespace mappi {
  namespace to {

    class kSeaTempr: public ThemAlg {
    public:
      kSeaTempr(QSharedPointer<to::DataStore>& ds);
      ~kSeaTempr();
      
      enum {
            Type = conf::kSeaTempr
      };
      virtual conf::ThemType type() const { return conf::kSeaTempr; }

      bool process(conf::InstrumentType instr);
      
    protected:
      bool readConfig(QList<conf::InstrumentType>* valid); //!< чтение конфиг файла
      //! список псевдонимов каналов необходимых для обработки
      bool channelsList(conf::InstrumentType instr, QList<uint8_t>* chl); 
      
    private:

      conf::NdviConf _conf;
    };

  }
}

