#ifndef METEO_COMMONS_UI_MAP_VIEW_DRAWTOOLS_PIXMAPMINWIDGET_H
#define METEO_COMMONS_UI_MAP_VIEW_DRAWTOOLS_PIXMAPMINWIDGET_H

#include "propwidget.h"

#include <qicon.h>
class QSettings;
namespace Ui {
class PixmapMinWidget;
}

class ActionButton;

namespace meteo {
namespace map {
class TextPosDlg;
class IconsetWidget;

class PixmapPropWidget : public PropWidget
{
  Q_OBJECT

public:
  explicit PixmapPropWidget(QWidget *parent = nullptr);
  ~PixmapPropWidget();

  virtual QImage pixmap() const { return pix_; }
  virtual void setPixmap(const QImage& pix ) { pix_ = pix.copy(); }

  virtual meteo::Property toProperty() const;
  virtual void setProperty(const meteo::Property& prop);
  virtual void initWidget();
  QColor color() { return color_;}

public slots:
  void slotOnDelBtn();
  void slotOffDelBtn();
  void slotDefaultProp();
  void slotSetProperty(meteo::Property prop);
  void slotSetColor( QColor color);
  void slotSetPix(QString str);
  void slotSetPixIndex( int index);
  void slotSetPixmapPos( Position pos );

private slots:
  void slotPixmapChanged( int index );

  void slotShowColorDlg();

  void slotSwitchIconDlg();
  void slotShowIconDlg();
  void slotHideIconDlg();
  void slotAddCommonPix();

private:
  QIcon icoForAlign(int a);
  void loadSettings();
  void saveSettings(const meteo::Property &prop);

signals:
  void posChanged(Position);
  void pixIndexChanged(int);

private:
  Ui::PixmapMinWidget *ui_ = nullptr;
  QImage pix_;
  meteo::Property prop_;
  IconsetWidget* iconsetDlg_ = nullptr;
  QGraphicsProxyWidget* iconsetDlgPW_ = nullptr;
  ActionButton* iconsetBtn_ = nullptr;
  ActionButton* colorBtn_ = nullptr;
  QSettings* pixmapSet_ = nullptr;
  QColor color_;

};

}
}


#endif
