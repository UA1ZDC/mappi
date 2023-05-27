#ifndef COORDWIDGET_H
#define COORDWIDGET_H

#include <qlabel.h>
#include <qlayout.h>
#include <qtoolbutton.h>

#include <meteo/commons/ui/map/view/widgets/mapwidget.h>
#include <commons/geobasis/geopoint.h>

namespace meteo {
namespace map {

class CoordWidget : public QDialog
{
  Q_OBJECT
public:


  CoordWidget(QWidget *parent = 0);
  ~CoordWidget();

  void initWidget();
  
  bool lock() const;
  void setLock(bool on);
  void setText(const QString& text);
  const QString text() const;
  void setBtnVisible(bool on);
  void setLockVisible(bool on);
  void setCloseVisible(bool on);
  void setSettingsVisible(bool on);
  void setViewportPos(const QPoint& pos);
  QPoint viewportPos() const;
  QLabel* label() const;
  void setFrameColor(const QColor& color);

  void adjustSize();
  
protected slots:
  void slotClicked(bool on);
  void slotClose();

protected:
  QToolButton* close_;
  QToolButton* lock_;
  QLabel* label_;
  QHBoxLayout* layout_;
  QPoint viewportPos_;
  QColor color_;
  
    
  void mousePressEvent(QMouseEvent *);
 // void paintEvent(QPaintEvent*);
  void moveEvent(QMoveEvent *);
  void adjustSize(int width,int height);
signals:
  void sclosed();
};

}
}

#endif
