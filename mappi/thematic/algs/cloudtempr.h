#pragma once

#include <mappi/thematic/algs/themalg.h>
#include <mappi/proto/thematic.pb.h>
#include <qvector.h>

namespace mappi {
  namespace to {

    class CloudTempr: public ThemAlg {
    public:
      CloudTempr(QSharedPointer<to::DataStore>& ds);
      ~CloudTempr();
      
      enum {
            Type = conf::kCloudTempr
      };
      virtual conf::ThemType type() const { return conf::kCloudTempr; }

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

