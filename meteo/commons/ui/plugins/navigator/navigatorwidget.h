#ifndef METEO_COMMONS_UI_MAP_NAVIGATOR_NAVIGATOR_WIDGET_H
#define METEO_COMMONS_UI_MAP_NAVIGATOR_NAVIGATOR_WIDGET_H

#include <QPixmap>
#include <QTransform>

#include <set>

#include <commons/geobasis/geopoint.h>
#include <commons/geobasis/projection.h>
#include <meteo/commons/ui/map/view/widgets/mapwidget.h>

class QColor;
class QEvent;
class QLabel;
class QPixmap;
class QSize;

namespace meteo {

class GeoVector;

namespace map {

class Document;
class MapWindow;

class NavigatorWidget : public MapWidget
{
  Q_OBJECT

public:
  explicit NavigatorWidget(MapWindow* window);
  ~NavigatorWidget();

  void turnSelectingPosition(bool active);

  void initFromDocument(Document* doc);

  const GeoPoint currentPosition() const;
  void setCurrentPosition(const GeoPoint& pos);
  void setDisplayRamka(const GeoVector& ramka);
  void clearCurrentPosition();

  bool checkGeoPoint(const GeoPoint& point) const;
  void setGeoPoint(const GeoPoint& point);
  void clearGeoPoint(const GeoPoint& point);
  void clearAllPoints();

  void setGeoVector(const GeoVector& points);
  void clearGeoVector(const GeoVector& points);

  // Установка свойств отрисовки объектов:
  // (все функции возвращают предыдущее значение свойства)
  static QColor setBackgroundColor(const QColor& color);
  static QColor setPointsColor(const QColor& color);
  static QColor setMarkerColor(const QColor& color);
  static int setPointsPenWidth(int pixel);
  static int setMarkerPenWidth(int pixel);
  static int setMarkerRadius(int pixel);

signals:
  void selectedPosition(const meteo::GeoPoint& pos);

private:
  bool setGeoPoint(const GeoPoint& point, bool visible);
  void setGeoVector(const GeoVector& points, bool visible);

  bool eventFilter(QObject* obj, QEvent* event);

  bool isValidGeoPoint(const GeoPoint& pos) const;

  void reDraw();

  void makeOriginalPixmap();
  void makePointsPixmap();
  void changePointsPixmap(const GeoVector& points, bool visible);
  void drawVisiblePoints(QPixmap* target);
  void drawPositionMarker(QPixmap* target);
  void drawDisplayRamka(QPixmap* target);

private:
  QLabel* paintArea_;
  Document* currentDoc_;

  std::set<GeoPoint> visiblePoints_;
  GeoPoint markerPosition_;
  GeoVector displayRamka_;

  QPixmap originalMap_;
  QPixmap pointsMap_;

  QTransform doc2pixmap_;
  QTransform pixmap2doc_;

  bool needReDraw_;

};

} // map
} // meteo

#endif // METEO_COMMONS_UI_MAP_NAVIGATOR_NAVIGATOR_WIDGET_H
