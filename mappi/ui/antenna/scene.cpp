#include "scene.h"
#include <qpainter.h>


namespace mappi {

namespace antenna {

static const int AZIMUT_CORRECTION_MAX = -359;

Scene::Scene(CoordSystem& coord, QObject* parent /*=*/) :
    QGraphicsScene(parent),
  coord_(coord),
  src_({ -360, -360 }),
  azimutCorrect_(AZIMUT_CORRECTION_MAX)
{
}

Scene::~Scene()
{
}

void Scene::setCurrentPosition(float azimut, float elevat)
{
  src_ = { azimut , elevat };
}

// void Scene::drawForeground(QPainter* painter, const QRectF& rect)
void Scene::drawBackground(QPainter* painter, const QRectF& rect)
{
  painter->setPen(QColor(Qt::darkBlue));

  float offset;

  CoordGrid azimutGrid(coord_.x());
  azimutGrid.init(true);
  qreal rightMargin = rect.x() + rect.width();
  while (azimutGrid.next(&offset) && (0.01 < rightMargin - (rect.x() + offset))) {
    int x = rect.x() + offset;
    painter->drawLine(QLineF(x, rect.top(), x, rect.bottom()));
  }

  offset = 0;

  CoordGrid elevatGrid(coord_.y());
  elevatGrid.init(true);
  qreal bottomMargin = rect.y() + rect.height();

  while (elevatGrid.next(&offset) && (0.01 < bottomMargin - (rect.y() + offset))) {
    int y = rect.y() + offset;
    painter->drawLine(QLineF(rect.left(), y, rect.right(), y));
  }

  // azimut zero
  if (azimutCorrect_ != AZIMUT_CORRECTION_MAX) {
    painter->setPen(QPen(Qt::red, 2, Qt::DotLine));
    offset = coord_.x().fromDegree(azimutCorrect_);

    int x = rect.x() + offset;
    painter->drawLine(QLineF(x, rect.top(), x, rect.bottom()));
  }

  // current position
  int x = coord_.x().fromDegree(src_.x);
  int y = coord_.y().fromDegree(src_.y);

  painter->setPen(QPen(Qt::darkGreen, 2, Qt::DashDotDotLine));
  painter->drawLine(rect.x() + x, rect.y(), rect.x() + x, rect.y() + rect.height());
  painter->drawLine(rect.x(), rect.y() + y, rect.x() + rect.width(), rect.y() + y);
}

}

}
