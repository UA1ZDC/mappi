#include "mapwidget.h"
#include <meteo/commons/ui/mainwindow/widgethandler.h>
#include <cross-commons/debug/tlog.h>

#include <commons/geobasis/geopoint.h>
#include <commons/obanal/tfield.h>
#include <meteo/commons/global/global.h>
#include <meteo/commons/ui/map/object.h>
#include <meteo/commons/ui/map/geopolygon.h>
#include <meteo/commons/ui/map/map.h>
#include <meteo/commons/global/global.h>
#include <meteo/commons/ui/map/view/mapview.h>
#include <meteo/commons/ui/map/view/mapwindow.h>
#include <meteo/commons/ui/map/field.h>

namespace meteo {
namespace map {

MapWidget::MapWidget( MapWindow* w )
  : QDialog(w),
  window_(w)
{
}

MapWidget::~MapWidget()
{
}

void MapWidget::setTitle(const QString& title)
{
  //debug_log << "MapWidget::setTitle"<<title;
  setWindowTitle(title);
}

void MapWidget::setPixmap(QPixmap pix)
{
  setWindowIcon(QIcon(pix));
}

void MapWidget::setMapWindow(MapWindow* w)
{
  setParent(w);
  setWindowFlags(Qt::Dialog);
  window_ = w;
}

MapView* MapWidget::mapview() const
{
  if( nullptr == window_ ){
    return nullptr;
  }
  return window_->mapview();
}

MapScene* MapWidget::mapscene() const
{
  if( nullptr == mapview() ){
    return nullptr;
  }
  return mapview()->mapscene();
}

Document* MapWidget::mapdocument() const
{
  if ( nullptr == mapscene() ) {
    return nullptr;
  }
  return mapscene()->document();
}

void MapWidget::resizeEvent(QResizeEvent* e)
{
  QDialog::resizeEvent(e);
}

void MapWidget::moveEvent(QMoveEvent* e)
{
  QDialog::moveEvent(e);
}

void MapWidget::closeEvent( QCloseEvent* e )
{
  global::saveSize(this);
  QDialog::closeEvent(e);
  emit closed();
}

void MapWidget::showEvent( QShowEvent* e )
{
  QDialog::showEvent(e);
  global::loadSize(this);
}

void MapWidget::hideEvent( QHideEvent* e )
{
  global::saveSize(this);
  QDialog::hideEvent(e);
}

void MapWidget::reject()
{
  QDialog::close();
  QDialog::reject();
}

}
}
