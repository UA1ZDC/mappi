#ifndef MAPPI_THEMATIC_ALGS_RGBIMG_H
#define MAPPI_THEMATIC_ALGS_RGBIMG_H

#include <mappi/thematic/algs/themalg.h>
#include <mappi/proto/thematic.pb.h>
#include <qvector.h>

namespace mappi {
  namespace to {

    class RgbImg: public ThemAlg {
    public:
      RgbImg(QSharedPointer<to::DataStore>& ds, conf::ThemType type);
      ~RgbImg();


      virtual conf::ThemType type() const { return _type; }

      bool process(conf::InstrumentType instr);
      
    protected:
      bool readConfig(QList<conf::InstrumentType>* valid); //!< чтение конфиг файла
      //! список псевдонимов каналов необходимых для обработки
      bool channelsList(conf::InstrumentType instr, QList<uint8_t>* chl);
      bool getChannels(const conf::RgbRange& range, QSharedPointer<Channel>* ch1, QSharedPointer<Channel>* ch2, float* gamma, float* coef);
      
    private:    
      QVector<uint8_t> _data;
      //      QStringList _chAlias;

      conf::RgbConf _conf;
      conf::ThemType _type = conf::kThemUnk;
    };

    
    // class RgbDayMicrophisics: public RgbImg {
    // public:
    //   RgbDayMicrophisics(DataStore* ds);
    //   ~RgbDayMicrophisics();

    //   enum {
    //   	    Type = conf::kRgbDayMicrophisics
    //   };
    //   virtual conf::ThemType type() const { return conf::kRgbDayMicrophisics; }
    // };
    
  }
}


#endif
  
