#ifndef METEO_MAP_VIEW_ACTIONS_HIDE_BUTTONS_ACTION_H
#define METEO_MAP_VIEW_ACTIONS_HIDE_BUTTONS_ACTION_H

#include <meteo/commons/ui/map/view/actions/action.h>

namespace meteo {
namespace map {

class HideButtonsAction : public Action
{
  Q_OBJECT

public:
  HideButtonsAction(MapScene* scene);

  void addActionsToMenu(Menu* menu) const;
  void setVisibleButtons(bool isVisible);

private slots:
  void slotSetVisibleButtons();

private:
  bool actionHasButton(Action* action) const;

private:
  bool isVisible_;

};

} // map
} // meteo

#endif // METEO_MAP_VIEW_ACTIONS_HIDE_BUTTONS_ACTION_H
