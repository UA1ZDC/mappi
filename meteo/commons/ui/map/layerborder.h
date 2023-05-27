#ifndef METEO_COMMONS_UI_MAP_LAYERBORDER_H
#define METEO_COMMONS_UI_MAP_LAYERBORDER_H

#include <meteo/commons/ui/map/layerpunch.h>

namespace meteo {
namespace map {

class LayerBorder : public Layer
{
public:
  LayerBorder(Document* map);
  enum {
    Type = kLayerBorder
  };
  int type() const override { return Type; }
  LayerMenu* layerMenu() override;
  void changeColor(QColor color);
  void changeWidth(int value);
  void changeStyle(Qt::PenStyle style);
  void loadParams();

};

}
}

#endif // METEO_COMMONS_UI_MAP_LAYERBORDER_H
