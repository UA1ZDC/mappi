#include "pixmaptoolgraphicsitem.h"
#include "textposdlg.h"


#include <qgraphicsitem.h>
#include <qgraphicsscene.h>
#include <qgraphicssceneevent.h>
#include <qgraphicsproxywidget.h>
#include <qapplication.h>
#include <qmath.h>

#include <cross-commons/debug/tlog.h>
#include <meteo/commons/ui/map/view/mapview.h>
#include <meteo/commons/ui/map/geopixmap.h>


namespace meteo {
namespace map {

const int buttonSize = 32;
const int zazor = 6;
const int minSizeBoundingRect = 70;
const QString rotateImgPath = QString(":/meteo/icons/map/rotate.png");
const QString deleteImgPath = QString(":/meteo/icons/delete-16.png");
const bool enableBtn = false;

PixmapToolGraphicsItem::PixmapToolGraphicsItem(double size, QGraphicsItem* parent) :
  QGraphicsObject(parent),
  size_(size),
  state_(kNormal)
{
  boundingRect_ = QRectF(0,0,size,size);
  boundingRect_.moveCenter(pos());
  setFlag(QGraphicsItem::ItemIsMovable,false);
  setFlag(QGraphicsItem::ItemIsSelectable, true);
  setAcceptHoverEvents(true);
  initStyles();
  defaultProp();
  textDlg_ = new TextPosDlg();
  textDlg_->resize(textDlg_->sizeHint());
  textDlg_->setAllowedAlign( Position::kNoPosition, false );
  textDlg_->setCurrent( prop_.pos() );
  textDlg_->hide();
  textDlgPW_ = new QGraphicsProxyWidget(this);
  textDlgPW_->setWidget(textDlg_);;
  textDlgPW_->hide();
  QObject::connect( textDlg_, SIGNAL( posChanged(int) ), textDlg_, SLOT( hide() ) );
  QObject::connect( textDlg_, SIGNAL( posChanged(int) ), this , SLOT( slotPosChanged(int) ) );
}

void PixmapToolGraphicsItem::initStyles()
{
  pen_[kNormalStyleRole]   = QPen(QBrush(Qt::black), 2);
  pen_[kHoverStyleRole]    = QPen(QBrush(Qt::black), 2);
  pen_[kSelectedStyleRole] = QPen(QBrush(Qt::black), 2);

  brush_[kNormalStyleRole]   = QBrush(Qt::white);
  brush_[kHoverStyleRole]    = QBrush(Qt::red);
  brush_[kSelectedStyleRole] = QBrush(Qt::red);
}

void PixmapToolGraphicsItem::setState(State newState)
{
  state_ = newState;
}

void PixmapToolGraphicsItem::resizeRect(const QPointF &pos, const QPointF &lastPos)
{
  if ( ( verticResize_ == kNoVerticResize )
       && ( horizontalResize_ == kNoHorizResize ) ) {
    return;
  }
  if ( (kResize != state_) || (false == grabed_) ) {
    return;
  }
  if ( nullptr == geoPixmap_ ) {
    return;
  }
  double coef = 0.005;
  prepareGeometryChange();
  QPointF diff = pos - lastPos;
  qreal length = qSqrt(qPow(diff.x(), 2) + qPow(diff.y(), 2));
  PointF scale = geoPixmap_->scaleXy();
  switch (ramkaState_) {
  case kBottomLeft:
    if ( ( diff.y() >= 0 ) && ( diff.x() <= 0 ) ) {
      scale.set_y( scale.y() + coef * abs( length ) );
      scale.set_x( scale.x() + coef * abs( length ) );
    }
    else if ( ( diff.y() <= 0 ) && ( diff.x() >= 0 ) ) {
      scale.set_y( scale.y() - coef * abs( length ) );
      scale.set_x( scale.x() - coef * abs( length ) );
    }
    break;
  case kBottomRight:
    if ( ( diff.y() >= 0 ) && ( diff.x() >= 0 ) ) {
      scale.set_y( scale.y() + coef * abs( length ) );
      scale.set_x( scale.x() + coef * abs( length ) );
    }
    else if ( ( diff.y() <= 0 ) && ( diff.x() <= 0 ) ) {
      scale.set_y( scale.y() - coef * abs( length ) );
      scale.set_x( scale.x() - coef * abs( length ) );
    }
    break;
  case kTopLeft:
    if ( ( diff.y() <= 0 ) && ( diff.x() <= 0 ) ) {
      scale.set_y( scale.y() + coef * abs( length ) );
      scale.set_x( scale.x() + coef * abs( length ) );
    }
    else if ( ( diff.y() >= 0 ) && ( diff.x() >= 0 ) ) {
      scale.set_y( scale.y() - coef * abs( length ) );
      scale.set_x( scale.x() - coef * abs( length ) );
    }
    break;
  case kTopRight:
    if ( ( diff.y() <= 0 ) && ( diff.x() >= 0 ) ) {
      scale.set_y( scale.y() + coef * abs( length ) );
      scale.set_x( scale.x() + coef * abs( length ) );
    }
    else if ( ( diff.y() >= 0 ) && ( diff.x() <= 0 ) ) {
      scale.set_y( scale.y() - coef * abs( length ) );
      scale.set_x( scale.x() - coef * abs( length ) );
    }
    break;
  default:
    break;
  }
  if ( scale.x() < 0.1 ) {
    scale.set_x( 0.1 );
  }
  if ( scale.y() < 0.1 ) {
    scale.set_y( 0.1 );
  }
  prop_.mutable_scalexy()->CopyFrom(scale);
  geoPixmap_->setScale( prop_.scalexy().x(), prop_.scalexy().y() );
  prepareGeometryChange();
  calcRects();
  this->update(boundingRect_);
  emit valueChanged(prop_);
}

void PixmapToolGraphicsItem::calcRotateAngle(const QPointF &pos)
{
  QPointF inCenter = innerRect().center();
  QPointF trCenter = rotateRect().center();
  qreal a,b,c; // находим угол по теореме косинусов
  a = qSqrt( qPow( pos.x() - trCenter.x(), 2 )  + qPow( pos.y() - trCenter.y(), 2 ) );
  b = qSqrt( qPow( inCenter.x() - trCenter.x(), 2 )  + qPow( inCenter.y() - trCenter.y(), 2 ) );
  c = qSqrt( qPow( inCenter.x() - pos.x(), 2 )  + qPow( inCenter.y() - pos.y(), 2 ) );
  qreal cos, angle;
  cos = (qPow(b,2) + qPow(c,2) - qPow(a,2)) / (2*b*c);
  angle = qAcos(cos);
  qreal line = ( ( trCenter.y() - inCenter.y()) * pos.x() + (inCenter.x()-trCenter.x())*pos.y()
                  + (trCenter.x()*inCenter.y()-inCenter.x()*trCenter.y() ) ); //уравнение прямой

  if ( line > 0 ) {
    angle = -angle;
  }
  angle = angle + angleStart_;
  prop_.set_rotateangle(RAD2DEG*angle);
  geoPixmap_->setRotateAngle(prop_.rotateangle());
  prepareGeometryChange();
  this->update(boundingRect_);
  emit valueChanged(prop_);
}

void PixmapToolGraphicsItem::calcRects() const
{
  calcInnerRect();
  calcBottomLeftRect();
  calcBottomRightRect();
  calcRotateRect();
  calcBoundingRect();
}

void PixmapToolGraphicsItem::changeShowTxt()
{
  if ( nullptr != textDlgPW_ ) {
    textDlgPW_->setVisible( !textDlgPW_->isVisible() );
  }
  state_ = kNormal;
}

void PixmapToolGraphicsItem::setGeoImg()
{
  if ( nullptr != geoPixmap_ ) {
    QImage workImg = QImage(imgPath_);
    workImg.scaled(innerRect().size().toSize());
    geoPixmap_->setImage(workImg);
  }
}

QRectF PixmapToolGraphicsItem::calcInnerRect() const
{
    map::Document* doc = document();
    if ( nullptr == doc) {
      return QRect();
    }
    if ( nullptr == geoPixmap_ ) {
      return QRect();
    }
    QList<QRect> list = geoPixmap_->boundingRect( doc->transform() );
    QRectF r;
    for ( auto rl : list ) {
      r = r.united(rl);
    }
    innerRect_ = r;
    const GeoPoint topLeft = doc->screen2coord( innerRect_.topLeft() - QPoint(0, buttonSize));
    emit anchorChanged( topLeft );
    return innerRect_;
}

QRectF PixmapToolGraphicsItem::calcRotateRect() const
{
  QPointF p1 = innerRect_.bottomRight();
//  p1.setY( p1.y() - buttonSize);
  rotateRect_ = QRectF( p1, QSize( buttonSize, buttonSize ));
  return rotateRect_;
}

QRectF PixmapToolGraphicsItem::calcBottomLeftRect() const
{
  QPointF p1 = innerRect_.bottomLeft();
  p1.setX(p1.x() - buttonSize);
  bottomLeftRect_ = QRectF( p1, QSize( buttonSize, buttonSize ));
  return bottomLeftRect_;
}

QRectF PixmapToolGraphicsItem::calcBottomRightRect() const
{
  QPointF p1 = innerRect_.bottomRight();
  bottomRightRect_ = QRectF( p1, QSize( buttonSize * 0.5, buttonSize * 0.5 ));
  return bottomRightRect_;
}

QRectF PixmapToolGraphicsItem::calcBoundingRect() const
{
  QPointF bl = bottomLeftRect_.bottomLeft();
  QPointF tr = innerRect().topLeft();
  QPointF tl;
  QPointF br = rotateRect_.bottomRight();;
  tl.setX(bl.x());
  tl.setY(tr.y());
//  br.setX(tr.x());
//  br.setY(bl.y());
  boundingRect_ =  QRectF(tl, br);
  return boundingRect_;
}

bool PixmapToolGraphicsItem::inInnerRect(const QPointF &pos)
{
  return innerRect().contains(pos);
}

bool PixmapToolGraphicsItem::inTopRightRect(const QPointF& pos)
{
  return rotateRect().contains(pos);
}

bool PixmapToolGraphicsItem::inBottomLeftRect(const QPointF& pos)
{
  return bottomLeftRect().contains(pos);
}

bool PixmapToolGraphicsItem::inBottomRightRect(const QPointF& pos)
{
  return bottomRightRect().contains(pos);
}

PixmapToolGraphicsItem::GrabRamka PixmapToolGraphicsItem::onRamkaInnerRect(const QPointF &pos)
{
  if ( true == inInnerRect(pos)) {
    QPointF p1 = innerRect().topLeft();
    QPointF p2 = innerRect().bottomRight();
    QRectF newRect(p1, p2);
    newRect.setWidth(newRect.width() - 6);
    newRect.setHeight(newRect.height() - 6);
    newRect.moveCenter(innerRect().center());
    if ( pos.y() >= newRect.bottom() ) {
      if ( pos.x() <= newRect.left() ) {
        return kBottomLeft;
      }
      else if ( pos.x() >= newRect.right() ) {
        return kBottomRight;
      }
      return kBottom;
    }
    else if ( pos.y() <= newRect.top()) {
      if ( pos.x() <= newRect.left()) {
        return kTopLeft;
      }
      else if ( pos.x() >= newRect.right()) {
        return kTopRight;
      }
      return kTop;
    }
    else if ( pos.x() <= newRect.left() ) {
      return kLeft;
    }
    else if ( pos.x() >= newRect.right()) {
      return kRight;
    }
  }
  return kNoGrab;
}

QRectF PixmapToolGraphicsItem::boundingRect() const
{
  calcRects();
  return boundingRect_;
}

void PixmapToolGraphicsItem::paint(QPainter *painter, const QStyleOptionGraphicsItem* /*o*/, QWidget* /*w*/)
{
  map::Document* doc = document();
  if ( nullptr == doc ) {
    error_log << QObject::tr("Невозможно нарисовать объект, т.к. нулевой указатель на Document");
    return;
  }
  QPointF pos(0,0);
  if (nullptr  != scene()) {
    MapScene* mapScene = qobject_cast<MapScene*>(scene());
    pos = mapScene->coord2screen(geoPos_);
    if (nullptr != textDlgPW_) {
      textDlgPW_->setPos( mapScene->coord2screen( mapScene->screen2coord( innerRect().bottomLeft().toPoint() ) ) );
    }
  }

//  calcRects();
//  StRole role = kNormalStyleRole;
//  if ( hovered_ ) {
//    role = kHoverStyleRole;
//  }
//  if ( selected_ ){
//    role = kSelectedStyleRole;
//  }
  if ( doc->isStub() ) {
    painter->save();
    painter->translate(calcScenePoint());
  }

  painter->setPen(pen_[ kSelectedStyleRole]);
  painter->setBrush(brush_[ kSelectedStyleRole]);
  QPointF p1 = boundingRect_.bottomLeft();
  QPointF p2 = boundingRect_.topLeft();
  QPointF p3 = boundingRect_.topRight();
  QPointF p4 = boundingRect_.bottomRight();
//  painter->drawLine(p1,p2);
//  painter->drawLine(p2,p3);
//  painter->drawLine(p3,p4);
//  painter->drawLine(p4,p1);

  QBrush brush(QColor(220, 220, 220));
  QPen pen(brush, 2);
  painter->setPen(pen);
  painter->setBrush(brush);
  painter->drawRect(rotateRect());
  if ( true == enableBtn ) {
    painter->drawRect(bottomLeftRect());
    painter->drawRect(bottomRightRect());


    //  QBrush brush(QColor(255, 0, 0));
    //  QPen pen(brush, 2);

    brush.setColor(QColor(100, 100, 100));
    pen.setBrush(brush);
    pen.setWidth(2);
    painter->setPen(pen);
    painter->setBrush(brush);
    p1 = bottomLeftRect().bottomLeft();
    p2 = bottomLeftRect().topLeft();
    p3 = bottomLeftRect().topRight();
    p4 = bottomLeftRect().bottomRight();
    painter->drawLine(p1,p2);
    painter->drawLine(p2,p3);
    painter->drawLine(p3,p4);
    painter->drawLine(p4,p1);

    brush.setColor(QColor(255, 0, 0));
    pen.setBrush(brush);
    pen.setWidth(2);
    painter->setPen(pen);
    painter->setBrush(brush);
    p1 = bottomRightRect().bottomLeft();
    p2 = bottomRightRect().topLeft();
    p3 = bottomRightRect().topRight();
    p4 = bottomRightRect().bottomRight();
    painter->drawLine(p1,p2);
    painter->drawLine(p2,p3);
    painter->drawLine(p3,p4);
    painter->drawLine(p4,p1);
  }

  brush.setColor(QColor(100, 100, 0));
  pen.setBrush(brush);
  pen.setWidth(2);
  painter->setBrush(brush);
  painter->setPen(pen);
  p1 = rotateRect().bottomLeft();
  p2 = rotateRect().topLeft();
  p3 = rotateRect().topRight();
  p4 = rotateRect().bottomRight();
  painter->drawLine(p1,p2);
  painter->drawLine(p2,p3);
  painter->drawLine(p3,p4);
  painter->drawLine(p4,p1);

  brush.setColor(QColor(150, 50, 50));
  pen.setBrush(brush);
  painter->setPen(pen);
  painter->setBrush(brush);
  pen.setWidth(2);
  p1 = innerRect().bottomLeft();
  p2 = innerRect().topLeft();
  p3 = innerRect().topRight();
  p4 = innerRect().bottomRight();
  painter->drawLine(p1,p2);
  painter->drawLine(p2,p3);
  painter->drawLine(p3,p4);
  painter->drawLine(p4,p1);

  brush.setColor(QColor(150, 0, 150));
  pen.setBrush(brush);
  painter->setPen(pen);
  painter->setBrush(brush);
  pen.setWidth(2);
  QRectF newRect(innerRect());
  newRect.setWidth(newRect.width() - 2*zazor);
  newRect.setHeight(newRect.height() - 2*zazor);
  newRect.moveCenter(innerRect().center());
  if (geoPixmap_ != nullptr) {
    geoPixmap_->setPos(textDlg_->currentPos());
    geoPixmap_->render(painter, newRect.toRect(), document()->transform());
  }


  brush.setColor(QColor(0, 0, 0));
  pen.setBrush(brush);
  painter->setPen(pen);
  painter->setBrush(brush);
  pen.setWidth(2);
  QImage rotateImg = QImage(rotateImgPath);
  painter->drawImage( rotateRect(), rotateImg );
  if ( true == enableBtn ) {
    QImage posImg = QImage( imgForAlign(textDlg_->currentPos()) ) ;
    painter->drawImage( bottomLeftRect(), posImg );
    QImage deleteImg = QImage(deleteImgPath);
    painter->drawImage( bottomRightRect(), deleteImg );
  }

  brush.setColor(QColor(0, 255, 0));
  pen.setBrush(brush);
  painter->setPen(pen);
  painter->setBrush(brush);
  pen.setWidth(2);
  QRectF marker;
  marker.setSize(QSizeF(5, 5));
  marker.moveCenter( pos );
  painter->drawRect( marker );

  brush.setColor(QColor(255, 0, 0));
  pen.setBrush(brush);
  painter->setPen(pen);
  painter->setBrush(brush);
  pen.setWidth(1);
  marker.setSize(QSizeF(6, 6));
  painter->drawLine( marker.bottomLeft(), marker.topLeft() );
  painter->drawLine( marker.topLeft(), marker.topRight() );
  painter->drawLine( marker.topRight(), marker.bottomRight() );
  painter->drawLine( marker.bottomRight(), marker.bottomLeft() );

  if ( doc->isStub() ) {
    painter->restore();
  }
}

QPoint PixmapToolGraphicsItem::calcScenePoint() const
{
  map::Document* doc = document();
  if ( 0 != doc ) {
    return doc->coord2screen(geoPos_);
  }
  return QPoint();
}

void PixmapToolGraphicsItem::setStyle(StyleRole role, const QPen &pen, const QBrush &brush)
{
  pen_[role] = pen;
  brush_[role] = brush;
}

void PixmapToolGraphicsItem::setGeoPos(const GeoPoint &pos)
{
  map::Document* doc = document();
  if ( 0 == doc ) {
    error_log << QObject::tr("Невозможно обновить позицию, т.к. нулевой указатель на Document");
    return;
  }

  prepareGeometryChange();
//  setPos(doc->coord2screenf(pos));
  geoPos_ = pos;
  emit geoPosChaged(doc->screen2coord( mapToScene(boundingRect().bottomLeft())));
  if ( nullptr != geoPixmap_ ) {
    GeoVector v;
    v.append(geoPos_);
    geoPixmap_->setSkelet(v);
  }

}

void PixmapToolGraphicsItem::initGeoPixmap()
{
  MapScene* mapScene = qobject_cast<MapScene*>(scene());
  geoPixmap_ = new GeoPixmap( mapScene->document()->projection() );
  geoPixmap_->setScale(1,1);
  setGeoImg();
  GeoVector v;
  v.append(geoPos_);
  geoPixmap_->setSkelet(v);
  geoPixmap_->setPos(textDlg_->currentPos());
  prepareGeometryChange();
  geoPixmap_->setScaleXy( prop_.scalexy() );
  geoPixmap_->setRotateAngle( prop_.rotateangle() );
  geoPixmap_->setPos( prop_.pos() );
  calcRects();
}

void PixmapToolGraphicsItem::setGeoPixmap( const GeoPixmap *geoPixmap )
{
  prepareGeometryChange();
  if ( nullptr != geoPixmap_ ) {
    delete geoPixmap_;
    geoPixmap_ = nullptr;
  }
  prop_.mutable_scalexy()->CopyFrom( geoPixmap->scaleXy() );
  prop_.set_rotateangle( geoPixmap->rotateAngle() );
  prop_.set_pos( geoPixmap->pos() );
  textDlg_->setCurrent(prop_.pos());

  MapScene* mapScene = qobject_cast<MapScene*>(scene());
  geoPixmap_ = new GeoPixmap( mapScene->document()->projection() );
  geoPixmap_->setScale(1,1);
  geoPixmap_->setSkelet( geoPixmap->skelet() );
  geoPixmap_->setPos(textDlg_->currentPos());
  prepareGeometryChange();
  geoPixmap_->setScaleXy( prop_.scalexy() );
  geoPixmap_->setRotateAngle( prop_.rotateangle() );
  geoPixmap_->setPos( prop_.pos() );
  geoPixmap_->setImage( geoPixmap->image() );
  calcRects();
}

void PixmapToolGraphicsItem::setImage(const QString &imgPath)
{
  prepareGeometryChange();
  imgPath_ = imgPath;
  setGeoImg();
}

void PixmapToolGraphicsItem::slotSetImgColor(const QColor &color)
{
  if ( nullptr != geoPixmap_ ) {
    if (color.alpha() != 0) {
      geoPixmap_->setColor(color);
    }
    else {
      setGeoImg();
    }
  }
}

void PixmapToolGraphicsItem::finish( Layer *l )
{
  if ( kNormal == state_) {
    if (  nullptr != geoPixmap_ ) {
      geoPixmap_->copy(l);
      emit forDelete();
    }
  }
}

meteo::Property PixmapToolGraphicsItem::toProperty()
{
  if ( nullptr != geoPixmap_ ) {
    prop_.mutable_scalexy()->CopyFrom( geoPixmap_->scaleXy() );
    prop_.set_rotateangle( geoPixmap_->rotateAngle() );
    prop_.set_pos( geoPixmap_->pos() );
  }
  else {
    defaultProp();
  }
  return prop_;
}

void PixmapToolGraphicsItem::setProperty(const meteo::Property &prop)
{
  prop_.CopyFrom(prop);
  if ( nullptr != geoPixmap_ ) {
    geoPixmap_->setScaleXy( prop_.scalexy() );
    geoPixmap_->setRotateAngle( prop_.rotateangle() );
    geoPixmap_->setPos( prop_.pos() );
  }
  if ( nullptr != textDlg_ ) {
    textDlg_->setCurrent( prop_.pos() );
  }
}

void PixmapToolGraphicsItem::defaultProp()
{
  prop_.set_pos(Position::kTopCenter);
  prop_.set_rotateangle( 0 );
  double scalexy = 1;
  prop_.mutable_scalexy()->CopyFrom( qpointf2pointf( QPointF( scalexy, scalexy ) ) );
}

void PixmapToolGraphicsItem::hoverEnterEvent(QGraphicsSceneHoverEvent* event)
{
  QGraphicsItem::hoverEnterEvent(event);
}

void PixmapToolGraphicsItem::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
{
  QApplication::restoreOverrideCursor();
  QGraphicsItem::hoverLeaveEvent(event);
}

void PixmapToolGraphicsItem::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
  QPointF pos = event->pos();
  QApplication::restoreOverrideCursor();
  if ( true == inInnerRect(pos)) {
    switch (onRamkaInnerRect(pos)) {
    case kTop:
    case kBottom:
//      QApplication::setOverrideCursor(QCursor(Qt::SizeVerCursor));
      break;
    case kLeft:
    case kRight:
//      QApplication::setOverrideCursor(QCursor(Qt::SizeHorCursor));
      break;
    case kTopLeft:
    case kBottomRight:
      QApplication::setOverrideCursor(QCursor(Qt::SizeFDiagCursor));
      break;
    case kTopRight:
    case kBottomLeft:
      QApplication::setOverrideCursor(QCursor(Qt::SizeBDiagCursor));
      break;
    default:
      QApplication::setOverrideCursor(QCursor(Qt::SizeAllCursor));
      break;
    }
  }
  else  if ( ( true == enableBtn )
             && ( ( true == inBottomLeftRect(pos) )
                  || ( true == inTopRightRect(pos) ) ) ) {
    QApplication::setOverrideCursor(QCursor(Qt::PointingHandCursor));
  }
  else {
  }
  QGraphicsItem::hoverMoveEvent(event);
}

void PixmapToolGraphicsItem::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
  if ( event->button() != Qt::MouseButton::LeftButton) {
    return;
  }
  QPointF pos = event->pos();
  if (true == inInnerRect(pos)) {
    grabed_ = true;
    ramkaState_ = onRamkaInnerRect(pos);
    if (kNoGrab != ramkaState_ ){
      state_ = kResize;
      switch (ramkaState_) {
      case kBottomLeft:
        verticResize_ = kResizeBottom;
        horizontalResize_ = kResizeLeft;
        break;
      case kLeft:
        verticResize_ = kNoVerticResize;
//        horizontalResize_ = kResizeLeft;
        horizontalResize_ = kNoHorizResize;
        break;
      case kTopLeft:
        verticResize_ = kResizeTop;
        horizontalResize_ = kResizeLeft;
        break;
      case kTop:
//        verticResize_ = kResizeTop;
        verticResize_ = kNoVerticResize;
        horizontalResize_ = kNoHorizResize;
        break;
      case kTopRight:
        verticResize_ = kResizeTop;
        horizontalResize_ = kResizeRight;
        break;
      case kRight:
        verticResize_ = kNoVerticResize;
//        horizontalResize_ = kResizeRight;
        horizontalResize_ = kNoHorizResize;
        break;
      case kBottomRight:
        verticResize_ = kResizeBottom;
        horizontalResize_ = kResizeRight;
        break;
      case kBottom:
//        verticResize_ = kResizeBottom;
        verticResize_ = kNoVerticResize;
        horizontalResize_ = kNoHorizResize;
        break;
      default:
        error_log << QObject::tr("Ошибка определения положения курсора на объекте.");
        break;
      }
    }
    else {
      state_ = kMove;
    }
  }
  else if (true == inTopRightRect(pos)) {
    state_ = kRotate;
    grabed_ = true;
    angleStart_ = geoPixmap_->rotateAngle() * DEG2RAD;

  }
  else if ( true == enableBtn && true == inBottomLeftRect(pos)) {
    state_ = kChangePos;
    grabed_ = false;
    changeShowTxt();
  }
  else if ( true == enableBtn && true == inBottomRightRect(pos) ) {
    state_ = kDelete;
    grabed_ = false;
  }
  QGraphicsItem::mousePressEvent(event);
}

void PixmapToolGraphicsItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
  if (true == grabed_) {
    grabed_ = false;
    state_ = kNormal;
    if  ( boundingRect_.width() < minSizeBoundingRect) {
      boundingRect_.setWidth(minSizeBoundingRect);

    }
    if (boundingRect_.height() < minSizeBoundingRect) {
      boundingRect_.setHeight( minSizeBoundingRect);
    }
  }
  else {
    if ( kDelete == state_) {
      state_ = kNormal;
      if ( true == inBottomRightRect(event->pos())) {
        this->deleteLater();
        emit forDelete();
      }
    }
  }
  QGraphicsItem::mouseReleaseEvent(event);
}

void PixmapToolGraphicsItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
  switch (state_) {
  case kMove:
    setGeoPos(document()->screen2coord(event->scenePos()));
    break;
  case kResize:
    resizeRect(event->scenePos(), event->lastScenePos());
    break;
  case kRotate:
    calcRotateAngle(event->pos());
  default:
    break;
  }
  QGraphicsItem::mouseMoveEvent(event);
}

Document* PixmapToolGraphicsItem::document() const
{
  map::MapScene* mapScene = qobject_cast<map::MapScene*>(scene());
  if ( nullptr != mapScene ) {
    return mapScene->document();
  }
  return nullptr;
}

void PixmapToolGraphicsItem::slotPosChanged(int pos)
{
  prepareGeometryChange();
  if ( nullptr != textDlgPW_ ) {
    textDlgPW_->hide();
  }
  if ( nullptr != geoPixmap_ ) {
    prop_.set_pos(static_cast<Position>(pos));
    geoPixmap_->setPos( prop_.pos() );
    emit valueChanged(prop_);
  }
}

QString PixmapToolGraphicsItem::imgForAlign( int a )
{
  QString imgPath;
  switch (a) {
    case meteo::kTopLeft:
      imgPath = QString(":/meteo/icons/text/text-pos-top-left.png");
      break;
    case meteo::kLeftCenter:
      imgPath = QString(":/meteo/icons/text/text-pos-mid-left.png");
      break;
    case meteo::kBottomLeft:
      imgPath = QString(":/meteo/icons/text/text-pos-bottom-left.png");
      break;
    case meteo::kBottomCenter:
      imgPath = QString(":/meteo/icons/text/text-pos-bottom-center.png");
      break;
    case meteo::kCenter:
      imgPath = QString(":/meteo/icons/text/text-pos-mid-center.png");
      break;
    case meteo::kTopCenter:
      imgPath = QString(":/meteo/icons/text/text-pos-top-center.png");
      break;
    case meteo::kBottomRight:
      imgPath = QString(":/meteo/icons/text/text-pos-bottom-right.png");
      break;
    case meteo::kRightCenter:
      imgPath = QString(":/meteo/icons/text/text-pos-mid-right.png");
      break;
    case meteo::kNoPosition:
      imgPath = QString(":/meteo/icons/misc/critical-message.png");
      break;
    case meteo::kTopRight:
    default:
      imgPath = QString(":/meteo/icons/text/text-pos-top-right.png");
      break;
  }
  return imgPath;
}

}
}

