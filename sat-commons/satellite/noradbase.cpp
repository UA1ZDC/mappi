#include <math.h>

#include <commons/mathtools/mnmath.h>
#include <commons/coord_systems/jullian.h>

#include "noradbase.h"
#include "satellitebase.h"

using namespace MnMath;

const double QO           = AE + 120.0 / EQUATOR_RAD;
const double S            = AE + 78.0  / EQUATOR_RAD;
const double QOMS2T       = pow((QO - S), 4);            //(QO - S)^4 ER^4
const double GEOSYNC_ALT  = 42241.892;  // km

NoradModelBase::NoradModelBase( /*const */SatelliteBase& sat ): _sat( sat )
{
  init();
}

NoradModelBase::~NoradModelBase()
{
}

// Initialize()
// Perform the initialization of member variables, specifically the variables
// used by derived-class objects to calculate ECI coordinates.
void NoradModelBase::init()
{
   // Initialize any variables which are time-independent when
   // calculating the ECI coordinates of the satellite.
   m_satInc = _sat.inclination();
   m_satEcc = _sat.eccentricity();

   m_cosio  = cos(m_satInc);
   m_theta2 = m_cosio * m_cosio;
   m_x3thm1 = 3.0 * m_theta2 - 1.0;
   m_eosq   = m_satEcc * m_satEcc;
   m_betao2 = 1.0 - m_eosq;
   m_betao  = sqrt(m_betao2);

   // The "recovered" semi-minor axis and mean motion.
   m_aodp  = _sat.semiMinor();   
   m_xnodp = _sat.meanMotionRec();

   // For perigee below 156 km, the values of S and QOMS2T are altered.
   m_perigee = EQUATOR_RAD * (m_aodp * (1.0 - m_satEcc) - AE);

   m_s4      = S;
   m_qoms24  = QOMS2T;

   if (m_perigee < 156.0)
   {
      m_s4 = m_perigee - 78.0;

      if (m_perigee <= 98.0)
      {
         m_s4 = 20.0;
      }

      m_qoms24 = pow((120.0 - m_s4) * AE / EQUATOR_RAD, 4.0);
      m_s4 = m_s4 / EQUATOR_RAD + AE;
   }

   const double pinvsq = 1.0 / (m_aodp * m_aodp * m_betao2 * m_betao2);

   m_tsi   = 1.0 / (m_aodp - m_s4);
   m_eta   = m_aodp * m_satEcc * m_tsi;
   m_etasq = m_eta * m_eta;
   m_eeta  = m_satEcc * m_eta;

   const double psisq  = fabs(1.0 - m_etasq);

   m_coef  = m_qoms24 * pow(m_tsi,4.0);
   m_coef1 = m_coef / pow(psisq,3.5);

   const double c2 = m_coef1 * m_xnodp * 
                     (m_aodp * (1.0 + 1.5 * m_etasq + m_eeta * (4.0 + m_etasq)) +
                     0.75 * CK2 * m_tsi / psisq * m_x3thm1 * 
                     (8.0 + 3.0 * m_etasq * (8.0 + m_etasq)));

   m_c1    = _sat.bstar() * c2;
   m_sinio = sin(m_satInc);

   const double a3ovk2 = -XJ3 / CK2 * pow(AE,3.0);

   m_c3     = m_coef * m_tsi * a3ovk2 * m_xnodp * AE * m_sinio / m_satEcc;
   m_x1mth2 = 1.0 - m_theta2;
   m_c4     = 2.0 * m_xnodp * m_coef1 * m_aodp * m_betao2 * 
              (m_eta * (2.0 + 0.5 * m_etasq) +
              m_satEcc * (0.5 + 2.0 * m_etasq) - 
              2.0 * CK2 * m_tsi / (m_aodp * psisq) *
              (-3.0 * m_x3thm1 * (1.0 - 2.0 * m_eeta + m_etasq * (1.5 - 0.5 * m_eeta)) +
              0.75 * m_x1mth2 * 
              (2.0 * m_etasq - m_eeta * (1.0 + m_etasq)) * 
              cos(2.0 * _sat.argPerigee())));

   const double theta4 = m_theta2 * m_theta2;
   const double temp1  = 3.0 * CK2 * pinvsq * m_xnodp;
   const double temp2  = temp1 * CK2 * pinvsq;
   const double temp3  = 1.25 * CK4 * pinvsq * pinvsq * m_xnodp;

   m_xmdot = m_xnodp + 0.5 * temp1 * m_betao * m_x3thm1 +
             0.0625 * temp2 * m_betao * 
             (13.0 - 78.0 * m_theta2 + 137.0 * theta4);

   const double x1m5th = 1.0 - 5.0 * m_theta2;

   m_omgdot = -0.5 * temp1 * x1m5th + 0.0625 * temp2 * 
              (7.0 - 114.0 * m_theta2 + 395.0 * theta4) +
              temp3 * (3.0 - 36.0 * m_theta2 + 49.0 * theta4);

   const double xhdot1 = -temp1 * m_cosio;

   m_xnodot = xhdot1 + (0.5 * temp2 * (4.0 - 19.0 * m_theta2) +
              2.0 * temp3 * (3.0 - 7.0 * m_theta2)) * m_cosio;
   m_xnodcf = 3.5 * m_betao2 * xhdot1 * m_c1;
   m_t2cof  = 1.5 * m_c1;
   m_xlcof  = 0.125 * a3ovk2 * m_sinio * 
              (3.0 + 5.0 * m_cosio) / (1.0 + m_cosio);
   m_aycof  = 0.25 * a3ovk2 * m_sinio;
   m_x7thm1 = 7.0 * m_theta2 - 1.0;
}


bool NoradModelBase::finalPosition(double incl, double  omega, 
			      double    e, double      a,
			      double   xl, double  xnode, 
			      double   xn, double tsince, 
			      EciPoint &eci )
{
  if ((e * e) > 1.0) {
    // error in satellite data
    return false;  
  }
  
  double beta = sqrt(1.0 - e * e);
  
  // Long period periodics 
  double axn  = e * cos(omega);
  double temp = 1.0 / (a * beta * beta);
  double xll  = temp * m_xlcof * axn;
  double aynl = temp * m_aycof;
  double xlt  = xl + xll;
  double ayn  = e * sin(omega) + aynl;
  
  // Solve Kepler's Equation 

  double capu   = fmod2p(xlt - xnode);
  double temp2  = capu;
  double temp3  = 0.0;
  double temp4  = 0.0;
  double temp5  = 0.0;
  double temp6  = 0.0;
  double sinepw = 0.0;
  double cosepw = 0.0;
  bool   fDone  = false;

  for (int i = 1; (i <= 10) && !fDone; i++) {
    sinepw = sin(temp2);
    cosepw = cos(temp2);
    temp3 = axn * sinepw;
    temp4 = ayn * cosepw;
    temp5 = axn * cosepw;
    temp6 = ayn * sinepw;
    
    double epw = (capu - temp4 + temp3 - temp2) / 
      (1.0 - temp5 - temp6) + temp2;
    if (fabs(epw - temp2) <= 1.0e-06) {
      fDone = true;
    } else {
      temp2 = epw;
    }
  }

//  m_last_raan = temp2;
  
  // Short period preliminary quantities 
  double ecose = temp5 + temp6;
  double esine = temp3 - temp4;
  double elsq  = axn * axn + ayn * ayn;
  temp  = 1.0 - elsq;
  double pl = a * temp;
  double r  = a * (1.0 - ecose);
  double temp1 = 1.0 / r;
  double rdot  = XKE * sqrt(a) * esine * temp1;
  double rfdot = XKE * sqrt(pl) * temp1;
  temp2 = a * temp1;
  double betal = sqrt(temp);
  temp3 = 1.0 / (1.0 + betal);
  double cosu  = temp2 * (cosepw - axn + ayn * esine * temp3);
  double sinu  = temp2 * (sinepw - ayn - axn * esine * temp3);
  double u     = atan2(sinu, cosu);
  m_last_u = u;
  double sin2u = 2.0 * sinu * cosu;
  double cos2u = 2.0 * cosu * cosu - 1.0;
 
  temp  = 1.0 / pl;
  temp1 = CK2 * temp;
  temp2 = temp1 * temp;
  
  // Update for short periodics 
  double rk = r * (1.0 - 1.5 * temp2 * betal * m_x3thm1) + 
    0.5 * temp1 * m_x1mth2 * cos2u;
  double uk = u - 0.25 * temp2 * m_x7thm1 * sin2u;
  double xnodek = xnode + 1.5 * temp2 * m_cosio * sin2u;
  double xinck  = incl + 1.5 * temp2 * m_cosio * m_sinio * cos2u;
  double rdotk  = rdot - xn * temp1 * m_x1mth2 * sin2u;
  double rfdotk = rfdot + xn * temp1 * (m_x1mth2 * cos2u + 1.5 * m_x3thm1);

  m_last_i = xinck;
  
  // Orientation vectors 
  double sinuk  = sin(uk);
  double cosuk  = cos(uk);
  double sinik  = sin(xinck);
  double cosik  = cos(xinck);
  double sinnok = sin(xnodek);
  double cosnok = cos(xnodek);
  double xmx = -sinnok * cosik;
  double xmy = cosnok * cosik;
  double ux  = xmx * sinuk + cosnok * cosuk;
  double uy  = xmy * sinuk + sinnok * cosuk;
  double uz  = sinik * sinuk;
  double vx  = xmx * cosuk - cosnok * sinuk;
  double vy  = xmy * cosuk - sinnok * sinuk;
  double vz  = sinik * cosuk;
  
  // Position
  double x = rk * ux;
  double y = rk * uy;
  double z = rk * uz;
  
  EciCoord vecPos(x, y, z);
  
   // Validate on altitude
  double altKm = (vecPos.r * (EQUATOR_RAD / AE));

   if ((altKm < EQUATOR_RAD) || (altKm > (2 * GEOSYNC_ALT)))
      return false;
   
   // Velocity
   double xdot = rdotk * ux + rfdotk * vx;
   double ydot = rdotk * uy + rfdotk * vy;
   double zdot = rdotk * uz + rfdotk * vz;

   EciCoord vecVel(xdot, ydot, zdot);

   JullianDate gmt = _sat.epoch();
   gmt.addMin(tsince);

   vecPos.mul( EQUATOR_RAD / AE );
   vecVel.mul( (EQUATOR_RAD / AE) * (MIN_PER_DAY / 86400) );
   eci = EciPoint(vecPos, vecVel, gmt);

   m_last_dt = m_last_u*vecPos.r/::sqrt(vecVel.x*vecVel.x + vecVel.y*vecVel.y + vecVel.z*vecVel.z);

   return true;
}

void NoradModelBase::lastParams( double* raan, double* u, double* dt, double* i ) const
{
  *raan = m_last_raan;
  *u = m_last_u;
  *dt = m_last_dt;
  *i = m_last_i;
}
