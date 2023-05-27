#ifndef COMMONS_SATELLITE_NORADSGP4_H
#define COMMONS_SATELLITE_NORADSGP4_H

#include <sat-commons/satellite/noradbase.h>

class NoradSGP4 : public NoradModelBase {
 public:
  NoradSGP4( SatelliteBase&  );
  virtual ~NoradSGP4();
  
  virtual bool getPosition(double tsince, EciPoint &eci );

 protected:
   double m_c5; 
   double m_omgcof;
   double m_xmcof;
   double m_delmo;
   double m_sinmo;  
};

#endif //NORADSGP4_H
