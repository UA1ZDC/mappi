#pragma once

#include "coordsystem.h"
#include <qgraphicsscene.h>


namespace mappi {

namespace antenna {

class Scene :
  public QGraphicsScene
{
  Q_OBJECT
public :
  explicit Scene(CoordSystem& coord, QObject* parent = nullptr);
  virtual ~Scene();

  inline void setZeroAzimut(int v) { azimutCorrect_ = v; }

public slots :
  void setCurrentPosition(float azimut, float elevat);

protected :
  // virtual void drawForeground(QPainter* painter, const QRectF& rect);
  virtual void drawBackground(QPainter* painter, const QRectF& rect);

private :
  CoordSystem& coord_;
  CoordSystem::point_t src_;

  float azimutCorrect_;
};

}

}
