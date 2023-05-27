#ifndef MAPPI_UI_SATELLITEIMAGE_SATLEGEND_H
#define MAPPI_UI_SATELLITEIMAGE_SATLEGEND_H

#include <commons/funcs/tcolorgrad.h>
#include <meteo/commons/ui/map/legend.h>

namespace meteo {
namespace map {

class SatLegend : public Legend
{
public:
  SatLegend(Document* doc);

protected:
  virtual bool buildPixmap();

private:
  TColorGradList palette_;
};

} // map
} // meteo

#endif // MAPPI_UI_SATELLITEIMAGE_SATLEGEND_H
