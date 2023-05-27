#ifndef PLOTTINGWIDGET_H
#define PLOTTINGWIDGET_H

#include "isosettingswidget.h"

#include <meteo/commons/ui/map/view/widgets/mapwidget.h>
#include <meteo/commons/ui/settings/settingswidget.h>

#include <meteo/commons/rpc/rpc.h>
#include <meteo/commons/proto/field.pb.h>
#include <meteo/commons/proto/meteo.pb.h>
#include <meteo/commons/proto/weather.pb.h>
#include <commons/funcs/tcolorgrad.h>
#include <meteo/commons/global/gradientparams.h>

class QDoubleSpinBox;

namespace Ui{
 class Plotting;
}

namespace obanal
{
  class TField;
}

namespace meteo {
namespace settings {
namespace internal {
class TMeteoSettings;
}
}

namespace map {

class Map;
class Layer;
class LayerIso;
class MapView;

class ItemWidget : public QWidget
{
  Q_OBJECT
public:
  ItemWidget( QWidget* parent = 0 );
  bool isChecked();
  void setChecked(bool on);

private:
  QToolButton* btn_;

signals:
  void toggled(bool);
};

class TreeWidgetItem : public QTreeWidgetItem {
public:
  TreeWidgetItem(QTreeWidget* parent);
  void setColorMin(const QColor& color);
  void setColorMax(const QColor& color);
  QColor min() const;
  QColor max() const;
private:
  QColor min_;
  QColor max_;
  bool operator<(const QTreeWidgetItem &other)const {
     int column = treeWidget()->sortColumn();
     if( 1 != column ){ //need refactor
       int a = text(column).toInt();
       int b = other.text(column).toInt();
       return a < b;
     }else{
       return QTreeWidgetItem::operator<(other);
     }
  }
};

class PlottingWidget : public MapWidget
{
  Q_OBJECT
public:
  PlottingWidget( MapWindow* parent );
  ~PlottingWidget();

  void setCurrentIsoDescriptor( int32_t descr );
  void setCurrentIsoDescriptor( const QString& name );
  void setCurrentIsoParams( const proto::FieldColor& fieldcolor );
  QString currentIsoName();

  void setLastDataDate(const QString& name);

  const proto::FieldColor& isoParams() const { return isoparams_; }

public slots:
  void updateFields();

protected:
  bool eventFilter( QObject* watched, QEvent* event );
  void closeEvent(QCloseEvent *e);

private:
  Ui::Plotting* ui_;
  QMap<QTreeWidgetItem*, ItemWidget*> itemsWidget_;
  IsoSettingsWidget* isoSettingsWidget_;

  int currentItem_;
  int currentLevel_;
  int currentTypeLevel_;
  QString currentTypeData_;
  int currentDataDescr_;
  int currentHour_;
  GradientParams allparams_;
  proto::FieldColor isoparams_;
  QMap< QString, proto::FieldColor > localparms_;
  QMap< QString, int > localOpacity_;// прозрачность заливки, %
  QList<int> levels_;
  QMap<int, QString> levelTypes_; //!< типы изолиний и их названия 



 // QTreeWidgetItem* search(const QString& text, bool hasLevel);
  meteo::field::DataDescResponse* sendRequest(const QDateTime& start, const QDateTime& end, uint32_t descr);
  void hideItems();
  void updateItems();
  void loadData();
  void setupComboBox();

  void addLayer(QTreeWidgetItem* item);
  void removeLayer(QTreeWidgetItem* item);
  void updateCurrentSettings();
  bool hasLayer(QTreeWidgetItem* item);
  bool hasLayer(Layer* layer);
  bool hasDocument();
  void setupCheckBox();
  Layer* layerByItem(QTreeWidgetItem* item);
  proto::WeatherLayer infoByItem( QTreeWidgetItem* item );

  bool fillGradient(int id, int32_t descr, int level, int levelType, const TColorGrad& defaultColor, int step);

  void sendSelectPointsEvent(QString fieldId) const;

private slots:
  void repaintSelf( bool deletelines = false, bool deletegrad = false );
  void repaintLines(LayerIso* liso);
  void repaintGradient(LayerIso* liso);
  void slotAddLayer();
  void slotDoubleClicked(QTreeWidgetItem *, int);
  void slotContextMenu(QPoint);
  void slotRemoveLayer();
  void slotItemSelectionChanged(QTreeWidgetItem* item, QTreeWidgetItem* prev);
  void slotChangeSelection();
  void updateCurrentLevel();
  void slotSettingsClicked();
  void slotRadioButtonClicked();
  void slotColorMinDialog(const QColor& color);
  void slotColorMaxDialog(const QColor& color);
  void slotStyleChanged(Qt::PenStyle style);
  void slotWidthChanged(int width);
  void slotStepChanged(float step);
  void slotMinChanged(float min);
  void slotMaxChanged(float max);
  void slotGradAlphaChanged();
  void slotSettingsChanged();
  void slotSorting(int);
  void slotSwitchLayer(bool);
  void slotHourChanged(const QString& text);
  void slotColorMinChanged();
  void slotColorMaxChanged();
  void slotStyleChanged();
  void slotWidthChanged();
  void slotStepChanged();
  void slotStepCustomChanged();
  void typeDataChange(const QString &at);
  void slotColorFillDialog();
  void updateParams();
};

}
}

#endif // PLOTTINGWIDGET_H
