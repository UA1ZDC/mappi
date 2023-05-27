#include <qfile.h>
#include <qstringlist.h>
#include <qtextstream.h>
#include <qmap.h>

#include <math.h>
#include <commons/mathtools/mnmath.h>

#include "hrpt_calibr.h"
#include "hrpt_const.h"

using namespace Hrpt;

/*! 
 * \brief Перевод показаний PRT в температуру
 * \param coef Коэффициент, предоставляемый для каждого PRT
 * \param count Значение показаний PRT
 * \param num Количество измерений (соотв-но, количество coef)
 */
float HrptCalibr::prtTemp(const float* coef, float count, uint num)
{
  float temp = 0;
  if (!coef) return temp;

  float tmpCount = count;
  temp = coef[0];
  for (uint k=1; k<num; k++, tmpCount*=count) {
    temp += coef[k]*tmpCount;
  }

  return temp;
}

/*!
 * \brief Функция Планка
 * \param waveNum Центральное волновое число канала
 * \param temp    Температура
 */
float HrptCalibr::planckFunc(float waveNum, float temp)
{
  return c1_planck*waveNum*waveNum*waveNum / ( expm1(c2_planck*waveNum/temp) );
}

/*!
 * \brief Преобразование измерений Земли в излучение
 * \param a a коэффициенты
 * \param count значение измерения Земли
 */
float HrptCalibr::sceneRadiance(const QVector<float>& a,float count)
{
  return a[0] + count*(a[1] + a[2]*count);
}

/*! 
 * \brief Convert Earth view radiance to equivalent blackbody temperature
 * \param radiance  Earth view radiance
 * \param t2r Коэффициенты для преобразования
 * \return Радиояркостную температуру. 0, если была ошибка при вычислении
 */
float HrptCalibr::earthTemperature( float radiance, const float t2r[3] )
{
  if (radiance == 0) return 0;
  float logVal =  1 + c1_planck * t2r[0]*t2r[0]*t2r[0]/radiance;
  if ( logVal <= 0  ) {
    return 0;
  }
  return  ( c2_planck * t2r[0] / log(logVal) - t2r[1])/t2r[2];
}

/*!
 * \brief Средняя температура чёрного тела
 * \param weight Веса для каждого PRT
 * \param temp Значения температур для каждого PRT
 * \param num Количество PRT (соотв-но, количество weight и temp)
 * \param warmLoad warm load correction factor
 */
float HrptCalibr::meanBbTemp(const float* weight, float* temp, uint num, float warmLoad)
{
  float weightSum = 0;
  float meanTemp = 0;

  for (uint i = 0; i < num; i++) {
    meanTemp  += weight[i]*temp[i];
    weightSum += weight[i];
  }
  
  if (weightSum ==0) return warmLoad;

  return meanTemp / weightSum + warmLoad;
}



/*!
 * \brief избавление от помех (правило 3 сигма) и усреднение
 * \param counts Данные для усреднения
 * \param n Размерность данных
 * \param factor коэффициент для определения границ попадания 
 * \return true в случае успеха, иначе false
 */
bool HrptCalibr::meanCounts(float* counts, uint n, float* meanCounts, float factor/* =2 */)
{
  float* tmp = new(std::nothrow) float[n];
  if (!tmp) {
    return false;
  }
  uint numTmp = 0;

  //при factor=3, попадут все значения эксперимента; при = 1, при небольших отклонениях не попадёт то, что надо => по умолчанию, factor =2 
  float mean = 0;
  float limit = factor*MnMath::standartDeviation(counts, n, &mean); 

  for (uint j = 0; j < n; j++) {
    if (counts[j] <= (mean + limit) &&
	counts[j] >= (mean - limit)) {
      tmp[numTmp++] = counts[j];
    } 
  }
  
  *meanCounts = MnMath::meanValue(tmp, numTmp);
    
  if (tmp) delete[] tmp;
  return true;
}

