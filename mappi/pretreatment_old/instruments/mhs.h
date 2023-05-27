#ifndef MAPPI_PRETREATMENT_INSTRUMENTS_MHS_H
#define MAPPI_PRETREATMENT_INSTRUMENTS_MHS_H

#include <mappi/pretreatment/formats/instrument.h>
#include <mappi/pretreatment/formats/channel.h>
#include <mappi/pretreatment/instruments/hrpt_const.h>

namespace mappi {
  namespace po {

    class Frame;
    
    //! Чтение данных MHS
    class Mhs : public InstrumentTempl<ushort> {
    public:
      Mhs(const mappi::conf::Instrument& conf, const conf::InstrFormat& format, Frame* parent);
      ~Mhs();
    
      // enum {
      // 	Type = conf::kMhs
      // };

      virtual InstrFrameType type() const { return  InstrFrameType(conf::kMhs, conf::kHrpt); }
      
      //      void setPretrSettings(const mappi::conf::PretrInstr& ) {};      

    protected:
      ChannelVector<ushort>& channels() { return _ch; };
      const ChannelVector<ushort>& channels() const { return _ch; };
      const Channel<ushort>& channel(int number) const { return _ch.at(number); };
     
    private:

      bool parse(const QList<QByteArray>& data, QDateTime* dtStart, QDateTime* dtEnd);
      bool correction(const SatViewPoint*) { return true; }
      bool calibrate();
      bool calibrate(Channel<ushort>* chd, const QVector<QVector<float>>& a, const float t2r[3]);

      bool readFactors(const QString& coefName);
      void addMhsData(const QByteArray& mhs, uint start, uint end,
		      uint /*num*/, uint numMinor, QVector<ushort>* housekeeping, QMap<int, QVector<float>>* rf,
		      QMap<int, QVector<float>>* cold, QMap<int, QVector<float>>* warm, QVector<float>* prt,
		      QVector<float>* resist);
      void createCalibrCoef(const QVector<ushort>& house, const QMap<int, QVector<float>>& cold,
			    const QMap<int, QVector<float>>& rf, QVector<float>& prt,
			    const QMap<int, QVector<float>>& warm, QVector<float>& resist);
      float calcMhsTw(const QVector<float>& prt, const QVector<float>& Ccal);

    private:
      ChannelVector<ushort> _ch; //каналы

      int _count = 0;//!< Количество считанных данных
      uint _numLines = 0; //!< Количество заполненных строк

      mappi::conf::Instrument _conf; //!< описание формата прибора
      Frame* _frame = nullptr;

      Factors _f;		//!< Коэффициенты для калибровки
      bool _fok = false;	//!< true - коэффициенты для калибровки загружены
      QMap<int, QVector<QVector<float>>> _aCoef; //канал, строка, 3 коэффициента
    };
    
  }
}

#endif
