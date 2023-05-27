#ifndef COORDPROXYWIDGET_H
#define COORDPROXYWIDGET_H

#include <qgraphicsproxywidget.h>

#include <commons/geobasis/geopoint.h>
#include <meteo/commons/ui/map/view/widgets/coordwidget.h>
namespace meteo {
namespace map {

class BaloonProxyWidget : public QGraphicsProxyWidget
{
  Q_OBJECT
public:
  enum State{
    Moving = 0,
    Pointer  = 1,
    Fixed  = 2
  };

  BaloonProxyWidget(QGraphicsItem *parent = 0, Qt::WindowFlags wFlags = 0);
  
  virtual ~BaloonProxyWidget();
  State state() const;
  void setState(State state);
  GeoPoint gp() const;
  void setGeoPoint(GeoPoint gp);
  QPoint clickPos() const;
  void adjustPos(const QPointF &);
  bool onHover ( ) const ;

  void setLeftOrient(bool l) { leftOrient_ = l; }
  
protected:
  void paint(QPainter* p, const QStyleOptionGraphicsItem * option, QWidget * widget);
  
  void mousePressEvent(QGraphicsSceneMouseEvent *event);
  void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
  void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
  void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
  void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
    
private:
  State state_;
  GeoPoint gp_;
  bool press_;
  QPoint click_;
  bool on_hover;
  bool leftOrient_ = false;
  
};

}
}
#endif // FIELDPROXYWIDGET_H
