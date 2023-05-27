#ifndef INTERPOLORDER_H
#define INTERPOLORDER_H

#include <math.h>

#include "linalgebra.h"
#include "tfield.h"

// This computes the Catmull-Rom spline from a list of points.
#include <cmath>
#include <vector>
#include <algorithm>
#include <iterator>
#include <boost/config.hpp>

namespace std_workaround {
template <class C>
inline BOOST_CONSTEXPR std::size_t size(const C& c)
{
  return c.size();
}
template <class T, std::size_t N>
inline BOOST_CONSTEXPR std::size_t size(const T(&)[N]) BOOST_NOEXCEPT
{
  return N;
}
}

namespace interpol{

namespace detail
{
template<class Point>
typename Point::value_type alpha_distance(Point const & p1, Point const & p2, typename Point::value_type alpha)
{
  using std::pow;
//  using std_workaround::size;
  typename Point::value_type dsq = 0;
  for (size_t i = 0; i < std_workaround::size(p1); ++i)
  {
    typename Point::value_type dx = p1[i] - p2[i];
    dsq += dx*dx;
  }
  return pow(dsq, alpha/2);
}
}

template <class Point, class RandomAccessContainer = std::vector<Point> >
class catmull_rom
{
  typedef typename Point::value_type value_type;
public:

  catmull_rom(RandomAccessContainer&& points, bool closed = false, value_type alpha = (value_type) 1/ (value_type) 2);

  catmull_rom(std::initializer_list<Point> l, bool closed = false, value_type alpha = (value_type) 1/ (value_type) 2) : catmull_rom<Point, RandomAccessContainer>(RandomAccessContainer(l), closed, alpha) {}

  value_type max_parameter() const
  {
    return m_max_s;
  }

  value_type parameter_at_point(size_t i) const
  {
    return m_s[i+1];
  }

  Point operator()(const value_type s) const;


  RandomAccessContainer&& get_points()
  {
    return std::move(m_pnts);
  }

private:
  RandomAccessContainer m_pnts;
  std::vector<value_type> m_s;
  value_type m_max_s;
};

template<class Point, class RandomAccessContainer >
catmull_rom<Point, RandomAccessContainer>::catmull_rom(RandomAccessContainer&& points, bool closed, typename Point::value_type alpha)
  : m_pnts(std::move(points))
{
  std::size_t num_pnts = m_pnts.size();
  if (num_pnts < 4)
  {
    throw std::domain_error("The Catmull-Rom curve requires at least 4 points.");
  }
  if (alpha < 0 || alpha > 1)
  {
    throw std::domain_error("The parametrization alpha must be in the range [0,1].");
  }

  using std::abs;
  m_s.resize(num_pnts+3);
  m_pnts.resize(num_pnts+3);
  m_pnts[num_pnts+1] = m_pnts[0];
  m_pnts[num_pnts+2] = m_pnts[1];

  auto tmp = m_pnts[num_pnts-1];
  for (std::ptrdiff_t i = num_pnts-1; i >= 0; --i)
  {
    m_pnts[i+1] = m_pnts[i];
  }
  m_pnts[0] = tmp;

  m_s[0] = -detail::alpha_distance<Point>(m_pnts[0], m_pnts[1], alpha);
  if (abs(m_s[0]) < std::numeric_limits<typename Point::value_type>::epsilon())
  {
    throw std::domain_error("The first and last point should not be the same.\n");
  }
  m_s[1] = 0;
  for (size_t i = 2; i < m_s.size(); ++i)
  {
    typename Point::value_type d = detail::alpha_distance<Point>(m_pnts[i], m_pnts[i-1], alpha);
    if (abs(d) < std::numeric_limits<typename Point::value_type>::epsilon())
    {
      throw std::domain_error("The control points of the Catmull-Rom curve are too close together; this will lead to ill-conditioning.\n");
    }
    m_s[i] = m_s[i-1] + d;
  }
  if(closed)
  {
    m_max_s = m_s[num_pnts+1];
  }
  else
  {
    m_max_s = m_s[num_pnts];
  }
}


template<class Point, class RandomAccessContainer >
Point catmull_rom<Point, RandomAccessContainer>::operator()(const typename Point::value_type s) const
{
//  using std_workaround::size;
  if (s < 0 || s > m_max_s)
  {
    throw std::domain_error("Parameter outside bounds.");
  }
  auto it = std::upper_bound(m_s.begin(), m_s.end(), s);
  //Now *it >= s. We want the index such that m_s[i] <= s < m_s[i+1]:
  size_t i = std::distance(m_s.begin(), it - 1);

  // Only denom21 is used twice:
  typename Point::value_type denom21 = 1/(m_s[i+1] - m_s[i]);
  typename Point::value_type s0s = m_s[i-1] - s;
  typename Point::value_type s1s = m_s[i] - s;
  typename Point::value_type s2s = m_s[i+1] - s;
  typename Point::value_type s3s = m_s[i+2] - s;

  Point A1_or_A3;
  typename Point::value_type denom = 1/(m_s[i] - m_s[i-1]);
  for(size_t j = 0; j < std_workaround::size(m_pnts[0]); ++j)
  {
    A1_or_A3[j] = denom*(s1s*m_pnts[i-1][j] - s0s*m_pnts[i][j]);
  }

  Point A2_or_B2;
  for(size_t j = 0; j < std_workaround::size(m_pnts[0]); ++j)
  {
    A2_or_B2[j] = denom21*(s2s*m_pnts[i][j] - s1s*m_pnts[i+1][j]);
  }

  Point B1_or_C;
  denom = 1/(m_s[i+1] - m_s[i-1]);
  for(size_t j = 0; j < std_workaround::size(m_pnts[0]); ++j)
  {
    B1_or_C[j] = denom*(s2s*A1_or_A3[j] - s0s*A2_or_B2[j]);
  }

  denom = 1/(m_s[i+2] - m_s[i+1]);
  for(size_t j = 0; j < std_workaround::size(m_pnts[0]); ++j)
  {
    A1_or_A3[j] = denom*(s3s*m_pnts[i+1][j] - s2s*m_pnts[i+2][j]);
  }

  Point B2;
  denom = 1/(m_s[i+2] - m_s[i]);
  for(size_t j = 0; j < std_workaround::size(m_pnts[0]); ++j)
  {
    B2[j] = denom*(s3s*A2_or_B2[j] - s1s*A1_or_A3[j]);
  }

  for(size_t j = 0; j < std_workaround::size(m_pnts[0]); ++j)
  {
    B1_or_C[j] = denom21*(s2s*B1_or_C[j] - s1s*B2[j]);
  }
  return B1_or_C;
}
}



int DecompBand(float *, int , int , int , int , float *, int *, int *);
int DecompBand(double *, int , int , int , int , double *, int *, int *);
int ReshBand(float *, int , int , int , int , float *, int *, float *);
int ReshBand(double *, int , int , int , int , double *, int *, double *);
int BandAny_Prod(float *, int, int, int, int, int,
                 float *, int, int, float *, int, int);
int BandSpecialAny_Prod(double *, double *, double *, int, int, int, int);
int BandSpecialAny_Prod(float *, float *, float *, int, int, int, int);
int BandSpecialDiag_Prod(double *, double *, double *, int, int, int);
int BandStrongAny_Prod(double *, int, int, int, int, double *, int, int, double *);
int BandStrongBandStrong_Prod(double *, int, int, int, int, double *, int, int, int,
                              double *, int, int, int);
int BandBand_Prod(float *, int, int, int, int, int,
                  float *, int, int, int, int, int,
                  float *, int, int, int, int);
int BandBand_Sum(float *, int, int, int, int, float *, int, int, int,
                 float *, int, int, int);
int BandStrong_Trans(double *, int, int, int, int);
int Band_Trans(float *, float *, int, int, int, int, int);
int initMatr_ParamSpline(float **, float **, int);
int initMatr_ParamSpline(double **, double **, int);
int factoriz(float **, float **, float **, int **, int *, int);
int factoriz(double **, double **, double **, int **, int *, int);
int factoriz_smooth(float **a, float **a1, float **h, float **diag_h_tr, float *diag,
                    int **indx, int *znak, int n, int m);
int oprFunSpline(float *, float *, float *, float *, const double *, int, int);
double oprFunSplineEasy(float *, float *, int, double);
double oprFunSplineEasy(double *, double *, int, double);
int InterpolLong(const float *, float *, float *, float *, int *, int, int);
int InterpolLat(const float *,  obanal::TField *, float *, float *, int, int, int);
int smootchLong( double *, double *, int, int, int, double);
int smootchLat(double *, double *, int, int, int, double);

int smootchLong(QVector <float> *fun, const QVector <float> &net_fi,
                int kol_la, int nach, int kon, double ur_smooth);
int smootchLat( QVector <float> *fun, const QVector <float> &net_fi,
                int kol_la, int nach, int kon, double ur_smooth);

int oprCoefparamSpline(float *, float *, float *, float *, int);
int oprCoefparamSpline(double *, double *, double *, double *, int);
int oprCoefparamSplineZam(float *, float *, float *, float *, int);
int raspredPoint(float *, int *, int, int);
int smootchPryam(float *, float *, int);
int smootchParab(float *, float *, int, int);
int smootchLine(const QPolygon &, QPolygon *, int);
int smootchLineDbl(float *, float *, int, int);

int smootchY(QVector <float> *fun, const QVector <float> &net_fi,
             int kol_la, int nach, int kon, double ur_smooth);
int smootchX( QVector <float> *fun, const QVector <float> &net_fi,
              int kol_la, int nach, int kon, double ur_smooth);

int InterpolX(const float *netfi, obanal::TField *rez,
              const QVector<float> &netfi_fact, const QVector<float> &rez_vs, int kol_fi, int kol_fi_fact, int kol_la );
int InterpolX2(const float *netfi, obanal::TField *rez,
               const QVector<float> &netfi_fact, const QVector<float> &rez_vs, int kol_fi, int kol_fi_fact, int kol_la );
int InterpolY(const float *netla, QVector<float> *rez,
              QVector<float> *la, QVector<float> *fun, const QVector<int> &kol_point,
              int kol_fi, int);

void uprost(float *old_koord,int old_kol, float **new_koord,  int *new_kol);
bool prorejPoint(const QPolygon &ar_old, QPolygon *ar_new, int kol_new);

#endif

