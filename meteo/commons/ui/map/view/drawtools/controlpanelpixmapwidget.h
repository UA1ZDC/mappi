#ifndef METEO_COMMONS_UI_MAP_VIEW_DRAWTOOLS_CONTROLPANELPIXMAPWIDGET_H
#define METEO_COMMONS_UI_MAP_VIEW_DRAWTOOLS_CONTROLPANELPIXMAPWIDGET_H

#include "geoproxywidget.h"

#include <QWidget>
#include <meteo/commons/proto/meteo.pb.h>
#include <commons/geobasis/geopoint.h>


class QSettings;

namespace Ui {
class ControlPanelPixmapWidget;
}


namespace meteo {
namespace map {

class IconsetWidget;
class TextPosDlg;

class ControlPanelPixmapWidget : public QWidget
{
  Q_OBJECT

public:
  explicit ControlPanelPixmapWidget( QWidget* parent = nullptr );
  ~ControlPanelPixmapWidget();
  void setParentProxyWidget( QGraphicsProxyWidget* parentProxy );
  void setColor( QColor color );

private:
  QString imgForAlign(int pos);

public slots:
  void slotSetColor( QColor color );
  void slotSetPixPos( Position pos );
  void slotSetPixIndex( int index );
  void slotSetGeoPosProxyWidget(GeoPoint point);

private slots:
  void slotChangeLock();
  void slotChangeColor();
  void slotSwitchPix();
  void slotChangePix(int index);
  void slotSwitchPos();
  void slotChangePos();
  void slotAddPix();
  void slotReset();
  void slotApply();
  void slotDelete();

signals:
  void colorChanged( QColor );
  void pixIndexChanged( int );
  void pixChanged( QString );
  void posChanged( Position );
  void reset();
  void apply();
  void forDelete();

private:
  Ui::ControlPanelPixmapWidget* ui_ = nullptr;
  QSettings* pixmapSet_ = nullptr;
  IconsetWidget* iconsetDlg_ = nullptr;
  QColor color_;
  GeoProxyWidget* parentProxy_ = nullptr;
  QGraphicsProxyWidget* posProxy_ = nullptr;
  QGraphicsProxyWidget* iconsetProxy_ = nullptr;
  Position pos_;
  TextPosDlg* posDlg_ = nullptr;
  bool locked_ = false;
  GeoPoint anchorPoit_;
};

}
}

#endif // METEO_COMMONS_UI_MAP_VIEW_DRAWTOOLS_CONTROLPANELPIXMAPWIDGET_H
