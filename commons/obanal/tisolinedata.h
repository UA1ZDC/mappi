#ifndef TISOLINEDATA_H
#define TISOLINEDATA_H


#include <commons/geobasis/geovector.h>
#include <commons/obanal/tfield.h>

#include <QPolygonF>

const int max_point_in_iso = 10000;
const int min_point_in_iso = 3;
int Num3_Elem(int, int, int, int, int);
int Num2_Elem(int, int, int);

class TIsoLineData
{

public:
  TIsoLineData(const obanal::TField*);
  virtual ~TIsoLineData();

  int makeOne(const float& val, QVector<meteo::GeoVector> *isolines);
  int makeOne1( float val, QVector<meteo::GeoVector> *isolines);
  int makeAll(int iso_min, int iso_max, int step,meteo::GeoVector *);
    
  /*void Iso_Vspom(float, float *, float *, float *, bool *, char *, float *,
                 int, int, float);*/
  int One_Iso(int, int, int, const float *, const float *, char *, float *, float *, int, int, int);
  int One_Iso_Sphere(int, int, int, const float *, const float *, char *, float *, float *, int, int, int);
  
  
  void Iso_Vspom(float iso, const float *netx, const float *nety, const float *fun, const bool *obres,
                 char *indf, float *argf, int kolx,  int koly);

private:
  int prepIso(float iso);
  int oneIsoSphere(int j, int k, int l, meteo::GeoVector &);
  int oneIso(int j, int k, int l,meteo::GeoVector &);

  // int n2e(int i, int j){ return i*_kolyn2+j; }
  int n3e(int i, int j, int k){return (i*_kolyn3+j)*4+k;}

  
  const obanal::TField* field_;
  QVector<char> indf_;
  QVector<float> argf_;
  
  int _kolyn2;
  int _kolyn3;
};

#endif // TISOLINE_H
