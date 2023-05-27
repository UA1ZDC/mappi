#ifndef MAPPI_UI_SATLAYER_SATLAYER_H
#define MAPPI_UI_SATLAYER_SATLAYER_H

#include <commons/funcs/tcolorgrad.h>
#include <sat-commons/satellite/satellitebase.h>

#include <meteo/commons/ui/map/layer.h>

#include <mappi/proto/satelliteimage.pb.h>
#include <mappi/ui/pos/posgrid.h>

namespace meteo {
namespace map {

class Document;
class LayerMenu;
class SatelliteImage;

} // map
} // meteo

namespace meteo {
namespace map{

class SatLayer: public Layer
{
public:
  SatLayer(Document* map, const QString& name = QString());
  virtual ~SatLayer();

  enum {
	Type = meteo::kLayerSatellite
  };
  int type() const { return Type; }

  void setThematicType(::mappi::conf::ThemType type) { params_.set_type(type); }
  ::mappi::conf::ThemType thematicType() const { return params_.type(); }

  void setThematicTypeName(const QString& name) { thematicTypeName_ = name; }
  QString thematicTypeName() const { return thematicTypeName_; }

  void setPalette(const TColorGradList& palette) { palette_ = palette; }
  TColorGradList palette() const { return palette_; }

  void setSessionParams(const ::mappi::proto::SatelliteImage& param) { params_ = param; }
  const ::mappi::proto::SatelliteImage& sessionParams() const { return params_; }

  void setTLE(const ::MnSat::TLEParams& tle) { tle_ = tle; }
  const ::MnSat::TLEParams& tle() const { return tle_; }

  PosGrid* setProjection(PosGrid* projection) { PosGrid* o = proj_; proj_ = projection; return o; }
  PosGrid* projection() const { return proj_; }

  SatelliteImage* currentObject() const;

  virtual LayerMenu* layerMenu();

private:
  QString thematicTypeName_;
  TColorGradList palette_;
  ::mappi::proto::SatelliteImage params_;
  ::MnSat::TLEParams tle_;

  PosGrid* proj_ = nullptr;
  LayerMenu* menu_ = nullptr;
};

} //map
} //meteo

#endif // MAPPI_UI_SATLAYER_SATLAYER_H
