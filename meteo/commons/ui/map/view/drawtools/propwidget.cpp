#include "propwidget.h"

#include <qdir.h>
#include <qevent.h>
#include <qsettings.h>

#include <cross-commons/app/paths.h>
#include <cross-commons/debug/tlog.h>
#include <cross-commons/debug/tmap.h>

namespace meteo {
namespace map {

PropWidget::PropWidget(QWidget *parent)
  : QWidget(parent),
    opt_(new QSettings(QDir::homePath() + "/." + MnCommon::applicationName() + "/drawtools.ini", QSettings::IniFormat))
{
  setMouseTracking(true);
  loadConf();
}

PropWidget::~PropWidget()
{
  opt_->deleteLater();
  opt_ = 0;
}

bool PropWidget::hasValue() const
{
  return false;
}

void PropWidget::setValue(double value, bool enable)
{
  Q_UNUSED( value );
  Q_UNUSED( enable );
}

double PropWidget::value() const
{
  return 0;
}

QString PropWidget::unit() const
{
  return QString();
}

QString PropWidget::text() const
{
  return QString();
}

void PropWidget::setText(const QString& text)
{
  Q_UNUSED( text );
}

QImage PropWidget::pixmap() const
{
  return QImage();
}

void PropWidget::setPixmap( const QImage& pix )
{
  Q_UNUSED(pix);
}

meteo::Property PropWidget::toProperty() const
{
  not_impl;
  return meteo::Property();
}

void PropWidget::setProperty(const meteo::Property& prop)
{
  Q_UNUSED( prop );

  not_impl;
}

void PropWidget::setVisible(bool visible)
{
  //loadConf();

  QWidget::setVisible(visible);
}

void PropWidget::initWidget()
{
  not_impl;
}

void PropWidget::setProxyWidget(QGraphicsProxyWidget *proxyWidget)
{
  proxyWidget_ = proxyWidget;
  initWidget();
}



void PropWidget::closeEvent(QCloseEvent* e)
{
  e->accept();
  emit closed();
}

void PropWidget::enterEvent(QEvent* e)
{
  emit enter();
  setBlockWheel(false);
  QWidget::enterEvent( e );
}

void PropWidget::leaveEvent(QEvent* e)
{
  emit leave();
  setBlockWheel(true);
  QWidget::leaveEvent( e );
}

void PropWidget::setBackgroundColor(QWidget* w, const QColor& color)
{
  if ( 0 == w ) { return; }

  QPalette p = w->palette();
  p.setColor(w->backgroundRole(), color);
  w->setPalette(p);
}

void PropWidget::setBackgroundBrush(QWidget* w, const QBrush& brush)
{
  if ( 0 == w ) { return; }

  QPalette p = w->palette();
  p.setBrush(w->backgroundRole(), brush);
  w->setPalette(p);
}

void PropWidget::loadConf()
{
  QByteArray ba = opt_->value("propwidget/geometry", QByteArray()).toByteArray();
  if ( !ba.isEmpty() ) {
    restoreGeometry(ba);
  }
}

bool PropWidget::eventFilter(QObject *obj, QEvent *e)
{
  Q_UNUSED(obj);
  if (e->type() == QEvent::Wheel) {
    if(blockWheel()) {
      return true;
    }
    else {
      return false;
    }
  }
  return false;
}

QString PropWidget::getPixmapIconPath()
{
  return opt_->value("pixmapiconpath").toString();
}

void PropWidget::setPixmapIconPath(QString &str)
{
  opt_->setValue("pixmapiconpath", str );
}

} // map
} // meteo
