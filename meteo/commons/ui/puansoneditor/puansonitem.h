#ifndef METEO_COMMONS_UI_MAP_VIEW_PUANSON_PUANSONITEM_H
#define METEO_COMMONS_UI_MAP_VIEW_PUANSON_PUANSONITEM_H

#include <qgraphicsitem.h>

#include <meteo/commons/ui/map/puanson.h>

class TMeteoData;

namespace meteo {

class Projection;

namespace puanson {

namespace proto {
  class Puanson;
  class CellRule;
}

class Item;
class Editor;

enum RectViewType
{
  kNoRect               = 0,
  kRuleRect             = 1,
  kPuansonRect          = 2,
  kPuansonRuleRect      = kPuansonRect | kRuleRect
};

enum AnchorViewType
{
  kNoAnchor             = 0,
  kRuleAnchor           = 1,
  kCenterAnchor         = 2,
  kPuansonRuleAnchor    = kCenterAnchor | kRuleAnchor
};

class GridItem : public QGraphicsItem
{
  public:
    GridItem();
    ~GridItem();

    void setBRect( const QRectF& brect ) { brect_ = brect; prepareGeometryChange(); }

    QRectF boundingRect() const { return brect_; }
    void paint( QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* parent = 0 );

  private:
    QRectF brect_;
};

class DescrItem : public QGraphicsItem
{
  public:
    enum { Type = UserType + 1001 };
    DescrItem( Item* parent, proto::CellRule* rule );
    ~DescrItem();

    int type() const { return Type; }

    QRectF boundingRect() const ;
    void paint( QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* parent = 0 );

    void mouseMoveEvent( QGraphicsSceneMouseEvent* e );
    void mouseReleaseEvent( QGraphicsSceneMouseEvent* e );
    void mousePressEvent( QGraphicsSceneMouseEvent* e );

    void setDescrSelected( bool fl );
    bool descrSelected() const { return selected_; }


  private:
    Item* item_;
    proto::CellRule* rule_;
    QPoint oldpnt_;
    bool press_;
    bool selected_;

  friend class Item;
};

class Item : public QGraphicsItem
{
  public:
    Item( Editor* editor );
    ~Item();

    void setSelectedDescr( int index );
    void descritemPressed( DescrItem* item );

    void setPunch( puanson::proto::Puanson* p );
    void updatePunch();

    void setDdff( int dd, int ff, control::QualityControl ddqual = control::RIGHT, control::QualityControl ffqual = control::RIGHT );
    void setParamValue( const proto::Id& id, const TMeteoParam& param );
    void setMeteodata( const TMeteoData& data );
    void loadTestMeteoData();

    void setRectType( int rect );
    void setAnchorType( int anchor );

    int rectType() const { return recttype_; }
    int anchorType() const { return anchortype_; }

    const TMeteoData& meteodata() const ;
    meteo::map::Puanson* puanson() const { return puanson_; }

    void buildChilds();

    QRectF boundingRect() const ;
    void paint( QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* parent = 0 );

  private:
    Editor* editor_;
    meteo::Projection* proj_;
    meteo::map::Puanson* puanson_;
    proto::Puanson* punch_;
    int recttype_;
    int anchortype_;
    QList<DescrItem*> items_;



  friend class DescrItem;

};

}
}

#endif
