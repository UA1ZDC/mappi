#ifndef METEO_COMMONS_UI_MAP_VIEW_DRAWTOOLS_PROPWIDGET_H
#define METEO_COMMONS_UI_MAP_VIEW_DRAWTOOLS_PROPWIDGET_H

#include <qwidget.h>

#include <meteo/commons/ui/map/object.h>

class QSettings;

namespace meteo {
namespace map {

class PropWidget : public QWidget
{
  Q_OBJECT
public:
  explicit PropWidget(QWidget* parent = 0);
  virtual ~PropWidget();

  virtual bool hasValue() const;
  virtual void setValue(double value, bool enable = true);
  virtual double value() const;
  virtual QString unit() const;

  virtual QString text() const;
  virtual void setText(const QString& text);

  virtual QImage pixmap() const ;
  virtual void setPixmap( const QImage& pix );

  virtual meteo::Property toProperty() const;
  virtual void setProperty(const meteo::Property& prop);

  virtual void setVisible(bool visible);
  virtual void initWidget();

  void setBlockWheel(bool block) { blockWheel_ = block; }
  bool blockWheel() { return blockWheel_;}

  void setProxyWidget( QGraphicsProxyWidget* proxyWidget );
  QGraphicsProxyWidget* getProxyWidget() { return proxyWidget_; }
  QString getPixmapIconPath();
  void setPixmapIconPath(QString& str);

public slots:
  virtual void slotOffDelBtn() { /*not_impl;*/ }
  virtual void slotOnDelBtn() { /*not_impl;*/ }

signals:
  void sizeChanged();
  void valueChanged();
  void finish();
  void deleteCurrent();
  void closed();
  void enter();
  void leave();
  void colorChanged(QColor);
  void insertSymb(QChar);

protected:
  virtual void closeEvent(QCloseEvent* e);
  virtual void enterEvent(QEvent* e);
  virtual void leaveEvent(QEvent* e);

  void setBackgroundColor(QWidget* w, const QColor& color);
  void setBackgroundBrush(QWidget* w, const QBrush& brush);

  void loadConf();
  bool eventFilter( QObject* obj, QEvent* e);

protected:
  QSettings* opt_;

private:
  QGraphicsProxyWidget* proxyWidget_ = nullptr;
  bool blockWheel_ = true;
};

} // map
} // meteo

#endif // METEO_COMMONS_UI_MAP_VIEW_DRAWTOOLS_PROPWIDGET_H
