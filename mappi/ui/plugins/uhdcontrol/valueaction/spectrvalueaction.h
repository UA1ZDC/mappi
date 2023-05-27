#ifndef COORDACTIONSPECTR_H
#define COORDACTIONSPECTR_H

#include <meteo/commons/ui/map/view/actions/action.h>
#include <mappi/ui/plugins/uhdcontrol/labelitem.h>
#include <mappi/ui/plugins/uhdcontrol/lineitem.h>
#include <meteo/commons/ui/graphitems/markeritem.h>
#include "combolabelitem.h"

namespace meteo {
class MarkerItem;
} // meteo

namespace meteo {
namespace spectr {
class LineItem;
class ComboLabelItem;

struct ComboItemsData {
  spectr::ComboLabelItem* comboLabelItem;
  spectr::LabelItem* labelItem;
  spectr::LineItem* lineItem;
  QList<MarkerItem*> markers;
};


} // spectr
} // meteo

namespace meteo {
namespace map {

class MapView;
class WidgetItem;

class SpectrValueAction : public Action
{
  Q_OBJECT

public:
  static const QString kName;

  explicit SpectrValueAction(MapScene* scene);

  void mouseMoveEvent(QMouseEvent* e);
  void mousePressEvent(QMouseEvent* e);
  void mouseReleaseEvent(QMouseEvent* e);
  void addActionsToMenu(Menu* am) const;

  void deactivate();

private slots:
  void slotActionToggled(bool toggled);
  void slotDeleteTriggered();

private:
  inline bool isActive() const;

  bool hasLayerSpectr() const;
  bool isWidgetUnderMouse(const QPointF& screenPos) const;
  void updateComboLabel();

protected:
  virtual bool eventFilter(QObject* obj, QEvent* event);

private:
  QAction* menuAct_;
  QAction* deleteAct_;
  spectr::LineItem* lineItem_;
  spectr::ComboLabelItem* labelItem_;
  QList<MarkerItem*> markers_;
  map::WidgetItem* closeBtnItem_;
  QMap<spectr::ComboLabelItem*,spectr::ComboItemsData> comboItemsData_;
  //! Расстояние пройденое курсором между событиями press и release, для исключения
  //! ложного срабатывания события mouseClickEvent(), при смещении указателя на
  //! большое расстояние.
  QLineF moveDistance_;
};

} // map
} // meteo

#endif // COORDACTIONSPECTR_H
