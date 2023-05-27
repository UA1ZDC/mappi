#ifndef TRANSPARENCYSLIDER_H
#define TRANSPARENCYSLIDER_H

#include "qwidget.h"
#include <meteo/commons/ui/map/view/widgets/mapwidget.h>
#include <meteo/commons/ui/custom/doubleslider.h>

namespace Ui {
class TransparencySlider;
}

namespace meteo{
namespace map{

class Layer;

class TransparencySlider : public MapWidget
{
  Q_OBJECT

public:
  explicit TransparencySlider(MapWindow *window);
  ~TransparencySlider();

  int blackValue() const;
  int whiteValue() const;

private slots:
  void slotRepaint();
  void slotLayerChanged(Layer* layer, int ch);
  void slotListChanged();
  void slotLockLayer();

private:
  Ui::TransparencySlider *ui = nullptr;
  DoubleSlider* sldr_ = nullptr;
  bool valueChanged_ = false;

  int allBlack_ = 0;
  int allWhite_ = 255;


  static const QString kAll;
};

} //map
} //meteo

#endif // TRANSPARENCYSLIDER_H
