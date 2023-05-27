#ifndef METEO_MAP_ACTIONS_ACTION_H
#define METEO_MAP_ACTIONS_ACTION_H

#include <qstring.h>
#include <qobject.h>
#include <qgraphicswidget.h>
#include <qevent.h>
#include <qwidget.h>
#include <qplugin.h>


namespace meteo {
namespace map {

class MapView;
class MapScene;
class Menu;

namespace internal {
class ActionSpace;
}

class Action : public QObject
{
  Q_OBJECT
  public:
    virtual ~Action();

  bool deleteble(){return is_deleteble_;}
    void setCorner( int crnr ) { corner_ = crnr; }
    void setCornerAddress( const QPoint& addr ) { address_ = addr; }
    void setHasCorner( bool fl ) { hasaddress_ = fl; }

    int corner() const { return corner_; }
    const QPoint& cornerAddress() const { return address_; }
    bool hasCorner() const { return hasaddress_; }

    QGraphicsItem* item() { return item_; }
    const QString& name() { return name_; }
    const QWidget* wgt() { return wgt_; }

    virtual void setScene( MapScene* sc );
    MapScene* scene() { return  scene_; }
    virtual void removeFromScene();

    const QList<QAction*>& actions() const { return actions_; }

    void setCellSize( int32_t sz ) { cellsize_ = sz; }

    virtual void contextMenuEvent( QContextMenuEvent* event ) { Q_UNUSED(event);}
    virtual void dragEnterEvent( QDragEnterEvent* event ) { Q_UNUSED(event);}
    virtual void dragLeaveEvent( QDragLeaveEvent* event ) { Q_UNUSED(event);}
    virtual void dragMoveEvent( QDragMoveEvent* event ) { Q_UNUSED(event);}
    virtual void dropEvent( QDropEvent* event ) { Q_UNUSED(event);}
    virtual void focusInEvent( QFocusEvent* event ) { Q_UNUSED(event);}
    virtual void focusOutEvent( QFocusEvent* event ) { Q_UNUSED(event);}
    virtual void inputMethodEvent( QInputMethodEvent* event ) { Q_UNUSED(event);}
    virtual void keyPressEvent( QKeyEvent* event ) { Q_UNUSED(event);}
    virtual void keyReleaseEvent( QKeyEvent* event ) { Q_UNUSED(event);}
    virtual void mouseDoubleClickEvent( QMouseEvent* event ) { Q_UNUSED(event);}
    virtual void mouseMoveEvent( QMouseEvent* event ) { Q_UNUSED(event);}
    virtual void mousePressEvent( QMouseEvent* event ) { Q_UNUSED(event);}
    virtual void mouseReleaseEvent( QMouseEvent* event ) { Q_UNUSED(event);}
    virtual void paintEvent( QPaintEvent* event ) { Q_UNUSED(event);}
    virtual void resizeEvent( QResizeEvent* event ) { Q_UNUSED(event);}
    virtual void showEvent( QShowEvent* event ) { Q_UNUSED(event);}
    virtual void wheelEvent( QWheelEvent* event ) { Q_UNUSED(event);}

    void setSize( const QSize& sz ) { size_ = sz; }
    const QSize& size() { return size_; }

    //! Добавить действия пользователя в menu (meteo::map::Menu) ( у класса Menu есть возможность указать секцию меню, напр. документ,слой, объект)
    virtual void addActionsToMenu( Menu* menu ) const ;
    //! Добавить действия пользователя в menu (QMenu) - реализация в базовом классе добавляет в menu все элементы actions_
    virtual void addActionsToMenu( QMenu* menu ) const ;
    virtual void init(){}

    bool disabled() const { return disabled_; }
    void setDisabled( bool fl ) { disabled_ = fl; }



  protected:
    MapScene* scene_;
    QWidget* wgt_;
    QGraphicsItem* item_;
    const QString name_;
    int32_t cellsize_;
    QSize size_;
    QList<QAction*> actions_;
    int corner_;
    QPoint address_;
    bool hasaddress_;
    bool disabled_ = false;
    bool is_deleteble_;
    Action( MapScene* scene, const QString& name );

    bool hasView() const ;
    MapView* view() const ;

};

class ActionPlugin : public QObject
{
  Q_OBJECT
  public:
    ActionPlugin( const QString& n )
      : QObject(), name_(n) {}
    virtual ~ActionPlugin(){}

    virtual Action* create( MapScene* scene = nullptr ) const = 0;

    const QString& actioName() const { return name_; }

  private:
    const QString name_;
};

}
}

#endif
