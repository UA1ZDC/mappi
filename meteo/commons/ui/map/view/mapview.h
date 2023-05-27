#ifndef METEO_MAP_VIEW_MAPVIEW_H
#define METEO_MAP_VIEW_MAPVIEW_H

#include <qstring.h>
#include <qmap.h>
#include <qgraphicsview.h>
#include <qgraphicswidget.h>
#include <QtGui>
#include <QtCore>


#include <meteo/commons/ui/map/document.h>
#include "mapscene.h"

class QSlider;
class QGraphicsOpacityEffect;

namespace meteo {
namespace map {

class Action;
class MapWindow;

class MapView : public QGraphicsView
{
  Q_OBJECT
  public:
    MapView( MapWindow* parent );
    ~MapView();

    void scrollToCenter();

    void turnSceneEvents() { if ( 0 != mapscene_ ) { mapscene_->turnEvents(); } }
    void muteSceneEvents() { if ( 0 != mapscene_ ) { mapscene_->muteEvents(); } }

    void setMapscene( MapScene* sc );
    MapScene* mapscene() const { return mapscene_; }
    bool hasMapscene() const { if ( 0 == mapscene_ ) { return false; }; return true; }

    MapWindow* window() const { return window_; }

    QList<Action*> actions() const { if ( 0 == mapscene_ ) { return QList<Action*>(); } return mapscene_->actions(); }

  protected:
    void contextMenuEvent( QContextMenuEvent* event );
    void dragEnterEvent( QDragEnterEvent* event );
    void dragLeaveEvent( QDragLeaveEvent* event );
    void dragMoveEvent( QDragMoveEvent* event );
    void dropEvent( QDropEvent* event );
    void focusInEvent( QFocusEvent* event );
    void focusOutEvent( QFocusEvent* event );
    void inputMethodEvent( QInputMethodEvent* event );
    void keyPressEvent( QKeyEvent* event );
    void keyReleaseEvent( QKeyEvent* event );
    void mouseDoubleClickEvent( QMouseEvent* event );
    void mouseMoveEvent( QMouseEvent* event );
    void mousePressEvent( QMouseEvent* event );
    void mouseReleaseEvent( QMouseEvent* event );
    void paintEvent( QPaintEvent* event );
    void resizeEvent( QResizeEvent* event );
    void showEvent( QShowEvent* event );
    void wheelEvent( QWheelEvent* event );
    void scrollContentsBy( int dx, int dy );

  private:
    MapScene* mapscene_;
    MapWindow* window_;

  signals:
    void sceneChanged( MapScene* scene );

  friend class Action;
  friend class MapScene;
};

}

}


#endif
