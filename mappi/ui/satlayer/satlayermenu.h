#ifndef MAPPI_UI_SATLAYER_SATLAYERMENU_H
#define MAPPI_UI_SATLAYER_SATLAYERMENU_H

#include <qobject.h>
#include <qlist.h>

#include <meteo/commons/proto/weather.pb.h>
#include <meteo/commons/ui/map/layermenu.h>

class QAction;
class QMenu;
class QWidgetAction;
class QPixmap;

namespace meteo {
namespace map {

class Layer;
class DoubleSpinAction;

class SatLayerMenu : public LayerMenu
{
  Q_OBJECT

public:
  SatLayerMenu( Layer* l, QObject* parent = 0 );
  ~SatLayerMenu();

  void addActions( QMenu* menu );

  static QPixmap createPixmap(Qt::PenStyle style, int width, bool isSelected, QString text = QString::null);
  static void changeStylePixmap(QMenu* act, int val);
  static void changeWidthPixmap(QMenu* act, int oldval);

private slots:
  void slotColorFillDialog();

private:
  QAction* paletteAct_;
};

} // map
} // meteo

#endif // MAPPI_UI_SATLAYER_SATLAYERMENU_H
