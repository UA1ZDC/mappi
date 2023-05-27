#ifndef DRAWWIDGET_H
#define DRAWWIDGET_H

#include <QtGui>
#include "../mapwidget.h"
#include <meteo/commons/ui/map/document.h>
#include <meteo/commons/ui/map/object.h>

namespace Ui{
 class DrawTools;
}

namespace meteo {
namespace map {

class Map;
class Layer;
class MapView;

class DrawWidget : public MapWidget
{
  Q_OBJECT
public:
  explicit DrawWidget(QWidget *parent = 0);
  ~DrawWidget();

private:
  Ui::DrawTools* ui_;
  Document* document_;

private slots:
};

} //map
} //meteo

#endif // DRAWWIDGET_H
