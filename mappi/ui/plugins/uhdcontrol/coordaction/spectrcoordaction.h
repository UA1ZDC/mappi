#ifndef SPECTRCOORDACTION_H
#define SPECTRCOORDACTION_H


#include <qlist.h>

#include <meteo/commons/ui/map/view/actions/action.h>

namespace meteo {
class MarkerItem;
} // meteo

namespace meteo {
namespace spectr {
class LineItem;
class LabelItem;
} // spectr
} // meteo

namespace meteo {
namespace map {
class MapView;
class WidgetItem;
} // map
} // meteo

namespace meteo {
namespace map {

class SpectrCoordAction : public Action
{
  Q_OBJECT

public:
  static const QString kName;

  explicit SpectrCoordAction(MapScene *scene);
  ~SpectrCoordAction();

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

  void removeItems();

private:
  QAction* deleteAct_;
  QAction* menuAct_;
  spectr::LineItem* lineItem_;
  spectr::LabelItem* dtItem_;
  QList<MarkerItem*> markers_;
  QList<spectr::LabelItem*> labels_;
  map::WidgetItem* closeBtnItem_;

  //! Расстояние пройденое курсором между событиями press и release, для исключения
  //! ложного срабатывания события mouseClickEvent(), при смещении указателя на
  //! большое расстояние.
  QLineF moveDistance_;
};

} // map
} // meteo

#endif // SPECTRCOORDACTION_H
