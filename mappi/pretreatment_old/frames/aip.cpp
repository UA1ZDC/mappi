#include "aip.h"
#include "hrpt.h"
#include "simpledeframer.h"

#include <mappi/pretreatment/formats/instrument.h>
#include <mappi/pretreatment/formats/satformat.h>

#include <cross-commons/debug/tlog.h>

#define NEED_VALID_READ_CNT 5  //!< Количество совпадений данных, по которому можно сделать вывод о их верности
#define AIP_HRPT_FRAME_SIZE 104*2
#define AIP_FRAME_SIZE 104

using namespace mappi;
using namespace po;

namespace {
mappi::po::Frame* createAip(const mappi::conf::Frame& conf, Frame* parent)
{
  return new mappi::po::Aip(conf, parent);
}

static const bool res = mappi::po::singleton::SatFormat::instance()->registerFrameHandler(mappi::conf::kAip, createAip);
}


Aip::Aip(const conf::Frame& conf, Frame* parent):
  Frame(conf, false, parent)
{
}


Aip::~Aip()
{
}

void Aip::clear()
{
}

void Aip::addFrame(const QByteArray& frame)
{
  //aip данные только в третьем младшем кадре HRPT
  if (_hrptMinor == -1) {
    _hrptMinor = static_cast<Hrpt*>(parent())->numMinor();
  } else {
    ++_hrptMinor;
    if (_hrptMinor == 4) {
      _hrptMinor = 1;
    }
  }
  if (_hrptMinor != 3) {
    return;
  }
  Frame::addFrame(frame);
}

bool Aip::processFrames(const QDateTime& dt)
{
  QByteArray src = data();



  _aipMinor = -1;
  determDataWordMinor(src, 80, &_aipMinor);
  if (_aipMinor == -1) {
    return false;
  }
  //var(_aipMinor);
  
  int pos = 0;
  //  while (_aipMinor != 0) {
  //    pos += AIP_HRPT_FRAME_SIZE;
  //    ++_aipMinor;
  //    if (_aipMinor == 80) {
  //      _aipMinor = 0;
  //    }
  //  }

  QByteArray dst;
  while (pos + 3 <= src.size()) {
    uchar byte1  = (uchar(src[pos]) << 6) + (uchar(src[pos + 1]) >> 2);
    uchar byte2  = (uchar(src[pos + 2]) << 6) + (uchar(src[pos + 3]) >> 2);

    //  bool  check1 = checkDataWordByte(uchar(src[pos + 1]), byte1);
    //  bool  check2 = checkDataWordByte(uchar(src[pos + 3]), byte2);
    // int hem1 = MnMath::hemDistance(byte1, 0);
    //  int hem2 = MnMath::hemDistance(byte2, 1);
    //  if ((!check1 && 1 >= hem1 && !check2 && 1 >= hem2) ||
    //      (1 >= hem1 + hem2 && (!check1 || !check2))) {
    //    byte1 = 0;
    //    byte2 = 1;
    //  }
    
    dst.append(byte1);
    dst.append(byte2);
    pos += 4;
  }

//  QFile fl("/home/gotur/tmp/aip.dat");
//  fl.open(QIODevice::ReadWrite|QIODevice::Truncate);
//  fl.write(dst);
//  fl.close();


  setFrameData(dst);
  
  return Frame::processFrames(dt);
}

//! парсинг подкадров
/*! 
  \param instr прибор, отвечающий за обработку заданного типа подкадра
  \param instrFormat описание подкадра
  \return номер последнего считанного байта
*/
int Aip::parseFrame(Instrument* instr, const conf::InstrFormat& instrFormat, const QByteArray& data)
{
  // QFile file("/tmp/aip.dat");
  // file.open(QIODevice::WriteOnly);
  // file.write(data);
  // file.close();

  int pos = 0;
  switch (instr->type().first) {
    case conf::kAmsuA2:
      debug_log << "amsu2";
    case conf::kAmsuA1:
    {
      QByteArray amsuAxwords;
      int start = instrFormat.sect(0).start();
      int size = instrFormat.sect(0).size();
      while (pos + AIP_FRAME_SIZE <= data.size())
      {
        const QByteArray &buffer = data.mid(pos , AIP_FRAME_SIZE );
        for (int j = 0; j < size; j += 2)
        {
          if (((buffer[j + start] << 8) | (buffer[j + start+1] ))!= 1)
          {
            amsuAxwords.push_back(buffer[j + start]);
            amsuAxwords.push_back(buffer[j + start+1]);
          }
        }
        pos += AIP_FRAME_SIZE;
      }
      instr->addNewFrame(_aipMinor, amsuAxwords);
      amsuAxwords.clear();

    }
    break;
    case conf::kMhs:
    default:
      while (pos + AIP_FRAME_SIZE <= data.size()) {
        instr->addNewFrame(_aipMinor, data.mid(pos + instrFormat.sect(0).start(), (instrFormat.sect(0).size())));
        for (int idx = 1; idx < instrFormat.sect_size(); idx++) {
          instr->addFrame(data.mid(pos + instrFormat.sect(idx).start(), instrFormat.sect(idx).size()));
        }
        pos += AIP_FRAME_SIZE;
      }
    break;

  }




  return pos;
}

void Aip::determDataWordMinor(const QByteArray& data, int max, int* minor)
{
  uint check =  0;  //Количество совпадений ожидаемых и фактических значений данных
  uint numRead = 0; //количество строк, потребовавшееся для нахождения значения
  int num = 0;     //Считанное значение

  int pos = 4*2; //положение major frame count в кадре AIP

  while (pos + AIP_HRPT_FRAME_SIZE < data.size() && (check < NEED_VALID_READ_CNT)) {
    num = (uchar(data[pos]) << 6) + (uchar(data[pos + 1]) >> 2);
    //debug_log << num << check;
    bool checkByte = checkDataWordByte(uchar(data[pos + 1]), num);
    if ( checkByte &&
         ( ((*minor + 1) == num) || (*minor == max && num == 0)) ) {
      ++check;
    } else {
      check = 0;
    }
    
    *minor = num;
    ++numRead;

    if (check >= NEED_VALID_READ_CNT) {
      break;
    }

    pos += AIP_HRPT_FRAME_SIZE;
  }

  if (check < NEED_VALID_READ_CNT) {
    *minor = -1;
  } else {
    *minor -= (numRead % (max + 1)) - 1;
    if (*minor < 0) *minor += max + 1;
  }
}


/*! 
 * \brief проверка байта с помощью битов parity и invert
 * \param  lo - младший исходный байт
 * \param  val - значение, полученное из двух исходный байт
 * \return true - байт прошёл проверку, false - не прошёл
 */
bool Aip::checkDataWordByte(uchar lo, uchar val)
{
  uint parity = (lo >> 1) & 0x1;
  int invert = lo & 0x1;

  if (invert ==  ((val >> 7) & 0x1)) {
    return false;
  }

  uint count = 0;
  for (uint mask = 1; mask < 0xff; mask *= 2) {
    if (0 != (val & mask)) {
      ++count;
    }
  }

  if (count%2 != parity) return false;

  return true;
}
