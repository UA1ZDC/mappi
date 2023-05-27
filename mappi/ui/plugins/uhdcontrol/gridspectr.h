#ifndef MAPPI_UI_PLUGINS_UHDCONTROL_GRIDSPECTR_H
#define MAPPI_UI_PLUGINS_UHDCONTROL_GRIDSPECTR_H

#include <qobject.h>
#include <qhash.h>
#include <meteo/commons/ui/map/view/mapscene.h>

namespace meteo {
class AxisSpectr;
} // meteo

namespace meteo {
namespace map {
class Document;
class Object;
} // map
} // meteo

namespace meteo {
//! Класс GridSpectr отвечает за формирование координатной сетки документа.
class GridSpectr : public QObject
{
  Q_OBJECT
public:
  explicit GridSpectr(map::Document* doc = 0, QObject *parent = 0);

  QList<const AxisSpectr*> visibleAxes(int axisPosition = -1) const;

  void calcGrid();
  void updateGrid();
  void setMapScene(map::MapScene* scene) { scene_ = scene; }
protected:
  virtual bool eventFilter(QObject* obj, QEvent* event);

private:
  map::MapScene* scene_ = nullptr;
  map::Document* document_;

  QString layerUuid_;

  QHash<const AxisSpectr*, QHash<int, QList<map::Object*> > > gridLines_;
  QHash<const AxisSpectr*,QString> layerAxes_;
};

} // meteo

#endif // MAPPI_UI_PLUGINS_UHDCONTROL_GRIDSPECTR_H
