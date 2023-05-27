#include "fieldsaction.h"

#include <qevent.h>
#include <qtoolbutton.h>
#include <qgraphicssceneevent.h>
#include <qgraphicsproxywidget.h>
#include <qgraphicswidget.h>
#include <qobject.h>
#include <qdebug.h>
#include <qicon.h>
#include <qlabel.h>
#include <qcursor.h>

#include <cross-commons/debug/tlog.h>
#include <commons/geobasis/geopoint.h>
#include <commons/obanal/tfield.h>
#include <meteo/commons/global/global.h>
#include <meteo/commons/ui/map/document.h>
#include <meteo/commons/ui/map/incut.h>
#include <meteo/commons/ui/map/geopolygon.h>
#include <meteo/commons/ui/map/layeriso.h>
#include <meteo/commons/ui/map/view/mapview.h>
#include <meteo/commons/ui/map/view/mapscene.h>
#include <meteo/commons/ui/map/view/widgetitem.h>
#include <meteo/commons/ui/mainwindow/widgethandler.h>
#include <meteo/commons/ui/map/view/menu.h>
#include <meteo/commons/ui/map/view/mapwindow.h>
/*
namespace {
const QString widgetName() { return QString("fieldsaction"); }
}*/

namespace meteo {
namespace map {

static const auto kMenuItem = QPair<QString, QString>("fieldseaction", QObject::tr("Значение поля в точке"));
static const auto kMenuPath = QList<QPair<QString, QString>>
    ({QPair<QString, QString>("view", QObject::tr("Вид"))});

FieldsAction::FieldsAction( MapScene* scene )
: BaloonAction(scene, "fieldsaction" )
{


//  action_ = scene_->mapview()->window()->addActionToMenuFromFile("fieldsaction");
  action_ = scene_->mapview()->window()->addActionToMenu(kMenuItem, kMenuPath);
  if ( 0 != action_ ) {
    action_->setCheckable(true);
    connect(action_,SIGNAL(triggered(bool)),SLOT(slotWidgetOpen(bool)));
    QMenu* pm = qobject_cast<QMenu*>( action_->parentWidget() );
    if ( 0 != pm ) {
      QObject::connect( pm, SIGNAL( aboutToShow() ), this, SLOT( slotAboutToShowParentMenu() ) );
    }
  }
  scene_->document()->turnEvents();
  if (scene_->document()->eventHandler() != 0) {
    scene_->document()->eventHandler()->installEventFilter(this);
  }
}

void FieldsAction::slotTurnCoordWidgetOff(){
  deleteWidget();
}


void FieldsAction::createFWidget( ){
  if (coords_ == 0) {
    coords_ = new FieldsWidget;
    coords_->setLock(true);
    connect(coords_, SIGNAL(sclosed ()),SLOT(slotTurnCoordWidgetOff()));
  }
  if (baloonProxy_ == 0) {
    baloonProxy_ = new BaloonProxyWidget();
    baloonProxy_->setZValue(15001);
    baloonProxy_->setWidget(coords_);
    baloonProxy_->setState(BaloonProxyWidget::Moving);
    scene_->addItem(baloonProxy_);
    scene_->addVisibleItem(baloonProxy_);
  }
}

void FieldsAction::mouseMoveEvent( QMouseEvent* e )
{
  if ( false == hasView() ||baloonProxy_ == 0) {
    return;
  }
  GeoPoint gp = scene_->screen2coord(view()->mapToScene(e->pos()).toPoint());
  QList<valLabel> resultstr = fieldsResult(gp);
  FieldsWidget* coords = dynamic_cast<FieldsWidget*>(coords_);
  coords->setText(resultstr);
  BaloonAction::mouseMoveEvent(e);
}

void FieldsAction::mouseReleaseEvent(QMouseEvent* e)
{
  if(!toMouseReleaseEvent(e)) return;

  FieldsWidget* coords = dynamic_cast<FieldsWidget*>(coords_);
  if(0 == coords) return;

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

bool FieldsAction::eventFilter(QObject* watched, QEvent* ev)
{
  if( false == hasView()
    ||0 == mapview()
    ||false == mapview()->hasMapscene()
    ||watched != mapview()->mapscene()->document()->eventHandler()){
    return Action::eventFilter(watched, ev);
  }
  else if ( meteo::map::LayerEvent::LayerChanged == ev->type() ) {
    LayerEvent* e = static_cast<LayerEvent*>(ev);
    if( 0 != e ){
      if( LayerEvent::Added == e->changeType() ||
          LayerEvent::Deleted == e->changeType() ){
        updateText();
        slotAboutToShowParentMenu();
      }
    }
  }
  return  Action::eventFilter(watched, ev);
}

QList<valLabel> FieldsAction::fieldsResult(const GeoPoint& gp) const
{
  QList<valLabel> result;
  if (scene_ != 0 && scene_->document() != 0) {
    foreach (Layer* each, scene_->document()->layers()) {
      if (each != 0 && each->hasValue()&& each->visible()) {
        bool ok = false;
        double val = each->fieldValue( gp, &ok );
        if (ok) {
          valLabel vl;
          vl = each->valueLabel();
          if(vl.val.isNull()){
            vl.val = QString::number(val,'f',1);
          }
        //  int nf,nl,ii;
         // each->field()->getNumPoFila(gp,&nf,&nl);
          //ii = each->field()->num(nf,nl);
         // vl.date = QString("num = %1 i= %2 j = %3").arg(ii).arg(nf).arg(nl);
          result.append(vl);
        }
      }
    }
  }

  return result;
}

void FieldsAction::updateText()
{
  foreach( QObject* obj, baloonList_ ){
    BaloonProxyWidget* proxy  = qobject_cast< BaloonProxyWidget* >( obj );
    if( 0 != proxy ){
      QList<valLabel> resstr = fieldsResult(proxy->gp());
      FieldsWidget* w = qobject_cast<FieldsWidget*>(proxy->widget());
      if( 0 != w ){
        w->setText(resstr);
        adjustProxy();
      }
    }
  }
}

bool FieldsAction::proxyIsVisible() const
{
  if(0 == scene_ || 0 == coords_ || 0 == baloonProxy_ ){
    return false;
  }
  QPoint pnt = baloonProxy_->pos().toPoint() + baloonProxy_->rect().center().toPoint();
  if( scene_->document()->documentRect().contains(pnt) ){
    return true;
  }
  return false;
}

MapView* FieldsAction::mapview()
{
  if(0 == scene_ || false == scene_->hasMapview() ){
    return 0;
  }
  return scene_->mapview();
}


void FieldsAction::slotWidgetOpen(bool ch)
{
    if( true == ch ){
      if(0 == coords_){ createFWidget(); }
      coords_->show();
    }
    else{
      deleteWidget();
    }
}

void FieldsAction::addActionsToMenu( Menu* menu ) const
 {
   if ( 0 == menu || 0 == scene_ ) {
    return;
  }
  Layer* l = scene_->document()->activeLayer();
  if ( 0 == l ) {
    return;
  }
  if ( 0 == action_ ) {
    return;
  }
  QList<Layer*> list = scene_->document()->layers();
  bool haspointval = false;
  for ( int i = 0, sz = list.size(); i < sz; ++i ) {
    if (0 != list[i] && true == list[i]->hasValue()&&list[i]->visible() ) {
      haspointval = true;
      break;
    }
  }
  action_->setEnabled(haspointval);
  if ( false == haspointval ) {
    return;
  }

  action_->setCheckable(true);
  action_->setChecked(proxyIsVisible()) ;
  menu->addLayerAction(action_);
}

void FieldsAction::slotAboutToShowParentMenu()
{
  if ( 0 == action_ ) {
    return;
  }
  action_->setCheckable(true);
  QList<Layer*> list = scene_->document()->layers();
  bool haspointval = false;
  for ( int i = 0, sz = list.size(); i < sz; ++i ) {
    if (0 != list[i] && true == list[i]->hasValue()&&list[i]->visible() ) {
      haspointval = true;
      break;
    }
  }
  if ( false == haspointval && true == action_->isChecked() ) {
    slotWidgetOpen(false);
  }
  action_->setChecked( 0 != baloonProxy_ ) ;
  action_->setEnabled(haspointval);
}

}
}
