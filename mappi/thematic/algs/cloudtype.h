#pragma once

#include <mappi/thematic/algs/themalg.h>
#include <mappi/proto/thematic.pb.h>
#include <qvector.h>

namespace mappi {
  namespace to {

    class CloudType: public ThemAlg {
    public:
      CloudType(QSharedPointer<to::DataStore>& ds);
      ~CloudType();
      
      enum {
            Type = conf::kCloudType
      };
      virtual conf::ThemType type() const { return conf::kCloudType; }

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

