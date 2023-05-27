#pragma once

#include "targetpoint.h"
#include "scene.h"
#include <qgraphicsview.h>


namespace mappi {

namespace antenna {

class View :
  public QGraphicsView
{
  Q_OBJECT
public :
  explicit View(CoordSystem& coord, QGraphicsScene* scene, QWidget* parent = nullptr);
  virtual ~View();

signals :
  void newPosition(float az, float en);
  void keepMoving();
  void stop();

public slots :
  void allowPosition(bool isSet);

protected :
  virtual void mousePressEvent(QMouseEvent* event) override;
  virtual void resizeEvent(QResizeEvent* event) override;

private :
  CoordSystem& coord_;
  TargetPoint* indicator_;

  CoordSystem::point_t dst_;
};

}

}
