#ifndef METEO_COMMONS_UI_PLUGINS_METEOGRAM_METEOGRAMACTION_H
#define METEO_COMMONS_UI_PLUGINS_METEOGRAM_METEOGRAMACTION_H

#include <meteo/commons/ui/map/view/actions/action.h>
//#include <meteo/commons/ui/map/view/actionbutton.h>

class ActionButton;

namespace meteo {
class MeteogramWindow;
} // meteo

namespace meteo {
namespace map {
class Document;
} // map
} // meteo

namespace meteo {

class MeteogramAction : public map::Action
{
  Q_OBJECT
public:
  MeteogramAction(map::MapScene* scene, map::Document* doc);
  virtual ~MeteogramAction();
  MeteogramWindow* window() const { return meteogramWindow_; }
 // void addActionsToMenu( map::Menu* menu ) const ;

public slots:
  void slotShow(bool show);

protected:
  virtual bool eventFilter(QObject *obj, QEvent *event);

private:
  ActionButton* btn_ = nullptr;
  QGraphicsProxyWidget* btnitem_ = nullptr;
  MeteogramWindow* meteogramWindow_ = nullptr;
  map::Document* document_ = nullptr;
};

} // meteo

#endif // METEO_COMMONS_UI_PLUGINS_METEOGRAM_METEOGRAMACTION_H
