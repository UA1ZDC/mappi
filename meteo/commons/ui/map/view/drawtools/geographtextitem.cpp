#include "geographtextitem.h"

#include <qpainter.h>
#include <qgraphicssceneevent.h>
#include <qstyle.h>
#include <qstyleoption.h>
#include <qtextcursor.h>
#include <qtextdocument.h>

#include <cross-commons/debug/tlog.h>
#include <meteo/commons/ui/map/document.h>
#include <meteo/commons/ui/map/layer.h>
#include <meteo/commons/ui/map/view/mapscene.h>
#include <meteo/commons/ui/map/geotext.h>
#include <meteo/commons/ui/graphitems/markeritem.h>


namespace meteo {
namespace map {

GeoGraphTextItem::GeoGraphTextItem(QGraphicsItem *parent)
  : QGraphicsTextItem(parent),
    alignment_(Qt::AlignHCenter)
{
  init();
}

GeoGraphTextItem::GeoGraphTextItem(const QString &text, QGraphicsItem *parent)
  : QGraphicsTextItem( text, parent ),
    alignment_(Qt::AlignHCenter)
{
  init();
}


void GeoGraphTextItem::init()
{
  pos_ = Position::kTopLeft;
  slotUpdateGeometry();
  QObject::connect( document(), SIGNAL(contentsChange(int,int,int)), this, SLOT(slotUpdateGeometry(int,int,int)) );
}

QRectF GeoGraphTextItem::boundingRect() const
{
  QRectF br = QGraphicsTextItem::boundingRect();
  switch (pos_) {
  case Position::kTopLeft:
    border_.setX(10);
    border_.setY(5);
    offset_.setX(br.width());
    offset_.setY(br.height());
    break;
  case Position::kTopCenter:
    border_.setX(0);
    border_.setY(5);
    offset_.setX(br.width() * 0.5);
    offset_.setY(br.height());
    break;
  case Position::kTopRight:
    border_.setX(-10);
    border_.setY(5);
    offset_.setX(0);
    offset_.setY(br.height());
    break;
  case Position::kLeftCenter:
    border_.setX(10);
    border_.setY(0);
    offset_.setX(br.width());
    offset_.setY(br.height() * 0.5);
    break;
  case Position::kCenter:
    border_.setX(0);
    border_.setY(0);
    offset_.setX(br.width() * 0.5);
    offset_.setY(br.height() * 0.5);
    break;
  case Position::kRightCenter:
    border_.setX(-10);
    border_.setY(0);
    offset_.setX(0);
    offset_.setY(br.height() * 0.5);
    break;
  case Position::kBottomLeft:
    border_.setX(10);
    border_.setY(-5);
    offset_.setX(br.width());
    offset_.setY(0);
    break;
  case Position::kBottomCenter:
    border_.setX(0);
    border_.setY(-5);
    offset_.setX(br.width() * 0.5);
    offset_.setY(0);
    break;
  case Position::kBottomRight:
    border_.setX(-10);
    border_.setY(-5);
    offset_.setX(0);
    offset_.setY(0);
    break;
  default:
    break;
  }
  innerRect_ = br;
  QPointF topLeftIn = innerRect_.topLeft() + QPointF(10,5);
  QPointF bottomRightIn = innerRect_.bottomRight() - QPointF(10,5);
  innerRect_.setTopLeft(topLeftIn);
  innerRect_.setBottomRight(bottomRightIn);
  QPointF topLeft = br.topLeft() - QPointF(5,5);
  QPointF bottomRight = br.bottomRight() + QPointF(5,5);
  br.setTopLeft(topLeft);
  br.setBottomRight(bottomRight);
  Document* docum = doc();
  if ( nullptr != docum ) {
    anchorPoint_ = docum->screen2coord(mapToScene(innerRect_.topLeft() - QPointF(0, 104)));
    emit anchorChanged( anchorPoint_ );
  }
  return br;
}

void GeoGraphTextItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *o, QWidget *w)
{
  setPos(calcScenePoint()-offset_+border_);
  anchorRect_.moveCenter(offset_-border_);
  anchorGreenRect_.moveCenter(offset_-border_);
  map::Document* document = doc();
  if ( nullptr == document ) {
    error_log << QObject::tr("Невозможно нарисовать объект, т.к. нулевой указатель на Document");
    return;
  }
  painter->save();
  QBrush brush(  pen2qpen(prop_.pen()).color() );
  QPen pen(brush, 1);
  if (o->state & (QStyle::State_Selected | QStyle::State_HasFocus)) {
    pen.setStyle(Qt::PenStyle::DashLine);
    painter->setPen(pen);
    painter->setBrush(brush);
    painter->drawLine(innerRect_.topLeft(), innerRect_.topRight());
    painter->drawLine(innerRect_.topRight(), innerRect_.bottomRight());
    painter->drawLine(innerRect_.bottomLeft(), innerRect_.bottomRight());
    painter->drawLine(innerRect_.topLeft(), innerRect_.bottomLeft());
  }

  if ( ( prop_.ramka() & TextRamka::kBrush ) ==  TextRamka::kBrush) {
    brush.setColor( brush2qbrush(prop_.brush()).color() );
    pen.setBrush(brush);
    pen.setWidth(2);
    painter->setPen(pen);
    painter->setBrush(brush);
    painter->drawRect(innerRect_);
  }

  brush.setColor(QColor(255, 0, 0));
  pen.setBrush(brush);
  pen.setWidth(2);
  painter->setPen(pen);
  painter->setBrush(brush);
  painter->drawRect(anchorRect_);
  brush.setColor(QColor(0, 255, 0));
  pen.setBrush(brush);
  pen.setWidth(2);
  painter->setPen(pen);
  painter->setBrush(brush);
  painter->drawRect(anchorGreenRect_);
  painter->restore();
  painter->save();
  QStyleOptionGraphicsItem myOption(*o);
  myOption.state &= ~QStyle::StateFlag::State_Selected;
  myOption.state &= ~QStyle::StateFlag::State_HasFocus;
  QGraphicsTextItem::paint( painter, &myOption, w );
  painter->restore();
}

QPoint GeoGraphTextItem::calcScenePoint() const
{
  map::Document* document = doc();
  if ( 0 != document ) {
    return document->coord2screen(geoPos_);
  }
  return QPoint();
}

Document* GeoGraphTextItem::doc() const
{
  map::MapScene* mapScene = qobject_cast<map::MapScene*>(scene());
  if ( nullptr != mapScene ) {
    return mapScene->document();
  }
  return nullptr;
}

void GeoGraphTextItem::setGeoPos(const GeoPoint &pos)
{
  map::Document* document = doc();
  if ( nullptr == document ) {
    error_log << QObject::tr("Невозможно обновить позицию, т.к. нулевой указатель на Document");
    return;
  }
  prepareGeometryChange();
  geoPos_ = pos;
  setPos(calcScenePoint()-offset_+border_);
}

void GeoGraphTextItem::setProperty(const meteo::Property &prop)
{
  prop_.CopyFrom(prop);
  this->setFont(font2qfont(prop_.font()));
  this->setDefaultTextColor( pen2qpen( prop_.pen() ).color());
  this->setAlign(prop_.pos());
  slotUpdateGeometry();
}

void GeoGraphTextItem::setTextAlignment(Qt::Alignment alignment)
{
  alignment_ = alignment;
  QTextBlockFormat format;
  format.setAlignment(alignment);
  QTextCursor cursor = textCursor();      // save cursor position
  int position = textCursor().position();
  cursor.select(QTextCursor::Document);
  cursor.mergeBlockFormat(format);
  cursor.clearSelection();
  cursor.setPosition(position);           // restore cursor position
  setTextCursor(cursor);
}

void GeoGraphTextItem::setAlign(int pos)
{
  pos_ = pos;
}

void GeoGraphTextItem::finish( Layer *l )
{
  MapScene* mapScene = qobject_cast<MapScene*>(scene());
  GeoText* geoText = new GeoText(mapScene->document()->projection());
  MarkerItem* nodeItem = new MarkerItem(4, 0);
  scene()->addItem(nodeItem);
  QBrush brush(QColor(110, 125, 235));
  QPen pen(brush, 2);
  nodeItem->setStyle(MarkerItem::kNormalStyleRole, pen, brush);
  nodeItem->setStyle(MarkerItem::kHoverStyleRole, QPen(QBrush(Qt::black), 2), brush);
  nodeItem->setStyle(MarkerItem::kSelectedStyleRole, pen, brush);
  nodeItem->setGeoPos(geoPos_);
  nodeItem->setPos(nodeItem->calcScenePoint());
  GeoVector v;
  v.append(nodeItem->geoPos());
  geoText->setSkelet(v);
  geoText->setText(this->toPlainText());
  geoText->setProperty(prop_);
  geoText->copy(l);
  delete geoText;
  delete nodeItem;
  this->deleteLater();
  emit forDelete();
}

void GeoGraphTextItem::slotInsertSymb(QChar symb)
{
  setPlainText(toPlainText().append(symb));
}

void GeoGraphTextItem::slotUpdateGeometry(int, int, int)
{
  slotUpdateGeometry();
}

void GeoGraphTextItem::slotUpdateGeometry()
{
  QPointF topRightPrev = boundingRect().topRight();
  setTextWidth(-1);
  setTextWidth(boundingRect().width());
  setTextAlignment(alignment_);
  QPointF topRight = boundingRect().topRight();
  if (alignment_ & Qt::AlignRight)
  {
      setPos(pos() + (topRightPrev - topRight));
  }
}

void GeoGraphTextItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
  grabed_ = true;
  if (event->button() == Qt::MouseButton::RightButton) {
    event->setAccepted(false);
  }
  QGraphicsTextItem::mousePressEvent(event);
}

void GeoGraphTextItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
  grabed_ = false;
  QGraphicsTextItem::mouseReleaseEvent(event);
}

void GeoGraphTextItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
  if (  true == grabed_ ) {
    setGeoPos(doc()->screen2coord(event->scenePos()));
  }
  QGraphicsTextItem::mouseMoveEvent(event);
}

void GeoGraphTextItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *) {

}


}
}
