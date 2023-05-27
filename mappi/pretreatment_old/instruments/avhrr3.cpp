#include "avhrr3.h"
//#include "geomcorr.h"
#include "hrpt_calibr.h"
#include "hrpt_const.h"

#include <sat-commons/satellite/satviewpoint.h>
#include <commons/mathtools/mnmath.h>

#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/paths.h>

#include <mappi/pretreatment/formats/frame.h>
#include <mappi/pretreatment/formats/satformat.h>
#include <mappi/pretreatment/formats/channel.h>
#include <mappi/pretreatment/frames/hrpt.h>

#include <qglobal.h>
#include <qvector.h>
#include <qfile.h>

#define NEED_VALID_READ_CNT 10  //!< Количество совпадений данных, по которому можно сделать вывод о их верности

//TODO это соответствует настройкам кадра
#define CALIBR_LENGTH 182
#define DATA_LENGTH 20480
#define AVHRR3_CHANNEL_COUNT 5

namespace {
mappi::po::Instrument* createAvhrr3(const mappi::conf::Instrument& conf, const mappi::conf::InstrFormat& format, mappi::po::Frame* frame)
{
  return new mappi::po::Avhrr3(conf, format, frame);
}

static const bool res = mappi::po::singleton::SatFormat::instance()->
                        registerInstrHandler(mappi::InstrFrameType(mappi::conf::kAvhrr3, mappi::conf::kHrpt), createAvhrr3);
}

using namespace mappi;
using namespace po;


Avhrr3::Avhrr3(const mappi::conf::Instrument& conf, const conf::InstrFormat& format, Frame* frame):
  InstrumentTempl(conf, format),
  _fok(false),
  _conf(conf),
  _frame(0)
{
  _ch.resize(_conf.chan_cnt());

  _frame = frame_cast<Hrpt*>(frame);

}

Avhrr3::~Avhrr3()
{
  clearChannels();
}

//! Проверка размерности всех используемых коэффициентов
bool Avhrr3::readFactors(const QString& coefName)
{
  bool ok = true;

  if (true != _f.readFile(coefName, QString::fromStdString(_conf.name()))) {
    //mn_syslog(LOG_ERROR,  TR_CANTREADFILE(coefName.latin1()));
    return false;
  }

  ok &= _f.checkWeight(1, AVHRR3_PRT_CNT);
  ok &= _f.checkPrt(AVHRR3_PRT_CNT, 5);
  ok &= _f.checkT2r(AVHRR3_IR_CNT,3);
  ok &= _f.checkNonLin(AVHRR3_IR_CNT,4);
  ok &= _f.checkThresh(AVHRR3_VIZ_CNT);
  ok &= _f.checkVizible(2*AVHRR3_VIZ_CNT, 2);
  // _f.checkA2r(); //не используется
  return ok;
}


//! парсинг данных AVHRR/3
bool Avhrr3::parse(const QByteArray& data, QDateTime* dt, QDateTime* )
{
  Q_UNUSED(dt);

  if ( 0 == _frame) {
    return false;
  }

  const mappi::conf::PretrSat* satconf = singleton::SatFormat::instance()->pretreatment(_frame->satName(), _frame->mode());
  if (nullptr != satconf) {
    _fok = readFactors(QString::fromStdString(satconf->coef()));
  }

  _chan3x = _frame->chan3Type();
  _lineChan3x = _frame->lineChan3Change();

  uint length = data.size() / ((_conf.chan_cnt()-1) * _conf.samples() * 2 + CALIBR_LENGTH); // "-1" - т.к. передаются данные только 5 каналов
  //var(_conf.chan_cnt());
  var(length);
  //length /= 3;
  
  uint idx = 0;
  for (int i = 0; i < _ch.size(); i++) {
    _ch[i].setup(length, _conf.samples(), idx + 1);
    _ch[i].setLimits(0, 0x3FF);
    idx++;
  }

  bool ok = readScanLines(data, length);
  if (_fok) {
    readCalibrData(data, length);
  }

  return ok;
}

//! Извлечение данных, разбор по каналам
bool Avhrr3::readScanLines(const QByteArray& data, int length)
{
  uint pos = CALIBR_LENGTH;
  int idx = 0;

  //debug_log << _lineChan3x << _chan3x;
  
  for (int row = 0; row < length; row++) {
    for (int col = 0; col < _conf.samples(); col++) {

      for (int ci = 0; ci < _ch.size(); ci++) {
        if (pos + 1 >= (uint)data.size()) {
          debug_log << "err pos=" << pos << data.size() << row << col << ci;
          return false;
        }

        if (ci == 2 || ci == 3) {
          if (_lineChan3x == -1 || row < _lineChan3x) { //канал не меняется или ещё первый
            if ((_chan3x == 1 && ci == 3) || //пропускаем 3 канал
                (_chan3x == 0 && ci == 2)) { //пропускаем 2 канал
              continue;
            }
          } else { //канал поменялся
            if ((_chan3x == 1 && ci == 2) || //пропускаем 2 канал
                (_chan3x == 0 && ci == 3)) { //пропускаем 3 канал
              continue;
            }
          }
        }

        //debug_log << "pos=" << pos << data.size() << row << col << ci << idx;
        _ch[ci].set(idx, (uchar(data[pos]) << 8) + (uchar)(data[pos + 1]));
        pos += 2;
      }

      ++idx;
    }
    
    pos += CALIBR_LENGTH;
  }

  return true;
}

//! Извлечение калибровочной информации
bool Avhrr3::readCalibrData(const QByteArray& ba, int length)
{
  uint pos = 0;

  int prtIdx = checkPrtIdx(ba);
  if (prtIdx == -1) return false;
  
  for (int row = 0; row < length; row++) {
    readCalibrLine(ba.mid(pos, CALIBR_LENGTH), &prtIdx, length, row);

    pos += DATA_LENGTH + CALIBR_LENGTH;
  }

  return true;
}

/*! 
* \brief Определение номера датчика PRT для первой строки (в каждом пятом HRPT все PRT значения д.б. равны 0)
* \param ba  Данные
* \return Номер датчика PRT для первой строки. В случае ошибки -1
*/
int Avhrr3::checkPrtIdx(const QByteArray& buf)
{
  int idx = -1;
  
  uint check = 0;
  uint numRead = 0;

  int pos = 10;

  

  while(pos + 5 < buf.size() && (check < NEED_VALID_READ_CNT)) {
    int sum = 0;
    for (uint i = 0; i < 3; i++) {
      sum += ((uint16_t)buf[pos + 2*i]<<8) + (uint8_t)buf[pos + 2*i + 1];
    }
    
    if (sum <= 15) { //вместо 15 в идеале д.б. 0, но дабы учесть возможность мелких помех..
      if (idx == 4) {
        check += 3;
      } else {
        check = 0;
      }
      idx = 4;
    }
    ++numRead;
    pos += DATA_LENGTH + CALIBR_LENGTH;
  }

  idx -= (numRead - 1) % 5;
  if (idx < 0) idx += 5;

  if (check < NEED_VALID_READ_CNT) {
    idx = -1;
  }

  return idx;
}

//! Чтение строки калибровочной информации
bool Avhrr3::readCalibrLine(const QByteArray& buf, int* prtIdx, int length, int num)
{
  if ((uint)buf.size() < sizeof(avhrr::CalibrData)) return false;

  bool allEmpty = true;
  avhrr::CalibrData cData;
  for (uint i = 0; i < sizeof(avhrr::CalibrData)/sizeof(uint16_t); i++) {
    cData.M[i] =  ((uint16_t)buf[2*i]<<8) + (uint8_t)(buf[2*i+1]);
    if (cData.M[i] != 0xffff) {
      allEmpty = false;
    }
  }
  
  if (!allEmpty) {
    for (uint i = 0; i < 3; i++) { //по три значения в каждом кадре
      _coef.prt[*prtIdx].append(cData.S.telemetry.prt[i]);
    }
    
    if (*prtIdx == AVHRR3_PRT_CNT) {
      *prtIdx = 0;
    } else {
      ++(*prtIdx);
    }
    
    for (uint i = 0; i < 10*AVHRR3_IR_CNT; i += AVHRR3_IR_CNT) {
      for (uint j = 0; j < AVHRR3_IR_CNT; j++) {
        _coef.Cbb[j].append(cData.S.calibrTargetView[i+j]);
        //debug_log << "Cbb" << j << cData.S.calibrTargetView[i+j];
      }
    }
    
    for ( uint i = 0; i < 10*AVHRR3_CHANNEL_COUNT; i += AVHRR3_CHANNEL_COUNT ) {
      for (uint j = 0, ch = 0; j < AVHRR3_CHANNEL_COUNT; j++, ch++) {
        _coef.Cs[ch].append(cData.S.spaceData[i+j]);
        //debug_log << "Cs" << ch << cData.S.spaceData[i+j];
        if (ch == 2) {
          _coef.Cs[3].append(_coef.Cs[2].last());
          ch++;
        }
      }
    }
  }
  
  if (num % LINE_PROCESS_COUNT == (LINE_PROCESS_COUNT - 1)) {
    calcACoef(num);
    _coef.clear();
  } else if (length - 1 - num == 0) {
    repeatLastACoef();
  }
  
  return true;
}

/*! 
* \brief  Вычисление a коэффициентов
* \param  lines Номер считываемого кадра HRPT
* \return 0 в случае успеха, иначе код ошибки
*/
int Avhrr3::calcACoef(int line)
{
  float prt[AVHRR3_PRT_CNT+1] = {0};
  
  for (uint i = 0; i < AVHRR3_PRT_CNT + 1; i++) {
    HrptCalibr::meanCounts(_coef.prt[i].data(), _coef.prt[i].size(),  &(prt[i]));
  }
  
  float Cbb[AVHRR3_IR_CNT] = {0};
  for (uint j = 0; j < AVHRR3_IR_CNT; j++) {
    HrptCalibr::meanCounts(_coef.Cbb[j].data(), _coef.Cbb[j].size(),  &(Cbb[j]));
    // if (j == 1) {
    //   debug_log << "Cbb" << j << Cbb[j];
    // }
  }
  
  float* Cs = new float[_conf.chan_cnt() - 1]; //"-1" - т.к. передаются данные только 5 каналов
  if (!Cs) return -1;
  for (int j = 0; j < _conf.chan_cnt() - 1; j++) {
    HrptCalibr::meanCounts(_coef.Cs[j].data(), _coef.Cs[j].size(),  &(Cs[j]));
    //    if (j == 3) {
    // debug_log << "Cs" << j << Cs[j];
    // }
  }

  float tPrt[AVHRR3_PRT_CNT] = {0}; //с учетом коэффициентов
  t_prt(prt, tPrt);
  
  for (int chan = AVHRR3_VIZ_CNT; chan < _conf.chan_cnt(); chan++ ) {//для LINE_PROCESS_COUNT строк сканирования для каждого канала по очереди
    QVector<float> a(3, 0);
    uint numIr = chan - 3;
    if (numIr != 0 || (_lineChan3x == -1 && _chan3x == 0) ||
        (_lineChan3x != -1 && _chan3x == 0 && line <  _lineChan3x) ||
        (_lineChan3x != -1 && _chan3x == 1 && line >= _lineChan3x)) {
      infraRedACoef(numIr, tPrt, Cs[chan - 1], Cbb[numIr], &a);
    }
    _a[chan].append(a);
  }
  
  if (Cs) delete[] Cs;
  
  return 0;
}

void Avhrr3::repeatLastACoef()
{
  for (int chan = AVHRR3_VIZ_CNT; chan < _conf.chan_cnt(); chan++ ) {
    _a[chan].append(_a.value(chan).last());
  }
}


//! парсинг данных AVHRR/3, realtime
// bool Avhrr3::parseRT(const QByteArray& data, const QDateTime& dt, int* remainBytes)
// {
//   Q_UNUSED(dt);

//   if (0 != remainBytes) {
//     *remainBytes = 0;
//   }

//   //считаем, что канал 3A и не меняется
//   _lineChan3x = -1;
//   _chan3x = 1;

//   uint length = data.size() / ((_conf.chan_cnt()-1) * _conf.samples() * 2 + CALIBR_LENGTH); // "-1" - т.к. передаются данные только 5 каналов
//   // var(length);
//   // var(data.size());
//   uint idx = 0;
//   for (int i=0; i<_ch.size(); i++) {
//     if (_ch[i].property().num == -1) {
//       _ch[i].setup(length, _conf.samples(), idx + 1);

// //      if (i == 2 || i == 3) {
// //	_ch[i]->setNum(7); //для отображения в комбике при приёме 3A/B канала
// //      }
//       _ch[i].setLimits(0, 0x3FF);
//     } else {
//       _ch[i].resetRT(length);
//     }
//     idx++;
//   }

//   bool ok = readScanLines(data, length);

//   return ok;
// }

//! некалиброванные, в реальном времени
// void Avhrr3::serializedImgDataRT(QByteArray* ba)
// {
//   QDataStream ds(ba, QIODevice::WriteOnly);
//   ds << _conf.rt_thin();
//   ds << _ch.size() - 1; //3a и 3b в одном
//   for (int idx = 0; idx < _ch.size(); idx++) {
//     if ((_chan3x == 1 && idx == 3) || 
//   	(_chan3x == 0 && idx == 2)) {
//       continue;
//     }
//     int rows, cols;
//     ds << _ch.at(idx).number();
//     ds << _ch.at(idx).imgRawData(_conf.rt_thin(), &rows, &cols);
//     ds << rows;
//     ds << cols;
//   }
// }


/***********************************/
//-------   functions for calibrate visible and near-IR channels   ------//
//по методике R.M. Mitchell "Calibration Status of the NOAA AVHRR Solar Ferlectance Channels", 1999 (параметр intercept равен slope*space_data).(уже без этой методики)
//и рекомендациям NOAA
//chanId - channel number (0 - 1 channel, 1 - 2 channel, 2 - 3A channel)

bool Avhrr3::calibrate()
{
  bool ok = true;
  
  if (!_fok) return false;
  if (_a.isEmpty()) {
    error_log << QObject::tr("Коэффициенты для калибровки ИК каналов не рассчитаны");
  }

  for (int numCh = 0; numCh < _conf.chan_cnt(); numCh++) {
    if (_lineChan3x == -1 &&
        ((numCh==2 && _chan3x != 1) || (numCh == 3 && _chan3x != 0) )) continue;
    if (numCh < AVHRR3_VIZ_CNT) {
      calibrateViz(numCh);
    } else if (!_a.isEmpty()) {
      calibrateIr(numCh);
    }
  }

  return ok;
}

/*! 
* \brief Калибровка данных AVHRR/3, канал видимого диапазона
* \param channel Номер канала данных data
*/
void Avhrr3::calibrateViz(int channel)
{
  auto chd = &_ch[channel];
  chd->setLimits(0, 100);

  QVector<float> a1(3, 0);
  a1[0] = _f.vizible()[channel*2][1];
  a1[1] = _f.vizible()[channel*2][0];

  QVector<float> a2(3, 0);
  a2[0] = _f.vizible()[channel*2+1][1];
  a2[1] = _f.vizible()[channel*2+1][0];

  float resValue;
  float thresh = _f.thresh()[channel];
  float maxLimit = chd->maxLimit();
  float minLimit = chd->minLimit();
  QVector<short>& data = chd->data();
  uint size = chd->size();
  //var(channel);

  //  if (channel == 0) {
  /*  QFile file(QString("NOAA19.ch") + QString::number(channel) + ".raw");
    //    QFile file("NOAA19.ch1.raw");
    //qDebug () << filename;
    file.open(QIODevice::WriteOnly);
    QDataStream ds(&file);
    ds.setByteOrder(QDataStream::LittleEndian);
    ds << int32_t(2048) << int32_t(4268);
    ushort empty = 1;
    for (int idx =0 ; idx < data.size(); idx++) {
      if (data.at(idx) == 1023 || data.at(idx) == 0) {
    ds << empty;
      } else {
    ds << data.at(idx);
      }
      //qDebug() << idx << data.at(idx);
    }
    //file.write((char*)data, col*row*2);
    file.close();
    //  }

  */
  for (uint idx = 0; idx < size; idx++) {
    if (data[idx] == short(0xffff)) {
      continue;
    }
    float value = data[idx];
    if (value <= thresh) {
      resValue = HrptCalibr::sceneRadiance(a1, value);
    } else {
      resValue = HrptCalibr::sceneRadiance(a2, value);
    }

    if (resValue > maxLimit) {
      if (resValue <= 1.1 * maxLimit) {
        resValue = maxLimit;
      } else {
        resValue = INVALID_CALIBR_VALUE;
      }
    } else if (resValue < minLimit) {
      if (resValue >= 1.1 * minLimit) {
        resValue = minLimit;
      } else {
        resValue = INVALID_CALIBR_VALUE;
      }
    }
    
    data[idx] = resValue;
  }
}


/*! 
 * \brief Калибровка данных AVHRR/3
 * \param channel Номер канала данных data
 */
void Avhrr3::calibrateIr(int channel)
{
  //var(channel);
  auto chd = &_ch[channel];
  
  float resValue = 0;
  chd->setLimits(80, 300);
  if (!_a.contains(channel)) {
    return;
  }

  const QList<QVector<float> >& a = _a[channel];

  float maxLimit = chd->maxLimit();
  float minLimit = chd->minLimit();
  const ::Hrpt::UniFactor& t2r = _f.t2r()[channel-3];
  uint columns = chd->columns();
  uint rows = chd->rows();
  QVector<short>& data = chd->data();

/*  // if (channel == 4) {
  QFile file(QString("NOAA19.ch") + QString::number(channel) + ".raw");
  //qDebug () << filename;
  file.open(QIODevice::WriteOnly);
  QDataStream ds(&file);
  ds.setByteOrder(QDataStream::LittleEndian);
  ds << int32_t(chd.columns()) << int32_t(chd.rows());
  ushort empty = 1;
  for (int idx =0 ; idx < data.size(); idx++) {
    if (data.at(idx) == 1023 || data.at(idx) == 0) {
      ds << empty;
    } else {
      ds << data.at(idx);
    }
    //qDebug() << idx << data.at(idx);
  }
  //file.write((char*)data, col*row*2);
  file.close();
  // }
  */
  int idx = 0;
  for (uint row = 0, num = 0; row < rows; row += LINE_PROCESS_COUNT, num++) {
    if (num >= (uint)a.size()) { return; } //TODO
    const QVector<float>& anum = a.at(num);
    for (int i = 0; (i < LINE_PROCESS_COUNT && i + row < rows); i++) { //каждый набор коэф-тов для 5 строк сканирования
      for (uint col = 0; col < columns; col++) {
        if (data[idx] == short(0xffff)) {
          ++idx;
          continue;
        }
        float value = data[idx];
        resValue = HrptCalibr::sceneRadiance(anum, value);
        resValue = HrptCalibr::earthTemperature(resValue, t2r.data());
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
        // if (channel == 4) {
        //   debug_log << "res" << data[idx] << resValue;
        // }
        data[idx] = resValue;
        idx++;
      }
    }
  }
}



/*! 
* \brief Расчёт a коэффициентов для каналов инфра-красного диапазона
* \param chanId Номер ИК-канала
* \param tPrt Температура PRT
* \param Cs_av Среднее значение сканирования космоса (space ounts)
* \param Cbb_av Среднее значение сканирования чёрного тела (blackbody counts)
* \param a Возвращаемые а коэффициенты
*/
void Avhrr3::infraRedACoef(int chanId, float tPrt[AVHRR3_PRT_CNT], float Cs_av, float Cbb_av,  QVector<float>* a)
{
  float Nbb = blackBodyRadiance( chanId, tPrt );
  float k = (_f.nonLin()[chanId][0] - Nbb) /  ( Cs_av - Cbb_av );
  float d = _f.nonLin()[chanId][0] - Cs_av * k;
  (*a)[0] =  _f.nonLin()[chanId][1] +
             d*(1 + _f.nonLin()[chanId][2] +  _f.nonLin()[chanId][3] * d);
  (*a)[1] = k * (1 + _f.nonLin()[chanId][2] + 2 * _f.nonLin()[chanId][3] * d);
  (*a)[2] = _f.nonLin()[chanId][3] * k * k;
}


//chanId: 0 - chan 3B, 1 - chan 4, 2 - chan 5
float Avhrr3::blackBodyRadiance( int chanId, float t_prt[AVHRR3_PRT_CNT] )
{
  float tEf_bb = _f.t2r()[chanId][1] +
                 _f.t2r()[chanId][2] * HrptCalibr::meanBbTemp(_f.weight()[0].data(), t_prt, AVHRR3_PRT_CNT, 0);
  float N_bb = HrptCalibr::planckFunc(_f.t2r()[chanId][0], tEf_bb);

  // if (chanId == 1) {

  //   debug_log << HrptCalibr::meanBbTemp(_f.weight()[0], t_prt, AVHRR3_PRT_CNT, 0) << tEf_bb << N_bb;
  // }
  
  return N_bb;
}

//!temperature T_prt for all PRTs
void Avhrr3::t_prt( const float prt_av[AVHRR3_PRT_CNT], float tPrt[AVHRR3_PRT_CNT] )
{
  for (uint i = 0; i < AVHRR3_PRT_CNT; i++) {
    tPrt[i] = HrptCalibr::prtTemp( _f.prt()[i].data(), prt_av[i], 5 );
  }
}


//! геометрическая корекция
bool Avhrr3::correction(const SatViewPoint* sat)
{
  if (_ch.size() != 0) {
    /*bool ok = */ sat->countGrid(MnMath::deg2rad(_conf.fovstep()), sat->dtStart().msecsTo(sat->dtEnd()) /  _ch[0].rows(), MnMath::deg2rad(_conf.scan_angle()), _points);
    //var(ok);
  }

  // if (! _pretr.geom()) {
  //   return true;
  // }
  
  // GeomCorrection geom(sat);
  // geom.calculate(MnMath::deg2rad(_conf.fovstep()), MnMath::deg2rad(_conf.scan_angle()),
  // 		 _conf.fov(), _conf.samples());

  // for (int i=0; i<_ch.size(); i++) {
  //   QVector<float> dataG;
  //   // var(_ch[i]->rows());
  //   // var(_ch[i]->columns());
  //   geom.correction(dataG, _ch[i]->data(), _ch[i]->rows(), _ch[i]->columns());
  //   // var(dataG.size());
  //   // var(dataG.size()/_ch[i]->rows());
  //   _ch[i]->setData(dataG, _ch[i]->rows(), dataG.size()/_ch[i]->rows());
  // }

  return false;
}

//! сохранение
bool Avhrr3::save(const meteo::global::StreamHeader& header, const QString& path, const QString& baseName,
                  const QDateTime& dtStart, const QDateTime& dtEnd, SatViewPoint* sat)
{
  Instrument::save(header, path, baseName, dtStart, dtEnd, sat);

  return true;
  
  //--- TODO временно, для Алексея Костромитинова :)
  QString csvName = MnCommon::varPath("mappi") + "/image/avhrr3/" +  baseName + "_coords.csv";
  var(csvName);
  QFile file(csvName);
  var(_points.size());
  file.open(QIODevice::WriteOnly);
  QTextStream ts(&file);
  for (int idx = 0; idx < _points.size(); idx++) {
    if (idx % _conf.samples() == 0) {
      ts << "\n";
    }
    ts << _points.at(idx).lat << "," << _points.at(idx).lon << ";";
  }
  file.close();

  QString coordName = MnCommon::varPath("mappi") + "/image/avhrr3/" +  baseName + "_coords.dat";
  var(coordName);
  file.setFileName(coordName);
  var(_points.size());
  file.open(QIODevice::WriteOnly);
  QDataStream ds(&file);
  for (int idx = 0; idx < _points.size(); idx++) {
    ds << _points.at(idx).lat << _points.at(idx).lon;
  }
  file.close();

  //---
  
  return true;
}


