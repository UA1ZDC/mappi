#ifndef MAPPI_PRETREATMENT_FORMATS_SATFORMAT_H
#define MAPPI_PRETREATMENT_FORMATS_SATFORMAT_H

#include <cross-commons/singleton/tsingleton.h>
#include <mappi/proto/satellite.pb.h>

#include <qmap.h>

class QDomNode;

namespace mappi {
  
  using InstrFrameType = QPair<::mappi::conf::InstrumentType, ::mappi::conf::FrameType> ;
  class SaveNotify;
  
  namespace po {   

    class Instrument;
    class Frame;

    typedef Instrument* (*CreateInstr)(const conf::Instrument&, const conf::InstrFormat& format, Frame*);
    typedef Frame* (*CreateFrame)(const conf::Frame&, Frame*);

    //! создание приборов, в соответствии с настройками
    class SatFormat {
    public:
      SatFormat();
      ~SatFormat();

      bool registerInstrHandler(InstrFrameType type, CreateInstr func );
      Instrument* createInstrument(const InstrFrameType& type, const conf::InstrFormat& format, Frame* parent) const;

      bool registerFrameHandler(conf::FrameType type, CreateFrame func );
      Frame* createFrame(conf::FrameType type, Frame* parent) const;
      Frame* createFrame(const conf::Frame& conf, Frame* parent) const;

      Frame* createFrame(conf::FrameType frameType,
			 const conf::InstrumentType instrType,
			 const conf::PretrInstr& conf,
			 po::Frame* frame,
			 SaveNotify* notify
			 );
      bool createInstrHierarchy(const mappi::conf::Frame& frame,
				const conf::InstrumentType instrType,
				QList<conf::Frame>* types);
      
      //! настройки по имени спутника
      const mappi::conf::PretrSat* pretreatment(const QString& name, conf::RateMode mode);
      // const mappi::conf::PretrInstr* instrumentRT(const QString& name, conf::RateMode mode,
      // 						  mappi::conf::DataLevel* level,
      // 						  mappi::conf::FrameType* frame_type);

    private:
      bool init();
      bool readInstrs();
      bool readFrames();
      bool readSats();

    private:
      QMap<InstrFrameType,  CreateInstr> _instHandler;
      QMap<conf::FrameType, CreateFrame> _frameHandler;

      QMap<conf::InstrumentType, mappi::conf::Instrument> _instrConf;
      QMap<conf::FrameType, mappi::conf::Frame> _frameConf;
      QMap<InstrFrameType, mappi::conf::InstrFrame> _instrFrameConf;
      QMap<QString, mappi::conf::PretrSat> _satConf;
    };

    namespace singleton {
      typedef TSingleton<mappi::po::SatFormat> SatFormat;
    }
    
  }
}

#endif 
