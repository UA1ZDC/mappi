#include <stdio.h>

#include <landmask.h>
#include <cross-commons/debug/tlog.h>

int main() {
  TAPPLICATION_NAME("meteo");
  QTextCodec::setCodecForCStrings( QTextCodec::codecForLocale() );
  
//  float lat = 28;
//  float lon = 87;
  for(int i= -90; i < 90 ; i++){
  float lat = i;
  float lon = 28.3;
  int el = meteo::map::LandMask::instance()->elevation( lat, lon );
  bool mask = meteo::map::LandMask::instance()->get(lat, lon);
//    if ( 4000. < el ) {
      debug_log<< "LLLat: "<<lat<< "Lon:"<<lon<< "Elevation: "<< el << mask;
//    }
  }
      
      //  }
//  int num = 0;
//  for ( float i = 0.; i < 360.; i += .1500 ) {
//    for ( float j = -90.; j < 90.; j += .1500 ) {
//      ++num;
//      lon = i;
//      lat = j;
//      if ( int el = lm.elevation( lat, lon ) ) {
//        if ( 5800. < el ) {
//          debug( "Lat: %f Lon: %f Elevation: %d", lat, lon, el );
//        }
//      }
//    }
//  }
//  debug("NUM: %d", num);
//  int i = 0;
//  int16_t* arr = lm.get_ptr_arr();
//  int16_t val;
//  for ( i = 0; i < 1080*2160; ++i ) {
//    if ( 5800 < ( val = arr[i] ) && 5900 > val ) {
//      debug( "VAL: %d", val );
//    }
//  }
//  debug( "NUM: %d", i );
  return EXIT_SUCCESS;
}
