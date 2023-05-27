#include "amsua1.h"
#include "hrpt_const.h"
#include "hrpt_calibr.h"

#include <cross-commons/debug/tlog.h>

#include <mappi/pretreatment/formats/channel.h>
#include <mappi/pretreatment/formats/frame.h>
#include <mappi/pretreatment/formats/satformat.h>
#include <mappi/pretreatment/formats/hrpt_frame.h>
#include <mappi/pretreatment/frames/simpledeframer.h>


#define AMSUA11_PRT_CNT (5)   //!<Количество PRT AMSU-A1-1
#define AMSUA12_PRT_CNT (5)   //!<Количество PRT AMSU-A1-2
#define AMSUA1_PRT_CNT (AMSUA11_PRT_CNT + AMSUA12_PRT_CNT) //!<Количество PRT AMSU-A1
#define AMSUA1_PLLO2OFF 6 //!< Смещение до коэффициентов, когда PLLO2

namespace Amsu {
const uint amsuInstr[13] = {2,2,2,0,0,2,1,1,1,1,1,1,0};
const uint pllo2off[13]  = {0,0,0,0,0,0,7,7,7,7,7,7,0};
}

namespace {
mappi::po::Instrument* createAmsuA1(const mappi::conf::Instrument& conf, const mappi::conf::InstrFormat& format, mappi::po::Frame* frame)
{
  return new mappi::po::AmsuA1(conf, format, frame);
}

static const bool res = mappi::po::singleton::SatFormat::instance()->
                        registerInstrHandler(mappi::InstrFrameType(mappi::conf::kAmsuA1, mappi::conf::kHrpt), createAmsuA1);
}

using namespace meteo;
using namespace mappi;
using namespace po;


AmsuA1::AmsuA1(const mappi::conf::Instrument& conf, const conf::InstrFormat& format, Frame* parent):
  InstrumentTempl(conf, format),
  _conf(conf),
  _frame(parent)
{
  _ch.resize(_conf.chan_cnt());
}

AmsuA1::~AmsuA1()
{
}

bool AmsuA1::parse(const QByteArray& data, QDateTime* dt, QDateTime* )
{
  Q_UNUSED(dt);



  if ( 0 == _frame) {
    return false;
  }
  const mappi::conf::PretrSat* satconf = singleton::SatFormat::instance()->pretreatment(_frame->satName(), _frame->mode());
  //debug_log << _frame->satName() << _frame->mode() << satconf;
  if (nullptr != satconf) {
    _fok = readFactors(QString::fromStdString(satconf->coef()));
  }
//  QFile fl("/home/gotur/tmp/outNashALL.dat");
//  fl.open(QIODevice::ReadWrite|QIODevice::Truncate);
//  fl.write(data);
//  fl.close();
  SimpleDeframer<uint32_t, 24, 1240 * 8, 0xFFFFFF> amsuA1Deframer;
  const QVector<QByteArray> &amsuA1Data = amsuA1Deframer.work(data);

  uint length = amsuA1Data.size();

  for (int idx = 0; idx < _ch.size(); idx++) {
    _ch[idx].setup(length, _conf.samples(), idx + 3);
    _ch[idx].setLimits(0, 0x7FFF);
  }

  QVector<ushort> housekeeping;
  QMap<int, QVector<float>> cold; //канал, значения
  QMap<int, QVector<float>> rf;   //номер rf
  QVector<float> prt1; //все prt по всем строкам
  QVector<float> prt2; //все prt по всем строкам
  QMap<int, QVector<float>> warm; //канал, значения


//  fl.setFileName("/home/gotur/tmp/outNash.dat");
//  fl.open(QIODevice::ReadWrite|QIODevice::Truncate);

//  for (uint idx = 0; idx < length; idx++) {
//    fl.write(amsuA1Data.at(idx));
// }
//  fl.close();
  int line = 0;

  for (uint idx = 0; idx < length; idx++) {
    //if (numMinor == 80) { numMinor = 0; _count = 0; line++; }
    addAmsuA1Data(amsuA1Data.at(idx), line, &housekeeping, &cold, &rf, &prt1, &prt2, &warm);
    line++;
  }



  createCalibrCoef(housekeeping, cold, rf, prt1, prt2, warm);
  return true;
}


bool AmsuA1::parse(const QList<QByteArray>& data, QDateTime* dt, QDateTime*)
{
  Q_UNUSED(dt)

  if (0 == _frame) {
    return false;
  }

  const mappi::conf::PretrSat* satconf = singleton::SatFormat::instance()->pretreatment(_frame->satName(), _frame->mode());
  //debug_log << _frame->satName() << _frame->mode() << satconf;
  if (nullptr != satconf) {
    _fok = readFactors(QString::fromStdString(satconf->coef()));
  }
  
  _count = 0;
  uint length = (data.size() / 80.) + 1; //пакет из 80 кадров. в каждой data[idx] по строке
  var(data.size());
  var(data.at(0).size());
  var(length);
  // var(_ch.size());

  for (int idx = 0; idx < _ch.size(); idx++) {
    _ch[idx].setup(length, _conf.samples(), idx + 3);
    _ch[idx].setLimits(0, 0x7FFF);
  }
  
  QVector<ushort> housekeeping;
  QMap<int, QVector<float>> cold; //канал, значения
  QMap<int, QVector<float>> rf;   //номер rf
  QVector<float> prt1; //все prt по всем строкам
  QVector<float> prt2; //все prt по всем строкам
  QMap<int, QVector<float>> warm; //канал, значения
  
  int line = 0;
  for (int idx = 0, numMinor = 0; idx < data.size(); idx++, numMinor++) {
    if (numMinor == 80) { numMinor = 0; _count = 0; line++; }
    addAmsuA1Data(data.at(idx), line, &housekeeping, &cold, &rf, &prt1, &prt2, &warm);
  }

  createCalibrCoef(housekeeping, cold, rf, prt1, prt2, warm);
  
  return true;
}


/*! 
 * \brief Формирование данных сканирования и калибровки AMSU-A1
 * \param amsuA1  Буфер с данными AMSU-A1, прочитанными из AIP
 */
void AmsuA1::addAmsuA1Data(const QByteArray& amsuA1, int line, QVector<uint16_t> *housekeeping, QMap<int, QVector<float>>* cold,
													 QMap<int, QVector<float>>* rf, QVector<float>* prt1, QVector<float>* prt2,
													 QMap<int, QVector<float>>* warm)
{
	const int HOUSEKEEPING_POS = 4;//
	const int RFMux_POS = 1096;//
	const int RFShelf_POS = 1152;//
	const int COLD_POS_START = 1036;//
	const int COLD_POS_END = 1086;//
	const int PRT1_POS_START = 1158;
	const int PRT1_POS_END = 1166;
	const int PRT2_POS_START = 1168;
	const int PRT2_POS_END = 1176;

	const int WARM_POS_START = 1188;//
	const int WARM_POS_END = 1238;//


	housekeeping->append((uchar(amsuA1[HOUSEKEEPING_POS]) << 8) + uchar(amsuA1[HOUSEKEEPING_POS+1]));

	for(int i = COLD_POS_START; i<= COLD_POS_END; i+=2) { //cold calibration
		int num = ((i - COLD_POS_START) / 2) % 13;
		(*cold)[num].append((uchar(amsuA1[i]) << 7) + (uchar(amsuA1[i+1]) >> 1));
	}

	(*rf)[0].append((uchar(amsuA1[RFMux_POS]) << 7) + (uchar(amsuA1[RFMux_POS+1]) >> 1));
	(*rf)[1].append((uchar(amsuA1[RFMux_POS+2]) << 7) + (uchar(amsuA1[RFMux_POS+2+1]) >> 1));
	(*rf)[2].append((uchar(amsuA1[RFShelf_POS]) << 7) + (uchar(amsuA1[RFShelf_POS+1]) >> 1));
	(*rf)[3].append((uchar(amsuA1[RFShelf_POS+2]) << 7) + (uchar(amsuA1[RFShelf_POS+2+1]) >> 1));

	for(int i = PRT1_POS_START; i<= PRT1_POS_END; i+=2) { //PRT
		prt1->append((uchar(amsuA1[i]) << 7) + (uchar(amsuA1[i+1]) >> 1));
	}
	for(int i = PRT2_POS_START; i<= PRT2_POS_END; i+=2) { //PRT
		prt2->append((uchar(amsuA1[i]) << 7) + (uchar(amsuA1[i+1]) >> 1));
	}
	for(int i = WARM_POS_START; i<= WARM_POS_END; i+=2) { //warm calibration
		int num = ((i - WARM_POS_START)/2) % 13;
		(*warm)[num].append((uchar(amsuA1[i]) << 7) + (uchar(amsuA1[i+1]) >> 1));
	}


	for (int i = 0; i < 1020; i += 34)
	{
		for (int j = 0; j < 13; j++)
		{
			int pos = i + 16 + 2 * j;
			//image_bufferA1[j][30 * linesA1 + i / 34] = (frame[i + 16 + 2 * j] << 8) | frame[16 + i + 2 * j + 1];
			//_ch[j].set(30 * line + i / 34 , (uchar(amsuA1[pos]) << 8) + (uchar(amsuA1[pos+1])));
			_ch[j].set(30 * line + i / 34 , ((uchar(amsuA1[pos])) << 7) + ((uchar(amsuA1[pos+1]))>>1));
		}
	}
}



//void AmsuA1::addAmsuA1Data(const QByteArray& amsuA1, int line, QVector<ushort>* housekeeping, QMap<int, QVector<float>>* cold,
//													 QMap<int, QVector<float>>* rf, QVector<float>* prt1, QVector<float>* prt2,
//													 QMap<int, QVector<float>>* warm)
//{
//	int position = 0;

//	for (uint i = 0; i < 26; i += 2) {
//		if (uchar(amsuA1[i]) == 0 && uchar(amsuA1[i+1]) == 1) continue;
//		int numByte = _count;
//		_count += 2;

//		if (numByte == 4) { //housekeeping
//			//_calData[numByte - 4] =  (((unsigned short)amsuA1[i]<<8) + amsuA1[i+1]);
//			housekeeping->append((uchar(amsuA1[i]) << 8) + uchar(amsuA1[i+1]));
//			continue;
//		}
//		if (numByte < 16) continue;
//		if (numByte >= 1036 && numByte <= 1086) { //cold calibration
//			//_calData[(numByte - 1036)/2 + 1] = (((unsigned short)amsuA1[i]<<8) + amsuA1[i+1])>>1;
//			int num = ((numByte - 1036) / 2) % 13;
//			(*cold)[num].append((uchar(amsuA1[i]) << 7) + (uchar(amsuA1[i+1]) >> 1));
//			continue;
//		}
//		if (numByte == 1096) { //RF Mux
//			//_calData[(numByte - 1096)/2 + 26 + 1] = (((unsigned short)amsuA1[i]<<8) + amsuA1[i+1])>>1;
//			(*rf)[0].append((uchar(amsuA1[i]) << 7) + (uchar(amsuA1[i+1]) >> 1));
//			continue;
//		}
//		if (numByte == 1098) { //RF Mux
//			//_calData[(numByte - 1096)/2 + 26 + 1] = (((unsigned short)amsuA1[i]<<8) + amsuA1[i+1])>>1;
//			(*rf)[1].append((uchar(amsuA1[i]) << 7) + (uchar(amsuA1[i+1]) >> 1));
//			continue;
//		}
//		if (numByte == 1152) {//RF Shelf
//			//_calData[(numByte - 1152)/2 + 26 + 3] = (((unsigned short)amsuA1[i]<<8) + amsuA1[i+1])>>1;
//			(*rf)[2].append((uchar(amsuA1[i]) << 7) + (uchar(amsuA1[i+1]) >> 1));
//			continue;
//		}
//		if (numByte == 1154) {//RF Shelf
//			//_calData[(numByte - 1152)/2 + 26 + 3] = (((unsigned short)amsuA1[i]<<8) + amsuA1[i+1])>>1;
//			(*rf)[3].append((uchar(amsuA1[i]) << 7) + (uchar(amsuA1[i+1]) >> 1));
//			continue;
//		}
//		if (numByte >= 1158 && numByte <= 1166) { //PRT
//			//_calData[(numByte - 1158)/2 + 26 + 5] = (((unsigned short)amsuA1[i]<<8) + amsuA1[i+1])>>1;
//			prt1->append((uchar(amsuA1[i]) << 7) + (uchar(amsuA1[i+1]) >> 1));
//			continue;
//		}
//		if (numByte >= 1168 && numByte <= 1176) { //PRT
//			//_calData[(numByte - 1158)/2 + 26 + 5] = (((unsigned short)amsuA1[i]<<8) + amsuA1[i+1])>>1;
//			prt2->append((uchar(amsuA1[i]) << 7) + (uchar(amsuA1[i+1]) >> 1));
//			continue;
//		}
//		if (numByte >= 1188 && numByte <= 1238) { //warm calibration
//			//_calData[(numByte - 1188)/2 + 26 + 10 + 5] = (((unsigned short)amsuA1[i]<<8) + amsuA1[i+1])>>1;
//			int num = ((numByte - 1188)/2) % 13;
//			(*warm)[num].append((uchar(amsuA1[i]) << 7) + (uchar(amsuA1[i+1]) >> 1));
//			continue;
//		}


//		int channel  = ((numByte - 16) % 34)/2;
//		if (channel >=0 && channel < _conf.chan_cnt()) {
//			position = (numByte - 16) / 34;
//			if (position > 29) continue;
//			_ch[channel].set(position + line*30, (uchar(amsuA1[i]) << 7) + (uchar(amsuA1[i+1]) >> 1)); //TODO в старом сдвиг на 8
//		}
//	}
//}

//! Проверка размерности всех используемых коэффициентов
bool AmsuA1::readFactors(const QString& coefName)
{
  bool ok = true;

  if (true != _f.readFile(coefName, QString::fromStdString(_conf.name()))) {
    error_log << QObject::tr("Ошибка открытия файла %1").arg(coefName);
    return false;
  }

  ok &= _f.checkPrt(AMSUA1_PRT_CNT+4, 4);
  ok &= _f.checkT2r(_conf.chan_cnt(), 3);
  ok &= _f.checkNonLin(_conf.chan_cnt() + AMSUA1_PLLO2OFF, 3);
  ok &= _f.checkWcr(_conf.chan_cnt() + AMSUA1_PLLO2OFF, 3);
  ok &= _f.checkCcr(_conf.chan_cnt() + AMSUA1_PLLO2OFF, 4);
  ok &= _f.checkLim(4, _conf.chan_cnt() + AMSUA1_PLLO2OFF);
  ok &= _f.checkId(2);
  ok &= _f.checkShelf(3, 3);
  ok &= _f.checkMux(3, 3);

  return ok;
}

void AmsuA1::createCalibrCoef(const QVector<uint16_t>& house, const QMap<int, QVector<float>>& cold,
															const QMap<int, QVector<float>>& rf, const QVector<float>& prt1,
															const QVector<float>& prt2, const QMap<int, QVector<float>>& warm)
{
  if (!_fok) return;
  
  QMap<int, QVector<float>> meanCold; // число каналов, число строк
  QVector<float> meanPrt1; //число строк
  QVector<float> meanPrt2; //число строк
  //  QVector<float> meanRf;   //число строк
  QMap<int, QVector<float>> meanWarm; //число каналов, число строк
  
  uint16_t housekeeping = Hrpt::checkHousekeeping(house);
  Hrpt::meanData(cold, 2, _f.lim()[2].data(), _f.lim()[3].data(), &meanCold);
  //createRf(rf, 4, &meanRf);
  Hrpt::createPrt(prt1, 0, AMSUA11_PRT_CNT, _f, &meanPrt1);
  Hrpt::createPrt(prt2, AMSUA11_PRT_CNT, AMSUA12_PRT_CNT, _f, &meanPrt2);
  Hrpt::meanData(warm, 2, _f.lim()[0].data(), _f.lim()[1].data(), &meanWarm);
  
  for (int channel = 0; channel < _conf.chan_cnt(); channel++) {
    int idx =  Amsu::amsuInstr[channel];
    if ( housekeeping && 0x8 == 1 && idx == 1) {
      idx = 0;
    };
    const float* rfConst = 0;
    QVector<float> rfTemp;
    uint chOff = 0;
    if (!determRf(channel, idx, rf, &rfConst, &rfTemp)) {
      continue;
    }
    
    int ch = channel;
    if (housekeeping && 0x8 == 0) {
      ch += Amsu::pllo2off[channel];
    }
    float r21 = rfConst[1] - rfConst[0];
    float r32 = rfConst[2] - rfConst[1];
    float n21 = (_f.nonLin()[ch][1] - _f.nonLin()[ch][0]) / r21;
    float n32 = (_f.nonLin()[ch][2] - _f.nonLin()[ch][1]) / r32;
    float i21 = (_f.wcr()[ch][1] - _f.wcr()[ch][0]) / r21;
    float i32 = (_f.wcr()[ch][2] - _f.wcr()[ch][1]) / r32;
    float spRad = HrptCalibr::planckFunc(_f.t2r()[channel+chOff][0],
        Hrpt::coldTemp(_f.ccr()[ch][(housekeeping & 0x6000)>>13]));

    int numLines = meanCold.value(channel).size();

    for (int j = 0; j < numLines; j++) {
      float dt1 = rfTemp[j] - rfConst[0];
      float dt2 = rfTemp[j] - rfConst[1];
      
      float intc;
      float u;
      if (dt2 <0) {
        intc = _f.wcr()[ch][0] + dt1 * i21;
        u = _f.nonLin()[ch][0] + dt1 * n21;
      } else {
        intc = _f.wcr()[ch][1] + dt2 * i32;
        u = _f.nonLin()[ch][1] + dt2 * n32;
      }
      
      float bbRad;
      if (idx == 2) {
        //prt = meanPrt2;
        bbRad = HrptCalibr::planckFunc(_f.t2r()[channel+chOff][0],
            _f.t2r()[channel+chOff][1] + _f.t2r()[channel+chOff][2]*(meanPrt2[j]+intc));
        //debug_log << channel << j << spRad << bbRad << idx << meanPrt2[j] << intc;
      } else {
        //prt = meanPrt1;
        bbRad = HrptCalibr::planckFunc(_f.t2r()[channel+chOff][0],
            _f.t2r()[channel+chOff][1] + _f.t2r()[channel+chOff][2]*(meanPrt1[j]+intc));
        //debug_log << channel << j << spRad << bbRad << idx << meanPrt1[j] << intc;
      }

      QVector<float> a(3);
      Hrpt::aCoefs(bbRad, spRad, meanWarm[channel][j], meanCold[channel][j], u, a.data());
      _aCoef[channel].append(a);
      // debug_log << meanWarm[channel][j] << meanCold[channel][j] << u;
      // debug_log << channel << j << a;
    }
  }
  
}

/*! 
 * \brief Определение reference temperature в зависимости от прибора
 * \param instr Прибор
 * \param ch    Канал
 * \param idx   Индекс прибора
 * \param rf    Температуры из потока данных для всех приборов
 * \param rfConst Требуемая температура из файла-коэффициентов
 * \param rfTemp  Требуемая температура из потока данных
 * \param chOff   Смещение индекса канала для получения коэффициентов
 */
bool AmsuA1::determRf(uint ch, uint idx, const QMap<int, QVector<float>>& rf, 
                      const float** rfConst,  QVector<float>* rfTemp)
{
  int key = 0;
  if (Amsu::amsuInstr[ch] == 2) { //A1-2
    if (_f.id()[1] == 0) {
      *rfConst = _f.shelf()[idx].data();
      key = 3;
      //*rfTemp = rf[3];
    } else {
      *rfConst = _f.mux()[idx].data();
      key = 1;
      //*rfTemp = rf[1];
    }
  } else {                       //A1-1
    if (_f.id()[0] == 0) {
      *rfConst = _f.shelf()[idx].data();
      key = 2;
      //*rfTemp = rf[2];
    } else {
      *rfConst = _f.mux()[idx].data();
      key = 0;
      //*rfTemp = rf[0];
    }
  }

  Hrpt::createRf(rf, key, AMSUA1_PRT_CNT, _f, rfTemp);

  return true;
}


bool AmsuA1::calibrate()
{
  bool ok = true;

  // return ok;
  if (!_fok) return false;
  if (_aCoef.isEmpty()) {
    error_log << QObject::tr("Коэффициенты для калибровки каналов не рассчитаны");
  }

  for (int numCh = 0; numCh < _conf.chan_cnt(); numCh++) {
    if (!_aCoef.contains(numCh)) {
      continue;
    }

    ok &= calibrate(&_ch[numCh], _aCoef[numCh], _f.t2r()[numCh].data());
  }
  
  return ok;
}

bool AmsuA1::calibrate(Channel<short>* chd, const QVector<QVector<float>>& a, const float t2r[3])
{
  float resValue = 0;
  chd->setLimits(0, 300);

  float maxLimit = chd->maxLimit();
  float minLimit = chd->minLimit();
  uint columns = chd->columns();
  uint rows = chd->rows();
  QVector<short>& data = chd->data();

  int idx = 0;
  for (uint row = 0; row < rows; row++) {
    // var(row);
    if (row >= (uint)a.size()) { return false; }
    const QVector<float>& anum = a.at(row);
    for (uint col = 0; col < columns; col++) {
      if (data[idx] == short(0xffff)) {
        ++idx;
        continue;
      }
      
      float value = (uint16_t) data[idx];
      resValue = HrptCalibr::sceneRadiance(anum, value);
      resValue = HrptCalibr::earthTemperature(resValue, t2r);
      if (resValue > maxLimit) {
        if (resValue <= 1.1 * maxLimit) {
          resValue = maxLimit;
        } else {
          resValue = INVALID_CALIBR_VALUE;
        }
      } else if ( resValue < minLimit ) {
        if (resValue >= 1.1 * minLimit) {
          resValue = minLimit;
        } else {
          resValue = INVALID_CALIBR_VALUE;
        }
      }
      data[idx] = resValue;
      //debug_log << value << resValue;
      idx++;
    }
  }
  
  return true;
}
