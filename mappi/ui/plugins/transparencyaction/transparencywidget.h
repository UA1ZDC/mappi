#ifndef TRANSPARENCYWIDGET_H
#define TRANSPARENCYWIDGET_H

#include "qwidget.h"

#include <meteo/commons/ui/map/view/widgets/mapwidget.h>
#include <meteo/commons/ui/custom/doubleslider.h>

namespace Ui {
class TransparencyWidget;
}

namespace meteo {
namespace map {

class Layer;

class TransparencyWidget : public MapWidget
{
  Q_OBJECT

public:
  explicit TransparencyWidget(MapWindow* window);
  ~TransparencyWidget();

private slots:
  void slotRepaint();
  void slotLayerChanged(Layer* layer, int ch);
  void slotListChanged();
  void slotLockLayer();

private:
  Ui::TransparencyWidget *ui;
  DoubleSlider* transparencySlider_;

  int allTransparency_ = 255;

  static const QString kAll;
};

}
}

#endif // TRANSPARENCYWIDGET_H
