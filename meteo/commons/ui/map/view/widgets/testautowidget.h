#ifndef TESTAUTOWIDGET_H
#define TESTAUTOWIDGET_H

#include <QtGui>
#include <meteo/commons/ui/map/view/widgets/mapwidget.h>

namespace meteo {
namespace map {

class TestAutoWidget : public MapWidget
{
  Q_OBJECT
public:
  TestAutoWidget(QWidget *parent = 0, const QString& options = 0);
  ~TestAutoWidget();
  void setOptions(const QString& options);

};

}
}

#endif

