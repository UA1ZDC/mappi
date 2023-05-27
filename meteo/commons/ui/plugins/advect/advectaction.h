#ifndef METEO_COMMONS_UI_PLUGINS_ADVECT_ADVECTACTION_H
#define METEO_COMMONS_UI_PLUGINS_ADVECT_ADVECTACTION_H

#include <meteo/commons/ui/map/view/actions/action.h>
#include <meteo/commons/ui/map/view/actionbutton.h>
#include <meteo/commons/ui/map/view/actions/traceaction.h>
#include "prepeartransferwidget.h"

#include <qpoint.h>

namespace meteo {
namespace map {

class DrawToolsAction;

class AdvectAction  : public TraceAction
{
  Q_OBJECT
public:
  AdvectAction( MapScene* scene );
  ~AdvectAction();
  void addActionsToMenu( Menu* menu ) const ;

private:
  virtual void mouseReleaseEvent( QMouseEvent* e );
  virtual void mousePressEvent( QMouseEvent* e );

private slots:
  void slotTransfer();
  void slotTransferObjects();
  void slotTryTransfer();
  void slotTryTransferObjects();
  void slotCurrentTraceChange(int index);
  void slotRemoveTraces();
  void slotHideTraces();
  void slotShowTraces();
  void slotHideArrows();
  void slotShowArrows();

signals:
  void currentTraceChange(int index) const;

private:
  void transferOneObject(const meteo::field::Skelet &skelet);

  QPoint point_;
  mutable QList<QString> objUUIDList_;
  PrepearTransferWidget* dlg_ = nullptr;
  DrawToolsAction* drawTools_ = nullptr;
  QMap< QString, QList<QString> > geoGroupMap_;

};

}
}

#endif //METEO_COMMONS_UI_PLUGINS_ADVECT_ADVECTACTION_H
