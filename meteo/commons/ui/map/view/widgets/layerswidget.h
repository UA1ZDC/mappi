#ifndef LAYERSWIDGET_H
#define LAYERSWIDGET_H

#include <qtoolbutton.h>
#include <qtreewidget.h>
#include <qspinbox.h>

#include <meteo/commons/ui/map/document.h>
#include <meteo/commons/ui/map/layeriso.h>

#include <meteo/commons/ui/map/view/menu.h>
#include <meteo/commons/ui/map/view/widgets/mapwidget.h>

namespace Ui{
 class Layers;
}

class AlphaEdit;

namespace meteo {
namespace map {

class Map;
class Layer;
class DoubleSpinAction;

namespace internal {
class LayerItemWidget : public QWidget
{
  Q_OBJECT
public:
  LayerItemWidget( Layer* l, QTreeWidgetItem* item, QWidget* parent = 0 );
  ~LayerItemWidget();
  bool isChecked();
  void setChecked(bool on);

  Layer* layer() const { return layer_; }
  QTreeWidgetItem* item() const { return item_; }

private:
  QToolButton* btn_;
  Layer* layer_;
  QTreeWidgetItem* item_;

signals:
  void toggled(bool);
};
}

class LayersWidget : public MapWidget
{
  Q_OBJECT
public:
  LayersWidget( MapWindow* view );
  ~LayersWidget();
  void updateLayerItems();

  void setDeleteVisible( bool fl );

protected:
  void showEvent( QShowEvent* event );
  bool eventFilter( QObject* watched, QEvent* event );

private:
  Ui::Layers* ui_;
  bool deletevisible_;

  QDoubleSpinBox* stepedit_;
  Layer* lastselected_;
  int lastpos_;


  bool hasDocument();
  void updateActiveLayer();

  void setSelectedLastLayer();

  void saveVisibilitySettings();

  QTreeWidgetItem* layerItem( Layer* l ) const ;
  Layer* itemLayer( QTreeWidgetItem* i ) const ;

private slots:
  void slotItemSelected();
  void slotLayerUp();
  void slotLayerDown();
  void slotLayerRemove();
  void slotLayerActive();
  void slotLayerVisibility(bool on);
  void slotContextMenu(QPoint pnt);
};

}
}

#endif
