#ifndef COMMONS_SATELLITE_NORADBASE_H
#define COMMONS_SATELLITE_NORADBASE_H

#include <commons/geobasis/coords.h>

using namespace Coords;
class SatelliteBase;

class NoradModelBase {
 public:
  NoradModelBase( SatelliteBase& sat );
  virtual ~NoradModelBase();

  virtual bool getPosition(double tsince, EciPoint &eci ) = 0;
  void lastParams( double* raan, double* u, double* t, double* i ) const ;

 private:
  void init();

 protected:
  bool finalPosition(double incl, double  omega, 
		     double    e, double      a,
		     double   xl, double  xnode, 
		     double   xn, double tsince, 
		     EciPoint &eci );
  
 protected:
  SatelliteBase& _sat;

   // Orbital parameter variables which need only be calculated one
   // time for a given orbit (ECI position time-independent).
   double m_satInc;  // inclination
   double m_satEcc;  // eccentricity

   double m_cosio;   double m_theta2;  double m_x3thm1;  double m_eosq;  
   double m_betao2;  double m_betao;   double m_aodp;    double m_xnodp;
   double m_s4;      double m_qoms24;  double m_perigee; double m_tsi;
   double m_eta;     double m_etasq;   double m_eeta;    double m_coef;
   double m_coef1;   double m_c1;      double m_c2;      double m_c3;
   double m_c4;      double m_sinio;   double m_a3ovk2;  double m_x1mth2;
   double m_xmdot;   double m_omgdot;  double m_xhdot1;  double m_xnodot;
   double m_xnodcf;  double m_t2cof;   double m_xlcof;   double m_aycof;
   double m_x7thm1;

   double m_last_raan;
   double m_last_u;
   double m_last_dt;
   double m_last_i;

};

#endif // COMMONS_SATELLITE_NORADBASE_H;
