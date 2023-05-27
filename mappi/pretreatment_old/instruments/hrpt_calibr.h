#ifndef MAPPI_PRETREATMENT_INSTRUMENTS_HRPT_CALIBR_H
#define MAPPI_PRETREATMENT_INSTRUMENTS_HRPT_CALIBR_H


//! Набор общих функций для калибровки
namespace HrptCalibr {
  bool meanCounts(float* counts, uint n, float* meanCounts, float factor = 2);
  float planckFunc(float waveNum, float temp);
  float prtTemp(const float* coef, float count, uint num );
  float sceneRadiance(const QVector<float>& a, float count);
  float meanBbTemp(const float* weight, float* temp, uint num, float warmLoad);
  float earthTemperature( float radiance, const float t2r[3] );
};



#endif
