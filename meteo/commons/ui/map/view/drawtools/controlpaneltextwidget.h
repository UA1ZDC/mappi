#ifndef METEO_COMMONS_UI_MAP_VIEW_DRAWTOOLS_CONTROLPANELTEXTWIDGET_H
#define METEO_COMMONS_UI_MAP_VIEW_DRAWTOOLS_CONTROLPANELTEXTWIDGET_H

#include "geoproxywidget.h"

#include <QWidget>
#include <meteo/commons/proto/meteo.pb.h>
#include <commons/geobasis/geopoint.h>

namespace Ui {
class ControlPanelTextWidget;
}

namespace meteo {
namespace map {

class TextPosDlg;
class SymbDlg;

class ControlPanelTextWidget : public QWidget
{
  Q_OBJECT
public:
  explicit ControlPanelTextWidget(QWidget *parent = nullptr);
  ~ControlPanelTextWidget();
  void setParentProxyWidget( QGraphicsProxyWidget* parentProxy );
  void setProperty( const Property& prop );
  Property toProperty() const;

public slots:
  void slotSetProperty(Property prop);
  void slotSetParentProxyPos( GeoPoint point );

private slots:
  void slotFontChanged();
  void slotLockChanged();
  void slotSymbolInsert();
  void slotRamkaChanged();
  void slotPriorChanged();
  void slotDrawAlways();
  void slotTextColorDialog();
  void slotFonColorDialog();
  void slotTextPosChanged();
  void slotSwitchPos();
  void slotSwitchSymb();
  void slotOnAction();
  void slotOffAction();

signals:
  void propertyChange( meteo::Property );
  void insertSymb(QChar);
  void apply();
  void forDelete();

private:
  void defaultProperty();
  QString imgForAlign(int pos);
  void setDrawAlwaysIcon();

private:
  Ui::ControlPanelTextWidget* ui_ = nullptr;
  GeoProxyWidget* parentProxy_ = nullptr;
  Property prop_;
  bool locked_ = true;
  TextPosDlg* posDlg_ = nullptr;
  SymbDlg* symbDlg_ = nullptr;
  QGraphicsProxyWidget* posProxy_ = nullptr;
  QGraphicsProxyWidget* symbProxy_ = nullptr;
  GeoPoint anchorPoint_;
};

}
}

#endif // METEO_COMMONS_UI_MAP_VIEW_DRAWTOOLS_CONTROLPANELTEXTWIDGET_H
