#ifndef MAPTOOLWIDGET_H
#define MAPTOOLWIDGET_H

#include <QWidget>


namespace Ui {
  class MapToolWidget;
}

namespace meteo {

namespace map {

class MapToolWidget :
  public QWidget
{
  Q_OBJECT
public :
  explicit MapToolWidget(QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
  virtual ~MapToolWidget();

signals :
  void mapOpen();
  void docOpen();
  void docDownload();
  void runJob();

private :
  Ui::MapToolWidget* ui_;
};

}

}

#endif
