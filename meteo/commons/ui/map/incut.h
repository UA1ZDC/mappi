#ifndef METEO_COMMONS_UI_MAP_INCUT_H
#define METEO_COMMONS_UI_MAP_INCUT_H

#include <qimage.h>
#include <qmap.h>
#include <qstringlist.h>

#include <meteo/commons/proto/map_document.pb.h>

namespace meteo {
namespace map {

class Document;

namespace internal {

static const QString kMAP_KEY           = QString("map");
static const QString kWMO_KEY           = QString("wmo");
static const QString kAUTHOR_KEY        = QString("author");
static const QString kDATE_KEY          = QString("date");
static const QString kTERM_KEY          = QString("term");
static const QString kSCALE_KEY         = QString("scale");
static const QString kPROJ_KEY          = QString("projection");
static const QString kBASELAYER_KEY     = QString("baselayer");
static const QString kLAYER_KEY         = QString("layer");

class IncutLabel;

}

class Incut
{
  struct Podpis {
    QString key;
    QString value;
    Qt::Alignment align;
    bool visible;
  };
  public:
    Incut( Document* doc );
    ~Incut();

    const QString& uuid() const { return uuid_; }

    void setPositionOnDocument( Position pos );
    void setPositionOnDocument( const QPoint& pos );

    void setTextAlignment( Qt::Alignment align );

    bool visible() const { return visible_; }
    void setVisible( bool fl );

    void setProto( const proto::IncutParams& params );
    void resetFromClearProto( const proto::IncutParams& params );
    proto::IncutParams proto() const ;
    proto::IncutParams getClearProto() const;


    void loadLastParams();
    void saveLastParams();

    QRectF boundingRect() const;
    Position position() const { return position_; }
    const QPoint& floatPosition() const { return floatposition_; }
    void drawOnDocument( QPainter* painter );

    void setImage( const QImage& pixmap );

    void setLabel( const QString& key, const QString& value );
    void setLabel( const QString& key, bool fl );

    bool labelVisible( const QString& key ) const ;
    bool mapVisible() const ;
    bool wmoVisible() const ;
    bool authorVisible() const ;
    bool dateVisible() const ;
    bool termVisible() const ;
    bool scaleVisible() const ;
    bool projectionVisible() const ;
    bool baseLayerVisible() const ;
    bool layerVisible() const ;

    void setMapName( bool visible );
    void setMapName( const QString& mapname );

    void setWMOHeader( bool visible );
    void setWMOHeader( const QString& mapname );

    void setAuthor( bool visible );
    void setAuthor( const QString& mapname );

    void setDate( bool visible );
    void setDate( const QString& mapname );

    void setTerm( bool visible );
    void setTerm( const QString& mapname );

    void setScale( bool visible );

    void setProjection( bool visible );

    void setBaseLayerVisible( bool visible );
    void setLayerVisible( bool visible );

    void copyFrom( Incut* i );

    void repaint() const ;
    void setAllLabelVisible(bool );

  private:
    bool ispixmap_;
    const QString uuid_;
    Document* document_;
    internal::IncutLabel* label_;
    Position position_;
    QPoint floatposition_;
    QMap< QString, Podpis > textlabels_;
    QStringList order_;
    bool visible_;

    void initLabels();
    QString incutText() const ;

    QString mapName() const ;
    QString WMOHeader() const ;
    QString author() const ;
    QString date() const ;
    QString term() const ;
    QString scale() const ;
    QString projection() const ;
    QString baseLayer() const ;
    QString layer() const ;
};

namespace internal {

class IncutLabel
{
  public:
    IncutLabel();
    ~IncutLabel();

    void setText( const QString& text );
    void setImage( const QImage& img );
    const QImage& image() const { return image_; }

    void draw( const QPoint& topleft, QPainter* pntr );

    void setAlignment( Qt::Alignment a ) { align_ = a; };
    Qt::Alignment alignment() const { return align_; }

    QSize size() const { return image_.size(); }

    int width() const { return image_.size().width();}
    int height() const { return image_.size().height();}


  private:
    QImage image_;
    Qt::Alignment align_;
};

}

}
}

#endif
