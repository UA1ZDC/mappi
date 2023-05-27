#include <qfile.h>
#include <qstringlist.h>
#include <qregexp.h>
#include <qdatetime.h>
#include <qtextstream.h>
#include <qvector.h>

#include <math.h>

#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/paths.h>

//#include <mappi/common/mappi_defines.h>
//#include <mn_errdefs.h>
//#include <spconsts.h>

#include "hrpt_const.h"
//#include "hrpt_read.h"
#include "hrpt_calibr.h"


const int SIZEOCT = 8;
#define OCT(a) int((unsigned char)a)
#define NOCT(a) int((a)/SIZEOCT)
#define GETBIT(a,n) (a>>(SIZEOCT-(n+1))&1)

const float Hrpt::c1_planck = 1.1910427e-5; //!< Константа формулы Планка
const float Hrpt::c2_planck = 1.4387752;    //!< Константа формулы Планка

#define MAPPIFACTOR_PATH       MnCommon::sharePath("mappi") + "/factors/"
#define MAPPIFACTOR_EXT        ".txt"

#define WEIGHT_FACTOR    "weight"
#define PRT_FACTOR       "prt"
#define TEMP2RAD_FACTOR  "t2r"
#define NONLINEAR_FACTOR "nonLin"
#define THRESHOLD_FACTOR "thr"
#define VIZIBLE_FACTOR   "vizible"
#define ALBEDO_FACTOR    "a2r"
#define SECONDARY_FACTOR "secondary"
#define RESIST_FACTOR    "rCal"
#define COUNT2RAD_FACTOR "c2r"
#define WARMCORR_FACTOR  "wcr"
#define COLDCORR_FACTOR  "ccr"
#define LIMIT_FACTOR     "lim"
#define MOMENT_FACTOR    "c2m"
#define ENERGY_FACTOR    "c2e"
#define RFSHELF_FACTOR   "prim"
#define RFMUX_FACTOR     "back"
#define ID_FACTOR        "id"

#define FACTOR_COMMENT   '#'
#define FACTOR_SEPARATOR ','

#define NEED_VALID_READ_CNT 5

//возвращает двоичное представление числа (для отладки)
QString Hrpt::db(unsigned value)
{
  QString value_binary("\0");
  
  if ((value)!=1 && value != 0) { value_binary=db(value/2);}
  char a=value%2+48;
  return value_binary+=a;
}


/**
 * переопределенная ф-я getBitData
 */
bool MnCommon::getBitData(uchar *abuf,int lenBuf,int astart,int awidth, unsigned int &ret_val){
  return getBitData((char *)abuf, lenBuf,astart,awidth, ret_val);
}

/**
 * определяет значение awidth битов массива abuf начиная с astart(astart-первый(старший) бит)
 * @param abuf входной массив
 * @param lenBuf размер входного массива
 * @param astart начало
 * @param awidth ширина
 * @param ret_val возвращаемое значение
 * @return true/false
 */
bool MnCommon::getBitData(char *abuf,int lenBuf,int astart,int awidth, unsigned int &ret_val)
{
  int i;
  ret_val=0;
  if(!abuf) return false;
  if(astart+awidth>lenBuf*SIZEOCT) return false;

  int h,no,nb,z,tb;

  for(i=astart;i<astart+awidth;i++)
  {
    no=NOCT(i);          //номер байта
    nb=i-SIZEOCT*(no); //номер бита
    h=OCT(abuf[no]);   //текущий байт
    tb = GETBIT(h,nb); //текущий бит
    z=(1<<(astart+awidth-i-1))*tb;
    ret_val+=z;
  }
  return true;
}



ushort Hrpt::checkHousekeeping(const QVector<ushort>& house)
{
  uint   check = 0;
  ushort nextVal = 0;
  ushort cur = 0;
  int idx = 0;
  
  while (idx < house.size() && check < NEED_VALID_READ_CNT) {
    nextVal = house.at(idx);
    if (cur == nextVal) {
      ++check;
    } else {
      check = 0;
    }
    
    cur = nextVal;
    idx++;
  }

  if (check < NEED_VALID_READ_CNT) {
    return 0;
  }

  return cur;
}

//! val - канал, все значения по строкам
void Hrpt::meanData(const QMap<int, QVector<float>>& val, int cnt, const float* min, const float* max, 
                    QMap<int, QVector<float>>* mean)
{
  if (val.size() == 0) {
    return;
  }

  auto it = val.constBegin();
  while (it != val.constEnd()) {
    QVector<float> one;
    // if (!min.contains(it.key()) || !max.contains(it.key())) {
    //   debug_log << "no min/max";
    //   limitSamples(it.value(), &one, cnt, 0, 0);
    // } else {
    limitSamples(it.value(), &one, cnt, min[it.key()], max[it.key()]);
    // }

    QVector<float>& dst = (*mean)[it.key()];
    meanWithNeighbour(one, &dst);
    ++it;
  }
}


//! 
void Hrpt::limitSamples(const QVector<float>& val, QVector<float>* mean, int cnt,
                        float min, float max)
{
  for (int idx = 0; idx < val.size(); idx += cnt) { //по строкам
    if (idx + cnt > val.size()) break;
    
    int curCnt = 0;
    float cur = 0;
    for (int cntIdx = 0; cntIdx < cnt; cntIdx++) { //по числу параметров в строке
      if ((min == 0 && max == 0) ||
          (val.at(idx + cntIdx) <= max && val.at(idx + cntIdx) >= min)) {
        cur += val.at(idx + cntIdx);
        curCnt++;
      }
    }
    if (curCnt == 0) {
      mean->append(0);
    } else {
      mean->append(cur / curCnt);
    }
  }
}


void Hrpt::meanWithNeighbour(const QVector<float>& src, QVector<float>* dst)
{
  int num = 3;//количество соседей для сворачивания
  for (int idx = 0; idx < src.size(); idx++) {
    if (idx < num || idx >= (src.size() - num)) {
      dst->append(src[idx]);
    } else {
      dst->append(meanCalibrValues(src, idx, num));
    }
  }
}

/*!
 * \brief Сворачивание значений space counts и blackbody counts по предыдущим и последующим строкам
 * \param Cx значения измерений космоса или чёрного тела для нескольких линий сканирования.
 Размер массива 2*num+1. Сворачиваться будет значение src[num].
 * \param num Количество линий сканирования до и после текущей, использующееся для усреднения
 */
float Hrpt::meanCalibrValues(const QVector<float>& Cx, int cur, int num)
{
  if (cur >= Cx.size()) return 0;
  
  if (cur < num || cur + num > Cx.size()) {
    return Cx.at(cur);
  }
  
  float mean = 0;
  float noNull = 0; //не нулевое значение
  
  for (int idx = -num; idx <= (int)num; idx++) {
    if (Cx.at(cur + idx) != 0) {
      noNull = Cx.at(cur + idx);
      //var(noNull);
      break;
    }
  }
  
  if (noNull == 0) {
    return 0;
  }

  for (int idx = -num; idx <= (int)num; idx++) {
    if (Cx.at(cur + idx) != 0) {
      mean += ( 1 - fabs(idx)/(num+1) ) * Cx.at(cur + idx);
    } else {
      mean += ( 1 - fabs(idx)/(num+1) ) * noNull;
    }
    //debug_log << "mean" << cur << idx << Cx.at(cur + idx) << mean;
  }
  
  return mean / (num + 1);
}


void Hrpt::createRf(const QMap<int, QVector<float>>& rf, int key, int rfOffset, Factors& f, QVector<float>* mean)
{
  const QVector<float>& cur = rf[key];
  for (int idx = 0; idx < cur.size(); idx++) {
    mean->append(HrptCalibr::prtTemp(f.prt()[key+rfOffset].data(), cur[idx], 4)); //4 - количество коэффициентов
  }
}

void Hrpt::createPrt(const QVector<float>& prt, int prtOffset, int prtCnt, Factors& f, QVector<float>* mean)
{
  int lines = prt.size() / prtCnt;
  QVector<QVector<float>> temp(lines);

  for (int idx = 0, tidx = 0; idx + prtCnt <= prt.size(); idx += prtCnt, tidx++) { //по строкам
    for (int num = 0; num < prtCnt; num++) { //по prt
      temp[tidx].append(HrptCalibr::prtTemp(f.prt()[num+prtOffset].data(), prt[idx + num], 4));
    }
  }

  for (int i = 0; i < lines; i++) {
    float w[prtCnt];
    for (int j = 0; j < prtCnt; j++) {
      if (i==0 ||
          ((temp[i][j] < (temp[i-1][j] + 0.2) && temp[i][j] > (temp[i-1][j] - 0.2) ) &&
           (i==lines-1 || (temp[i][j] < (temp[i+1][j] + 0.2) && temp[i][j] > (temp[i+1][j] - 0.2) ))
           )
          ) {
        w[j] = 1;
      } else {
        w[j]=0;
      }
    }

    mean->append(HrptCalibr::meanBbTemp(w, temp[i].data(), prtCnt, 0));
  }
}


/*!
 * \brief Коррекция температуры космоса
 */
float Hrpt::coldTemp(float deltaCold)
{
  return 2.73 + deltaCold;
}

/*!
 * \brief Расчёт коэффициентов для преобразования измерений Земли в излучение
 * \param bbRad  Излучение Планка, соотв-щее температура чёрного тела
 * \param spRad  Излучение Планка, соотв-щее температура космоса
 * \param bbMean Среднее значение измерений чёрного тела
 * \param spMean Среднее значение измерений космоса
 * \param u      Неизвестный параметр (коэффициент нелинейности)
 * \param a      Три возвращаемых коэффициента
 */
void Hrpt::aCoefs(float bbRad, float spRad, float bbMean, 
                  float spMean, float u, float* a)
{
  if (!a) return;
  float gain = (bbMean - spMean) / (bbRad - spRad);
  if (bbMean ==0 || spMean==0 || bbRad==0 || spRad==0){
    a[0] = a[1] = a[2] = 0;
    return;
  }
  a[0] = bbRad - bbMean/gain + u*bbMean*spMean/gain/gain;
  a[1] = 1/gain - u*(spMean+bbMean)/gain/gain;
  a[2] = u / gain / gain;
}


Factors::Factors()
{
}

Factors::~Factors()
{
  clear();
}

/*! 
 * \brief Очистка памяти
 */
void Factors::clear()
{
  _weight.clear();
  _prt.clear();
  _t2r.clear();
  _nonLin.clear();
  _vizible.clear();
  _a2r.clear();
  _rCal.clear();
  _ccr.clear();
  _wcr.clear();
  _lim.clear();
  _c2m.clear();
  _c2e.clear();
  _shelf.clear();
  _mux.clear();
  
  _thresh.clear();
  _secondary.clear();
  _c2r.clear();
  _id.clear();
}

/*! 
 * \brief Чтение констант из файла
 * \param instr Прибор
 * \return 0 в случае успеха, иначе код ошибки
 */
bool Factors::readFile(const QString& fileName, const QString& instr)
{
  clear();
  var(instr);
  var(MAPPIFACTOR_PATH + fileName);
  QFile file(MAPPIFACTOR_PATH + fileName);
  if (!file.open(QIODevice::ReadOnly)) {
    error_log << QObject::tr("Ошибка открытия файла %1").arg(MAPPIFACTOR_PATH + fileName);
    return false;
  }
  
  QTextStream stream( &file );

  if ( !fseekInstrument(stream, instr) ) {
    file.close();
    error_log << QObject::tr("Не найден инструмент %1").arg(instr);
    return false;
  }

  bool ok = readFactors(stream);

  file.close();
  return ok;
}

/*! 
 * \brief Поиск позиции с константами прибора в файле
 * \param instr Прибор
 * \return Позиция в файле. Если данные для прибора не найдены -1
 */
bool Factors::fseekInstrument( QTextStream& stream, const QString& instr)
{
  while ( !stream.atEnd() ) {
    if ( "["+instr+"]" ==  stream.readLine()) {
      return true;
    }
  }
  return false;
}

/*! 
 * \brief Чтение констант
 * \param file Файл для считывания
 * \return  0 в случае успеха, иначе код ошибки
 */
bool Factors::readFactors(QTextStream& stream)
{
  QString line;
  while ( !stream.atEnd() ) {
    line = stream.readLine().trimmed();
    if (line.isEmpty() || line[0]==FACTOR_COMMENT) {
      continue;
    }
    
    if (line.at(0) == '[' && line.at(line.length()-1) == ']') { //следующий коэффициент
      break;
    }
    
    QStringList strList = line.split(FACTOR_SEPARATOR);
    if (strList.count() != 3) {
      error_log << QObject::tr("Ошибка чтения названия коэффициентов") << strList;
      return false;
    }

    if (!readCoeffs(stream, strList[0], strList[1].toUInt(), strList[2].toUInt())) {
      error_log << QObject::tr("Ошибка чтения коэффициентов %1").arg(strList[0]);
      return false;
    }
  }
  return true;
}

/*!
 * \brief Чтение констант
 * \param stream Поток данных
 * \param id Идентификатор константы
 * \param rows Количество строк констант
 * \param cols Количество колонок констант
 * \return true в случае успеха, иначе false
 */
bool Factors::readCoeffs(QTextStream& stream, const QString& id, 
                         uint rows, uint cols)
{
  if (rows == 0 || cols == 0) return false;

  //debug_log << id << rows << cols;
  
  //очистка памяти для всех выделяемых здесь переменных (все - члены класса), осуществляется при вызове интерфейсной функциии в деструкторе. Т.е. эта функция всегда должна быть закрыта и ей доблжна предшествовать функция clear()
  if (id == WEIGHT_FACTOR) {
    return fillCoefs(stream, &_weight, rows, cols);
  }
  
  if (id ==  PRT_FACTOR) {
    return fillCoefs(stream, &_prt, rows, cols);
  }

  if (id == TEMP2RAD_FACTOR) {
    return  fillCoefs(stream, &_t2r, rows, cols);
  }

  if (id == NONLINEAR_FACTOR) {
    return  fillCoefs(stream, &_nonLin, rows, cols);
  }
  
  if (id == THRESHOLD_FACTOR) {
    if (!_thresh.alloc(cols)) { return false; }
    return fillCoefs(stream, &_thresh, cols);
  }

  if (id == VIZIBLE_FACTOR) {
    return  fillCoefs(stream, &_vizible, rows, cols);
  }

  if (id == ALBEDO_FACTOR) {
    return  fillCoefs(stream, &_a2r, rows, cols);
  }

  if (id == SECONDARY_FACTOR) {
    if (!_secondary.alloc(cols)) { return false; }
    return fillCoefs(stream, &_secondary, cols);
  }

  if (id == RESIST_FACTOR) {
    if (!_rCal.alloc(rows, cols)) { return false; }
    return  fillCoefs(stream, &_rCal, rows, cols);
  }

  if (id == COUNT2RAD_FACTOR) {
    if (!_c2r.alloc(cols)) { return false; }
    return fillCoefs(stream, &_c2r, cols);
  }

  if (id == WARMCORR_FACTOR) {
    return  fillCoefs(stream, &_wcr, rows, cols);
  }

  if (id == COLDCORR_FACTOR) {
    return  fillCoefs(stream, &_ccr, rows, cols);
  }

  if (id == LIMIT_FACTOR) {
    return  fillCoefs(stream, &_lim, rows, cols);
  }

  if (id == MOMENT_FACTOR) {
    return  fillCoefs(stream, &_c2m, rows, cols);
  }

  if (id == ENERGY_FACTOR) {
    return  fillCoefs(stream, &_c2e, rows, cols);
  }

  if (id == RFSHELF_FACTOR) {
    return  fillCoefs(stream, &_shelf, rows, cols);
  }

  if (id == RFMUX_FACTOR) {
    return  fillCoefs(stream, &_mux, rows, cols);
  }

  if (id == ID_FACTOR) {
    if (!_id.alloc(cols)) { return false; }
    return fillCoefs(stream, &_id, cols);
  }
  return false;
}

/*! 
 * \brief Заполнение коэффициентами двумерного массива
 * \param stream Поток данных
 * \param data Массив для сохранения коэффициентов
 * \param rows Количество строк данных (1-я размерность массива)
 * \param cols Количество колонок данных (2-я размерность массива)
 * \return  true в случае успеха, иначе false
 */
bool Factors::fillCoefs(QTextStream& stream, float** data, uint rows, uint cols)
{
  if (!data) return false;
  QString line;
  for (uint i =0; i < rows; i++) {
    line = stream.readLine().trimmed();
    if (line.isEmpty()) {
      return false;
    }
    QStringList strList = line.split(FACTOR_SEPARATOR);

    if (strList.count() != (int)cols) {
      return false;
    }
    for (uint j=0; j < cols; j++) {
      data[i][j] = strList[j].toFloat();
    }
  }
  
  return true;
}

/*! 
 * \brief Заполнение коэффициентами двумерного массива
 * \param stream Поток данных
 * \param data Массив для сохранения коэффициентов
 * \param rows Количество строк данных (1-я размерность массива)
 * \param cols Количество колонок данных (2-я размерность массива)
 * \return  true в случае успеха, иначе false
 */
bool Factors::fillCoefs(QTextStream& stream, Hrpt::TwoFactor* data, uint rows, uint cols)
{
  if (nullptr == data) return false;

  if (!data->alloc(rows, cols)) { return false; }
  
  QString line;
  for (uint i =0; i < rows; i++) {
    line = stream.readLine().trimmed();
    if (line.isEmpty()) {
      return false;
    }
    QStringList strList = line.split(FACTOR_SEPARATOR);

    if (strList.count() != (int)cols) {
      return false;
    }

    for (uint j=0; j < cols; j++) {
      (*data)[i][j] = strList[j].toFloat();
    }
  }
  
  return true;
}


/*! 
 * \brief Заполнение коэффициентами одномерного массива
 * \param stream Поток данных
 * \param data Массив для сохранения коэффициентов
 * \param cols Количество колонок данных (2-я размерность массива)
 * \return true в случае успеха, иначе false
 */
bool Factors::fillCoefs(QTextStream& stream, Hrpt::UniFactor* data, uint cols)
{
  if (nullptr == data) return false;
  
  char c = FACTOR_SEPARATOR;
  for (uint i=0; i< cols; i++) {
    stream >> (*data)[i];
    if (i < cols-1) stream >> c;
    if (c != FACTOR_SEPARATOR) return false;
  }

  return true;
}

/*! 
 * \brief Заполнение коэффициентами одномерного массива
 * \param stream Поток данных
 * \param data Массив для сохранения коэффициентов
 * \param cols Количество колонок данных (2-я размерность массива)
 * \return true в случае успеха, иначе false
 */
bool Factors::fillCoefs(QTextStream& stream, float* data, uint cols)
{
  if (!data) return false;
  char c = FACTOR_SEPARATOR;
  QString line;
  for (uint i=0; i< cols; i++) {
    stream >> data[i];
    if (i < cols-1) stream >> c;
    if (c != FACTOR_SEPARATOR) return false;
  }

  return true;
}


bool Factors::checkWeight(uint row, uint col)
{
  return _weight.check(row, col);
}


bool Factors::checkPrt(uint row, uint col)
{
  return _prt.check(row, col);
}


bool Factors::checkT2r(uint row, uint col)
{
  return _t2r.check(row, col);
}


bool Factors::checkNonLin(uint row, uint col)
{
  return _nonLin.check(row, col);
}

bool Factors::checkThresh(unsigned col)
{
  return _thresh.check(col);
}

bool Factors::checkVizible(uint row, uint col)
{
  return _vizible.check(row, col);
}

bool Factors::checkA2r(uint row, uint col)
{
  return _a2r.check(row, col);
}


bool Factors::checkSecondary(uint col)
{
  return _secondary.check(col);
}


bool Factors::checkRCal(uint row, uint col)
{
  return _rCal.check(row, col);
}


bool Factors::checkC2r(uint col)
{
  return _c2r.check(col);
}



bool Factors::checkCcr(uint row, uint col)
{
  return _ccr.check(row, col);
}


bool Factors::checkWcr(uint row, uint col)
{
  return _wcr.check(row, col);
}


bool Factors::checkLim(uint row, uint col)
{
  return _lim.check(row, col);
}


bool Factors::checkC2m(uint row, uint col)
{
  return _c2m.check(row, col);
}


bool Factors::checkC2e(uint row, uint col)
{
  return _c2e.check(row, col);
}



bool Factors::checkShelf(uint row, uint col)
{
  return _shelf.check(row, col);
}


bool Factors::checkMux(uint row, uint col)
{
  return _mux.check(row, col);
}


bool Factors::checkId(uint col)
{
  return _id.check(col);
}

