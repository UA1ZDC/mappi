#ifndef CITYACTION_H
#define CITYACTION_H

#include "action.h"
#include "citywidget.h"
#include <QtGui>

namespace meteo {
namespace map {

class MapView;

class CityAction : public Action
{
  Q_OBJECT
public:
  CityAction( MapScene* scene );
private:
  QToolButton* btn_;
  QGraphicsProxyWidget* btnitem_;
  CityWidget* widget_;

private slots:
  void slotShow(bool on);
};

}
}
#endif // CITYACTION_H
