#ifndef MAPPI_PRETREATMENT_INSTRUMENTS_AVHRR3_H
#define MAPPI_PRETREATMENT_INSTRUMENTS_AVHRR3_H

#include <mappi/pretreatment/formats/instrument.h>
#include <mappi/proto/satellite.pb.h>
#include <mappi/pretreatment/formats/hrpt_frame.h>
#include <mappi/pretreatment/instruments/hrpt_const.h>
#include <mappi/pretreatment/formats/channel.h>

#include <commons/geobasis/coords.h>

#include <qvector.h>

#define LINE_PROCESS_COUNT 35 //!< Количество строк сканирования, используемое для усреднения коэффициентов

namespace mappi {
  namespace po {

    class Frame;
    class Hrpt;

    //! Прибор AVHRR/3
    namespace avhrr {
      //! Telemetry в HRPT файле
      struct TelemetrType {
	uint16_t rampCalibr[5];
	uint16_t prt[3];
	uint16_t chPatchTemp;
	uint16_t res;
      };
      
      //! часть данных HRPT от Telemetry до Sync Data включительно
      union CalibrData {
	uint16_t M[91];
	struct {
	  TelemetrType telemetry;
	  uint16_t calibrTargetView[30];
	  uint16_t spaceData[50];
	  uint16_t syncData;
	} S;
      };
      
      //!коэффициенты для калибровки 
      struct CalibrCoef {
	QVector<QVector<float> > prt; //5 PRT по 3*LINE_PROCESS_COUNT значений //[5][LINE_PROCESS_COUNT/5*3];
	QVector<QVector<float> > Cbb;//для 3 ИК каналов по 10*LINE_PROCESS_COUNT значений
	QVector<QVector<float> > Cs; //6 каналов, для каждого по 10*LINE_PROCESS_COUNT значений //[6][10*LINE_PROCESS_COUNT];
	CalibrCoef() {
	  prt.resize(5);
	  Cs.resize(6);
	  Cbb.resize(3);
	}
	void clear() {
	  prt.clear();
	  Cbb.clear();
	  Cs.clear();
	  prt.resize(5);
	  Cs.resize(6);
	  Cbb.resize(3);
	}
      };
    }


    //Сейчас не очищает состояние. Надо создавать и удалять

    //! Чтение данных AVHRR/3
    class Avhrr3 : public InstrumentTempl<short> {
    public:
      Avhrr3(const mappi::conf::Instrument& conf, const conf::InstrFormat& format, Frame* parent);
      ~Avhrr3();
    
      // enum {
      // 	Type = conf::kAvhrr3
      // };

      virtual InstrFrameType type() const { return  InstrFrameType(conf::kAvhrr3, conf::kHrpt); }

    protected:
      ChannelVector<short>& channels() { return _ch; };
      const ChannelVector<short>& channels() const { return _ch; };
      const Channel<short>& channel(int number) const { return _ch.at(number); };
      
      //      void serializedImgDataRT(QByteArray* ba);

    private:
      virtual bool parse(const QByteArray& data, QDateTime* dtStart, QDateTime* dtEnd);
      //      virtual bool parseRT(const QByteArray& data, const QDateTime& dt, int*);
      virtual bool correction(const SatViewPoint* sat);
      virtual bool calibrate();
      virtual bool save(const meteo::global::StreamHeader& header, const QString& path, const QString& baseName, 
			const QDateTime& , const QDateTime& , SatViewPoint* sat);
      
      //      virtual void setPretrSettings(const mappi::conf::PretrInstr& settings) { _pretr = settings; }
      bool readFactors(const QString& coefName);

      bool readScanLines(const QByteArray& ba, int length);
      bool readCalibrData(const QByteArray& ba, int length);

      bool readCalibrLine(const QByteArray& buf, int* prtIdx, int length, int num);
      int  checkPrtIdx(const QByteArray& ba);
      int calcACoef(int line);
      void repeatLastACoef();
      void infraRedACoef(int chanId, float tPrt[AVHRR3_PRT_CNT], float Cs_av, float Cbb_av, QVector<float>* a);
      float blackBodyRadiance( int chanId, float t_prt[AVHRR3_PRT_CNT] );
      void t_prt( const float prt_av[AVHRR3_PRT_CNT], float tPrt[AVHRR3_PRT_CNT] );

      void calibrateViz(int channel);
      void calibrateIr(int channel);

    private:
      int _chan3x;           //!< 0=AVHRR Ch3B, 1=AVHRR Ch3A
      int _lineChan3x;       //!< Номер линии сканирования, если изменился 3A/B канал,иначе -1

      Factors _f;		//!< Коэффициенты для калибровки
      avhrr::CalibrCoef _coef;  //!< Параметры для калибровки со спутника
      bool _fok;		//!< true - коэффициенты для калибровки загружены
      
      ChannelVector<short> _ch; //каналы

      QMap<int, QList< QVector<float> > > _a; //рассчитанные коэффициенты для калибровки (номер канала, коэффициенты для LINE_PROCESS_COUNT строк сканирования)
      mappi::conf::Instrument _conf; //!< описание формата прибора
      // mappi::conf::PretrInstr _pretr; //!< настройки предваритальной обработки
      Hrpt* _frame = nullptr;

      QVector<Coords::GeoCoord> _points; //TODO временно, для Алексея Костромитинова :)
    };
  
  }
}

#endif
