#ifndef GRIBDATAWIDGET_H
#define GRIBDATAWIDGET_H

#include <meteo/commons/ui/map/view/widgets/mapwidget.h>
#include <meteo/commons/rpc/rpc.h>
#include <meteo/commons/proto/field.pb.h>
#include <meteo/commons/proto/meteo.pb.h>
#include <commons/funcs/tcolorgrad.h>

namespace Ui{
 class GribData;
}

namespace meteo {
namespace map {

class Map;
class Layer;
class MapView;

class GribDataWidget : public MapWidget
{
  Q_OBJECT
public:
  explicit GribDataWidget(QWidget *parent = 0, const QString& options = 0);
  ~GribDataWidget();
  void setOptions(const QString& options);

protected:
  bool eventFilter( QObject* watched, QEvent* event );

private:
  Ui::GribData* ui_;
  rpc::Channel* channel_;
  rpc::Channel* sprinf_;
  QMenu *menu_;
  QMap<QTreeWidgetItem*, Layer*> layers_;
  bool levels_;

  QString currentLevel_;
  QMap<int, QString> desc_;
  //QMap<QTreeWidgetItem*, ItemWidget*> itemsWidget_;
  QMultiMap<int,QTreeWidgetItem*> centers_;
  MapView* view_;

  QTreeWidgetItem* search(const QString& text, bool hasLevel);
  meteo::field::DataDescResponse* sendRequest();
  void hideItems();
  void updateItems();
  bool hasLevels();
  void addLayer(QTreeWidgetItem* item);
  void removeLayer(QTreeWidgetItem* item);
  void hideNoLevelItems();//need refactor
  bool hasDocument();

private slots:
  void slotAddLayer();
  void slotContextMenu(QPoint);
  void slotRemoveLayer();
  void slotItemSelectionChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);
  void updateFields();
  void slotRadioButtonClicked();
  void slotSorting(int);
  void slotSwitchLayer(bool);
  void slotHourChanged(const QString& text);
};

}
}

#endif // GribDataWidget_H
