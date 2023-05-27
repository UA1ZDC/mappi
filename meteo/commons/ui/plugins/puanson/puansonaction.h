#ifndef METEO_MAP_VIEW_ACTIONS_PUANSONACTION_H
#define METEO_MAP_VIEW_ACTIONS_PUANSONACTION_H

#include <qmenu.h>
#include <qpoint.h>

#include <meteo/commons/ui/map/view/actions/action.h>
#include <meteo/commons/ui/map/puanson.h>

namespace meteo {
namespace map {

class MapView;
class Menu;
class PuansonView;
class PuansonItem;

class PuansonAction : public Action
{
  Q_OBJECT
  public:
    PuansonAction( MapScene* scene );
    ~PuansonAction();

    void mouseDoubleClickEvent( QMouseEvent* e );
    void mouseMoveEvent( QMouseEvent* event );

    void addActionsToMenu( Menu* menu ) const ;

    void setPeep( bool fl ) { peep_ = fl; }

  protected:
    bool eventFilter( QObject* o, QEvent* e );

  private:
    void showPunchMeteodata( Puanson* punch );
    void peepPuanson( Puanson* punch );

  private:
    PuansonView* view_;
    bool peep_;
    bool value_;
    QAction* peepaction_;
    QAction* leaveaction_;
    QAction* peepunchaction_;
    QAction* peepinfo_;
    QAction* value_action_;

    PuansonItem* currentpunch_;
    QList<PuansonItem*> peeps_;
    mutable PuansonItem* foremove_;

  private slots:
    void slotPeep();
    void slotPeepPunch();
    void slotPeepLeave();
    void slotShowPeepInfo();
    void slotShowValue();
};

class ActPlug : public ActionPlugin
{
  Q_OBJECT
  Q_PLUGIN_METADATA( IID "puansonview" FILE "puansonview.json" )
  public:
    ActPlug() : ActionPlugin("puansonaction") {}
    ~ActPlug(){}
    Action* create( MapScene* sc = 0 ) const { return new PuansonAction(sc); }
};

}
}

#endif
