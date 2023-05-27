#ifndef METEO_COMMONS_SERVICES_OBANAL_TOBANAL_H
#define METEO_COMMONS_SERVICES_OBANAL_TOBANAL_H

#include <memory>
#include <stdint.h>

#include <commons/obanal/tfield.h>

#include "tobanaldb.h"
#include "tfield_calc.h"

struct RegionParam;

class TObanal
{
  using ObanalDbPtr      = std::shared_ptr<TObanalDb>;
  using FieldCalcPtr     = std::unique_ptr<obanal::TFieldCalc>;

  ObanalDbPtr      db_;
  FieldCalcPtr     fa_;

public:
  TObanal();
  ~TObanal();

 bool calculateAndSaveGrib(meteo::GeoData* all_data, const QVector<RegionParam>& corners,
                            meteo::field::DataDesc* fdescr);
 bool calculateAndSaveField(meteo::GeoData* all_data,  meteo::field::DataDesc*);
private:
 QString createInfo( meteo::field::DataDesc* fdescr, int);
 void loadSettings();

 QMap <QVariant,float> grib_factors_map_; // множитель для преобразования значения величины перед интерполяцией
 QMap <QVariant,float> src_factors_map_; // множитель для преобразования значения величины перед интерполяцией
 QMap <QVariant,float> predel_grad_map_; // предельный градиент(на градус) величины для градиентного контроля
 QList <QVariant> land_sea_mask_need_; // список дескрипторов, для которых отображение происходит только на воде
 QMap <QVariant,float> deltas_map_; // дельта для преобразования значения величины перед интерполяцией

 float rast_        = 5;
 int kolstan_       = 20;
 int kolstmin_      = 3;
 float rastfrprorej_=0.5;
 float shir_skl_    =0.261799;


};

#endif // METEO_COMMONS_SERVICES_OBANAL_TOBANAL_H
