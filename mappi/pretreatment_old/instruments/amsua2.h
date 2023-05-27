#ifndef MAPPI_PRETREATMENT_INSTRUMENTS_AMSUA2_H
#define MAPPI_PRETREATMENT_INSTRUMENTS_AMSUA2_H

#include <mappi/pretreatment/formats/instrument.h>
#include <mappi/pretreatment/instruments/hrpt_const.h>

namespace mappi {
  namespace po {

    class Frame;

    //! Чтение данных AMSU-A2
    class AmsuA2 : public InstrumentTempl<short> {
    public:
      AmsuA2(const mappi::conf::Instrument& conf, const conf::InstrFormat& format, Frame* parent);
      ~AmsuA2();

      // enum {
      // 	Type = conf::kAmsuA2
      // };
      
      virtual InstrFrameType type() const { return  InstrFrameType(conf::kAmsuA2, conf::kHrpt); }
      // virtual conf::InstrumentType type() const { return conf::kAmsuA2; }
      
      //      void setPretrSettings(const mappi::conf::PretrInstr& ) {};      

    protected:
      ChannelVector<short>& channels() { return _ch; };
      const ChannelVector<short>& channels() const { return _ch; };
      const Channel<short>& channel(int number) const { return _ch.at(number); };      

    private:
      bool parse(const QByteArray& data, QDateTime* dt, QDateTime* );
      bool parse(const QList<QByteArray>& data, QDateTime* dtStart, QDateTime* dtEnd);
      bool correction(const SatViewPoint*) { return true; }
      bool calibrate();
      bool readFactors(const QString& coefName);
      void createCalibrCoef(const QVector<ushort>& housekeeping, const QMap<int, QVector<float>>& cold,
			    const QMap<int, QVector<float>>& rf, const QVector<float>& prt,
			    const QMap<int, QVector<float>>& warm);

      void addAmsuA2Data(const QByteArray& amsuA2, int line,  QVector<ushort>* housekeeping, QMap<int, QVector<float>>* cold,
			 QMap<int, QVector<float>>* rf, QVector<float>* prt, QMap<int, QVector<float>>* warm);

      void writeCalibrCoefs(ushort housekeeping, const QMap<int, QVector<float>>& meanCold,
			    const QMap<int, QVector<float>>& meanWarm, 
			    const QVector<float>& meanPrt, const float* rfConst,  const QVector<float>& rfTemp);
      bool calibrate(Channel<short>* chd, const QVector<QVector<float>>& a, const float t2r[3]);
      
    private:
      ChannelVector<short> _ch; //каналы
      
      int _count;//!< Количество считанных данных
      
      mappi::conf::Instrument _conf; //!< описание формата прибора
      Frame* _frame;

      Factors _f;		//!< Коэффициенты для калибровки
      bool _fok;		//!< true - коэффициенты для калибровки загружены
      QMap<int, QVector<QVector<float>>> _aCoef; //канал, строка, 3 коэффициента
    };

  }
}

#endif
