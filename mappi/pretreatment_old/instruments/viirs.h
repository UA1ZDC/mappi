#ifndef MAPPI_PRETREATMENT_INSTRUMENTS_VIIRS_H
#define MAPPI_PRETREATMENT_INSTRUMENTS_VIIRS_H

#include <mappi/pretreatment/formats/instrument.h>
#include <mappi/proto/satellite.pb.h>
#include <mappi/pretreatment/formats/hrpt_frame.h>

#include <qvector.h>

namespace mappi {
  namespace po {

    class Frame;
    class CCSDSHeader;
    
    // Channel settings
    struct ViirsChannelSettings
    {
      int channel;
      QString name;
      int zoneWidth[6];
      int zoneHeight;
      int totalWidth;
      int oversampleZone[6];
      bool invert;
    };

    
    //! одна строка сканирования
    class ViirsScan {
    public:
      ViirsScan(const QDateTime& adt):_dt(adt) {}
      ViirsScan(int size) { _scan.fill(0, size); }
      ~ViirsScan() {}
      
      bool parseSegments(int apid, const QByteArray& detectorData);
      const QVector<ushort>& data() const { return _scan; }
      QVector<ushort>* data() { return &_scan; }
      const QDateTime& dt() const { return _dt; }
      
    private:
      QVector<ushort> decompress(const QByteArray& segment, uint resSize, int oversample);
      
    private:
      QDateTime _dt;
      QVector<ushort> _scan;      
    };

    
    //! сканирование всеми детекторами (полоса)
    struct ViirsDetectorsScan {
      QDateTime dt;
      QList<ViirsScan> pck; //!< набор строк, созданных линейкой детекторов (их количество = ViirsChannelSettings::zoneHeight)
      int size; //!< указанный размер в стартовом пакета (д. соотв. настройкам канала = число детекторов)
      
      ViirsDetectorsScan(const QDateTime& adt, int pckCnt):dt(adt), size(pckCnt) {}
      
      bool addPacket(const QDateTime& adt, int apid, const QByteArray& detectorData);
      void addEmpty(int apid, int cnt);

      void interpolateLines(int apid);
      void interpolateZone(float coef, int cutSize, int resCutSize, int height, int posStart, int posEnd);
    };
    
    
    //! строки сканирования канала
    class ViirsChannel {
    public:
      ViirsChannel() {}
      ~ViirsChannel() {}
	
      void setStart(int id, const QDateTime& dt, int pckCount, int num) {
	_apid = id;
	_isStarted = true;

	if (_pckCount !=0 && _data.last().size != _data.last().pck.size()) {
	  debug_log << QObject::tr("error pck size") << _data.last().size << _data.last().pck.size();
	}
	
	_pckCount = pckCount;
	_curNum = num;
	ViirsDetectorsScan scan(dt, pckCount - 1); //без стартового пакета
	_data.append(scan);
      }
      
      bool parseScienceData(const QDateTime& dt, int num, const QByteArray& detectorData) {
	if (!_isStarted) {
	  return false;
	}

	if (_curNum == 0x3fff) { //2047
	  _curNum = -1;
	}

	if (num != _curNum + 1 && num - 1 - _curNum < _pckCount) {
	  debug_log << "add empty" << num - 1 - _curNum; 
	  _data.last().addEmpty(_apid, num - 1 - _curNum);
	  _pckCount -= num - 1 - _curNum;
	}
	
	_pckCount--;
	_curNum = num;
	
	//var(_pckCount);
	
	if (_pckCount < 0) {
	  debug_log << "unwanted" << _pckCount;
	  return false;
	}
	
	return _data.last().addPacket(dt, _apid, detectorData);
      }

      QVector<ushort> alldata() const {	
	QVector<ushort> all;
	for (auto detectors : _data) {
	  for (const ViirsScan& scan : detectors.pck) {
	    all.append(scan.data());
	  }
	}
	return all;
      }

      const QList<ViirsDetectorsScan>& data() const { return _data; }
      QList<ViirsDetectorsScan>* data() { return &_data; }
      int apid() const { return _apid; }
      
    private:
      int _apid;
      bool _isStarted = false;
      int _pckCount = 0; //!< количество пакетов (детекторов) в полосе сканирования
      int _curNum = 0; //!< текущий номер пакета
      QList<ViirsDetectorsScan> _data;//!< строки в виде наборов полос, созданных линейкой детекторов
    };


    //! калибровочные данные для линейки детекторов (полосы) одного канала
    struct ViirsCalibrChannel {
      //размер 48 - M, 96 - I, 64(16) - DNB
      QVector<ushort> space; 
      QVector<ushort> blackBody;
      QVector<ushort> solarDiffuser;
    };


    //! калибровочные данные для полосы, сканирование всеми каналами
    struct ViirsCalibrScan {
      QDateTime dt;
      QMap<int, ViirsCalibrChannel> pck; //!< набор значений для линейки детекторов
      
      int size; //!< указанный размер в стартовом пакета 
      
      ViirsCalibrScan(const QDateTime& adt, int pckCnt):dt(adt), size(pckCnt) {}
      ~ViirsCalibrScan() {}
      
      bool addPacket(const QDateTime& adt, const QByteArray& calibrData);
      // void addEmpty(int cnt);    
      
    };
    

    //! строки сканирования данных калибровки
    class ViirsCalibr {
    public:
      ViirsCalibr() {}
      ~ViirsCalibr() {}
	
      void setStart(const QDateTime& dt, int pckCount, int num, uint8_t dnbSeq) {
	_isStarted = true;

	if (_pckCount !=0 && _data.last().size != _data.last().pck.size()) {
	  debug_log << QObject::tr("error pck size") << _data.last().size << _data.last().pck.size();
	}
	
	_pckCount = pckCount;
	_curNum = num;
	_dnbSeq = dnbSeq;
	
	ViirsCalibrScan scan(dt, pckCount - 1); //без стартового пакета
	_data.append(scan);
      }
      
      bool parseScienceData(const QDateTime& dt, int num, const QByteArray& calibrData) {
	if (!_isStarted) {
	  return false;
	}

	if (_curNum == 0x3fff) { //2047
	  _curNum = -1;
	}

	if (num != _curNum + 1 && num - 1 - _curNum < _pckCount) {
	  debug_log << "add empty" << num - 1 - _curNum; 
	  //	  _data.last().addEmpty(num - 1 - _curNum);
	  _pckCount -= num - 1 - _curNum;
	}
	
	_pckCount--;
	_curNum = num;
	
	//var(_pckCount);
	
	if (_pckCount < 0) {
	  debug_log << "unwanted" << _pckCount;
	  return false;
	}
	
	return _data.last().addPacket(dt, calibrData);
      }

      // QVector<ushort> alldata() const {	
      // 	QVector<ushort> all;
      // 	for (auto detectors : _data) {
      // 	  for (const ViirsScan& scan : detectors.pck) {
      // 	    all.append(scan.data());
      // 	  }
      // 	}
      // 	return all;
      // }

      const QList<ViirsCalibrScan>& data() const { return _data; }
      QList<ViirsCalibrScan>* data() { return &_data; }

      
    private:
      bool _isStarted = false;
      int _pckCount = 0; //!< количество пакетов (детекторов) в полосе сканирования
      int _curNum = 0;   //!< текущий номер пакета
      uint8_t _dnbSeq = 0;
      QList<ViirsCalibrScan> _data; //!< строки в виде наборов полос, созданных линейкой детекторов
    };
    

   //! Чтение данных VIIRS
    class Viirs : public InstrumentTempl<ushort> {
    public:
      Viirs(const mappi::conf::Instrument& conf, const conf::InstrFormat& format, Frame* parent);
      ~Viirs();
    
      virtual InstrFrameType type() const { return mappi::InstrFrameType(conf::kViirs, mappi::conf::kCadu); }
      
      //      void setPretrSettings(const mappi::conf::PretrInstr& ) {};      
      float fovStep(int channel) const;
    protected:
      ChannelVector<ushort>& channels() { return _ch; };
      const ChannelVector<ushort>& channels() const { return _ch; };
      const Channel<ushort>& channel(int number) const { return _ch.at(number); };

      //virtual bool needUnion() { return true; }
      
    private:
       bool parse(const QList<QByteArray>& dataAll, QDateTime* dtStart, QDateTime* dtEnd);
      //bool parse(const QByteArray& dataAll, const QDateTime& dt);

      bool correction(const SatViewPoint* ) { return true; }
      bool calibrate() { return false; }

      bool recoverDiffChannels(int diffNum, int baseNum, int step);
      void fillDeletedLines();
      
      void readCalibrData(const CCSDSHeader& header, const QByteArray& data);
      
    private:
      ChannelVector<ushort> _ch; //каналы
      QMap<int, ViirsChannel> _band; //<канал, данные>

      ViirsCalibr _calBand;

      mappi::conf::Instrument _conf; //!< описание формата прибора
      Frame* _frame;

    };




  }
}



#endif
  
