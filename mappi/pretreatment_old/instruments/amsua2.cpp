#include "amsua2.h"
#include "hrpt_const.h"
#include "hrpt_calibr.h"

#include <cross-commons/debug/tlog.h>

#include <mappi/pretreatment/formats/channel.h>
#include <mappi/pretreatment/formats/frame.h>
#include <mappi/pretreatment/formats/satformat.h>
#include <mappi/pretreatment/formats/hrpt_frame.h>
#include <mappi/pretreatment/frames/simpledeframer.h>

#define AMSUA2_PRT_CNT   7 //!<Количество PRT AMSU-A2

namespace {
mappi::po::Instrument* createAmsuA2(const mappi::conf::Instrument& conf, const mappi::conf::InstrFormat& format, mappi::po::Frame* frame)
{
  return new mappi::po::AmsuA2(conf, format, frame);
}

static const bool res = mappi::po::singleton::SatFormat::instance()->
                        registerInstrHandler(mappi::InstrFrameType(mappi::conf::kAmsuA2, mappi::conf::kHrpt), createAmsuA2);
}

using namespace meteo;
using namespace mappi;
using namespace po;


AmsuA2::AmsuA2(const mappi::conf::Instrument& conf, const conf::InstrFormat& format, Frame* parent):
  InstrumentTempl(conf, format),
  _conf(conf),
  _frame(parent)
{
  _ch.resize(_conf.chan_cnt());
}

AmsuA2::~AmsuA2()
{
}

bool AmsuA2::parse(const QByteArray& data, QDateTime* dt, QDateTime* )
{
  Q_UNUSED(dt)

  if (0 == _frame) {
    return false;
  }

  const mappi::conf::PretrSat* satconf = singleton::SatFormat::instance()->pretreatment(_frame->satName(), _frame->mode());
  //debug_log << _frame->satName() << _frame->mode() << satconf;
  if (0 != satconf) {
    _fok = readFactors(QString::fromStdString(satconf->coef()));
  }

  _count = 0;
  SimpleDeframer<uint32_t, 24, 312 * 8, 0xFFFFFF> amsuA2Deframer;
  const QVector<QByteArray> &amsuA2Data = amsuA2Deframer.work(data);

  uint length = amsuA2Data.size();

  for (int idx = 0; idx < _ch.size(); idx++) {
    _ch[idx].setup(length, _conf.samples(), idx + 1);
    _ch[idx].setLimits(0, 0x7FFF);
  }

  QVector<ushort> housekeeping;
  QMap<int, QVector<float>> cold; //канал, значения
  QMap<int, QVector<float>> rf; //номер rf
  QVector<float> prt;//все prt по всем строкам
  QMap<int, QVector<float>> warm; //канал, значения


  //  QFile fl("/home/gotur/tmp/outNashALL.dat");
  //  fl.open(QIODevice::ReadWrite|QIODevice::Truncate);
  //  fl.write(data);
  //  fl.close();
  //  fl.setFileName("/home/gotur/tmp/outNash.dat");
  //  fl.open(QIODevice::ReadWrite|QIODevice::Truncate);
  //  for (uint idx = 0; idx < length; idx++) {
  //    fl.write(amsuA1Data.at(idx));
  // }
  //  fl.close();

  int line = 0;

  for (uint idx = 0; idx < length; idx++) {
    //if (numMinor == 80) { numMinor = 0; _count = 0; line++; }
    addAmsuA2Data(amsuA2Data.at(idx), line, &housekeeping, &cold, &rf,  &prt, &warm);
    line++;
  }

  createCalibrCoef(housekeeping, cold, rf, prt, warm);
  return true;
}




bool AmsuA2::parse(const QList<QByteArray>& data, QDateTime* dt, QDateTime*)
{
  Q_UNUSED(dt)

  if (0 == _frame) {
    return false;
  }

  const mappi::conf::PretrSat* satconf = singleton::SatFormat::instance()->pretreatment(_frame->satName(), _frame->mode());
  //debug_log << _frame->satName() << _frame->mode() << satconf;
  if (0 != satconf) {
    _fok = readFactors(QString::fromStdString(satconf->coef()));
  }
  
  _count = 0;
  uint length = (data.size() / 80.); //пакет из 80 кадров. в каждой data[idx] по строке

  for (int idx = 0; idx < _ch.size(); idx++) {
    _ch[idx].setup(length, _conf.samples(), idx + 1);
    _ch[idx].setLimits(0, 0x7FFF);
  }

  QVector<ushort> housekeeping;
  QMap<int, QVector<float>> cold; //канал, значения
  QMap<int, QVector<float>> rf; //номер rf
  QVector<float> prt;//все prt по всем строкам
  QMap<int, QVector<float>> warm; //канал, значения

  int line = 0;
  for (int idx = 0, numMinor = 0; idx < data.size(); idx++, numMinor++) {
    if (numMinor == 80) {
      numMinor = 0;
      _count = 0;
      ++line;
    }
    
    addAmsuA2Data(data.at(idx), line, &housekeeping, &cold, &rf, &prt, &warm);
  }

  createCalibrCoef(housekeeping, cold, rf, prt, warm);
  
  return true;
}

/*! 
 * \brief Формирование данных сканирования и калибровки AMSU-A2
 * \param amsuA2  Буфер с данными AMSU-A2, прочитанными из AIP
 */
void AmsuA2::addAmsuA2Data(const QByteArray& amsuA2, int line, QVector<ushort>* housekeeping, QMap<int, QVector<float>>* cold,
                           QMap<int, QVector<float>>* rf, QVector<float>* prt, QMap<int, QVector<float>>* warm)
{

  const int HOUSEKEEPING_POS = 4;//
  const int COLD_POS_START = 252;//
  const int COLD_POS_END = 258;//
  const int RFMux_POS = 264;
  const int RFShelf_POS = 280;
  const int PRT_POS_START = 284;
  const int PRT_POS_END = 296;


  const int WARM_POS_START = 304;
  const int WARM_POS_END = 310;

  housekeeping->append((uchar(amsuA2[HOUSEKEEPING_POS]) << 8) + uchar(amsuA2[HOUSEKEEPING_POS+1]));

  for(int i = COLD_POS_START; i<= COLD_POS_END; i+=2) { //cold calibration
    int num = ((i - COLD_POS_START) / 2) % 2;
    (*cold)[num].append((uchar(amsuA2[i]) << 7) + (uchar(amsuA2[i+1]) >> 1));
  }
  (*rf)[0].append((uchar(amsuA2[RFMux_POS]) << 7) + (uchar(amsuA2[RFMux_POS+1]) >> 1));//RF Mux
  (*rf)[1].append((uchar(amsuA2[RFShelf_POS]) << 7) + (uchar(amsuA2[RFShelf_POS+1]) >> 1));//RF Shelf

  for(int i = PRT_POS_START; i<= PRT_POS_END; i+=2) { //PRT
    prt->append((uchar(amsuA2[i]) << 7) + (uchar(amsuA2[i+1]) >> 1));
  }
  for(int i = WARM_POS_START; i<= WARM_POS_END; i+=2) { //warm calibration
    int num = ((i - WARM_POS_START)/2) % 2;
    (*warm)[num].append((uchar(amsuA2[i]) << 7) + (uchar(amsuA2[i+1]) >> 1));
  }

  for (int i = 0; i < 240; i += 8)
  {
    int pos = i + 12;
//    _ch[0].set(30 * line + i / 8 , (uchar(amsuA2[pos]) << 8) + (uchar(amsuA2[pos+1])>>1));
  //  _ch[1].set(30 * line + i / 8 , (uchar(amsuA2[pos+2]) << 8) + (uchar(amsuA2[pos+1+2])>>1));
    _ch[0].set(30 * line + i / 8 , (uchar(amsuA2[pos]) << 7) + (uchar(amsuA2[pos+1])>>1));
    _ch[1].set(30 * line + i / 8 , (uchar(amsuA2[pos+2]) << 7) + (uchar(amsuA2[pos+1+2])>>1));

  }
}



//! Проверка размерности всех используемых коэффициентов
bool AmsuA2::readFactors(const QString& coefName)
{
  bool ok = true;

  if (true != _f.readFile(coefName, QString::fromStdString(_conf.name()))) {
    error_log << QObject::tr("Ошибка открытия файла %1").arg(coefName);
    return false;
  }

  ok &= _f.checkPrt(AMSUA2_PRT_CNT + 2, 4);
  ok &= _f.checkT2r(_conf.chan_cnt(), 3);
  ok &= _f.checkNonLin(_conf.chan_cnt(), 3);
  ok &= _f.checkWcr(_conf.chan_cnt(), 3);
  ok &= _f.checkCcr(_conf.chan_cnt(), 4);
  ok &= _f.checkLim(4, _conf.chan_cnt());
  ok &= _f.checkId(1);
  ok &= _f.checkShelf(1, 3);
  ok &= _f.checkMux(1, 3);

  return ok;
}

//! 
void AmsuA2::createCalibrCoef(const QVector<ushort>& house, const QMap<int, QVector<float>>& cold,
															const QMap<int, QVector<float>>& rf, const QVector<float>& prt,
															const QMap<int, QVector<float>>& warm)
{
	if (!_fok) return;

	QMap<int, QVector<float>> meanCold; // число каналов, число строк
	QVector<float> meanPrt;  //число строк
	QVector<float> meanRf; //число строк
	QMap<int, QVector<float>> meanWarm; //число каналов, число строк

	int key = 0;
	if (_f.id()[0] == 0) {
		key = 1;
	}

	ushort housekeeping = Hrpt::checkHousekeeping(house);
	Hrpt::meanData(cold, 2, _f.lim()[2].data(), _f.lim()[3].data(), &meanCold);
	Hrpt::createRf(rf, key, AMSUA2_PRT_CNT, _f, &meanRf);
	Hrpt::createPrt(prt, 0, AMSUA2_PRT_CNT, _f, &meanPrt);
	Hrpt::meanData(warm, 2, _f.lim()[0].data(), _f.lim()[1].data(), &meanWarm);

	if (_f.id()[0] == 0) {
		writeCalibrCoefs(housekeeping, meanCold, meanWarm, meanPrt, _f.shelf()[0].data(), meanRf);
	} else {
		writeCalibrCoefs(housekeeping, meanCold, meanWarm, meanPrt, _f.mux()[0].data(),  meanRf);
	}
}

void AmsuA2::writeCalibrCoefs(ushort housekeeping, const QMap<int, QVector<float>>& meanCold,
															const QMap<int, QVector<float>>& meanWarm,
															const QVector<float>& meanPrt, const float* rfConst,  const QVector<float>& rfTemp)
{
  QVector<float> a(3);
  float r21 = rfConst[1] - rfConst[0];
  float r32 = rfConst[2] - rfConst[1];

  for (int ch = 0; ch < _conf.chan_cnt(); ch++) {
    float n21 = (_f.nonLin()[ch][1] - _f.nonLin()[ch][0]) / r21;
    float n32 = (_f.nonLin()[ch][2] - _f.nonLin()[ch][1]) / r32;
    float i21 = (_f.wcr()[ch][1] - _f.wcr()[ch][0]) / r21;
    float i32 = (_f.wcr()[ch][2] - _f.wcr()[ch][1]) / r32;
    
    float spRad = HrptCalibr::planckFunc(_f.t2r()[ch][0],
        Hrpt::coldTemp(_f.ccr()[ch][(housekeeping & 0x6000)>>13]));
    
    int numLines = meanPrt.size();
    //debug_log << "num" << ch << meanPrt.size() << meanCold[ch].size() << meanWarm[ch].size();
    for (int j = 0; j < numLines; j++) {
      float dt1 = rfTemp[j] - rfConst[0];
      float dt2 = rfTemp[j] - rfConst[1];

      float intc;
      float u;
      if (dt2 < 0) {
        intc = _f.wcr()[ch][0] + dt1 * i21;
        u = _f.nonLin()[ch][0] + dt1 * n21;
      } else {
        intc = _f.wcr()[ch][1] + dt2 * i32;
        u = _f.nonLin()[ch][1] + dt2 * n32;
      }

      float bbRad = HrptCalibr::planckFunc(_f.t2r()[ch][0],
          _f.t2r()[ch][1] + _f.t2r()[ch][2]*(meanPrt[j]+intc));
      Hrpt::aCoefs(bbRad, spRad, meanWarm[ch][j], meanCold[ch][j], u, a.data());
      
      _aCoef[ch].append(a);
      //debug_log << "channel" << ch << j << a;
      // var(_aCoef);
    }
  }
}


bool AmsuA2::calibrate()
{
  bool ok = true;
  
  if (!_fok) return false;
  if (_aCoef.isEmpty()) {
    error_log << QObject::tr("Коэффициенты для калибровки каналов не рассчитаны");
  }

  for (int numCh = 0; numCh < _conf.chan_cnt(); numCh++) {
    if (!_aCoef.contains(numCh)) {
      continue;
    }

    ok &= calibrate(&_ch[numCh], _aCoef.value(numCh), _f.t2r()[numCh].data());
  }
  
  return ok;
}

bool AmsuA2::calibrate(Channel<short>* chd, const QVector<QVector<float>>& a, const float t2r[3])
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
      if (uint16_t(data[idx]) == uint16_t(0xffff)) {
        ++idx;
        continue;
      }
      
      float value = (uint16_t)data[idx];
      resValue = HrptCalibr::sceneRadiance(anum, value);
      resValue = HrptCalibr::earthTemperature(resValue, t2r);
      //debug_log << resValue << anum;
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
