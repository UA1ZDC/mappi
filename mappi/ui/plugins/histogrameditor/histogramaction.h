#ifndef MAPPI_UI_PLUGINS_HISTOGRAMEDITOR_HISTOGRAMACTION_H
#define MAPPI_UI_PLUGINS_HISTOGRAMEDITOR_HISTOGRAMACTION_H

#include <qmenu.h>

#include <meteo/commons/ui/map/view/actions/action.h>
#include "histogrameditor.h"

namespace meteo {

class HistogramAction : public map::Action
{
  Q_OBJECT

public:
  static const QString kName;

  HistogramAction(map::MapScene* mapscene);
  ~HistogramAction();
  void addActionsToMenu(QMenu *menu) const;

protected:
  virtual bool eventFilter(QObject* obj, QEvent* event);

private:
  map::MapScene* mapscene_ = nullptr;
  QAction* menuAct_ = nullptr;
  HistogramEditor* widget_;

private slots:
  void slotShowWidget();
};

}//meteo

#endif // MAPPI_UI_PLUGINS_HISTOGRAMEDITOR_HISTOGRAMACTION_H
