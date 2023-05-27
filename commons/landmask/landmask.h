#ifndef LANDMASK_H
#define LANDMASK_H

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include <stdio.h>
#include <math.h>
#include <memory>

#include <qstring.h>

//#include <commons/defines/mn_defines.h>
#include <cross-commons/app/paths.h>
#include <cross-commons/funcs/mn_errdefs.h>
#include <cross-commons/singleton/tsingleton.h>

//#define   PI  3.14159265 есть в math.h
//#define   SINC  0.0015911
//#define   RADEG 57.29578       /* 180.0 / PI */ есть в mnmath.h
//#define   F 0.0033528132   /* 1.0 / 298.257 */
//#define   OMF2  0.99330562     /* (1.0 - F) * (1.0 - F) */

#define REC_LEN 1024
#define NITEMPTR REC_LEN * 64
#define BOX_MAX 100
#define ATLAS_HEAD_LEN 80

/* #define ATLAS_SIZE_FI 1200 */
/* #define ATLAS_SIZE_LA 2400 */
/* #define ATLAS_SIZE ATLAS_SIZE_LA*ATLAS_SIZE_FI */
/* #define ATLAS_STEP_LA 0.15 */
/* #define ATLAS_STEP_FI 0.15 */

namespace meteo {
  namespace map {
    namespace internal {
      
  struct box_info_str
  {
    box_info_str(){
      box_num=0;
      last_access_cnt=0;
      box_ptr=0;
    };
    ~box_info_str(){
      if(0 != box_ptr ) delete[] box_ptr;
    };
    int box_num;  /* box number stored in this slot */
    int last_access_cnt;  /* last access count for this boxes data */
    unsigned short *box_ptr; /* pointer to the box's data */
  } ;
  
  struct atlas_info
  {
    char box_num;  /* box number stored in this slot */
    int last_access_cnt;  /* last access count for this boxes data */
    unsigned short *box_ptr; /* pointer to the box's data */
  } ;
  
  //! Заголовок в файле-атласе
  struct AtlasHeader {
    int month; //!< месяц
    int size;  //!< размер строки в атласе, байт
    float latBeg; //!< широта первой точки, градусы
    float lonBeg; //!< долгота первой точки
    float latStep; //!< шаг широты, изменяется по строкам, градусы
    float lonStep; //!<  шаг долготы, изменяется по столбцам, градусы
    int numPixel;  //!<  количество долгот (столбцов) в атласе
    int numLines;  //!<  количество широт (строк без строки заголовка) в атласе
    bool norm; //!< нет в заголовке файла.  true, если долгота в атласе [-180,180[. false  [0,360[
  };


class LandMask
{
public:
  LandMask();
  ~LandMask();

public:
  int16_t* get_ptr_arr() const;


  //FOR LANDMASK
  int init();
 // int init_sst();
  int init(const char *landfile);
  int get( float lat, float lon);
  int swapc_bytes(char *in, int  nbyte,int ntime);
  int b128_wd_bit( float lat_off, float lon_off, int *box_wd, int *box_bit );
  int b128_box_num( float lat, float lon, float *lat_off, float *lon_off );

  //for ATLAS 
  int init_sst(uint month);
  int init_albedo(uint month);
  int init_wv(uint month);
  int init_atlas(const QString& fileName);
  short get_atlas_data(float f,float l);
  short get_wv_data(float f,float l, float alt);

  //for ELEVATION
  int init_elevation();
  int elevation( float lat, float lon ) const ;


private:
  int total_wv(double psurf, double*hum, float* cwv);
  void intplog(const float* pi, double* ti, double* pf, float* tf, int ni,int nf);

private:
  FILE *stream;
  
  int16_t *ptr_arr;
  int access_cnt;
  int last_box_num;
  int boxes_active;
  int last_box_index;
  box_info_str box_info[BOX_MAX];

  AtlasHeader ahead;
  friend class TSingleton<meteo::map::internal::LandMask>;
  
};
    }
    typedef TSingleton<meteo::map::internal::LandMask> LandMask;

}
}

#endif
