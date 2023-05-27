#ifndef METEO_COMMONS_UI_MAP_VIEW_DRAWTOOLS_POLYGONPROPWIDGET_H
#define METEO_COMMONS_UI_MAP_VIEW_DRAWTOOLS_POLYGONPROPWIDGET_H

#include "propwidget.h"
#include "arrowdlg.h"

#include <qgraphicsproxywidget.h>

class QColorDialog;
class QPushButton;

namespace Ui {
class PolygonPropWidget;
}

namespace meteo {
namespace map {

//!
class PolygonPropWidget : public PropWidget
{
  Q_OBJECT

  enum Role { kTitleRole = Qt::DisplayRole,
              kNameRole  = Qt::UserRole,
              kIcoPath,
            };

public:
  enum Preset { kLine, kPolygon };

  PolygonPropWidget(Preset preset, QWidget* parent = nullptr);
  virtual ~PolygonPropWidget();

  virtual bool hasValue() const;
  virtual void setValue(double value, bool enable= true);
  virtual double value() const;
  virtual QString unit() const;

  virtual meteo::Property toProperty() const;
  virtual void setProperty(const meteo::Property& prop);
  virtual void initWidget();

protected:
  virtual bool eventFilter(QObject* obj, QEvent* e);

public slots:
  void slotOnDelBtn();
  void slotOffDelBtn();
  void slotSaveSettings();

private slots:
  void slotSelectColor();
  void slotInitPatterns();
  void slotInitArrowPlaces();
  void slotUpdateFillBtnPalette(bool enable);
  void slotShowArrowDlg();
  void slotArrowTypeChanged();
  void slotLabelSwitch(bool enable);
  void slotChangeLock();

private:
  int lineWidth() const;
  bool closedFlag() const;
  Qt::PenStyle lineStyle() const;
  bool isDouble() const;
  int factor() const;
  QColor lineColor() const;
  QColor fillColor() const;
  Qt::BrushStyle fillStyle() const;
  ArrowPlace arrowPlace() const;
  ArrowType arrowType() const;

  void setLineColor(const QColor& color);
  void setFillColor(const QColor& color);
  void setPatternStyle(Qt::BrushStyle style);
  void updateIco(QPushButton* btn);

  void loadOrnaments();
  void fillLineCombo();
  void loadSettings();
  void saveSettings(const meteo::Property &prop);

  static bool fF2X_one( const Projection& proj, const GeoPoint& geoCoord, QPoint* meterCoord );
  static bool fX2F_one( const Projection& proj, const QPoint& meterCoord, GeoPoint* geoCoord );
  static bool fF2X_onef( const Projection& proj, const GeoPoint& geoCoord, QPointF* meterCoord );
  static bool fX2F_onef( const Projection& proj, const QPointF& meterCoord, GeoPoint* geoCoord );

private:
  Ui::PolygonPropWidget* ui_;
  QGraphicsProxyWidget* arrowProxy_ = nullptr;
  ArrowDlg* arrowDlg_ = nullptr;
  bool locked_ = false;
  Preset preset_;
};

} // map
} // meteo

#endif // METEO_COMMONS_UI_MAP_VIEW_DRAWTOOLS_POLYGONPROPWIDGET_H
