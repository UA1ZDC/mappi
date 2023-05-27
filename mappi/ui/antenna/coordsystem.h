#pragma once


namespace mappi {

namespace antenna {

class CoordAxis
{
public :
  CoordAxis();
  CoordAxis(float min, float max);
  ~CoordAxis();

  inline float min() const { return min_; }
  inline void setMin(float v) { min_ = v; }

  inline float max() const { return max_; }
  inline void setMax(float v) { max_ = v; }

  void setRange(float min, float max);

  inline float scale() const { return scale_; }
  void initScale(float width);

  float fromDegree(float degree);
  float toDegree(float point);

private :
  float min_;
  float max_;
  float scale_;
};


class CoordSystem
{
public :
  typedef struct {
    float x;
    float y;
  } point_t;

public :
  CoordSystem();
  CoordSystem(float minX, float maxX, float minY, float maxY);
  ~CoordSystem();

  inline CoordAxis& x() { return x_; }
  inline const CoordAxis& x() const { return x_; }

  inline CoordAxis& y() { return y_; }
  inline const CoordAxis& y() const { return y_; }

  void setScope(float width, float height);

  point_t fromDegree(const point_t& point);
  point_t fromDegree(float az, float en);

  point_t toDegree(const point_t& point);
  point_t toDegree(float x, float y);

private :
  CoordAxis x_;
  CoordAxis y_;
};


class CoordGrid
{
public :
    static const int OPTIMAL_STEP = 30;

public :
  CoordGrid(const CoordAxis& axis);
  ~CoordGrid();

  void init(bool isAhead = false);
  bool next(float* offset, int* degree = nullptr, bool excludeLast = false);

private :
  const CoordAxis& axis_;
  float offset_;
  int degree_;
};

}

}
