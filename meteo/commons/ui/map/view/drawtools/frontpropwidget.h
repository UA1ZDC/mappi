#ifndef METEO_COMMONS_UI_MAP_VIEW_DRAWTOOLS_FRONTPROPWIDGET_H
#define METEO_COMMONS_UI_MAP_VIEW_DRAWTOOLS_FRONTPROPWIDGET_H

#include <qstandarditemmodel.h>

#include <meteo/commons/proto/meteo.pb.h>

#include "propwidget.h"

class QColorDialog;
class QPushButton;
class QToolButton;

namespace Ui {
class FrontPropWidget;
}

namespace meteo {
namespace map {

struct FrontInfo {
  QString title;
  meteo::Property prop;
};

//!
class FrontPropWidget : public PropWidget
{
  Q_OBJECT

  enum Role {
    kTitleRole      = Qt::DisplayRole,
    kNameRole       = Qt::UserRole,
  };

public:
  FrontPropWidget(QWidget* parent = nullptr);
  virtual ~FrontPropWidget();

  virtual meteo::Property toProperty() const;
  virtual void setProperty(const meteo::Property& prop);

public slots:
  void slotOnDelBtn();
  void slotOffDelBtn();
  void slotSaveSettings();

private:
  int factor() const;

  void initOrnaments();
  void initWidget() {}
  void selectFrontByName(const QString& name);

  void updateIco(QToolButton* btn);
  void loadSettings();
  void saveSettings(const meteo::Property &prop);

  static bool fF2X_one( const Projection& proj, const GeoPoint& geoCoord, QPoint* meterCoord );
  static bool fX2F_one( const Projection& proj, const QPoint& meterCoord, GeoPoint* geoCoord );
  static bool fF2X_onef( const Projection& proj, const GeoPoint& geoCoord, QPointF* meterCoord );
  static bool fX2F_onef( const Projection& proj, const QPointF& meterCoord, GeoPoint* geoCoord );

private:
  Ui::FrontPropWidget* ui_;
  mutable meteo::Property prop_;
  QMap<QString,FrontInfo>  presets_;
};

} // map
} // meteo

#endif // METEO_COMMONS_UI_MAP_VIEW_DRAWTOOLS_FRONTPROPWIDGET_H
