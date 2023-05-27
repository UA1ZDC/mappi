#ifndef MAPPI_PRETREATMENT_FRAMES_HRPT_H
#define MAPPI_PRETREATMENT_FRAMES_HRPT_H

#include <mappi/pretreatment/formats/frame.h>
#include <mappi/pretreatment/frames/frametypes.h>
#include <mappi/proto/satellite.pb.h>

class QByteArray;
class QDateTime;
class QDate;

namespace mappi {
  namespace po {

    class Instrument;

    //! обработка HRPT формата
    class Hrpt : public Frame {
    public:
      Hrpt(const conf::Frame& conf, Frame* parent);
      virtual ~Hrpt();

      enum {
	Type = conf::kHrpt
      };

      virtual conf::FrameType type() const { return conf::kHrpt; }

      void clear();

      int chan3Type() const { return _ch3x; }
      int lineChan3Change() const { return _lineChan3x; }
      int numMinor() const { return _numMinor; }

    private:
      bool parseFormatCustom(conf::DataLevel level, const QDateTime& dt, QByteArray* adata);

      bool setSatellite(uint addr);

      bool readId(const QByteArray& data);
      bool readDt(const QDateTime& dt, const QByteArray& data);
      bool readStartDt(const QDate& date, const QByteArray& data, QDateTime* dtStream, int* num);
      bool readEndDt(const QDate& date, const QByteArray& data, QDateTime* dtEnd, int* anum);
      bool readDt(const QByteArray& buf, const QDate& d, QDateTime& dtStream);
      bool readChan3x(const QByteArray& data);
      bool missingLinesCount(const QByteArray& data, int pos, const QDateTime& curDt, 
			     int* missed, int* numDt, QDateTime* nextDt);

      int parseFrame(Instrument* frame, const conf::InstrFormat& instrFormat, const QByteArray& data);
      bool parseFormatCustomRT(conf::DataLevel level, const QDateTime& dt, QByteArray* adata);

      bool isBigEndian(const QByteArray& data);
    private:
      mappi::conf::Frame _format;

      int _numMinor = -1; //!< Номер minor frame HRPT
      int _ch3x = -1;     //!< 0=AVHRR Ch3B, 1=AVHRR Ch3A
      int _lineChan3x = -1; //!<  Номер линии сканирования, если изменился 3A/B канал,иначе -1
      int _startNum = -1; //!< Номер первой строки с достоверной датой/временем
      int _endNum = -1; //!< Номер последней строки с достоверной датой/временем
    };

  }
}

#endif
