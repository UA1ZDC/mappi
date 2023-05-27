#ifndef SYNOPWIDGET_H
#define SYNOPWIDGET_H

#include <meteo/commons/ui/map/view/widgets/mapwidget.h>
#include <commons/geobasis/geopoint.h>
#include <QtWidgets>

namespace meteo {
namespace map {


class SynopWidget : public QDialog
{
  Q_OBJECT

public:
  SynopWidget(QWidget *parent = 0);
  virtual ~SynopWidget(){}


  void setSit(const QString& text);
  void setHeader1(const QString& text);

  void setFenom(const QString& text);
  //const QString text() const;
//  void setViewportPos(const QPoint& pos);
//  QPoint viewportPos() const;
  //QLabel* label() const;
  //void setFrameColor(const QColor& color);

private slots:
  //void slotClose();

private:
  QLabel* labelHeader1_;
  QLabel* labelSit_;
  QLabel* labelHeader2_;
  QLabel* labelFenom_;
  QVBoxLayout* layout_;
  //QPoint viewportPos_;

};



}
}

#endif // SYNOPWIDGET_H
