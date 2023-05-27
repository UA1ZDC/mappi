#ifndef METEO_COMMONS_UI_MAP_LEGEND_H
#define METEO_COMMONS_UI_MAP_LEGEND_H

#include <qlabel.h>
#include <qmap.h>

#include <meteo/commons/proto/map_document.pb.h>
#include <meteo/commons/proto/puanson.pb.h>
#include <meteo/commons/proto/map_radar.pb.h>

namespace meteo {
namespace map {

class Document;
class Layer;

namespace internal {

class IncutLabel;

}

class Legend
{
  public:
    Legend( Document* doc );
    virtual ~Legend();

    const QString& uuid() const { return uuid_; }

    void setPositionOnDocument( Position pos ) { position_ = pos; repaint(); }
    void setPositionOnDocument( const QPoint& pos ) { floatposition_ = pos; position_ = kFloat; repaint(); }

    bool visible() const { return visible_; }
    void setVisible( bool fl );

    Orientation orient() const { return orient_; }
    void setOrient( Orientation o ) { orient_ = o; repaint(); }

    void loadLastParams();
    void saveLastParams();

    QRectF boundingRect() const;
    Position position() const { return position_; }
    const QPoint& floatPosition() const { return floatposition_; }
    void drawOnDocument( QPainter* painter );

    void setImage( const QImage& image );

    void copyFrom( Legend* i );

    void repaint() const ;

    void setProto( const proto::LegendParams& params );
    proto::LegendParams proto() const ;

    void setHLabel( const QString& lbl ) { hlabel_ = lbl; }
    void setVLabel( const QString& lbl ) { vlabel_ = lbl; }
    void setLabel( const QString& lbl ) { hlabel_ = lbl; vlabel_ = lbl; }

    static Layer* legendLayer( Document* doc );

    Document* document() const { return document_; }

  protected:
    const QString uuid_;
    Document* document_;
    internal::IncutLabel* label_;
    Position position_;
    QPoint floatposition_;
    bool visible_;
    bool custom_;
    Orientation orient_;
    QString hlabel_;
    QString vlabel_;

    virtual bool buildPixmap();
    bool buildByReadyImage( Layer* l );
    bool buildByField( Layer* l );
    bool buildByMrl( Layer* l );
    bool buildPunchLegend( const puanson::proto::CellRule& rule, const proto::RadarColor& clr );
    QSize calcPunchSize( const puanson::proto::CellRule& rule ) const ;

};

}
}

#endif
