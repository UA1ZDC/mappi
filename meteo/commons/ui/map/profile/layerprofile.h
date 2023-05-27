#ifndef METEO_COMMONS_UI_MAP_PROFILE_LAYERPROFILE_H
#define METEO_COMMONS_UI_MAP_PROFILE_LAYERPROFILE_H

#include <commons/meteo_data/meteo_data.h>
#include <commons/meteo_data/tmeteodescr.h>

#include <meteo/commons/proto/meteo.pb.h>
#include <meteo/commons/proto/puanson.pb.h>
#include <meteo/commons/ui/map/layer.h>
#include <meteo/commons/ui/map/profile/axisprofile.h>
#include <meteo/commons/ui/map/axis/layeraxis.h>

namespace meteo {
namespace map {

class GeoPolygon;
class Property;
class Document;
class AxisProfile;

} // map
} // meteo

namespace meteo {
namespace map {

class LayerProfile: public LayerAxis
{
public:
  enum { Type = LayerType_MAX + 202 };

  LayerProfile( Document* d, const QString& name = QString());
  virtual ~LayerProfile();

  virtual int type() const { return Type; }

  QString addProfile(float x, const QVector<float>& levels, float angle = 0);
  QVector<TMeteoData> getDataByCoords(const QVector<GeoPoint>& coords);

  void setPuanson(const ::meteo::puanson::proto::Puanson& puanson) { puanson_ = puanson; }

//  virtual int32_t dataSize() const;
//  virtual int32_t serializeToArray(char* data) const;
//  virtual int32_t parseFromArray(const char* data);

private:
  ::meteo::puanson::proto::Puanson puanson_;
};

} // map
} // meteo

#endif // METEO_COMMONS_UI_MAP_PROFILE_LAYERPROFILE_H
