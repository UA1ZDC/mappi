#ifndef METEO_COMMONS_UI_VERTICALCUT_CORE_LAYER_CUT_H
#define METEO_COMMONS_UI_VERTICALCUT_CORE_LAYER_CUT_H

#include <commons/funcs/tcolorgrad.h>

#include <meteo/commons/proto/meteo.pb.h>
#include <meteo/commons/ui/map/layerpunch.h>
#include <meteo/commons/ui/map/layeriso.h>

#include "vprofile.h"

namespace meteo {


namespace map {

class LayerVProfile: public LayerPunch
{
public:
  enum { Type = LayerType_MAX + 1 };

  LayerVProfile( Document* d, const QString& name = QString() );
  virtual ~LayerVProfile() {}

  virtual int type() const { return Type; }

  void setPlaceData(const QList<zond::PlaceData>&, const QVector<float> &levels);
  void addWind(const QList<zond::PlaceData>& data, const QVector<float>& levels);
  void addWind(LayerIso* ddLayer, LayerIso* ffLayer, const GeoPoint& coord, float angle = 0);

//private:
 // QList<zond::PlaceData*> places_;
};

} // map
} // meteo

#endif // METEO_COMMONS_UI_VERTICALCUT_CORE_LAYER_CUT_H
