#ifndef METEO_COMMONS_UI_MAP_VIEW_DRAWTOOLS_GEOGRAPHTEXTITEM_H
#define METEO_COMMONS_UI_MAP_VIEW_DRAWTOOLS_GEOGRAPHTEXTITEM_H

#include <qgraphicsitem.h>
#include <commons/geobasis/geopoint.h>
#include <meteo/commons/proto/meteo.pb.h>

namespace meteo {
namespace map {

class Document;
class Layer;

class GeoGraphTextItem : public QGraphicsTextItem
{
  Q_OBJECT
public:
  explicit GeoGraphTextItem( QGraphicsItem* parent = nullptr );
  explicit GeoGraphTextItem( const QString &text, QGraphicsItem* parent = nullptr );
  QRectF boundingRect() const override;
  GeoPoint anchorPoint() const { return anchorPoint_; }
  QPoint calcScenePoint() const;
  void paint( QPainter* painter, const QStyleOptionGraphicsItem *o, QWidget *w  ) override;
  void setGeoPos( const GeoPoint& pos );
  void setProperty( const meteo::Property& prop );
  void setTextAlignment( Qt::Alignment alignment );
  void setAlign( int pos );
  void finish( Layer *l );

public slots:
  void slotInsertSymb(QChar symb);
  void slotUpdateGeometry(int, int, int);
  void slotUpdateGeometry();

protected:
  void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
  void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
  void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
  void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;

signals:
  void forDelete();
  void anchorChanged( GeoPoint ) const;

private:
  Document* doc() const;
  void init();

private:
  GeoPoint geoPos_;
  mutable GeoPoint anchorPoint_;
  meteo::Property prop_;
  mutable QRectF anchorRect_ = QRectF( 0, 0, 6, 6 );
  mutable QRectF anchorGreenRect_ = QRectF( 2, 2, 2, 2 );
  mutable QRectF innerRect_;
  bool grabed_ = false;
  mutable int pos_ = 0;
  mutable QPointF offset_ = QPointF(0,0);
  mutable QPointF border_ = QPointF(0,0);
  Qt::Alignment alignment_;
};

}
}

#endif // METEO_COMMONS_UI_MAP_VIEW_DRAWTOOLS_GEOGRAPHTEXTITEM_H
