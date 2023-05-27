#ifndef RULERACTION_H
#define RULERACTION_H

#include <meteo/commons/ui/map/view/actions/action.h>
#include <meteo/commons/ui/map/view/actionbutton.h>
#include <meteo/commons/ui/map/view/actions/traceaction.h>

namespace meteo {
namespace map {

class RulerAction  : public TraceAction
{
  Q_OBJECT
public:
  RulerAction ( MapScene* scene );
  void addActionsToMenu( Menu* menu ) const ;
  virtual TraceItem* addNode( const GeoPoint &gp, bool reverse = false,
                      TraceItem::CollideItemOption collide = TraceItem::HideMark);
  virtual TraceItem* insertNode( const GeoPoint &gp, TraceItem* after, TraceItem* before,
                        TraceItem::CollideItemOption collide = TraceItem::HideMark);

protected:
  bool eventFilter( QObject* o, QEvent* e );

private:
  ActionButton* btn_;

  virtual void mouseReleaseEvent( QMouseEvent* e );
};

}
}

#endif
