#ifndef MAPPI_PRETREATMENT_INSTRUMENTS_HRPT_CONST_H
#define MAPPI_PRETREATMENT_INSTRUMENTS_HRPT_CONST_H

#include <cross-commons/funcs/mn_funcs.h>

class QString;
class QTextStream;
class QDateTime;

class Factors;

namespace MnCommon {
  bool getBitData(char *abuf,int lenBuf,int astart,int awidth,unsigned int &ret_val);
  bool getBitData(uchar *abuf,int lenBuf,int astart,int awidth, unsigned int &ret_val);
} 


//! Работа с потоком HRPT
namespace Hrpt {
  extern const float c1_planck, c2_planck;

  //возвращает двоичное представление числа (для отладки)
  QString db(unsigned value);

  ushort checkHousekeeping(const QVector<ushort>& house);
  void meanData(const QMap<int, QVector<float>>& val, int cnt, const float* min, const float* max, 
		QMap<int, QVector<float>>* mean);
  void limitSamples(const QVector<float>& val, QVector<float>* mean, int cnt, float min, float max);
  void meanWithNeighbour(const QVector<float>& src, QVector<float>* dst);
  void createRf(const QMap<int, QVector<float>>& rf, int key, int rOffset,  Factors& f, QVector<float>* mean);
  void createPrt(const QVector<float>& prt, int prtOffset, int prtCnt,  Factors& f, QVector<float>* mean);
  float coldTemp(float deltaCold);
  void aCoefs(float bbRad, float spRad, float bbMean, 
	      float spMean, float u, float* a );
  float meanCalibrValues(const QVector<float>& Cx, int cur, int num);
  
  //! Одномерный массив коэффициентов калибровки
  class UniFactor : public QVector<float> {
  public:
    unsigned col; //!< Размерность
    UniFactor() {}
    void clear() { col = 0; QVector::clear(); }
    //! Выделение памяти
    bool alloc(unsigned cols) {
      resize(cols);
      col = cols;
      return true;
    }
    //! Проверка соответствия размерности
    bool check(unsigned cols) {
      return (col == cols);
    }
  };
  
  //! Двумерный массив коэффициентов калибровки
  class TwoFactor : public QVector<UniFactor> {
  public:
    unsigned row; //!< Количество строк 
    unsigned col; //!< Количество столбцов
    TwoFactor() { row=col=0; }
    //!Очистка памяти
    void clear() { QVector::clear(); row=col=0; }
    //! Выделение памяти
    bool alloc(unsigned rows, unsigned cols) {
      resize(rows);
      for (uint idx =0; idx < rows; idx++) {
	operator[](idx).resize(cols);
      }
      row = rows;
      col = cols;
      return true;
    }
    
    //!Проверка соответствия размерностей
    bool check(unsigned rows, unsigned cols) { 
      return (row == rows && col == cols);
    }
  };
};


//! Параметры для калибровки
class Factors {
 public:
  Factors();
  ~Factors();

  //  int readFile(unsigned satId, const QString& instr);
  bool readFile(const QString& fileName, const QString& instr);
  const Hrpt::TwoFactor& weight() { return _weight; }
  const Hrpt::TwoFactor& prt()    { return _prt; }
  const Hrpt::TwoFactor& t2r()    { return _t2r; }
  const Hrpt::TwoFactor& nonLin() { return _nonLin; }
  const Hrpt::UniFactor& thresh() { return _thresh; }
  const Hrpt::TwoFactor& vizible() { return _vizible; }
  const Hrpt::TwoFactor& a2r()    { return _a2r; }
  const Hrpt::UniFactor& secondary() { return _secondary; }
  const Hrpt::TwoFactor& rCal()  { return _rCal; }
  const Hrpt::UniFactor& c2r()   { return _c2r; }
  const Hrpt::TwoFactor& wcr()   { return _wcr; }
  const Hrpt::TwoFactor& ccr()   { return _ccr; }
  const Hrpt::TwoFactor& lim()   { return _lim; }
  const Hrpt::TwoFactor& c2m()   { return _c2m; }
  const Hrpt::TwoFactor& c2e()   { return _c2e; }
  const Hrpt::TwoFactor& shelf() { return _shelf; }
  const Hrpt::TwoFactor& mux()   { return _mux; }
  const Hrpt::UniFactor& id()    { return _id; }
  bool checkWeight(unsigned row, unsigned col);
  bool checkPrt(unsigned row, unsigned col);
  bool checkT2r(unsigned row, unsigned col);
  bool checkNonLin(unsigned row, unsigned col);
  bool checkThresh(unsigned col);
  bool checkVizible(unsigned row, unsigned col);
  bool checkA2r(unsigned row, unsigned col);
  bool checkSecondary(unsigned col);
  bool checkRCal(unsigned row, unsigned col);
  bool checkC2r(unsigned col);
  bool checkCcr(unsigned row, unsigned col);
  bool checkWcr(unsigned row, unsigned col);
  bool checkLim(unsigned row, unsigned col);
  bool checkC2m(unsigned row, unsigned col);
  bool checkC2e(unsigned row, unsigned col);
  bool checkShelf(unsigned row, unsigned col);
  bool checkMux(unsigned row, unsigned col);
  bool checkId(unsigned col);

 private:

  void clear();
  bool fseekInstrument( QTextStream& stream, const QString& instr);
  bool readFactors( QTextStream& stream);
  bool readCoeffs( QTextStream& stream, const QString& id, 
			   unsigned rows, unsigned cols);
  // QString createFileName(unsigned satId);
  
  bool fillCoefs(QTextStream& stream, float** data,  
		 unsigned rows, unsigned cols);
  bool fillCoefs(QTextStream& stream, float* data, unsigned cols);
  
  bool fillCoefs(QTextStream& stream, Hrpt::TwoFactor* data, uint rows, uint cols);
  bool fillCoefs(QTextStream& stream, Hrpt::UniFactor* data, uint cols);
 private:
   Hrpt::TwoFactor _weight;
   Hrpt::TwoFactor _prt;
   Hrpt::TwoFactor _t2r;
   Hrpt::TwoFactor _nonLin;
   Hrpt::TwoFactor _vizible;
   Hrpt::TwoFactor _a2r;
   Hrpt::TwoFactor _rCal;
   Hrpt::TwoFactor _ccr;
   Hrpt::TwoFactor _wcr;
   Hrpt::TwoFactor _lim;
   Hrpt::TwoFactor _c2m;
   Hrpt::TwoFactor _c2e;
   Hrpt::TwoFactor _shelf;
   Hrpt::TwoFactor _mux;
   
   Hrpt::UniFactor _thresh;
   Hrpt::UniFactor _secondary;
   Hrpt::UniFactor _c2r;
   Hrpt::UniFactor _id;
};

#endif 
