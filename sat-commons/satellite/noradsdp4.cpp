#include <math.h>

#include <commons/mathtools/mnmath.h>
#include <commons/coord_systems/jullian.h>

#include "noradsdp4.h"
#include "satellite.h"

using namespace MnMath;



NoradSDP4::NoradSDP4( SatelliteBase& sat ):
  NoradModelBase( sat )
{
  m_sing = sin( _sat.argPerigee());
  m_cosg = cos( _sat.argPerigee());
   
  dp_savtsn = 0.0;
  dp_zmos = 0.0;
  dp_se2 = 0.0;
  dp_se3 = 0.0;
  dp_si2 = 0.0;
  dp_si3 = 0.0;
  dp_sl2 = 0.0;
  dp_sl3 = 0.0;
  dp_sl4 = 0.0;
  dp_sghs = 0.0;
  dp_sgh2 = 0.0;
  dp_sgh3 = 0.0;
  dp_sgh4 = 0.0;
  dp_sh2 = 0.0;
  dp_sh3 = 0.0;
  dp_zmol = 0.0;
  dp_ee2 = 0.0;
  dp_e3 = 0.0;
  dp_xi2 = 0.0;
  dp_xi3 = 0.0;
  dp_xl2 = 0.0;
  dp_xl3 = 0.0;
  dp_xl4 = 0.0;
  dp_xgh2 = 0.0;
  dp_xgh3 = 0.0;
  dp_xgh4 = 0.0;
  dp_xh2 = 0.0;
  dp_xh3 = 0.0;
  dp_xqncl = 0.0;

  dp_thgr = 0.0;
  dp_omegaq = 0.0;
  dp_sse = 0.0;
  dp_ssi = 0.0;
  dp_ssl = 0.0;
  dp_ssh = 0.0;
  dp_ssg = 0.0;
  dp_d2201 = 0.0;
  dp_d2211 = 0.0;
  dp_d3210 = 0.0;
  dp_d3222 = 0.0;
  dp_d4410 = 0.0;
  dp_d4422 = 0.0;
  dp_d5220 = 0.0;
  dp_d5232 = 0.0;
  dp_d5421 = 0.0;
  dp_d5433 = 0.0;
  dp_xlamo = 0.0;
  dp_del1 = 0.0;
  dp_del2 = 0.0;
  dp_del3 = 0.0;
  dp_fasx2 = 0.0;
  dp_fasx4 = 0.0;
  dp_fasx6 = 0.0;
  dp_xfact = 0.0;
  dp_xli = 0.0;
  dp_xni = 0.0;
  dp_atime = 0.0;
  dp_stepp = 0.0;
  dp_stepn = 0.0;
  dp_step2 = 0.0;

  dp_iresfl = false;
  dp_isynfl = false;
}

NoradSDP4::~NoradSDP4()
{
}


bool NoradSDP4::DeepInit(double *eosq,  double *sinio,  double *cosio,
			 double *betao, double *aodp,   double *theta2,
			 double *sing,  double *cosg,   double *betao2,
			 double *xmdot, double *omgdot, double *xnodott)
{
  eqsq   = *eosq;
  siniq  = *sinio;   
  cosiq  = *cosio;   
  rteqsq = *betao;
  ao     = *aodp;    
  cosq2  = *theta2;  
  sinomo = *sing;    
  cosomo = *cosg;
  bsq    = *betao2;  
  xlldot = *xmdot;   
  omgdt  = *omgdot;  
  xnodot = *xnodott;

  // Deep space initialization 
  JullianDate jd = _sat.epoch();

  dp_thgr = jd.toGMST();

  double eq   = _sat.eccentricity();
  double aqnv = 1.0 / ao;

  dp_xqncl = _sat.inclination();

  double xmao   = _sat.meanAnomaly();
  double xpidot = omgdt + xnodot;
  double sinq   = sin(_sat.raan());
  double cosq   = cos(_sat.raan());

  dp_omegaq = _sat.argPerigee();

  // Initialize lunar solar terms 
  double day = jd.fromJan1_12h_1900();

  if (day != dpi_day)
    {
      dpi_day    = day;
      dpi_xnodce = 4.5236020 - 9.2422029E-4 * day;
      dpi_stem   = sin(dpi_xnodce);
      dpi_ctem   = cos(dpi_xnodce);
      dpi_zcosil = 0.91375164 - 0.03568096 * dpi_ctem;
      dpi_zsinil = sqrt(1.0 - dpi_zcosil * dpi_zcosil);
      dpi_zsinhl = 0.089683511 *dpi_stem / dpi_zsinil;
      dpi_zcoshl = sqrt(1.0 - dpi_zsinhl * dpi_zsinhl);
      dpi_c      = 4.7199672 + 0.22997150 * day;
      dpi_gam    = 5.8351514 + 0.0019443680 * day;
      dp_zmol    = fmod2p(dpi_c - dpi_gam);
      dpi_zx     = 0.39785416 * dpi_stem / dpi_zsinil;
      dpi_zy     = dpi_zcoshl * dpi_ctem + 0.91744867 * dpi_zsinhl * dpi_stem;
      dpi_zx     = atan2(dpi_zx,dpi_zy) + dpi_gam - dpi_xnodce;
      dpi_zcosgl = cos(dpi_zx);
      dpi_zsingl = sin(dpi_zx);
      dp_zmos    = 6.2565837 + 0.017201977 * day;
      dp_zmos    = fmod2p(dp_zmos);
    }

  dp_savtsn = 1.0e20;

  double zcosg = zcosgs;
  double zsing = zsings;
  double zcosi = zcosis;
  double zsini = zsinis;
  double zcosh = cosq;
  double zsinh = sinq;
  double cc  = c1ss;
  double zn  = zns;
  double ze  = zes;
//  double zmo = dp_zmos;
  double xnoi = 1.0 / m_xnodp;

  double a1;  double a3;  double a7;  double a8;  double a9;  double a10;
  double a2;  double a4;  double a5;  double a6;  double x1;  double x2;
  double x3;  double x4;  double x5;  double x6;  double x7;  double x8;
  double z31; double z32; double z33; double z1;  double z2;  double z3;
  double z11; double z12; double z13; double z21; double z22; double z23;
  double s3;  double s2;  double s4;  double s1;  double s5;  double s6;
  double s7;  
  double se  = 0.0;  double si = 0.0;  double sl = 0.0;  
  double sgh = 0.0;  double sh = 0.0;

  // Apply the solar and lunar terms on the first pass, then re-apply the
  // solar terms again on the second pass.

  for (int pass = 1; pass <= 2; pass++)
    {
      // Do solar terms 
      a1 =  zcosg * zcosh + zsing * zcosi * zsinh;
      a3 = -zsing * zcosh + zcosg * zcosi * zsinh;
      a7 = -zcosg * zsinh + zsing * zcosi * zcosh;
      a8 = zsing * zsini;
      a9 = zsing * zsinh + zcosg * zcosi * zcosh;
      a10 = zcosg * zsini;
      a2 = cosiq * a7 +  siniq * a8;
      a4 = cosiq * a9 +  siniq * a10;
      a5 = -siniq * a7 +  cosiq * a8;
      a6 = -siniq * a9 +  cosiq * a10;
      x1 = a1 * cosomo + a2 * sinomo;
      x2 = a3 * cosomo + a4 * sinomo;
      x3 = -a1 * sinomo + a2 * cosomo;
      x4 = -a3 * sinomo + a4 * cosomo;
      x5 = a5 * sinomo;
      x6 = a6 * sinomo;
      x7 = a5 * cosomo;
      x8 = a6 * cosomo;
      z31 = 12.0 * x1 * x1 - 3.0 * x3 * x3;
      z32 = 24.0 * x1 * x2 - 6.0 * x3 * x4;
      z33 = 12.0 * x2 * x2 - 3.0 * x4 * x4;
      z1 = 3.0 * (a1 * a1 + a2 * a2) + z31 * eqsq;
      z2 = 6.0 * (a1 * a3 + a2 * a4) + z32 * eqsq;
      z3 = 3.0 * (a3 * a3 + a4 * a4) + z33 * eqsq;
      z11 = -6.0 * a1 * a5 + eqsq*(-24.0 * x1 * x7 - 6.0 * x3 * x5);
      z12 = -6.0 * (a1 * a6 + a3 * a5) +
	eqsq * (-24.0 * (x2 * x7 + x1 * x8) - 6.0 * (x3 * x6 + x4 * x5));
      z13 = -6.0 * a3 * a6 + eqsq * (-24.0 * x2 * x8 - 6.0 * x4 * x6);
      z21 = 6.0 * a2 * a5 + eqsq * (24.0 * x1 * x5 - 6.0 * x3 * x7);
      z22 = 6.0*(a4 * a5 + a2 * a6) +
	eqsq * (24.0 * (x2 * x5 + x1 * x6) - 6.0 * (x4 * x7 + x3 * x8));
      z23 = 6.0 * a4 * a6 + eqsq*(24.0 * x2 * x6 - 6.0 * x4 * x8);
      z1 = z1 + z1 + bsq * z31;
      z2 = z2 + z2 + bsq * z32;
      z3 = z3 + z3 + bsq * z33;
      s3  = cc * xnoi;
      s2  = -0.5 * s3/rteqsq;
      s4  = s3 * rteqsq;
      s1  = -15.0 * eq * s4;
      s5  = x1 * x3 + x2 * x4;
      s6  = x2 * x3 + x1 * x4;
      s7  = x2 * x4 - x1 * x3;
      se  = s1 * zn * s5;
      si  = s2 * zn * (z11 + z13);
      sl  = -zn * s3 * (z1 + z3 - 14.0 - 6.0 * eqsq);
      sgh = s4 * zn * (z31 + z33 - 6.0);
      sh  = -zn * s2 * (z21 + z23);

      if (dp_xqncl < 5.2359877E-2)
	sh = 0.0;

      dp_ee2 = 2.0 * s1 * s6;
      dp_e3  = 2.0 * s1 * s7;
      dp_xi2 = 2.0 * s2 * z12;
      dp_xi3 = 2.0 * s2 * (z13 - z11);
      dp_xl2 = -2.0 * s3 * z2;
      dp_xl3 = -2.0 * s3 * (z3 - z1);
      dp_xl4 = -2.0 * s3 * (-21.0 - 9.0 * eqsq) * ze;
      dp_xgh2 = 2.0 * s4 * z32;
      dp_xgh3 = 2.0 * s4 * (z33 - z31);
      dp_xgh4 = -18.0 * s4 * ze;
      dp_xh2 = -2.0 * s2 * z22;
      dp_xh3 = -2.0 * s2 * (z23 - z21);

      if (pass == 1)
	{
	  // Do lunar terms 
	  dp_sse = se;
	  dp_ssi = si;
	  dp_ssl = sl;
	  dp_ssh = sh / siniq;
	  dp_ssg = sgh - cosiq * dp_ssh;
	  dp_se2 = dp_ee2;
	  dp_si2 = dp_xi2;
	  dp_sl2 = dp_xl2;
	  dp_sgh2 = dp_xgh2;
	  dp_sh2 = dp_xh2;
	  dp_se3 = dp_e3;
	  dp_si3 = dp_xi3;
	  dp_sl3 = dp_xl3;
	  dp_sgh3 = dp_xgh3;
	  dp_sh3 = dp_xh3;
	  dp_sl4 = dp_xl4;
	  dp_sgh4 = dp_xgh4;
	  zcosg = dpi_zcosgl;
	  zsing = dpi_zsingl;
	  zcosi = dpi_zcosil;
	  zsini = dpi_zsinil;
	  zcosh = dpi_zcoshl * cosq + dpi_zsinhl * sinq;
	  zsinh = sinq * dpi_zcoshl - cosq * dpi_zsinhl;
	  zn = znl;
	  cc = c1l;
	  ze = zel;
//	  zmo = dp_zmol;
	}
    }

  dp_sse = dp_sse + se;
  dp_ssi = dp_ssi + si;
  dp_ssl = dp_ssl + sl;
  dp_ssg = dp_ssg + sgh - cosiq / siniq * sh;
  dp_ssh = dp_ssh + sh / siniq;

  // Geopotential resonance initialization for 12 hour orbits 
  dp_iresfl = false;
  dp_isynfl = false;

  bool   bInitOnExit = true;
  double g310;
  double f220;
  double bfact = 0.0;

  if ((m_xnodp >= 0.0052359877) || (m_xnodp <= 0.0034906585))
    {
      if ((m_xnodp < 8.26E-3) || (m_xnodp > 9.24E-3) || (eq < 0.5))
	{
	  bInitOnExit = false;
	}
      else
	{
	  dp_iresfl = true;

	  double eoc  = eq * eqsq;
	  double g201 = -0.306 - (eq - 0.64) * 0.440;

	  double g211;   double g322;
	  double g410;   double g422;   
	  double g520;

	  if (eq <= 0.65)
	    {
	      g211 = 3.616 - 13.247 * eq + 16.290 * eqsq;
	      g310 = -19.302 + 117.390 * eq - 228.419 * eqsq + 156.591 * eoc;
	      g322 = -18.9068 + 109.7927 * eq - 214.6334 * eqsq + 146.5816 * eoc;
	      g410 = -41.122 + 242.694 * eq - 471.094 * eqsq + 313.953 * eoc;
	      g422 = -146.407 + 841.880 * eq - 1629.014 * eqsq + 1083.435 * eoc;
	      g520 = -532.114 + 3017.977 * eq - 5740.0 * eqsq + 3708.276 * eoc;
	    }
	  else
	    {
	      g211 = -72.099 + 331.819 * eq - 508.738 * eqsq + 266.724 * eoc;
	      g310 = -346.844 + 1582.851 * eq - 2415.925 * eqsq + 1246.113 * eoc;
	      g322 = -342.585 + 1554.908 * eq - 2366.899 * eqsq + 1215.972 * eoc;
	      g410 = -1052.797 + 4758.686 * eq - 7193.992 * eqsq + 3651.957 * eoc;
	      g422 = -3581.69 + 16178.11 * eq - 24462.77 * eqsq + 12422.52 * eoc;

	      if (eq <= 0.715)
		g520 = 1464.74 - 4664.75 * eq + 3763.64 * eqsq;
	      else
		g520 = -5149.66 + 29936.92 * eq - 54087.36 * eqsq + 31324.56 * eoc;
	    }

	  double g533;   
	  double g521;   
	  double g532;

	  if (eq < 0.7)
	    {
	      g533 = -919.2277 + 4988.61 * eq - 9064.77 * eqsq + 5542.21 * eoc;
	      g521 = -822.71072 + 4568.6173 * eq - 8491.4146 * eqsq + 5337.524 * eoc;
	      g532 = -853.666 + 4690.25 * eq - 8624.77 * eqsq + 5341.4 * eoc;
	    }
	  else
	    {
	      g533 = -37995.78 + 161616.52 * eq - 229838.2 * eqsq + 109377.94 * eoc;
	      g521 = -51752.104 + 218913.95 * eq - 309468.16 * eqsq + 146349.42 * eoc;
	      g532 = -40023.88 + 170470.89 * eq - 242699.48 * eqsq + 115605.82 * eoc;
	    }

	  double sini2 = siniq * siniq;
	  f220 = 0.75*(1.0 + 2.0 * cosiq + cosq2);
	  double f221 = 1.5 * sini2;
	  double f321 = 1.875 * siniq*(1.0 - 2.0 * cosiq - 3.0 * cosq2);
	  double f322 = -1.875 * siniq*(1.0 + 2.0 * cosiq - 3.0 * cosq2);
	  double f441 = 35.0 * sini2 * f220;
	  double f442 = 39.3750 * sini2 * sini2;
	  double f522 = 9.84375 * siniq*(sini2*(1.0 - 2.0 * cosiq - 5.0 * cosq2) +
					 0.33333333*(-2.0 + 4.0 * cosiq + 6.0 * cosq2));
	  double f523 = siniq*(4.92187512 * sini2*(-2.0 - 4.0 * cosiq + 10.0 * cosq2) +
			       6.56250012 * (1.0 + 2.0 * cosiq - 3.0 * cosq2));
	  double f542 = 29.53125 * siniq*(2.0 - 8.0 * cosiq + cosq2 * (-12.0 + 8.0 * cosiq + 10.0 * cosq2));
	  double f543 = 29.53125 * siniq*(-2.0 - 8.0 * cosiq + cosq2 * (12.0 + 8.0 * cosiq - 10.0 * cosq2));
	  double xno2 = m_xnodp * m_xnodp;
	  double ainv2 = aqnv * aqnv;
	  double temp1 = 3.0 * xno2 * ainv2;
	  double temp = temp1 * root22;

	  dp_d2201 = temp * f220 * g201;
	  dp_d2211 = temp * f221 * g211;
	  temp1 = temp1 * aqnv;
	  temp = temp1 * root32;
	  dp_d3210 = temp * f321 * g310;
	  dp_d3222 = temp * f322 * g322;
	  temp1 = temp1 * aqnv;
	  temp = 2.0 * temp1 * root44;
	  dp_d4410 = temp * f441 * g410;
	  dp_d4422 = temp * f442 * g422;
	  temp1 = temp1 * aqnv;
	  temp  = temp1 * root52;
	  dp_d5220 = temp * f522 * g520;
	  dp_d5232 = temp * f523 * g532;
	  temp = 2.0 * temp1 * root54;
	  dp_d5421 = temp * f542 * g521;
	  dp_d5433 = temp * f543 * g533;
	  dp_xlamo = xmao + _sat.raan() + _sat.raan() - dp_thgr - dp_thgr;
	  bfact = xlldot + xnodot + xnodot - thdt - thdt;
	  bfact = bfact + dp_ssl + dp_ssh + dp_ssh;
	}
    }
  else
    {
      // Synchronous resonance terms initialization 
      dp_iresfl = true;
      dp_isynfl = true;
      double g200 = 1.0 + eqsq * (-2.5 + 0.8125 * eqsq);
      g310 = 1.0 + 2.0 * eqsq;
      double g300 = 1.0 + eqsq * (-6.0 + 6.60937 * eqsq);
      f220 = 0.75 * (1.0 + cosiq) * (1.0 + cosiq);
      double f311 = 0.9375 * siniq * siniq * (1.0 + 3 * cosiq) - 0.75 * (1.0 + cosiq);
      double f330 = 1.0 + cosiq;
      f330 = 1.875 * f330 * f330 * f330;
      dp_del1 = 3.0 * m_xnodp * m_xnodp * aqnv * aqnv;
      dp_del2 = 2.0 * dp_del1 * f220 * g200 * q22;
      dp_del3 = 3.0 * dp_del1 * f330 * g300 * q33 * aqnv;
      dp_del1 = dp_del1 * f311 * g310 * q31 * aqnv;
      dp_fasx2 = 0.13130908;
      dp_fasx4 = 2.8843198;
      dp_fasx6 = 0.37448087;
      dp_xlamo = xmao + +_sat.raan() + _sat.argPerigee() - dp_thgr;
      bfact = xlldot + xpidot - thdt;
      bfact = bfact + dp_ssl + dp_ssg + dp_ssh;
    }

  if (bInitOnExit)
    {
      dp_xfact = bfact - m_xnodp;

      // Initialize integrator 
      dp_xli = dp_xlamo;
      dp_xni = m_xnodp;
      dp_atime = 0.0;
      dp_stepp = 720.0;
      dp_stepn = -720.0;
      dp_step2 = 259200.0;
    }

  *eosq   = eqsq;    
  *sinio  = siniq;   
  *cosio  = cosiq;   
  *betao  = rteqsq;
  *aodp   = ao;      
  *theta2 = cosq2;   
  *sing   = sinomo;  
  *cosg   = cosomo;
  *betao2 = bsq;     
  *xmdot  = xlldot;  
  *omgdot = omgdt;   
  *xnodott = xnodot;

  return true;
}


bool NoradSDP4::DeepCalcDotTerms(double *pxndot, double *pxnddt, double *pxldot)
{
  // Dot terms calculated 
  if (dp_isynfl)
    {
      *pxndot = dp_del1 * sin(dp_xli - dp_fasx2) + 
	dp_del2 * sin(2.0 * (dp_xli - dp_fasx4)) +
	dp_del3 * sin(3.0 * (dp_xli - dp_fasx6));
      *pxnddt = dp_del1 * cos(dp_xli - dp_fasx2) +
	2.0 * dp_del2 * cos(2.0 * (dp_xli - dp_fasx4)) +
	3.0 * dp_del3 * cos(3.0 * (dp_xli - dp_fasx6));
    }
  else
    {
      double xomi  = dp_omegaq + omgdt * dp_atime;
      double x2omi = xomi + xomi;
      double x2li  = dp_xli + dp_xli;

      *pxndot = dp_d2201 * sin(x2omi + dp_xli - g22) + 
	dp_d2211 * sin(dp_xli - g22)         +
	dp_d3210 * sin(xomi + dp_xli - g32)  +
	dp_d3222 * sin(-xomi + dp_xli - g32) +
	dp_d4410 * sin(x2omi + x2li - g44)   +
	dp_d4422 * sin(x2li - g44)           +
	dp_d5220 * sin(xomi + dp_xli - g52)  +
	dp_d5232 * sin(-xomi + dp_xli - g52) +
	dp_d5421 * sin(xomi + x2li - g54)    +
	dp_d5433 * sin(-xomi + x2li - g54);

      *pxnddt = dp_d2201 * cos(x2omi + dp_xli - g22) +
	dp_d2211 * cos(dp_xli - g22)         +
	dp_d3210 * cos(xomi + dp_xli - g32)  +
	dp_d3222 * cos(-xomi + dp_xli - g32) +
	dp_d5220 * cos(xomi + dp_xli - g52)  +
	dp_d5232 * cos(-xomi + dp_xli - g52) +
	2.0 * (dp_d4410 * cos(x2omi + x2li - g44) +
	       dp_d4422 * cos(x2li - g44)         +
	       dp_d5421 * cos(xomi + x2li - g54)  +
	       dp_d5433 * cos(-xomi + x2li - g54));
    }

  *pxldot = dp_xni + dp_xfact;
  *pxnddt = (*pxnddt) * (*pxldot);

  return true;
}


void NoradSDP4::DeepCalcIntegrator(double *pxndot, double *pxnddt, 
				   double *pxldot, const double &delt)
{
  DeepCalcDotTerms(pxndot, pxnddt, pxldot);

  dp_xli = dp_xli + (*pxldot) * delt + (*pxndot) * dp_step2;
  dp_xni = dp_xni + (*pxndot) * delt + (*pxnddt) * dp_step2;
  dp_atime = dp_atime + delt;
}


bool NoradSDP4::DeepSecular(double *xmdf, double *omgadf, double *xnode,
			    double *emm,  double *xincc,  double *xnn,
			    double *tsince)
{
  xll    = *xmdf;    
  omgasm = *omgadf;  
  xnodes = *xnode;  
  xn     = *xnn;  
  t      = *tsince;

  // Deep space secular effects 
  xll    = xll + dp_ssl * t;
  omgasm = omgasm + dp_ssg * t;
  xnodes = xnodes + dp_ssh * t;
  _em    = _sat.eccentricity() + dp_sse * t;
  xinc   = _sat.inclination()  + dp_ssi * t;

  if (xinc < 0.0)
    {
      xinc   = -xinc;
      xnodes = xnodes + M_PI;
      omgasm = omgasm - M_PI;
    }

  double xnddt = 0.0;
  double xndot = 0.0;
  double xldot = 0.0;
  double ft    = 0.0;
  double delt  = 0.0;

  bool fDone = false;

  if (dp_iresfl) 
    {
      while (!fDone)
	{
	  if ((dp_atime == 0.0)                ||
	      ((t >= 0.0) && (dp_atime <  0.0)) ||
	      ((t <  0.0) && (dp_atime >= 0.0)))
	    {
	      if (t < 0)
		delt = dp_stepn;
	      else
		delt = dp_stepp;

	      // Epoch restart 
	      dp_atime = 0.0;
	      dp_xni = m_xnodp;
	      dp_xli = dp_xlamo;

	      fDone = true;
	    }
	  else
	    {
	      if (fabs(t) < fabs(dp_atime))
		{
		  delt = dp_stepp;

		  if (t >= 0.0)
		    delt = dp_stepn;

		  DeepCalcIntegrator(&xndot, &xnddt, &xldot, delt);
		}
	      else
		{
		  delt = dp_stepn;

		  if (t > 0.0)
		    delt = dp_stepp;

		  fDone = true;
		}
	    }
	}

      while (fabs(t - dp_atime) >= dp_stepp)
	{
	  DeepCalcIntegrator(&xndot, &xnddt, &xldot, delt);
	}

      ft = t - dp_atime;

      DeepCalcDotTerms(&xndot, &xnddt, &xldot);

      xn = dp_xni + xndot * ft + xnddt * ft * ft * 0.5;

      double xl   = dp_xli + xldot * ft + xndot * ft * ft * 0.5;
      double temp = -xnodes + dp_thgr + t * thdt;

      xll = xl - omgasm + temp;

      if (!dp_isynfl)
	xll = xl + temp + temp;
    }

  *xmdf   = xll; 
  *omgadf = omgasm;
  *xnode  = xnodes;
  *emm    = _em;
  *xincc  = xinc;
  *xnn    = xn;
  *tsince = t;

  return true;
}


bool NoradSDP4::DeepPeriodics(double *e,      double *xincc,
			      double *omgadf, double *xnode,
			      double *xmam)
{
  _em    = *e;       
  xinc   = *xincc; 
  omgasm = *omgadf;
  xnodes = *xnode;
  xll    = *xmam;

  // Lunar-solar periodics 
  double sinis = sin(xinc);
  double cosis = cos(xinc);

  double sghs = 0.0;
  double shs  = 0.0;
  double sh1  = 0.0;
  double pe   = 0.0;
  double pinc = 0.0;
  double pl   = 0.0;
  double sghl = 0.0;

  if (fabs(dp_savtsn - t) >= 30.0)
    {
      dp_savtsn = t;

      double zm = dp_zmos + zns * t;
      double zf = zm + 2.0 * zes * sin(zm);
      double sinzf = sin(zf);
      double f2  = 0.5 * sinzf * sinzf - 0.25;
      double f3  = -0.5 * sinzf * cos(zf);
      double ses = dp_se2 * f2 + dp_se3 * f3;
      double sis = dp_si2 * f2 + dp_si3 * f3;
      double sls = dp_sl2 * f2 + dp_sl3 * f3 + dp_sl4 * sinzf;

      sghs = dp_sgh2 * f2 + dp_sgh3 * f3 + dp_sgh4 * sinzf;
      shs = dp_sh2 * f2 + dp_sh3 * f3;
      zm = dp_zmol + znl * t;
      zf = zm + 2.0 * zel * sin(zm);
      sinzf = sin(zf);
      f2 = 0.5 * sinzf * sinzf - 0.25;
      f3 = -0.5 * sinzf * cos(zf);

      double sel  = dp_ee2 * f2 + dp_e3 * f3;
      double sil  = dp_xi2 * f2 + dp_xi3 * f3;
      double sll  = dp_xl2 * f2 + dp_xl3 * f3 + dp_xl4 * sinzf;

      sghl = dp_xgh2 * f2 + dp_xgh3 * f3 + dp_xgh4 * sinzf;
      sh1  = dp_xh2 * f2 + dp_xh3 * f3;
      pe   = ses + sel;
      pinc = sis + sil;
      pl   = sls + sll;
    }

  double pgh  = sghs + sghl;
  double ph   = shs + sh1;
  xinc = xinc + pinc;
  _em  = _em + pe;

  if (dp_xqncl >= 0.2)
    {
      // Apply periodics directly 
      ph  = ph / siniq;
      pgh = pgh - cosiq * ph;
      omgasm = omgasm + pgh;
      xnodes = xnodes + ph;
      xll = xll + pl;
    }
  else
    {
      // Apply periodics with Lyddane modification 
      double sinok = sin(xnodes);
      double cosok = cos(xnodes);
      double alfdp = sinis * sinok;
      double betdp = sinis * cosok;
      double dalf  =  ph * cosok + pinc * cosis * sinok;
      double dbet  = -ph * sinok + pinc * cosis * cosok;

      alfdp = alfdp + dalf;
      betdp = betdp + dbet;

      double xls = xll + omgasm + cosis * xnodes;
      double dls = pl + pgh - pinc * xnodes * sinis;

      xls    = xls + dls;
      xnodes = atan2(alfdp, betdp);
      xll    = xll + pl;
      omgasm = xls - xll - cos(xinc) * xnodes;
    }

  *e      = _em;      
  *xincc  = xinc;    
  *omgadf = omgasm;  
  *xnode  = xnodes;
  *xmam   = xll;

  return true;
}

// This procedure returns the ECI position and velocity for the satellite
// in the orbit at the given number of minutes since the TLE epoch time
// using the NORAD Simplified General Perturbation 4, "deep space" orbit
// model.
//
// tsince  - Time in minutes since the TLE epoch (GMT).
// pECI    - pointer to location to store the ECI data.
//           To convert the returned ECI position vector to km,
//           multiply each component by: 
//              (XKMPER_WGS72 / AE).
//           To convert the returned ECI velocity vector to km/sec, 
//           multiply each component by:
//              (XKMPER_WGS72 / AE) * (MIN_PER_DAY / 86400).
bool NoradSDP4::getPosition(double tsince, EciPoint &eci )
{
     DeepInit(&m_eosq, &m_sinio, &m_cosio,  &m_betao, &m_aodp,   &m_theta2, 
            &m_sing, &m_cosg,  &m_betao2, &m_xmdot, &m_omgdot, &m_xnodot);

   // Update for secular gravity and atmospheric drag 
   double xmdf   = _sat.meanAnomaly() + m_xmdot * tsince;
   double omgadf = _sat.argPerigee() + m_omgdot * tsince;
   double xnoddf = _sat.raan() + m_xnodot * tsince;
   double tsq    = tsince * tsince;
   double xnode  = xnoddf + m_xnodcf * tsq;
   double tempa  = 1.0 - m_c1 * tsince;
   double tempe  = _sat.bstar() * m_c4 * tsince;
   double templ  = m_t2cof * tsq;
   double xn     = m_xnodp;
   double em;
   double xinc;

   DeepSecular(&xmdf, &omgadf, &xnode, &em, &xinc, &xn, &tsince);

   double a    = pow(XKE / xn, TWO_THRD) * sqr(tempa);
   double e    = em - tempe;
   double xmam = xmdf + m_xnodp * templ;

   DeepPeriodics(&e, &xinc, &omgadf, &xnode, &xmam);

   double xl = xmam + omgadf + xnode;

   xn = XKE / pow(a, 1.5);

   return finalPosition(xinc, omgadf, e, a, xl, xnode, xn, tsince, eci );
}
