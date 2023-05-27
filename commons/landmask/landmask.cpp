#include <new>

#include <qbytearraylist.h>

#include <cross-commons/app/paths.h>
#include <cross-commons/debug/tlog.h>

#include "landmask.h"

#define EL_LAT_NUM      1080
#define EL_LON_NUM      2160
#define ELNUM           EL_LAT_NUM * EL_LON_NUM
#define ELFILENAME      "elevation.dat"

#define WV_LEVELS_CNT (11) //!< Количество уровней влажности в атласе вложности
#define WV_PIXEL_DATA_CNT (WV_LEVELS_CNT+1) //!< Количество значений для одной точки в атласе владности (0 - давление, остальные влажность по уровням)

const float pniv[] = {1000.,950.,900.,850.,700.,500.,400.,300.,200.,100.,50.};//!< pressure on levels

namespace meteo {
  namespace map {
    template<> internal::LandMask* LandMask::_instance = 0;
namespace internal {
      
LandMask::LandMask(){
  stream = 0;
  last_box_num = -1;
  ptr_arr=0;
  init_elevation();
  init();
}

LandMask::~LandMask(){
  if(stream) {fclose(stream);stream=0;}
  if(ptr_arr) {delete []ptr_arr;ptr_arr=0;}
}


int LandMask::init() {
  QString filename;    
  filename = MnCommon::sharePath("meteo") + "/landmask/landmask.dat";
  return init(filename.toUtf8());
}

/**
 * initialize the 218 x 128 per degree resolution processing
 * to get the a mask
 * @param landfile name of mask file to use
 * @return 
 */
int LandMask::init( const char *landfile)
{
  int16_t rec_arr[REC_LEN];
  
  if( ( stream = fopen( landfile, "rb" ) ) == NULL )
  {
    return ERR_OPENFILE;
  }
  
  /*
   *  read in first record and check it out
   */
  if( fread( rec_arr, sizeof(int16_t), REC_LEN, stream) != REC_LEN )
  {
    return ERR_FILEDATA;
  }
  
  swapc_bytes((char*)rec_arr, 2, REC_LEN);
  
  if( rec_arr[0] != 128 || rec_arr[2] != 2048 ||
    rec_arr[3] != -180 || rec_arr[4] != 180 ||
    rec_arr[5] != -90 ||rec_arr[6] != 90 )
  {
    return ERR_FILEDATA;
  }
  if(ptr_arr) {delete []ptr_arr;ptr_arr=0;} 
  ptr_arr = new(std::nothrow) int16_t [NITEMPTR];
  if(!ptr_arr) return ERR_MEMORY;
  
  /*
   *  read the next 64 REC_LEN short records as pointers to the state
   *  of the 1 degree box
   */
  if( fread( ptr_arr, sizeof(int16_t), NITEMPTR, stream) != NITEMPTR )
  {
    return ERR_FILEDATA;
  }
  
  swapc_bytes((char*)ptr_arr, 2, NITEMPTR);
  
  access_cnt = 0;  /* current count of mask accesses in the program */
  boxes_active = 0;  /* # boxes read into the storage structure */
  
  return ERR_NOERR;
}


/**
 * Изменяет порядок байт в массиве
 * @param in исходный массив
 * @param nbyte количество байт в слове
 * @param ntime количество слов
 * @return по идее должна возвращать 0 (или ошибку ERR_MEMORY)
 *  
 */
int LandMask::swapc_bytes(char *in, int  nbyte,int   ntime)
{
  char *tmpbuf=0, *ptr=0;
  int  i, j, k;
  
  tmpbuf = new(std::nothrow) char[nbyte+1];
  if(!tmpbuf) return ERR_MEMORY;
  
  for (j=0; j<ntime; j++)
  {
    ptr = in + j * nbyte;
    memcpy(tmpbuf, ptr, nbyte);
    
    for (i=0, k=nbyte-1; i<nbyte; i++, k--)
      tmpbuf[i] = ptr[k];
    
    memcpy(ptr, tmpbuf, nbyte);
  }
  delete []tmpbuf;
  
  return ERR_NOERR; 
}


/**
 * For an incoming latitude, longitude find if it is over
 *      mask or not using the 128 by 128 / degree mask file
 * @param lat latitude of point from -90. to 90.
 * @param lon longitudeof point from -180. to 180.
 * @return 1 for land mask , 0 for sea mask 
 */
int LandMask::get( float lat, float lon)  {
  int land_fl=0, box_num=0, box_index=0, low_access_cnt=0,
  low_access_index=0, i=0, box_wd=0, box_bit=0;
  float lat_off=0., lon_off=0.;
  unsigned short mask[16] = { 0x8000, 0x4000, 0x2000, 0x1000,
    0x800,  0x400,  0x200,  0x100,
    0x80,   0x40,   0x20,   0x10,
    0x8,    0x4,    0x2,    0x1 };
    
    /*
     *  check that b128_msk_init was called - the stream should be non-null
     */
    access_cnt++;  /* incriment count of times called */
    
    if( stream == NULL ) 
    {
      return ERR_OPENFILE;
    }
    /*
     *  get the box number for this lat, lon
     */
    box_num = b128_box_num( lat, lon, &lat_off, &lon_off );
    /*
     *  maybe the box is all mask or non-mask and we can just return that info
     */
    if( *( ptr_arr + box_num ) < 2 )
    {
      land_fl = *( ptr_arr + box_num );
    }
    else
    {
      /*
       *  the box contains a mix of land, water. see if we're looking at last box 
       */
      if( box_num == last_box_num )
      {
        box_index = last_box_index; /* index to locally stored box info */
      }
      else
      {
        /*
         *  not latest box, it still may be in another array position
         */
        box_index = -1;
        //low_access_cnt = 1285 * 10000;  /* while searching, record the lowest */
        low_access_cnt = access_cnt;  /* while searching, record the lowest */
        low_access_index = -1;          /* access count and its location */
        
        for( i = 0; i < boxes_active; i++ )
        {
          if( box_info[i].box_num == box_num )
          {
            /*
             *  we found the box we need in array
             */
            box_index = i;
            box_info[i].last_access_cnt = access_cnt;
            break;
          }
          if( box_info[i].last_access_cnt < low_access_cnt )
          {
            low_access_cnt = box_info[i].last_access_cnt;
            low_access_index = i;  /* record least recently used updated */
          }
        }
        /*
         *  if we didn't find the box in the struct array, set one up
         */
        if( box_index == -1 )
        {
          /*
           *  first case: add a box to the array in an empty slot
           */
          if( boxes_active < BOX_MAX - 1 )
          {
            boxes_active++;
            box_index = boxes_active - 1;
            box_info[box_index].box_ptr =   (unsigned short *)malloc( REC_LEN * sizeof( int16_t ) ); //FIXME похоже нигде не очищается
            /*  for debug / info
             *          printf( "Incrimenting boxes_active to %d\n", boxes_active );
             *          printf( "          lat: %f   lon: %f\n", lat, lon );
             */
          }
          /*
           *  2nd case is to re-use the least used array location
           */
          else
          {
            box_index = low_access_index;
            /*  for debug / info
             *          printf( "Re-using box index %d\n", box_index );
             *          printf( "          lat: %f   lon: %f\n", lat, lon );
             */
          }
          /*
           *  in either case, read in the proper record into the slot
           */
          if (box_index < 0) { //TODOs
            printf("access_cnt=%d box_index=%d\n", access_cnt, box_index);
            return ERR_PARAM;//!!!
          }
          box_info[box_index].box_num = box_num;
          box_info[box_index].last_access_cnt = access_cnt;
          if( fseek( stream, 
            ptr_arr[box_num] * REC_LEN * sizeof( int16_t ), SEEK_SET )
            != 0 )
          {
            return ERR_FILEDATA;
          }
          else
          {
            if( fread( box_info[box_index].box_ptr, sizeof( int16_t ), 
              REC_LEN, stream ) != REC_LEN )
            {
              return ERR_FILEDATA;
            }
            swapc_bytes((char *)box_info[box_index].box_ptr, 2, REC_LEN);
          }
        }
      }
      /*
       *  we have the box, get the exact word and bit to extract
       */
      b128_wd_bit( lat_off, lon_off, &box_wd, &box_bit );
      land_fl = ( ( *( box_info[ box_index ].box_ptr + box_wd ) &
      mask[ box_bit ] ) == 0 ) ? 0 : 1;
      last_box_index = box_index;
      last_box_num = box_num;
    }
    return land_fl;
}


/**
 * for the 128 / degree land mask file, find the 
 *      index of the degree box from the lat, lon
 * @param lat latitude of point from -90. to 90.
 * @param lon longitudeof point from -180. to 180.
 * @param lat_off all positive latitude used during call to 
 *                  b128_wd_bit to get mask at a 128th of a degree point
 * @param lon_off all positive longitude used during call to 
 *                  b128_wd_bit to get mask at a 128th of a degree point
 * @return index (0 rel) of the 1 degree box
 */
int LandMask::b128_box_num( float lat, float lon, float *lat_off, float *lon_off )
{
  int box_num, lat_index, lon_index;
  /*
   *  make positive lat and lon and check them
   */
  *lat_off = 90. + lat;
  *lon_off = 180. + lon;
   
    if( *lat_off < 0. ) *lat_off = 0.;
    if( *lat_off > 180. ) *lat_off = 180.;
    if( *lon_off < 0. ) *lon_off = 0.;
    if( *lon_off > 360. ) *lon_off = 360.;
   /*
  *  Take care of case of 90 lat and 180 lon properly
  */
  lat_index = ( *lat_off == 180. ) ? 179 : (int)(*lat_off);
  lon_index = ( *lon_off == 360. ) ? 359 : (int)(*lon_off);
  /*
   *  compute the box #
   */
  box_num = lon_index + 360 * lat_index;
  
  return box_num;
}

int LandMask::init_sst(uint month)
{
  QString file = MnCommon::sharePath("meteo") + QString("/landmask/atlas/atlas_sst_%1.dat").arg(month, 2, 10, QChar('0'));
  return init_atlas(file);
}

int LandMask::init_albedo(uint month)
{
  QString file = MnCommon::sharePath("meteo") + QString("/landmask/atlas/atlas_albedo_%1.dat").arg(month, 2, 10, QChar('0'));
  return init_atlas(file);
}

int LandMask::init_wv(uint month)
{
  QString file = MnCommon::sharePath("meteo") + QString("/landmask/atlas/atlas_wv_%1.dat").arg(month, 2, 10, QChar('0'));
  return init_atlas(file);
}

/**
 * initialize the
 * to get the a atlas
 * @param landfile name of mask file to use
 * @return
 */
int LandMask::init_atlas(const QString& fileName)
{
  char hdr_arr[ATLAS_HEAD_LEN];
  uint res = 0;

  if( ( stream = fopen( fileName.toLocal8Bit().constData(), "rb" ) ) == NULL )
  {
    printf("err open atlasfile=%s\n", fileName.toLocal8Bit().constData());
    return ERR_OPENFILE;
  }

  if( fread( hdr_arr, sizeof(char), ATLAS_HEAD_LEN, stream) != ATLAS_HEAD_LEN)
  {
    return ERR_FILEDATA;
  }

//  sscanf(hdr_arr+16, "%d %d %f %f %f %f %d %d\n", &ahead.month,  &ahead.size,  &ahead.latBeg,  &ahead.lonBeg,
//         &ahead.latStep,  &ahead.lonStep,  &ahead.numPixel,  &ahead.numLines);

  QByteArray ba = QByteArray::fromRawData(hdr_arr, ATLAS_HEAD_LEN);
  QByteArrayList list = ba.split(' ');
  list.removeAll(QByteArray());

  if ( list.size() != 10 ) { return ERR_FILEDATA; }

  ahead.month = list.at(2).toInt();
  ahead.size  = list.at(3).toInt();
  ahead.latBeg = list.at(4).toDouble();
  ahead.lonBeg = list.at(5).toDouble();
  ahead.latStep = list.at(6).toDouble();
  ahead.lonStep = list.at(7).toDouble();
  ahead.numPixel = list.at(8).toInt();
  ahead.numLines = list.at(9).toInt();

  float lonmax = ahead.lonBeg + ahead.lonStep*(ahead.numPixel-1);
  if (ahead.lonBeg < 0) {
    ahead.norm = true;
  } else if (lonmax >= 180) {
    ahead.norm = false;
  } else {
    ahead.norm = true;
  }

  if(ptr_arr) {delete []ptr_arr;ptr_arr=0;}
  //ptr_arr = new(std::nothrow) Q_INT16[ATLAS_SIZE];
  uint cnt = ahead.size/ sizeof(qint16) * ahead.numLines;
  ptr_arr = new(std::nothrow) qint16[cnt];
  if(!ptr_arr) return ERR_MEMORY;

  fseek(stream, ahead.size, SEEK_SET);

  // if((res= fread( ptr_arr, sizeof(Q_INT16), ATLAS_SIZE, stream)) != ATLAS_SIZE)
  if((res= fread( ptr_arr, sizeof(qint16), cnt, stream)) != cnt) {
    if(ptr_arr) {delete []ptr_arr;ptr_arr=0;}
    return ERR_FILEDATA;
  }

  return ERR_NOERR;
}

//! возращает albedo в %*100, sst в  градусах Цельсия*100
short LandMask::get_atlas_data(float f, float l)
{
  int i,j;
  double dfi, dla, dx, dy, vs_1, vs_2;

  if(!ptr_arr) return false;

  if (ahead.norm) {
    while(l >= 180.) l -= 360.;
  } else {
    while(l < 0.) l += 360.;
  }

  // dfi=ATLAS_STEP_FI;
  // dla=ATLAS_STEP_LA;
  dfi = ahead.latStep;
  dla = ahead.lonStep;
  // int size_fi=ATLAS_SIZE_FI;
  int size_la=ahead.numPixel; //ATLAS_SIZE_LA;
  // double fi_start=-90.;
  // double la_start=0.;
  double fi_start=ahead.latBeg;
  double la_start=ahead.lonBeg;


  i=int((f-fi_start)/dfi)/*%(size_fi-1)*/;
  j=int((l-la_start)/dla)/*%(size_la-1)*/;

  dx=(f-(i*dfi+fi_start))/dfi;
  dy=(l-(j*dla+la_start))/dla;

  // if( (((i+1)*size_la+j+1)<ATLAS_SIZE) && ((i*size_la+j)>-1))
  if( (((i+1)*size_la+j+1)<ahead.numPixel*ahead.numLines) && ((i*size_la+j)>-1))
  {
    vs_1=(1.-dy)*ptr_arr[i*size_la+j]+dy*ptr_arr[i*size_la+j+1];
    vs_2=(1.-dy)*ptr_arr[(i+1)*size_la+j]+dy*ptr_arr[(i+1)*size_la+j+1];
    return static_cast<short>((1.-dx)*vs_1+dx*vs_2);
  }

  return -9999;
}

/*!
 * \brief Получение данных атласа влажности
 * \param f широта, градусы
 * \param l долгота, градусы
 * \param alt высота, м
 * \return значение влажности,  g/kg*100
 */
short LandMask::get_wv_data(float f, float l, float alt)
{
  int i,j;
  double dfi, dla, dx, dy, vs_1, vs_2;


  if(!ptr_arr) return false;

  if (ahead.norm) {
    while(l >= 180.) l -= 360.;
  } else {
    while(l < 0.) l += 360.;
  }

  dfi = ahead.latStep;
  dla = ahead.lonStep;

  int size_la=ahead.numPixel*WV_PIXEL_DATA_CNT;
  double fi_start=ahead.latBeg;
  double la_start=ahead.lonBeg;
  double wv[WV_PIXEL_DATA_CNT]; //на каждую точку 12 значений, 0 - давление, остальные влажность по уровням

  i=int((f-fi_start)/dfi)/*%(size_fi-1)*/;
  j=int((l-la_start)/dla)/*%(size_la-1)*/;

  dx=(f-(i*dfi+fi_start))/dfi;
  dy=(l-(j*dla+la_start))/dla;

  if( (((i+1)*size_la+j+1)<size_la*ahead.numLines) && ((i*size_la+j)>-1))
  {
    for (uint num=0; num< WV_PIXEL_DATA_CNT; num++) {
      vs_1=(1.-dy)*ptr_arr[i*size_la+j*WV_PIXEL_DATA_CNT + num]+dy*ptr_arr[i*size_la+(j+1)*WV_PIXEL_DATA_CNT + num];
      vs_2=(1.-dy)*ptr_arr[(i+1)*size_la+j*WV_PIXEL_DATA_CNT + num]+dy*ptr_arr[(i+1)*size_la+(j+1)*WV_PIXEL_DATA_CNT + num];
      wv[num] = (1.-dx)*vs_1+dx*vs_2;
      if (num != 0) {
        wv[num] /= 100*1000; //spec. humidity in g/g
      }
    }

    wv[0] *= exp(-alt/8150); // computes the surface pressure for the fov altitude

    float cwv;
    total_wv(wv[0], &wv[1], &cwv);
    return static_cast<short>(cwv*100);
  }

  return -9999;
}


/**
 * 
 * @param lat_off all positive latitude used during call to 
 *                  b128_wd_bit to get mask at a 128th of a degree point
 * @param lon_off all positive longitude used during call to 
 *                  b128_wd_bit to get mask at a 128th of a degree point
 * @param box_wd word # in array of 128 sq box to find
 * @param box_bit bit # in word of desired mask value
 * @return 
 */
int LandMask::b128_wd_bit( float lat_off, float lon_off, int *box_wd, int *box_bit )
{
  int lat_sub, lon_sub;
  int lon_wd;
  double dumb;
  
  /*
   *  find distance in to the bit from edge of 128 by 128 array
   */
  lat_sub = (int) (modf( lat_off, &dumb ) * 128 );
  if( lat_off == 180. ) lat_sub = 127;
  
  lon_sub = (int) (modf( lon_off, &dumb ) * 128 );
  if( lon_off == 360. ) lon_sub = 127;
  /*
   *  get the word in longitude direction and bit and then get linear word
   */
  lon_wd = lon_sub / 16;
  *box_bit = lon_sub - lon_wd * 16;
  *box_wd = lon_wd + lat_sub * 8;
  
  return 0;
}

int LandMask::init_elevation() {
  QString fname = MnCommon::sharePath("meteo") + "/landmask/elevation.dat";
  stream = fopen( fname.toUtf8(), "r" );
  if ( 0 == stream ) {
    return ERR_OPENFILE;
  }
  if ( 0 == ( ptr_arr = new(std::nothrow) int16_t[ELNUM] ) ) {
    fclose( stream );
    return ERR_MEMORY;
  }
  
  if ( ELNUM != fread( ptr_arr, sizeof(int16_t), ELNUM, stream ) ) {
    fclose( stream );
    return ERR_READFILE;
  }
  
  fclose( stream );
  stream = 0;
  return ERR_NOERR;
}

int LandMask::elevation( float lat, float lon ) const {
  if ( 0 == ptr_arr ) {
    return ERR_MEMORY;
  }
  if ( lat > 90. || lat < -90. || lon < -180. || lon > 360. ) {
    return ERR_PARAM;
  }
  if ( lon > 180. ) {
    lon -= 360.;
  }
  int i,j;
  double dx, dy, vs_1, vs_2;
  const float ELEVSTEP = 0.1670;
  
  
  float lat_start = -90.;
  float lon_start = -180.;
  
  i = int( ( lat - lat_start ) / ELEVSTEP );
  j = int( ( lon - lon_start ) / ELEVSTEP );
  //return ptr_arr[i * EL_LON_NUM + j];
  
  dx = ( lat - ( i * ELEVSTEP + lat_start ) ) / ELEVSTEP ;
  dy = ( lon - ( j * ELEVSTEP + lon_start ) ) / ELEVSTEP ;
  
  if ( ( ( ( i + 1 ) * EL_LON_NUM + j + 1 ) < ELNUM ) && ( ( i * EL_LON_NUM + j ) > -1 ) ) {
    vs_1 = ( 1. - dy ) * ptr_arr[ i * EL_LON_NUM + j] + dy * ptr_arr[ i * EL_LON_NUM + j + 1];
    vs_2 = ( 1. - dy ) * ptr_arr[ ( i + 1 ) * EL_LON_NUM + j] + dy * ptr_arr[( i + 1 ) * EL_LON_NUM + j + 1];
    return static_cast<int16_t>( ( 1. - dx ) * vs_1 + dx * vs_2 );
  }
  else {
    return -401;
  }
  
}

//       real hum(nbniv)          ! specific humidity on levels   (g/g)
//       real psurf               ! surface pressure              (hpa)
//       real cwv                 ! total water vapor content     (g/cm2 = cm)
int LandMask::total_wv(double psurf, double* hum, float* cwv)
{
  // pressure on levels
  float pres[WV_LEVELS_CNT];
  float hval;
  float dp[WV_LEVELS_CNT];

  for (uint lev=0; lev < WV_LEVELS_CNT; lev++) {
    pres[lev] = pniv[lev];
  }

  int firstlev = 0;
  for (uint lev=0; lev < WV_LEVELS_CNT; lev++) {
    if (pniv[lev] <= psurf && firstlev == 0) {
      firstlev = lev;
      intplog(pniv, hum, &psurf, &hval, WV_LEVELS_CNT, 1);
      hum[lev] = hval;
      pres[lev] = psurf;
    }
  }

  if ( firstlev >= (WV_LEVELS_CNT-1) || firstlev == 0) {
    *cwv = -9999.;
    return ERR_UNKNOWN;
  } else {
    //layers centered on pres levels
    dp[WV_LEVELS_CNT-1] = (pres[WV_LEVELS_CNT-1] + pres[WV_LEVELS_CNT-2]) /2.;
    dp[firstlev] = (pres[firstlev] - pres[firstlev+1]) /2.;
    for (uint lev = firstlev+1; lev < WV_LEVELS_CNT-1; lev++ ) {
      dp[lev] = fabs((pres[lev+1] + pres[lev]) /2. -
         (pres[lev-1] + pres[lev]) /2. );
    }

    //total water vapor content in kg/m2
    // (divide by 10 to obtain g/cm2 or precipitable centimeters units)
    float uh2o = 0.;
    for (uint lev=firstlev; lev < WV_LEVELS_CNT; lev++) {
      uh2o = uh2o + hum[lev] * dp[lev] /9.81;
    }

    *cwv = uh2o*100.;// in kg/m2
    *cwv=  *cwv/10.;// in g/cm2 or cm
  }

  return ERR_NOERR;
}

//!  interpolation logarithmique sur les pressions
//ni размер pi и ti
//nf размер pf, tf
void LandMask::intplog(const float* pi, double* ti, double* pf, float* tf, int ni, int nf)
{
  int i,f;
  float slopt,dlpi,dlpf;
  float pseuil;
  bool termine;

  if(pi[0] > pi[ni-1]) {

    for ( f=0; f < nf; f++ ) {
      if (pf[f] >= pi[0]) {
  dlpi=log(pi[1]/pi[0]);
  slopt=(ti[1]-ti[0])/dlpi;
  dlpf=log(pf[f]/pi[0]);
  tf[f]=ti[0]+slopt*dlpf;
      }	else if (pf[f] <= pi[ni-1]) {
  dlpi=log(pi[ni-1]/pi[ni-2]);
  slopt=(ti[ni-1]-ti[ni-2])/dlpi;
  dlpf=log(pf[f]/pi[ni-2]);
  tf[f]=ti[ni-2]+slopt*dlpf;
      }  else {
  i=1;
  termine=false;
  do {
    if(pi[i] <= 1 ) {
      pseuil=0.0001;
    } else if (pi[i] <= 10.) {
      pseuil=0.001;
    } else {
      pseuil=0.01;
    }
    if(fabs(pi[i]-pf[f]) <= pseuil) {
      tf[f]=ti[i];
      termine=true;
    } else {
      if(pi[i] <  pf[f]) {
        dlpi=log(pi[i]/pi[i-1]);
        slopt=(ti[i]-ti[i-1])/dlpi;
        dlpf=log(pf[f]/pi[i-1]);
        tf[f]=ti[i-1]+slopt*dlpf;
        termine=true;
      }
    }
    i=i+1;
  } while (!termine);
      }
    }

  } else {

    for  ( f=0; f < nf; f++ ) {
      if(pf[f] <= pi[0]) {
  dlpi=log(pi[1]/pi[0]);
  slopt=(ti[1]-ti[0])/dlpi;
  dlpf=log(pf[f]/pi[0]);
  tf[f]=ti[0]+slopt*dlpf;
      } else if(pf[f] >= pi[ni-1]) {
  dlpi=log(pi[ni-1]/pi[ni-2]);
  slopt=(ti[ni-1]-ti[ni-2])/dlpi;
  dlpf=log(pf[f]/pi[ni-2]);
  tf[f]=ti[ni-2]+slopt*dlpf;
      } else {
  i=1;
  termine=false;
  do {

    if(pi[i] <= 1) {
      pseuil=0.0001;
    } else if(pi[i] <= 10.) {
      pseuil=0.001;
    } else {
      pseuil=0.01;
    }
    if(fabs(pi[i]-pf[f]) <= pseuil) {
      tf[f]=ti[i];
      termine=true;
    } else {
      if(pi[i] > pf[f]) {
        dlpi=log(pi[i]/pi[i-1]);
        slopt=(ti[i]-ti[i-1])/dlpi;
        dlpf=log(pf[f]/pi[i-1]);
        tf[f]=ti[i-1]+slopt*dlpf;
        termine=true;
      }
    }
    i=i+1;
  } while (!termine);
      }
    }
  }

}

int16_t* LandMask::get_ptr_arr() const {
  return ptr_arr;
}
  
    
  }
  } 
}
