#ifndef MAPPI_PRETREATMENT_FRAMES_FRAME_TYPES_H
#define MAPPI_PRETREATMENT_FRAMES_FRAME_TYPES_H

#include <stdint.h>

namespace mappi {
  namespace po {
    
    //!Соответствует ID в формате HPRT
    union HrptId {
      uint16_t M[2];
      //! Для NOAA HRPT
      struct { 
	uint16_t ch3x  :1; //!< 0=AVHRR Ch3B, 1=AVHRR Ch3A
	uint16_t input :1;
	uint16_t stabil:1;
	uint16_t addr  :4;
	uint16_t frame :2;
	uint16_t sync  :1;
      uint16_t  :6;
      uint16_t  :16;
      } S;
    };
  }

}
  
#endif
