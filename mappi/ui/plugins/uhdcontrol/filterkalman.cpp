#include "filterkalman.h"

namespace kalman {

FilterKalman::FilterKalman(double q, double r, double f, double h)
{
  q_=q;
  r_=r;
  f_=f;
  h_=h;
}

void FilterKalman::setState(double state, double covariance)
{
  state_=state;
  covariance_=covariance;
}

void FilterKalman::correct(double data)
{
  x0_=f_*state_;
  p0_=f_*covariance_*f_+q_;

  double k=h_*p0_/(h_*p0_*h_+r_);
  state_=x0_+k*(data-h_*x0_);
  covariance_=(1-k*h_)*p0_;
}

}//kalman
