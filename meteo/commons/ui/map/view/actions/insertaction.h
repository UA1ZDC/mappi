#ifndef METEO_MAP_VIEW_ACTIONS_INSERTACTION_H
#define METEO_MAP_VIEW_ACTIONS_INSERTACTION_H

#include "action.h"

namespace meteo {
namespace map {

class Layer;

class InsertAction : public Action
{
  Q_OBJECT
public:
  InsertAction( MapScene* scene );
  ~InsertAction();
  void addActionsToMenu( Menu* menu) const;

private slots:
  void slotDataChanged();
  void slotPaste();

protected:
  void keyPressEvent(QKeyEvent* event);

private:
  Layer* getLayer();
  Layer* createLayer();

private:
  QClipboard* clipboard_ = nullptr;
  QAction* showAction_ = nullptr;
  bool show_ = false;
  QString layerUuid_;


};

}
}

#endif // METEO_MAP_VIEW_ACTIONS_INSERTACTION_H
