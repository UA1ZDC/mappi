#include "indexes.h"
#include "zond.h"

namespace zond {

  //Ki=T850-T500+Td850-∆Td700
  bool kiIndex(const zond::Zond& zond, float* result)
  {
    if (nullptr == result) return false;
    
    zond::Uroven u850;
    zond::Uroven u500;
    zond::Uroven u700;
    
    if (!zond.getUrPoP(850, &u850) ||
	!zond.getUrPoP(500, &u500) ||
      !zond.getUrPoP(700, &u700)) {
      return false;
    }
    
    //TODO тут ещё и сомнительные попадают
    if (!u850.isGood(zond::UR_T)  ||
	!u500.isGood(zond::UR_T)  ||
	!u850.isGood(zond::UR_Td) ||
	!u700.isGood(zond::UR_D)) {
      return false;
    }
    
    *result = u850.value(zond::UR_T) - u500.value(zond::UR_T) + 
      u850.value(zond::UR_Td) - u700.value(zond::UR_D);
    
    return true;
  }

  //VT = T850 - T500, где T850 
  bool verticalTotalsIndex(const zond::Zond& zond, float* result)
  {
    if (nullptr == result) return false;
    
    zond::Uroven u850;
    zond::Uroven u500;
    
    if (!zond.getUrPoP(850, &u850) ||
	!zond.getUrPoP(500, &u500)
	) {
      return false;
    }
    
    //TODO тут ещё и сомнительные попадают
    if (!u850.isGood(zond::UR_T)  ||
	!u500.isGood(zond::UR_T) ) {
      return false;
    }
    
    *result = u850.value(zond::UR_T) - u500.value(zond::UR_T);
    
    return true;
  }

  //CT = Td850 - T500
  bool crossTotalsIndex(const zond::Zond& zond, float* result)
  {
    if (nullptr == result) return false;
    
    zond::Uroven u850;
    zond::Uroven u500;
    
    if (!zond.getUrPoP(850, &u850) ||
	!zond.getUrPoP(500, &u500)) {
      return false;
    }
    
    //TODO тут ещё и сомнительные попадают
    if (!u500.isGood(zond::UR_T)  ||
	!u850.isGood(zond::UR_Td)) {
      return false;
    }
    
    *result = u850.value(zond::UR_Td) - u500.value(zond::UR_T);
    
    return true;
  }

  // TT = VT + CT, Miller (1972);
  bool totalTotalsIndex(const zond::Zond& zond, float* result)
  {
    if (nullptr == result) return false;
    
    zond::Uroven u850;
    zond::Uroven u500;
    
    if (!zond.getUrPoP(850, &u850) ||
	!zond.getUrPoP(500, &u500)) {
      return false;
    }
    
    //TODO тут ещё и сомнительные попадают
    if (!u850.isGood(zond::UR_T)  ||
	!u500.isGood(zond::UR_T)  ||
	!u850.isGood(zond::UR_Td)) {
      return false;
    }
    
    *result =  u850.value(zond::UR_T) - u500.value(zond::UR_T) +
      u850.value(zond::UR_Td) - u500.value(zond::UR_T);
    
    return true;
  }

  //SWEAT = 12⋅Td850 + 20⋅(TT- 49) + 3.888⋅F850 + 1.944⋅F500 + (125⋅[sin(D500 - D850)+0.2])
  bool sweatIndex(const zond::Zond& zond, float* result)
  {
    if (nullptr == result) return false;
    
    float TTadd;
    bool ok = totalTotalsIndex(zond, &TTadd);
    if (!ok || TTadd <= 49) {
      TTadd = 0;
    } else {
      TTadd = 20 * (TTadd - 49);
    }

    zond::Uroven u850;
    zond::Uroven u500;
    
    if (!zond.getUrPoP(850, &u850) ||
	!zond.getUrPoP(500, &u500)) {
      return false;
    }
    
    //TODO тут ещё и сомнительные попадают
    if (!u850.isGood(zond::UR_Td) ||
	!u850.isGood(zond::UR_ff) ||
	!u500.isGood(zond::UR_ff) ||
	!u850.isGood(zond::UR_dd)  ||
	!u500.isGood(zond::UR_dd)) {
      return false;
    }

    float add = 0;
    if ( (u850.value(zond::UR_dd) >= 130 && u850.value(zond::UR_dd) <= 250) ||
	 (u500.value(zond::UR_dd) >= 210 && u500.value(zond::UR_dd) <= 310) ||
	 (u500.value(zond::UR_dd) - u850.value(zond::UR_dd) > 0) || //TODO на границе 360 ?
	 u500.value(zond::UR_ff) <= 7 || u850.value(zond::UR_ff) <= 7
	 ) {
      add = 0;
    } else {
      add = 125 * sin(MnMath::deg2rad(u500.value(zond::UR_dd) - u850.value(zond::UR_dd))) + 0.2f;
    }
    
    *result = 12 * u850.value(zond::UR_Td) + TTadd + 3.888f * u850.value(zond::UR_ff) +
      1.944f * u500.value(zond::UR_ff) + add;
    
    return true;
  }


} // namespace zond
