#ifndef SCALEWIDGET_H
#define SCALEWIDGET_H

#include <QWidget>
#include <qslider.h>


namespace Ui{
  class ScaleWidget;
}

namespace meteo {
namespace map {
class ScaleWidget : public QWidget
{
  Q_OBJECT
public:
  explicit ScaleWidget(QWidget *parent = 0, int curZoom = 10, int minScale = 0, int maxScale = 10, int step = 1);
  ~ScaleWidget();
  void setZoom(int zoom);

signals:
  void zoom(int);

private slots:
  void slotZoomIn();
  void slotZoomOut();
  void slotZoom(int zoom);

private:
  Ui::ScaleWidget* ui_;
  int step_;
};

}
}

#endif // SCALEWIDGET_H
