#include "hidebuttonsaction.h"

#include "../mapscene.h"
#include "../menu.h"

#include <QAbstractButton>
#include <QAction>
#include <QGraphicsProxyWidget>

namespace meteo {
namespace map {

HideButtonsAction::HideButtonsAction(MapScene* scene) :
  Action(scene, "hidebuttonsaction"),
  isVisible_(true)
{
}

void HideButtonsAction::addActionsToMenu(Menu* menu) const
{
  QAction* action = new QAction(tr("Скрыть кнопки"), menu);
  action->setCheckable(true);
  action->setChecked(!isVisible_);
  connect(action, SIGNAL(triggered()), this, SLOT(slotSetVisibleButtons()));

  menu->addDocumentAction(action);
}

void HideButtonsAction::setVisibleButtons(bool isVisible){
    foreach (Action* each, scene_->actions()) {
      if (actionHasButton(each)) {
        each->item()->setVisible(isVisible);
      }
    }

    foreach (Action* each, scene_->baseactions()) {
      if (actionHasButton(each)) {
        each->item()->setVisible(isVisible);
      }
    }
}


void HideButtonsAction::slotSetVisibleButtons()
{
  isVisible_ = !isVisible_;
  setVisibleButtons( isVisible_);

}

bool HideButtonsAction::actionHasButton(Action *action) const
{
  if (action != 0) {
    if (action->item() != 0) {
      QGraphicsProxyWidget* proxy = qgraphicsitem_cast<QGraphicsProxyWidget*>(action->item());
      if (proxy != 0 && qobject_cast<QAbstractButton*>(proxy->widget()) != 0) {
        return true;
      }
    }
    if (action->wgt() != 0) {
      if (qobject_cast<const QAbstractButton*>(action->wgt()) != 0) {
        return true;
      }
    }
  }
  return false;
}

} // map
} // meteo
