#include "coordsystem.h"
#include <math.h>


namespace mappi {

namespace antenna {

// class CoordAxis
CoordAxis::CoordAxis() :
  min_(1),
  max_(1),
  scale_(1)
{
}

CoordAxis::CoordAxis(float min, float max) :
  min_(min),
  max_(max),
  scale_(1)
{
}

CoordAxis::~CoordAxis()
{
}

void CoordAxis::setRange(float min, float max)
{
  min_ = min;
  max_ = max;
}

void CoordAxis::initScale(float width)
{
  scale_ = width / (::abs(min_) + max_);
}

float CoordAxis::fromDegree(float degree)
{
  return (::abs(min_) + degree) * scale_;
}

float CoordAxis::toDegree(float point)
{
  return (point / scale_) - ::abs(min_);
}


// class CoordSystem
CoordSystem::CoordSystem()
{
}

CoordSystem::CoordSystem(float minX, float maxX, float minY, float maxY) :
  x_(CoordAxis(minX, maxX)),
  y_(CoordAxis(minY, maxY))
{
}

CoordSystem::~CoordSystem()
{
}

void CoordSystem::setScope(float width, float height)
{
  x_.initScale(width);
  y_.initScale(height);
}

CoordSystem::point_t CoordSystem::fromDegree(const CoordSystem::point_t& point)
{
  return fromDegree(point.x, point.y);
}

CoordSystem::point_t CoordSystem::fromDegree(float az, float en)
{
  return { x_.fromDegree(az), y_.fromDegree(en) };
}

CoordSystem::point_t CoordSystem::toDegree(const CoordSystem::point_t& point)
{
  return toDegree(point.x, point.y);
}

CoordSystem::point_t CoordSystem::toDegree(float x, float y)
{
  return { x_.toDegree(x), y_.toDegree(y) };
}


// class CoordGrid
CoordGrid::CoordGrid(const CoordAxis& axis) :
  axis_(axis),
  offset_(0),
  degree_(0)
{
}

CoordGrid::~CoordGrid()
{
}

void CoordGrid::init(bool isAhead /*=*/)
{
  int tail = (int)::abs(axis_.min()) % OPTIMAL_STEP;
  if (0 < tail) {
    offset_ = axis_.scale() * tail;
    degree_ = axis_.min() + tail;
    return ;
  }

  offset_ = 0;
  degree_ = axis_.min();

  if (isAhead) {
    offset_ = axis_.scale() * OPTIMAL_STEP;
    degree_ = axis_.min() + OPTIMAL_STEP;
  }
}

bool CoordGrid::next(float* offset, int* degree /*=*/, bool excludeLast /*=*/)
{
  if ((axis_.max() < degree_) || (excludeLast && (axis_.max() == degree_)))
    return false;

  if (offset)
    *offset = offset_;

  if (degree)
    *degree = degree_;

  offset_ += OPTIMAL_STEP * axis_.scale();
  degree_ += OPTIMAL_STEP;

  return true;
}

}

}
