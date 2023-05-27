#ifndef MAPPI_THEMATIC_ALGS_CLOUDMASK_H
#define MAPPI_THEMATIC_ALGS_CLOUDMASK_H


#include <mappi/thematic/algs/themalg.h>
#include <mappi/proto/thematic.pb.h>
#include <qvector.h>

namespace mappi {
  namespace to {

    class CloudMask: public ThemAlg {
    public:
      CloudMask(QSharedPointer<to::DataStore>& ds);
      ~CloudMask();
      
      enum {
            Type = conf::kCloudMask
      };
      virtual conf::ThemType type() const { return conf::kCloudMask; }

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
  
