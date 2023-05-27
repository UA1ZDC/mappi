#include "mousecoordaction.h"

#include "coordwidget.h"
#include "coordproxywidget.h"
//#include "renderitembutton.h"

#include <qevent.h>
#include <qicon.h>
#include <qgraphicsproxywidget.h>

#include <cross-commons/debug/tlog.h>
#include <commons/geobasis/geopoint.h>
#include <commons/landmask/landmask.h>
#include <meteo/commons/ui/map/view/mapscene.h>
#include <meteo/commons/ui/map/view/mapview.h>
#include <meteo/commons/ui/map/view/actionbutton.h>
#include <meteo/commons/ui/map/ramka.h>

namespace meteo {
namespace map {

MouseCoordAction::MouseCoordAction( MapScene* scene )
  : BaloonAction(scene, "mousecoordaction"),
    btncoords_(new ActionButton)
{
  btncoords_->setCheckable(true);
  btncoords_->setIcon(QIcon(":/meteo/icons/map/earth.png"));
  btncoords_->setToolTip( QObject::tr("Координаты указателя мыши") );
  connect(btncoords_, SIGNAL(toggled(bool)), SLOT(slotTurnCoordWidget(bool)));

  item_ = scene_->addWidget(btncoords_);
  item_->setZValue(10001);

  setHasCorner(true);
  setCorner(kBottomLeft);
  setCornerAddress(QPoint( 0,0 ));
  setTextFormat("%1 : %2");
}

MouseCoordAction::~MouseCoordAction()
{
  delete btncoords_;
}

void MouseCoordAction::slotTurnCoordWidgetOff(){
  btncoords_->disconnect();
  btncoords_->setChecked(false);
  connect(btncoords_, SIGNAL(toggled(bool)), SLOT(slotTurnCoordWidget(bool)));
  deleteWidget();
}

void MouseCoordAction::slotTurnCoordWidget(bool on)
{
//  debug_log<<coords_<<baloonProxy_<<on;
  if (on == true) {
    createWidget();
    onDefaultPos();
  }
  else {
    deleteWidget();
  }
}

void MouseCoordAction::createWidget(){
  if (coords_ == 0) {
    coords_ = new CoordWidget;
    coords_->initWidget();
    coords_->setMinimumHeight(btncoords_->height()+10);
    connect(coords_, SIGNAL(sclosed ()),SLOT(slotTurnCoordWidgetOff()));
  }
  if (baloonProxy_ == 0) {
    baloonProxy_ = new BaloonProxyWidget();
    baloonProxy_->setWidget(coords_);
    baloonProxy_->setState(BaloonProxyWidget::Fixed);

    scene_->addItem(baloonProxy_);
    scene_->addVisibleItem(baloonProxy_);
  }
}

QString MouseCoordAction::coordToString(const GeoPoint& gp)
{
  return (  gp.strLat().rightJustified(20)
  + gp.strLon().rightJustified(20));
}

QString MouseCoordAction::coordToString(const GeoPoint& gp, const QString& textFormat)
{
  return gp.toString(textFormat.contains("%3"), textFormat);
}


void MouseCoordAction::mouseMoveEvent( QMouseEvent* e )
{
  if ( false == hasView() || baloonProxy_ == 0) {
    return;
  }
  GeoPoint gp = scene_->screen2coord(view()->mapToScene(e->pos()).toPoint());
  QString coordstr = textFormat_.isEmpty() ? coordToString(gp)
  : coordToString(gp, textFormat_);
  coords_->setText(coordstr);
  BaloonAction::mouseMoveEvent(e);
  if (e->button() != Qt::LeftButton
    && !coords_->lock()) {
    onDefaultPos();
  }
}

void MouseCoordAction::mouseReleaseEvent(QMouseEvent* e)
{
  if(!toMouseReleaseEvent(e)) return;
  if( 0 != baloonProxy_ ){
    GeoPoint gp = scene_->screen2coord(view()->mapToScene(e->pos()).toPoint());
    QWidget* w = baloonProxy_->widget();
    RenderItem* item = new RenderItem(gp, 0, scene_);
    QToolButton* btn = item->buttonClose();
    renderList_.insert(btn, item);
    connect(item->buttonClose(), SIGNAL(clicked(bool)), SLOT(slotRemoveRenderItem()));
    QSize sz(w->width(), w->height()-10);
    item->setItemSize(sz);

    QSize pixsize(sz - QSize(2,2));

    QPixmap pixmap(pixsize);
    pixmap.fill(Qt::white);
    QRect r(QPoint(0,0), pixsize);
    QPainter pix_painter(&pixmap);
    w->render(&pix_painter, QPoint(), r);



    item->setPixmap(pixmap);
    item->setGeoPoint(gp);
  }
}

void BaloonAction::slotRemoveProxy(QObject *obj){
  baloonList_.removeAll(obj);
}

void BaloonAction::slotRemoveRenderItem()
{
  QToolButton* btn = qobject_cast<QToolButton*>(sender());
  if( renderList_.keys().contains(btn)){
    baloonList_.removeAll(btn);
    RenderItem* item = renderList_[btn];
    scene_->removeItem(item);
    scene_->document()->removeItem(item);
    //delete item;
  }
}

BaloonAction::BaloonAction( MapScene* scene, const QString & an )
  : Action(scene, an),
  coords_(0),
  baloonProxy_(0),
  handing_(false)
{

}

BaloonAction::~BaloonAction()
{
 deleteWidget();
}

void BaloonAction::deleteWidget(){
  if (baloonProxy_ != 0) {
    baloonProxy_->deleteLater();
    baloonProxy_ = 0;
    coords_ = 0;
  }
}


void BaloonAction::resizeEvent(QResizeEvent* )
{
  if (coords_ && !coords_->lock()) {
    onDefaultPos();
  }
}

void BaloonAction::onDefaultPos(){
  if (0 != baloonProxy_ && 0!=item_ ){
    baloonProxy_->setPos(QPointF(item_->pos().x() + item_->boundingRect().width() + 5,
                              item_->pos().y()));
  }
}

bool BaloonAction::toMouseReleaseEvent(QMouseEvent* e)
{
  if (e->button() != Qt::LeftButton
    || 0 == baloonProxy_
    || 0 == coords_ ) {
    onDefaultPos();
    adjustProxy();
    return false;
  }
  if (!coords_->lock()) {
  //  baloonProxy_->setState(BaloonProxyWidget::Fixed);
    onDefaultPos();
    adjustProxy();
    return false;
  }
    if (e->modifiers() == Qt::SHIFT) {
      coords_->setLock(false);
      coords_->setBtnVisible(false);
      baloonProxy_->setState(BaloonProxyWidget::Fixed);
      onDefaultPos();
      adjustProxy();
      return false;
    }

    if(onHover() ) {
      adjustProxy();
      return false;
    }
    if(handing_) {
      GeoPoint gp = scene_->screen2coord(view()->mapToScene(e->pos()).toPoint());
      baloonProxy_->setGeoPoint(gp);
      baloonProxy_->adjustPos(view()->mapToScene(e->pos()));
      adjustProxy();
      return false;
    }
return true;

}

void BaloonAction::mouseMoveEvent( QMouseEvent* e )
{
  if ( false == hasView() ||baloonProxy_ == 0) {
    return;
  }
  if (e->buttons() == Qt::LeftButton ){
    handing_ = true;
  } else {
    handing_ = false;
  }
  if (coords_->lock()) {
    GeoPoint gp = scene_->screen2coord(view()->mapToScene(e->pos()).toPoint());
    baloonProxy_->setGeoPoint(gp);

    QRect ramkaRect = scene_->document()->ramka()->calcRamka(0,0).boundingRect();
    QRect r = baloonProxy_->boundingRect().toRect();
    auto x = view()->mapToScene(e->pos()).x();
    int adj = 0;
    if ( x + r.width() > ramkaRect.right() ) {
      baloonProxy_->setLeftOrient(true);
      adj = r.width();
    }
    else {
      baloonProxy_->setLeftOrient(false);
    }
    auto pos = e->pos();
    pos.setX(pos.x() - adj);
    baloonProxy_->adjustPos(view()->mapToScene(pos));

    baloonProxy_->setState(BaloonProxyWidget::Moving);
  }
}



void BaloonAction::wheelEvent(QWheelEvent* )
{
  adjustProxy();
}

bool BaloonAction::onHover(){
  foreach( QObject* obj, baloonList_ ){
    BaloonProxyWidget* proxy  = qobject_cast< BaloonProxyWidget* >( obj );
    if(proxy && proxy->onHover()){
      return true;
    }
  }
  return false;
}

void BaloonAction::adjustProxy()
{
  foreach( QObject* obj, baloonList_ ){
    BaloonProxyWidget* proxy  = qobject_cast< BaloonProxyWidget* >( obj );
    if(proxy){
      proxy->adjustPos(scene_->document()->coord2screen(proxy->gp()));
    }
  }
}

} // map
} // meteo
