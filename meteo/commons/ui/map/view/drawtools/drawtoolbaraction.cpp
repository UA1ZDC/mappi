#include "drawtoolbaraction.h"

#include <meteo/commons/ui/map/view/actionbutton.h>
#include <meteo/commons/ui/map/view/widgethandler.h>
#include <meteo/commons/ui/map/view/drawtools/drawtoolbarwidget.h>

namespace meteo {
namespace map {

DrawToolbarAction::DrawToolbarAction(MapScene* scene) : Action(scene, kDrawToolsActionName),
  actBtn_(new ActionButton)
{
  actBtn_->setIcon(QIcon(":/meteo/icons/map/paint.png"));
  actBtn_->setToolTip( QObject::tr("Рисование") );
  widgetdraw_ = scene_->addWidget(actBtn_);
  item_ = widgetdraw_;
  item_->setZValue(10001);
}

void DrawToolbarAction::mouseReleaseEvent(QMouseEvent* e)
{
  Q_UNUSED( e );

  if ( widgetdraw_->isUnderMouse() ) {
    QWidget* w = WidgetHandler::instance()->createWidget(kDrawToolsWidgetName);

    // метод mouseReleaseEvent() вызывается до того как состояние кнопки изменится
    bool btnLastCheckState = actBtn_->isChecked();
    if ( 0 != w ) { w->setShown(!btnLastCheckState); }

    widgetdraw_->update();
  }
}

} // map
} // meteo
