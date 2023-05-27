#ifndef MAPPI_UI_PLUGINS_UHDCONTROL_FILTERKALMAN_H
#define MAPPI_UI_PLUGINS_UHDCONTROL_FILTERKALMAN_H

namespace kalman {

class FilterKalman
{
public:
  FilterKalman(double q,double r,double f=1,double h=1);
  void setState(double state, double covariance);
  void correct (double data);
  double getState() { return state_;}

private:
  double x0_;
  double p0_;
  double f_;
  double q_;
  double h_;
  double r_;
  double state_;
  double covariance_;

};

} //kalman

#endif // MAPPI_UI_PLUGINS_UHDCONTROL_FILTERKALMAN_H
