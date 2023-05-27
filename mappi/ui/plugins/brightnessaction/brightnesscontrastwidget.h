#ifndef BRIGHTNESSCONTRASTWIDGET_H
#define BRIGHTNESSCONTRASTWIDGET_H

#include "qwidget.h"

#include <meteo/commons/ui/map/view/widgets/mapwidget.h>
#include <meteo/commons/ui/custom/doubleslider.h>

namespace Ui {
class BrightnessContrastWidget;
}

namespace meteo {
namespace map {

class Layer;

class BrightnessContrastWidget : public MapWidget
{
  Q_OBJECT

public:
  explicit BrightnessContrastWidget(MapWindow* window);
  ~BrightnessContrastWidget();

private slots:
  void slotRepaint();
  void slotLayerChanged(Layer* layer, int ch);
  void slotListChanged();
  void slotLockLayer();

private:
  Ui::BrightnessContrastWidget *ui;

  DoubleSlider* brightnessSlider_;
  DoubleSlider* contrastSlider_;

  int allContrast_ = 100;
  int allBrightness_ = 0;

  static const QString kAll;
};

}
}


#endif // BRIGHTNESSCONTRASTWIDGET_H
