#include "mhs.h"
#include "hrpt_calibr.h"

#include <cross-commons/debug/tlog.h>

#include <mappi/pretreatment/formats/channel.h>
#include <mappi/pretreatment/formats/frame.h>
#include <mappi/pretreatment/formats/satformat.h>
#include <mappi/pretreatment/formats/hrpt_frame.h>

#include <qfile.h>

#define MHS_PRT_CNT        5  //!<<Количество PRT MHS

namespace {
mappi::po::Instrument* createMhs(const mappi::conf::Instrument& conf, const mappi::conf::InstrFormat& format, mappi::po::Frame* frame)
{
  return new mappi::po::Mhs(conf, format, frame);
}

static const bool res = mappi::po::singleton::SatFormat::instance()->
                        registerInstrHandler(mappi::InstrFrameType(mappi::conf::kMhs, mappi::conf::kHrpt), createMhs);
}

using namespace meteo;
using namespace mappi;
using namespace po;


Mhs::Mhs(const mappi::conf::Instrument& conf, const conf::InstrFormat& format, Frame* parent):
  InstrumentTempl(conf, format),
  _conf(conf),
  _frame(parent)
{
  _ch.resize(_conf.chan_cnt());
  
}

Mhs::~Mhs()
{
  clearChannels();
}

bool Mhs::parse(const QList<QByteArray>& data, QDateTime* dt, QDateTime*)
{
  Q_UNUSED(dt)
  
  if (0 == _frame) {
    return false;
  }

  // QFile file("/tmp/mhs.dat");
  // file.open(QIODevice::WriteOnly);
  // for (int idx = 0; idx < data.size(); idx++) {
  //   file.write(data.at(idx));
  // }
  // file.close();

  const mappi::conf::PretrSat* satconf = singleton::SatFormat::instance()->pretreatment(_frame->satName(), _frame->mode());
  debug_log << _frame->satName() << _frame->mode() << satconf;
  if (nullptr != satconf) {
    _fok = readFactors(QString::fromStdString(satconf->coef()));
  }

  _numLines = 0;
  _count = 0;
  uint length = (data.size() / 80.) * 3.; //пакет из 80 кадров. в каждой data[idx] по три строки
  // var(data.size());
  // var(length);
  // var(_ch.size());

  for (int idx = 0; idx < _ch.size(); idx++) {
    _ch[idx].setup(length, _conf.samples(), idx + 1);
    _ch[idx].setLimits(0, 0xFFFF);
  }
  // var(length);
  // var(_conf.samples());

  QVector<ushort> housekeeping; //Status Word
  QMap<int, QVector<float>> rf; //номер rf
  QMap<int, QVector<float>> cold; //канал, значения
  QMap<int, QVector<float>> warm; //канал, значения
  QVector<float> prt;  //канал, значения
  QVector<float> resist;  //канал, значения
  
  //данные начинаются с 0-го кадра AIP
  //idx с 2 т.к. номер кадра AIP на два опережает номер кадра MHS (находится в последнем байте data)
  for (int idx = 2, numMinor = 0; idx < data.size(); idx++, numMinor++) {
    if (numMinor == 80) { numMinor = 0; }
    const QByteArray& mhs = data.at(idx);
    if (0 == numMinor)       { addMhsData(mhs, 48, 49, 2, numMinor, &housekeeping, &rf, &cold, &warm, &prt, &resist); }
    else if (numMinor <  26) { addMhsData(mhs,  0, 49, 2, numMinor, &housekeeping, &rf, &cold, &warm, &prt, &resist); }
    else if (numMinor == 26) { addMhsData(mhs,	0, 33, 2, numMinor, &housekeeping, &rf, &cold, &warm, &prt, &resist); }
    
    else if (numMinor == 27) { addMhsData(mhs, 32, 49, 0, numMinor, &housekeeping, &rf, &cold, &warm, &prt, &resist); }
    else if (numMinor <= 52) { addMhsData(mhs,	0, 49, 0, numMinor, &housekeeping, &rf, &cold, &warm, &prt, &resist); }
    else if (numMinor == 53) { addMhsData(mhs,	0, 17, 0, numMinor, &housekeeping, &rf, &cold, &warm, &prt, &resist); }
    
    else if (numMinor == 54) { addMhsData(mhs, 14, 49, 1, numMinor, &housekeeping, &rf, &cold, &warm, &prt, &resist); }
    else		     { addMhsData(mhs,	0, 49, 1, numMinor, &housekeeping, &rf, &cold, &warm, &prt, &resist); }
  }

  createCalibrCoef(housekeeping, cold, rf, prt, warm, resist);
  
  return true;
}

/*! 
 * \brief Выделение данных MHS
 * \param mhs Данные minor кадра
 * \param start Начальный байт в данных mhs, соответствующий данным сканирования
 * \param end Последний байт в дынных mhs, соответствующий данным сканирования
 * \param numMinor Номер minor кадра
 */
void Mhs::addMhsData(const QByteArray& mhs, uint start, uint end,
										 uint /*num*/, uint numMinor, QVector<ushort>* housekeeping, QMap<int, QVector<float>>* rf,
										 QMap<int, QVector<float>>* cold, QMap<int, QVector<float>>* warm,QVector<float>* prt,
										 QVector<float>* resist)
{
  if (numMinor == 0 || numMinor == 27 || numMinor == 54) {
    _count = 0;
  }

  //var(numMinor);
  if (_numLines > _ch.at(0).rows()) {
    //м.б. при не полной строке
    debug_log << QObject::trUtf8("Количество строк превышает выделенный размер данных (%1, %2)").arg(_numLines).arg(_ch.at(0).rows());
  }

  for (uint idx = start; idx <= end; idx++) {

    int s = 49;//количество байт в Science Data Packet Fields до Pixel Data
    if (_count == 9) {   //Mux/Shelf
      //_mhsCal[0] = mhs[idx];
      (*rf)[0].append(mhs[idx]);
    } else if (_count == 12) { //Mux/Shelf
      //_mhsCal[1] = mhs[idx];
      (*rf)[1].append(mhs[idx]);
    } else if (_count == 39) { //Status Word
      //_mhsCal[2] = mhs[idx];
      housekeeping->append(mhs[idx]);
    } else if (_count >=s && _count <= (s+1175)) {
      int pos =  (_count - s)/12;
      int channel = (_count - s)%12 - 2;
      if (pos < _conf.samples()) {
        if (channel >= 0) { //записываем только данные каналов, пропуская Mid-pixel Position
          // if (_numLines == 170 || _numLines == 169) {
          //   debug_log << _count << pos << _numLines << _conf.samples()  << pos + _numLines*_conf.samples() <<
          //     89 - pos + _numLines*_conf.samples()
          // 	      << "ch" << channel / 2 << uchar(mhs[idx]);
          // }
          if (channel%2 == 0) {
            _ch[channel/2].set(89 - pos + _numLines*_conf.samples(), uchar(mhs[idx]) << 8);
          } else {
            _ch[channel/2].update(89 - pos + _numLines*_conf.samples(),
                                  _ch[channel/2].get(89 - pos + _numLines*_conf.samples()) + uchar(mhs[idx]));
          }
        }
      } else {
        if (channel >= 0) {
          uint num = ((_count - s - 1080) / 12 )*10 + channel; //записываем только данные каналов, пропуская Mid-pixel Position
          if (num < 40) {
            if (num%2 == 0) (*cold)[channel/2].append(uchar(mhs[idx]) << 8);
            else            (*cold)[channel/2].last() += uchar(mhs[idx]);
          } else {
            num -= 40;
            if (num%2 == 0) (*warm)[channel/2].append(uchar(mhs[idx]) << 8);
            else            (*warm)[channel/2].last() += uchar(mhs[idx]);
          }

        }
      }
    } else if ( _count >= (s+1176) && _count < (s+1176+10)) {
      //OBCT Subfield Format
      int num = _count - s - 1176;
      if (num%2 == 0) (*prt).append((uchar(mhs[idx])<<4)&0xf0);
      else            (*prt).last() += uchar(mhs[idx]);
    } else if ( _count >= (s+1176 + 10) && _count < (s+1176+16)) {
      //3 PRT CAL (по 2 байта)
      int num = _count - s - 1186;
      if (num%2 == 0) (*resist).append((uchar(mhs[idx])<<4)&0xf0);
      else            (*resist).last() += uchar(mhs[idx]);
      
    }
    //var(_count);
    _count++;
    if (_count >= 1286) {
      _count = 0;
      ++_numLines;


    }
  }

  //var(_numLines);
}


//! Проверка размерности всех используемых коэффициентов
bool Mhs::readFactors(const QString& coefName)
{
  bool ok = true;
  if (true != _f.readFile(coefName, QString::fromStdString(_conf.name()))) {
    error_log << QObject::tr("Ошибка открытия файла %1").arg(coefName);
    return false;
  }
  
  ok &= _f.checkPrt(MHS_PRT_CNT*2 +2, 5);
  ok &= _f.checkT2r(_conf.chan_cnt(), 3);
  ok &= _f.checkRCal(2, 3);
  ok &= _f.checkNonLin(6, _conf.chan_cnt());
  
  ok &= _f.checkWcr(_conf.chan_cnt(), 3);
  ok &= _f.checkCcr(_conf.chan_cnt(), 4);
  ok &= _f.checkLim(4, _conf.chan_cnt());
  
  ok &= _f.checkId(1);
  ok &= _f.checkShelf(1, 3);
  ok &= _f.checkMux(1, 3);

  return ok;
}

void Mhs::createCalibrCoef(const QVector<ushort>& house, const QMap<int, QVector<float>>& cold,
													 const QMap<int, QVector<float>>& rf,  QVector<float>& prt,
													 const QMap<int, QVector<float>>& warm, QVector<float>& resist)
{
  if (!_fok) return;
  
  QMap<int, QVector<float>> meanCold; // число каналов, число строк
  QVector<float> meanRf; //число строк
  QMap<int, QVector<float>> meanWarm; //число каналов, число строк
  
  int key = 0;
  if (_f.id()[0] == 0) {
    key = 1;
  }
  
  ushort stat = Hrpt::checkHousekeeping(house);
  Hrpt::createRf(rf, key, MHS_PRT_CNT, _f, &meanRf);
  Hrpt::meanData(cold, 4, _f.lim()[2].data(), _f.lim()[3].data(), &meanCold);
  Hrpt::meanData(warm, 4, _f.lim()[0].data(), _f.lim()[1].data(), &meanWarm);

  Hrpt::UniFactor rfConst;
  if (key == 0) {
    rfConst = _f.shelf()[0];
  } else  {
    rfConst = _f.mux()[0];
  }

  
  uint linesCnt = resist.size() / 3; //по три на каждую строку каждого канала
  var(linesCnt);
  
  for (uint line = 0; line < linesCnt; line++) {
    float inst_temp = meanRf[line];
    
    float f21 = (inst_temp - rfConst[0])/(rfConst[1]-rfConst[0]);
    float f32 = (inst_temp - rfConst[1])/(rfConst[2]-rfConst[1]);
    float g21 = 1.0 - f21;
    float g32 = 1.0 - f32;
    float u=0, intc =0;
    
    for (int ch = 0; ch < _conf.chan_cnt(); ch++) {

      if ( f32 < 0) {
        intc = g21*_f.wcr()[ch][0] + f21*_f.wcr()[ch][1];
        u = g21 * _f.nonLin()[0+key*3][ch] + f21 * _f.nonLin()[1+key*3][ch];
      } else {
        intc = g32*_f.wcr()[ch][1] + f32*_f.wcr()[ch][2];
        u = g32 * _f.nonLin()[1+key*3][ch] + f32 * _f.nonLin()[2+key*3][ch];
      }
      
      float Tw = calcMhsTw(prt.mid(line*MHS_PRT_CNT, MHS_PRT_CNT), resist.mid(line*3, 3)) + intc;
      float Rc = HrptCalibr::planckFunc(_f.t2r()[ch][0], Hrpt::coldTemp(_f.ccr()[ch][(stat>>4)&0x3]));
      float Rw = HrptCalibr::planckFunc(_f.t2r()[ch][0], _f.t2r()[ch][1] + _f.t2r()[ch][2]*Tw);

      QVector<float> a(3);
      Hrpt::aCoefs(Rw, Rc, meanWarm[ch][line], meanCold[ch][line], u, a.data());
      
      _aCoef[ch].append(a);
    }
    
  }
  
}



/*! 
 * \brief Вычисление OBCT temperature (Tw)
 * \param prt OBCT PRT count, num = MHS_PRT_CNT
 * \param Ccal count of the PRT CALn, num = 3
 * \param ch Канал
 */
float Mhs::calcMhsTw(const QVector<float>& prt, const QVector<float>& Ccal)
{
  float sumRcal = 0, sumCRcal = 0;
  float sumCcal = 0, sumCcalx2 = 0;
  for (uint i=0; i < 3 ; i++) {
    sumRcal += _f.rCal()[(uint)_f.id()[0]][i];
    sumCcal += Ccal[i];
    sumCcalx2 +=  Ccal[i]* Ccal[i];
    sumCRcal  +=  _f.rCal()[(uint)_f.id()[0]][i]* Ccal[i];
  }
  float alpha = (sumRcal * sumCcalx2 - sumCcal * sumCRcal) /
                (3 * sumCcalx2 - sumCcal*sumCcal);
  float betta = (3 * sumCRcal -  sumRcal * sumCcal) /
                (3 * sumCcalx2 - sumCcal*sumCcal);

  float Tk[MHS_PRT_CNT] = {0};
  uint offset = (uint)_f.id()[0]*MHS_PRT_CNT;
  for (uint nPrt = 0; nPrt < MHS_PRT_CNT; nPrt++) {
    Tk[nPrt] += HrptCalibr::prtTemp(_f.prt()[nPrt+offset].data(), alpha + betta * prt[nPrt], 4);
  }

  float w[MHS_PRT_CNT] = {1,1,2,1,1};//TODO где-то может быть 0

  return HrptCalibr::meanBbTemp(w, Tk, MHS_PRT_CNT, 0);
}

bool Mhs::calibrate()
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

bool Mhs::calibrate(Channel<ushort>* chd, const QVector<QVector<float>>& a, const float t2r[3])
{
  float resValue = 0;
//  chd->setLimits(0, 500);

  float maxLimit = chd->maxLimit();
  float minLimit = chd->minLimit();
  uint columns = chd->columns();
  uint rows = chd->rows();
  QVector<ushort>& data = chd->data();

  int idx = 0;
  for (uint row = 0; row < rows; row++) {
    // var(row);
    if (row >= (uint)a.size()) { return false; }
    const QVector<float>& anum = a.at(row);
    for (uint col = 0; col < columns; col++) {
      if (data[idx] == ushort(0xffff)) {
        ++idx;
        continue;
      }
      
      float value = data[idx];
      resValue = HrptCalibr::sceneRadiance(anum, value);
      resValue = HrptCalibr::earthTemperature(resValue, t2r);
      
      //debug_log << value << resValue << maxLimit << minLimit;
      
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
      idx++;
    }
  }
  
  return true;
}


