#ifndef MAPPI_UI_PLUGINS_SESSIONVIEWERPLUGIN_GEORASTR_H
#define MAPPI_UI_PLUGINS_SESSIONVIEWERPLUGIN_GEORASTR_H

#include <sat-commons/satellite/satviewpoint.h>
#include <commons/funcs/tcolorgrad.h>

#include <meteo/commons/ui/map/object.h>
#include <meteo/commons/ui/map/document.h>
#include <meteo/commons/proto/map_document.pb.h>

#include <mappi/global/streamheader.h>
#include <mappi/proto/satelliteimage.pb.h>
#include <mappi/ui/pos/posgrid.h>

#include "satelliteimage.h"

namespace meteo {
namespace map {
class GeoRastr : public map::SatelliteImage
{
public:
  GeoRastr(map::Layer* layer);
  GeoRastr(meteo::Projection* projection);
  GeoRastr(Object* parent);
  virtual ~GeoRastr();

  enum {
    kType = UserType + 11,
  };
  virtual int type() const { return kType; }

  virtual Object* copy(map::Layer* l) const;
  virtual Object* copy(Object* o ) const;
  virtual Object* copy( meteo::Projection* grid ) const;

  virtual double coordsValue(const meteo::GeoPoint& gp, bool* ok = 0) const;

  virtual bool load(const QString& fileName);

  virtual void buildCache();
  virtual void resetCache();
  QString getPretName()const;
  QString getPretUnit()const;

private:
  double readValue(QDataStream* ds, int type) const;

private:
  ::meteo::global::PoHeader header_;

  QPoint cnt_;

  int headerSize_ = -1;
};

} // map
} // meteo

#endif // MAPPI_UI_PLUGINS_SESSIONVIEWERPLUGIN_GEORASTR_H
