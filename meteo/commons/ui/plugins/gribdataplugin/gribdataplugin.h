#ifndef GRIBDATAPLUGIN_H
#define GRIBDATAPLUGIN_H

#include <QtGui>
#include "gribdatawidget.h"
#include <meteo/commons/ui/map/view/widgetplugin.h>

namespace meteo{
namespace map{

class GribDataPlugin : public WidgetPlugin
{
  Q_OBJECT
public:
  GribDataPlugin();
  ~GribDataPlugin();
  QWidget* createWidget(QWidget* parent) const;
};

}
}
#endif

