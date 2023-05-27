#include "puansonitem.h"

#include <qdebug.h>
#include <qpainter.h>
#include <qgraphicsscene.h>
#include <qgraphicssceneevent.h>

#include <commons/geobasis/stereoproj.h>
#include <commons/meteo_data/meteo_data.h>
#include <meteo/commons/proto/puanson.pb.h>
#include <meteo/commons/ui/map/puanson.h>
#include <cross-commons/debug/tlog.h>

#include "editor.h"

namespace meteo {
namespace puanson {

GridItem::GridItem()
  : QGraphicsItem(0)
{
}

GridItem::~GridItem()
{
}

void GridItem::paint( QPainter* pntr, const QStyleOptionGraphicsItem* o, QWidget* p )
{
  Q_UNUSED(o);
  Q_UNUSED(p);
  pntr->save();

  QRect r = scene()->sceneRect().toRect();
  QPoint center = r.center();
  int top = r.top();
  int bottom = r.bottom();
  int left = r.left();
  int right = r.right();

  pntr->setPen( Qt::gray );

  QPoint pnt = center;
  while ( pnt.x() < right ) {
    pntr->drawLine( QPoint( pnt.x(), top ), QPoint( pnt.x(), bottom ) );
    pnt.setX( pnt.x() + 20 );
  }
  pnt = center;
  while ( pnt.x() > left ) {
    pntr->drawLine( QPoint( pnt.x(), top ), QPoint( pnt.x(), bottom ) );
    pnt.setX( pnt.x() - 20 );
  }
  pnt = center;
  while ( pnt.y() < bottom ) {
    pntr->drawLine( QPoint( left, pnt.y() ), QPoint( right, pnt.y() ) );
    pnt.setY( pnt.y() + 20 );
  }
  pnt = center;
  while ( pnt.y() > top  ) {
    pntr->drawLine( QPoint( left, pnt.y() ), QPoint( right, pnt.y() ) );
    pnt.setY( pnt.y() - 20 );
  }

  pntr->restore();
}

DescrItem::DescrItem( Item* i, proto::CellRule* r )
  : QGraphicsItem(i),
  item_(i),
  rule_(r),
  press_(false),
  selected_(false)
{
  setFlag( QGraphicsItem::ItemIsMovable );
}

DescrItem::~DescrItem()
{
}

QRectF DescrItem::boundingRect() const
{
  QRect r = item_->puanson_->boundingRect( *rule_ );
  r.setTopLeft(r.topLeft() - QPoint(6,6) );
  r.setBottomRight(r.bottomRight() + QPoint(6,6) );
  return r;
}

void DescrItem::paint( QPainter* p, const QStyleOptionGraphicsItem* o, QWidget* w )
{
  Q_UNUSED(o);
  Q_UNUSED(w);
  if ( 0 == rule_ ) {
    return;
  }
  if ( proto::kNoPosition == rule_->align() ) {
    return;
  }
  if ( false == rule_->visible() ) {
    return;
  }
  if ( kRuleRect == (item_->rectType() & kRuleRect) ) {
    p->save();
    p->setPen( QColor( 56, 123, 248 ) );
    QRectF r = item_->puanson_->boundingRect( *rule_ );
    p->drawRect(r);
    p->restore();
  }
  if ( kRuleAnchor == (item_->anchorType() & kRuleAnchor) ) {
    p->save();
    p->setPen( Qt::red );
    p->setBrush( QBrush( Qt::red ) );
    p->drawEllipse( rule_->pos().x() - 3, rule_->pos().y() - 3, 6, 6 );
    p->restore();
  }
  if ( true == selected_ ) {
    p->save();
    p->setPen( Qt::black );
    p->drawRect( boundingRect() );
    p->restore();
  }
}

void DescrItem::mouseMoveEvent( QGraphicsSceneMouseEvent* e )
{
  if ( false == press_ ) {
    return QGraphicsItem::mouseMoveEvent(e);
  }
  QPoint newpnt = e->pos().toPoint();
  if ( newpnt != oldpnt_ ) {
    QPoint delta = newpnt - oldpnt_;
    proto::Position pos = rule_->pos();
    pos.set_x( pos.x() + delta.x() );
    pos.set_y( pos.y() + delta.y() );
    rule_->mutable_pos()->CopyFrom(pos);
    oldpnt_ = newpnt;
    item_->updatePunch();
    item_->editor_->puansonChangedOnScene();
    prepareGeometryChange();
    return;
  }
  else if ( pos() != QPointF(0,0) ) {
    setPos( QPoint(0,0) );
  }
}

void DescrItem::mouseReleaseEvent( QGraphicsSceneMouseEvent* e )
{
  if ( Qt::LeftButton == e->button() ) {
    press_ = false;
  }
//  QGraphicsItem::mouseReleaseEvent(e);
}

void DescrItem::mousePressEvent( QGraphicsSceneMouseEvent* e )
{
  if ( Qt::LeftButton == e->button() ) {
    press_ = true;
    oldpnt_ = e->pos().toPoint();
    item_->descritemPressed(this);
  }
//  QGraphicsItem::mousePressEvent(e);
} 

void DescrItem::setDescrSelected( bool fl )
{
  selected_ = fl;
  prepareGeometryChange();
}

Item::Item( Editor* e )
  : QGraphicsItem(),
  editor_(e),
  proj_( new StereoProj( GeoPoint::fromDegree( 60, 30 ) ) ),
  puanson_(new meteo::map::Puanson(proj_) ),
  recttype_( kPuansonRuleRect ),
  anchortype_( kPuansonRuleAnchor )
{
  puanson_->setScreenPos( QPoint(0,0) );
}

Item::~Item()
{
  for ( int i = 0, sz = items_.size(); i < sz; ++i ) {
    delete items_[i];
  }
  items_.clear();
  delete puanson_; puanson_ = 0;
  delete proj_; proj_ = 0;
}
    
void Item::setSelectedDescr( int index )
{
  if ( items_.size() <= index || 0 > index ) {
    error_log << QObject::tr("Неизвестная ошибка. Не найден элемент с индексом %1")
      .arg(index);
    return;
  }
  for ( int i = 0, sz = items_.size(); i < sz; ++i ) {
    bool selected = false;
    if ( index == i ) {
      selected = true;
    }
    DescrItem* item = items_[i];
    if ( item->descrSelected() != selected ) {
      item->setDescrSelected(selected);
    }
  }
}
    
void Item::descritemPressed( DescrItem* item )
{
  int indx = items_.indexOf(item);
  setSelectedDescr(indx);
  QTreeWidgetItem* treeitem = editor_->descrtree()->topLevelItem(indx);
  if ( 0 != treeitem && editor_->descrtree()->currentItem() != treeitem && false == item->isSelected() ) {
    editor_->descrtree()->setCurrentItem(treeitem);
  }
}

void Item::setPunch( proto::Puanson* p )
{
  if ( 0 == puanson_ ) {
    error_log << QObject::tr("Свойство puanson_ == 0. Невозможно установить шаблон пуансона");
    return;
  }
  puanson_->setPunch(*p);
  punch_ = p;
  buildChilds();
  int indx = -1;
  QTreeWidgetItem* item = editor_->descrtree()->currentItem();
  if ( 0 != item ) {
    indx = editor_->descrtree()->indexOfTopLevelItem(item);
  }
  setSelectedDescr(indx);
  QGraphicsItem::prepareGeometryChange();
}

void Item::updatePunch()
{
  puanson_->setPunch(*punch_);
  int indx = -1;
  QTreeWidgetItem* item = editor_->descrtree()->currentItem();
  if ( 0 != item ) {
    indx = editor_->descrtree()->indexOfTopLevelItem(item);
  }
  setSelectedDescr(indx);
  QGraphicsItem::prepareGeometryChange();
}

void Item::buildChilds()
{
  for ( int  i = 0, sz = items_.size(); i < sz; ++i ) {
    delete items_[i];
  }
  items_.clear();

  DescrItem* item = 0;
  for ( int i = 0, sz = punch_->rule_size(); i < sz; ++i ) {
    item = new DescrItem( this, punch_->mutable_rule(i) );
    items_.append(item);
  }
}

void Item::setDdff( int dd, int ff, control::QualityControl ddqual, control::QualityControl ffqual )
{
  if ( 0 == puanson_ ) {
    error_log << QObject::tr("Свойство puanson_ == 0. Невозможно установить шаблон пуансона");
    return;
  }
  puanson_->setDdff( dd, ff, ddqual, ffqual );
  QGraphicsItem::prepareGeometryChange();
}

void Item::setParamValue( const proto::Id& id, const TMeteoParam& param )
{
  if ( 0 == puanson_ ) {
    error_log << QObject::tr("Свойство puanson_ == 0. Невозможно установить шаблон пуансона");
    return;
  }
  puanson_->setParamValue( id, param );
  QGraphicsItem::prepareGeometryChange();
}

void Item::setMeteodata( const TMeteoData& data )
{
  if ( 0 == puanson_ ) {
    error_log << QObject::tr("Свойство puanson_ == 0. Невозможно установить данные в пуансон");
    return;
  }
  puanson_->setMeteodata(data);
  QGraphicsItem::prepareGeometryChange();
}

void Item::loadTestMeteoData()
{
  if ( 0 == puanson_ ) {
    error_log << QObject::tr("Свойство puanson_ == 0. Невозможно установить данные в пуансон");
    return;
  }
  puanson_->loadTestMeteoData();
  prepareGeometryChange();
}

void Item::setRectType( int rt )
{
  if ( recttype_ == rt ) {
    return;
  }
  recttype_ = rt;
  prepareGeometryChange();
  for ( int i = 0, sz = items_.size(); i < sz; ++i ) {
    items_[i]->prepareGeometryChange();
  }
}

void Item::setAnchorType( int at )
{
  if ( anchortype_ == at ) {
    return;
  }
  anchortype_ = at;
  prepareGeometryChange();
  for ( int i = 0, sz = items_.size(); i < sz; ++i ) {
    items_[i]->prepareGeometryChange();
  }
}

const TMeteoData& Item::meteodata() const
{
  return puanson_->meteodata();
}

QRectF Item::boundingRect() const
{
  if ( 0 == puanson_ ) {
    error_log << QObject::tr("Нулевой указатель на пуансон");
    return QRectF();
  }

  QList<QRect> l = puanson_->boundingRect( QTransform() );
  if ( 0 == l.size() ) {
    return QRectF();
  }
  QRect r = l[0];
  for ( int i = 1, sz = l.size(); i < sz; ++i ) {
    r = r.united(l[i]);
  }
  QRectF rf(r.topLeft() - QPoint(2,2), r.size()+QSize(4,4));
  return rf;
}

void Item::paint( QPainter* p, const QStyleOptionGraphicsItem* o, QWidget* w )
{
  Q_UNUSED(o);
  Q_UNUSED(w);
  if ( 0 == puanson_ ) {
    error_log << QObject::tr("Свойство puanson_ == 0. Невозможно нарисовать пуансон");
    return;
  }
  if ( false == puanson_->hasPunch() ) {
    error_log << QObject::tr("Не установлен шаблон пуансона. Невозможно нарисовать пуансон");
    return;
  }
  puanson_->loadTestMeteoData();
  QRectF r = boundingRect();
  puanson_->render( p, r.toRect(), QTransform() );
  if ( kPuansonRect == (recttype_ & kPuansonRect ) ) {
    p->save();
    p->setPen( QColor( 56, 123, 248 ) );
    p->drawRect(r);
    p->restore();
  }
  if ( kCenterAnchor == ( anchortype_ & kCenterAnchor ) ) {
    p->save();
    p->setPen(Qt::red);
    p->drawLine( QPoint( -8, 0 ), QPoint( 8, 0) );
    p->drawLine( QPoint( 0, -8 ), QPoint( 0, 8) );
    p->restore();
  }
}

}
}
