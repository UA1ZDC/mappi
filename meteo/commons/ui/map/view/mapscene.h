#ifndef METEO_MAP_VIEW_MAPSCENE_H
#define METEO_MAP_VIEW_MAPSCENE_H

#include <qgraphicsscene.h>

#include <meteo/commons/ui/map/document.h>

namespace meteo {
namespace map {

class MapView;

class Action;
class MapLayer;
class MapScene;

namespace internal {

//! отвечает за расположение экшенов на сцене
class ActionSpace
{
  public:
    ActionSpace( int pos, MapScene* scene );
    ~ActionSpace();

    void addAction( const QPoint& pos, Action* a );

    Action* takeAction( const QPoint& pos );
    Action* takeAction( const QString& action );
    void takeAction( Action* a );

    bool hasAction( const QString& name ) const ;
    bool hasAction( Action* a ) const ;

    void setSize( const QSize& size ) { size_ = size; }
    void setCellSize( int32_t sz ) { cellsize_ = sz; }
    void setPos( int pos ) { pos_ = pos; }
    int pos() const { return pos_; }
    void arrangeActions();

  private:
    int pos_;
    QSize size_;
    int32_t cellsize_;
    MapScene* scene_;
    QMap< QPoint, Action*> actions_;

    QSize size() const ;
    QRect rect() const ;
};

}

class  MapScene : public QGraphicsScene
{
  Q_OBJECT
  public:
    enum DrawState
    {
      kRenderLayers             = 0,
      kRenderCachePicture       = 1
    };
    MapScene( Document* document, MapView* parent );
    ~MapScene();

    MapView* mapview() const ;
    bool hasMapview() const { if ( 0 == view_ ) { return false; } return true; }

    void turnEvents();
    void muteEvents();

    Document* document() const { return document_; }

    void setDocument( Document* doc );
    void rmDocument();
    Document* takeDocument();

    double scale() const { return document_->scale(); }
    void setScale( double scale );

    void setScreenCenter( const QPoint& point );

    bool eventFilter( QObject* watched, QEvent* event );

    bool isMapItem( QGraphicsItem* item );

    QPoint coord2screen( const GeoPoint& coord ) const { return document_->coord2screen(coord); }
    GeoPoint screen2coord( const QPoint& point ) const { return document_->screen2coord(point); }

    void addAction(Action* action );
    void addAction(Action* action, int cornerpos, const QPoint& address = QPoint( -1, -1 ) );
    Action* takeAction( const QString& name );
    Action* getAction(const QString& name) const;
    void removeAction( Action* action );
    QList<Action*> baseactions() const { return baseactions_; }
    QList<Action*> actions() const { return actions_; }
    bool hasAction( const QString& name ) const ;

    void arrangeActions();

    void initActions();
    void initBaseActions();

    const QStringList& pluginGroups() const;
    void setPluginGroups( const QStringList& groups );
    void addPluginGroup( const QString& gr );
    void rmPluginGroup( const QString& gr );

    void setDrawState( DrawState st ) { state_ = st; }
    DrawState drawState() const { return state_; }

    void addVisibleItem( QGraphicsItem* item );
    void removeVisibleItem( QGraphicsItem* item );

    void drawBackground( QPainter* painter, const QRectF& rect );

  protected:
    void drawItems( QPainter* painter, int numitems, QGraphicsItem* items[], const QStyleOptionGraphicsItem opt[], QWidget* wgt );
    void drawForeground( QPainter* painter, const QRectF& rect );

    void wheelEvent( QGraphicsSceneWheelEvent* e );
    void keyPressEvent(QKeyEvent *event);

  private:
    Document* document_;
    QPixmap docpicture_;
    MapView* view_;
    QList<Action*> baseactions_;
    QList<Action*> actions_;
    QList<internal::ActionSpace> spaces_;
    QStringList plugroups_;
    DrawState state_;
    QPixmap cacheimage_;
    GeoVector cachedbunch_;
    QList<QGraphicsItem*> visibleitems_;

    void setMapview( MapView* v );
    void reloadActionPlugins();

    QTransform cachedTransform() const ;

    internal::ActionSpace* space( int pos );

  signals:
    void documentChanged( Document* doc );
    void documentRemoved( Document* doc );

  friend class MapView;
};

}
}

bool operator<( const QPoint& p1, const QPoint& p2 );

#endif
