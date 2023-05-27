#ifndef METEO_NOVOST_UI_CUSTOM_GEOPOINTEDITOR_H
#define METEO_NOVOST_UI_CUSTOM_GEOPOINTEDITOR_H

#include <qwidget.h>
#include <qmdisubwindow.h>

#include <commons/geobasis/geopoint.h>
#include <meteo/commons/ui/map/view/actions/traceaction.h>

namespace Ui {
class GeoPointEditor;
}

class GeoPointEditor : public QWidget
{
  Q_OBJECT

  Q_PROPERTY(double latitudeDecDegree READ latDegree WRITE setLatDegree)
  Q_PROPERTY(double longitudeDecDegree READ lonDegree WRITE setLonDegree)
  Q_PROPERTY(double altitudeMeter READ altitudeMeter WRITE setAltitudeMeter)
  Q_PROPERTY(double latitudeRadian READ latRadian WRITE setLatRadian)
  Q_PROPERTY(double longitudeRadian READ lonRadian WRITE setLonRadian)
  Q_PROPERTY(QString latitudeDegMinSec READ latDegMinSec WRITE setLatDegMinSec)
  Q_PROPERTY(QString longitudeDegMinSec READ lonDegMinSec WRITE setLonDegMinSec)
  Q_PROPERTY(bool oneLine READ isOneLine WRITE setOneLine)
  Q_PROPERTY(bool labelVisible READ isLabelVisible WRITE setLabelVisible)
  Q_PROPERTY(CoordFormat coordFormat READ coordFormat WRITE setCoordFormat)
  Q_PROPERTY(bool readOnly READ isReadOnly WRITE setReadOnly)
  Q_PROPERTY(bool mapButtonVisible READ isMapButtonVisible WRITE setMapButtonVisible)
  Q_PROPERTY(bool altitudeVisible READ isAltitudeVisible WRITE setAltitudeVisible)

  Q_ENUMS(CoordFormat)

public:
  enum CoordFormat {
    kDecDegree,
    kDegMinSec
  };

  static const QList<GeoPointEditor*>& getAllEditors();

  GeoPointEditor( QWidget* parent = nullptr);
  GeoPointEditor(GeoPointEditor*) = delete;
  GeoPointEditor(GeoPointEditor&) = delete;
  virtual ~GeoPointEditor();

  void setCoord(const meteo::GeoPoint& pnt);

  void setLatRadian(double lat);
  void setLonRadian(double lon);

  void setLatDegree(double lat);
  void setLonDegree(double lon);

  void setAltitudeMeter(double alt);

  meteo::GeoPoint coord() const ;

  double latRadian() const ;
  double lonRadian() const ;

  double latDegree() const ;
  double lonDegree() const ;

  double altitudeMeter() const;

  QString latDegMinSec() const;
  void setLatDegMinSec(const QString& dms);

  QString lonDegMinSec() const;
  void setLonDegMinSec(const QString& dms);

  meteo::map::TraceAction* getAssociatedAction();
  meteo::map::MapScene* getAssociatedScene();

  //! Возвращает true, если все виджеты редактора отображаются в одну строку.
  bool isOneLine() const { return oneLine_; }
  //! Если flag равен true, то всё виджеты редактора будут отображаться в одну строку.
  void setOneLine(bool flag);

  //! Возвращает true, если подписи отображаются, иначе false.
  bool isLabelVisible() const { return labelVisible_; }
  //! Скрывает или показывает подписи, в соответствии со значением visible.
  void setLabelVisible(bool visible);

  //! Возвращает текущий формат ввода и отображения координат.
  CoordFormat coordFormat() const { return format_; }
  //! Устанавливает формат ввода и отображения координат.
  void setCoordFormat(CoordFormat format);

  //!
  void setReadOnly(bool readOnly);
  //!
  bool isReadOnly() const { return readOnly_; }

  //! Возвращает true, если кнопка запроса координат с карты отображается.
  bool isMapButtonPressed() const;

  //! Возвращает true, если кнопка запроса координат с карты отображается.
  bool isMapButtonVisible() const;
  //! Изменяет видимость кнопки получения координат с карты на visible.
  void setMapButtonVisible(bool visible);

  void setSearchVisible(bool visible);
  bool isSearchVisible() const;
  bool isSearchChecked() const;
  bool isSearchEnabled() const;
  //!
  bool isAltitudeVisible() const;
  //!
  void setAltitudeVisible(bool visible);

  // блокируем внутенние сигналы
  void blockAllSignals(bool bl);

  void setPropertyError( const QString st, const bool bl );

signals:
  void tryChanged(); //!< Сигнал генерируется при изменении на форме, исключая editingFinished
  void changed();

  void coordChanged(const meteo::GeoPoint& coord);  
  void signalOnMap(bool isOnMap);

public slots:
  void slotUpdateValue();

private slots:
  void slotMapActionDestroyed(QObject* obj);
  void slotOnMapRequest(bool state);
  void setMapBtnPressed(bool state);

private:
  void openMapWindow();
  void restoreFromMapWindow();
  void unpressMapRequestBtn();  
  void onCoordChanged();

private:

  static QList<GeoPointEditor*> allCreatedGeoPointEditors_;

  Ui::GeoPointEditor* ui_;
  bool oneLine_;
  bool labelVisible_;
  CoordFormat format_;
  bool readOnly_;

  QMdiSubWindow* mapSwitchLastActiveMdiSubwindow_;
  QMdiSubWindow* mapSwitchCurrentActiveMdiSubwindow_;
  meteo::map::TraceAction* mapAction_;
};


#endif
