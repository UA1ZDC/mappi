#ifndef GeodezPlugin_H
#define GeodezPlugin_H

#include <QtGui>
#include <meteo/commons/ui/map/view/widgetplugin.h>

namespace meteo{
namespace map{

class GeodezPlugin : public WidgetPlugin
{
  Q_OBJECT
public:
  GeodezPlugin();
  ~GeodezPlugin();
  QWidget* createWidget(QWidget* parent = 0) const;
};

}
}
#endif

