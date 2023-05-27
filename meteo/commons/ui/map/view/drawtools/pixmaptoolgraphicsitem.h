#ifndef METEO_COMMONS_UI_MAP_VIEW_DRAWTOOLS_PIXMAPTOOLGRAPHICSITEM_H
#define METEO_COMMONS_UI_MAP_VIEW_DRAWTOOLS_PIXMAPTOOLGRAPHICSITEM_H

#include <qgraphicsitem.h>
#include <qpen.h>

#include <commons/geobasis/geopoint.h>
#include <meteo/commons/proto/meteo.pb.h>


class QGraphicsProxyWidget;

namespace meteo {
namespace map {

class Document;
class GeoPixmap;
class TextPosDlg;
class Layer;

class PixmapToolGraphicsItem : public QGraphicsObject
{
  Q_OBJECT
public:
  enum StyleRole {
    kNormalStyleRole,   //!< Общий стиль.
    kHoverStyleRole,    //!< Используется когда на маркер наведён указатель мыши.
    kSelectedStyleRole  //!< Используется когда маркер выделен.
  };
  enum State {
    kNormal,
    kMove,
    kResize,
    kDelete,
    kChangePos,
    kRotate
  };
  enum GrabRamka{
    kNoGrab,
    kLeft,
    kTopLeft,
    kTop,
    kTopRight,
    kRight,
    kBottomRight,
    kBottom,
    kBottomLeft
  };

  enum VerticResize {
    kNoVerticResize,
    kResizeTop,
    kResizeBottom
  };
  enum HorizontalResize {
    kNoHorizResize,
    kResizeLeft,
    kResizeRight
  };

  PixmapToolGraphicsItem(double size = 200 , QGraphicsItem* parent = nullptr );
  QRectF boundingRect() const;
  void paint(QPainter* painter, const QStyleOptionGraphicsItem *o, QWidget *w );
  QPoint calcScenePoint() const;
  void setStyle(StyleRole role, const QPen& pen, const QBrush& brush);
  void setGeoPos( const GeoPoint& pos);
  GeoPoint geoPos() { return geoPos_; }
  void setSize( double size ) { size_ = size; }
  void initGeoPixmap();
  void setGeoPixmap(const GeoPixmap *geoPixmap );
  void setImage( const QString& imgPath );
  double size() const { return size_; }
  void finish( Layer *l );
  bool inInnerRect(const QPointF& pos);
  bool inTopRightRect(const QPointF& pos);
  bool inBottomLeftRect(const QPointF& pos);
  bool inBottomRightRect(const QPointF& pos);
  meteo::Property toProperty();
  void setProperty(const meteo::Property& prop);
  void defaultProp();

protected:
  void hoverEnterEvent(QGraphicsSceneHoverEvent* event);
  void hoverLeaveEvent(QGraphicsSceneHoverEvent* event);
  void hoverMoveEvent(QGraphicsSceneHoverEvent* event);

  void mousePressEvent(QGraphicsSceneMouseEvent* event);
  void mouseReleaseEvent(QGraphicsSceneMouseEvent* event);
  void mouseMoveEvent(QGraphicsSceneMouseEvent* event);

private:
  Document* document() const;
  void initStyles();
  void setState( State newState );
  void resizeRect(const QPointF& pos, const QPointF& lastPos);
  void calcRotateAngle(const QPointF& pos);
  void calcRects() const ;
  void changeShowTxt();
  void setGeoImg();
  QRectF calcInnerRect() const;
  QRectF calcRotateRect() const ;
  QRectF calcBottomLeftRect() const;
  QRectF calcBottomRightRect() const;
  QRectF calcBoundingRect() const;
  QRectF innerRect() const { return innerRect_; }
  QRectF rotateRect() const { return rotateRect_; }
  QRectF bottomLeftRect() const { return bottomLeftRect_; }
  QRectF bottomRightRect() const { return bottomRightRect_; }
  GrabRamka onRamkaInnerRect(const QPointF& pos);
  QString imgForAlign( int a );

public slots:
  void slotSetImgColor( const QColor& color );

private slots:
  void slotPosChanged(int);

signals:
  void anchorChanged(GeoPoint) const;
  void geoPosChaged(GeoPoint);
  void valueChanged(meteo::Property);
  void forDelete();

private:
  qreal size_ = 0;
  State state_ = kNormal ;
  bool grabed_ = false;
  GrabRamka ramkaState_ = kNoGrab;
  VerticResize verticResize_ = kNoVerticResize;
  HorizontalResize horizontalResize_ = kNoHorizResize;
  GeoPixmap* geoPixmap_ = nullptr;
  GeoPoint geoPos_;
  mutable QRectF boundingRect_;
  mutable QRectF innerRect_;
  mutable QRectF bottomLeftRect_;
  mutable QRectF rotateRect_;
  mutable QRectF bottomRightRect_;
  QMap<StyleRole,QPen> pen_;
  QMap<StyleRole,QBrush> brush_;
  TextPosDlg* textDlg_ = nullptr;
  QGraphicsProxyWidget* textDlgPW_ = nullptr;
  QString imgPath_ = QString(":/meteo/icons/geopixmap/baricheskaj_logbina.png");
  meteo::Property prop_;
  qreal angleStart_ = 0;
};

}
}



#endif // METEO_COMMONS_UI_MAP_VIEW_DRAWTOOLS_PIXMAPTOOLGRAPHICSITEM_H
