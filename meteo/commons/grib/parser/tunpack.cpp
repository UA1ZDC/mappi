#include "tunpack.h"


#include <qobject.h>
#include <qimage.h>
#include <qimagereader.h>
#include <qfile.h>

#include <math.h>
#include <png.h>

#include <cross-commons/debug/tlog.h>
#include <commons/mathtools/mnmath.h>

#include "trepresent.pb.h"
#include "tgribformat.h"

#include <openjpeg.h>


struct png_stream {
  unsigned char *stream_ptr;     /*  location to write PNG stream  */
  png_uint_32 stream_len;               /*  number of bytes written       */
};


typedef struct
{
  OPJ_UINT8* pData;
  OPJ_SIZE_T dataSize;
  OPJ_SIZE_T offset;
} opj_input_memory_stream;

static OPJ_SIZE_T opj_input_memory_stream_read(void * p_buffer, OPJ_SIZE_T p_nb_bytes, void * p_user_data);
static OPJ_OFF_T opj_input_memory_stream_skip(OPJ_OFF_T p_nb_bytes, void * p_user_data);
static OPJ_BOOL opj_input_memory_stream_seek(OPJ_OFF_T p_nb_bytes, void * p_user_data);
static void opj_input_memory_stream_free (void * p_user_data);

namespace grib {
  void customPngRead(png_structp png_ptr,png_bytep data, png_size_t length);
  bool decodePng(unsigned char *pngbuf, png_uint_32 *width, png_uint_32 *height, uint8_t *dst);
}


//template 0 (checked)
bool grib::SimplePoint(float* dst, uint32_t dstSize, uint8_t* src, uint32_t size, google::protobuf::Message* def)
{
  TRepresent::PointData* base = (static_cast<TRepresent::SimplePoint*>(def))->release_pd();
  if (!base) return false;

  if (dstSize *  base->bits() / 8 > size) {
    error_log << QObject::tr("Не соответствие размера данных. Ожидалось %1, реально %2").arg(dstSize *  base->bits() / 8).arg(size);
    return false;
  }

  if (dstSize *  base->bits() / 8 != size) {
    warning_log << QObject::tr("Не соответствие размера данных. Ожидалось %1, реально %2").arg(dstSize *  base->bits() / 8).arg(size);
  }
  
  
  // var(base->Utf8DebugString());
  //debug_log << dstSize << size << dstSize *  base->bits() / 8 << base->bits();
  return grib::unpackBits(dst, dstSize, src, size*8 / base->bits(), base);  
}


//TODO сделать реализацию. Этот образец ещё не утверждён
//template 1
bool grib::MatrixPoint(float* dst, uint32_t dstSize, uint8_t* src, uint32_t size, google::protobuf::Message* def)
{
 TRepresent::PointData* base = (static_cast<TRepresent::MatrixPoint*>(def))->release_pd();
  if (!base) return false;
  return grib::unpackBits(dst, dstSize, src, size*8 / base->bits(), base);  
}


//template 2 (checked)
bool grib::ComplexPoint(float* dst, uint32_t dstSize, uint8_t* src, uint32_t /*size*/, google::protobuf::Message* def)
{
  TRepresent::ComplexPoint* cp = static_cast<TRepresent::ComplexPoint*>(def);
  TRepresent::PointData* pd = cp->release_pd();

  // printvar(QString::fromStdString(cp->DebugString()));
  // printvar(QString::fromStdString(pd->DebugString()));
  
  float ref = pd->r() * pow(10, -pd->d());
  float scale = pow(2, pd->e()) * pow(10, -pd->d());

  // var(ref);
  // var(scale);

  if (cp->ng() == 0) {
    for (uint32_t i = 0; i < dstSize; i++) {
      dst[i] = ref;
    }
    return true;
  }

  uint64_t* gRef = new(std::nothrow) uint64_t[cp->ng()]; //group ref values
  uint64_t* gWidth = new(std::nothrow) uint64_t[cp->ng()]; //group widths
  uint64_t* gLength = new(std::nothrow) uint64_t[cp->ng()]; //group lengths
  if (!gRef || !gWidth || !gLength) {
    return false;
  }

  //TODO проверка размеров и индексов, которые пытаемся считать и записать
  int offset = 0;
  grib::unpackBits(gRef, src, pd->bits(), cp->ng());
  offset += (cp->ng()*pd->bits()+7)/8;
  grib::unpackBits(gWidth, src + offset, cp->bits_w(), cp->ng(), cp->r_w());
  offset += (cp->ng()*cp->bits_w()+7)/8;
  grib::unpackBits(gLength, src + offset, cp->bits_l(), cp->ng()-1, cp->r_l(), cp->inc_l());
  offset += ((cp->ng())*cp->bits_l()+7)/8;
  gLength[cp->ng()-1] = cp->last_l();

  // uint64_t total=0;
  // for (uint i=0;i<cp->ng();i++) {
  //   total += gLength[i];
  // }
  // var(total);
  
  uint8_t shiftBit=0; //уже считанных из байта
  uint32_t shiftByte = 0;
  uint32_t dNum=0;
  // float maxd =0;
  // float mind = 99999999.0;
  for (uint32_t grNum=0; grNum < cp->ng(); grNum++) {
    for (uint32_t i=0; i< gLength[grNum]; i++) {
      uint64_t val = unpackVal(src+offset, gWidth[grNum], &shiftBit, &shiftByte);
      if (!fillMissing(gWidth[grNum], gRef[grNum], &val, cp, pd->bits() )) {
	dst[dNum++] = ref + (gRef[grNum] + val)*scale;
	// if (dst[dNum-1] > maxd) maxd = dst[dNum-1];
	// if (dst[dNum-1] < mind) mind = dst[dNum-1];
	//	printf("%d %f\n", dNum-1, dst[dNum-1]);
      } else {
	dst[dNum++] = val;
	//printf("%d %f\n", dNum-1, dst[dNum-1]);
      }
      if (dNum >= dstSize) break;
    }
  }

  // var(mind);
  // var(maxd);

  delete[] gRef;
  delete[] gWidth;
  delete[] gLength;

  return true;
}

// -
//template 3 (checked)
bool grib::ComplexSpDiffPoint(float* dst, uint32_t dstSize, uint8_t* src, uint32_t /*size*/, google::protobuf::Message* def)
{
  TRepresent::ComplexPoint* cp = static_cast<TRepresent::ComplexPoint*>(def);
  TRepresent::PointData* pd = cp->release_pd();

  //printvar(QString::fromStdString(cp->DebugString()));
  // printvar(QString::fromStdString(pd->DebugString()));

  int offset = 0;
  uint64_t extra[2];
  int64_t min;

  grib::char2dec(src + offset, cp->octets(), &(extra[0]), true);
  offset += cp->octets();
  if (cp->order() == 2) {
    grib::char2dec(src + offset, cp->octets(), &(extra[1]), true);
    offset += cp->octets();
  }
  grib::char2dec(src + offset, cp->octets(), &min, true);
  offset += cp->octets();

  //--
  float ref = pd->r() * pow(10, -pd->d());
  float scale = pow(2, pd->e()) * pow(10, -pd->d());

  // var(ref);
  // var(scale);

  if (cp->ng() == 0) {
    for (uint32_t i = 0; i < dstSize; i++) {
      dst[i] = ref;
    }
    return true;
  }

  uint64_t* gRef = new(std::nothrow) uint64_t[cp->ng()]; //group ref values
  uint64_t* gWidth = new(std::nothrow) uint64_t[cp->ng()]; //group widths
  uint64_t* gLength = new(std::nothrow) uint64_t[cp->ng()]; //group lengths
  bool* miss = new(std::nothrow) bool[dstSize];
  if (!gRef || !gWidth || !gLength || !miss) {
    return false;
  }

  grib::unpackBits(gRef, src + offset, pd->bits(), cp->ng());
  offset += (cp->ng()*pd->bits()+7)/8;
  grib::unpackBits(gWidth, src  + offset, cp->bits_w(), cp->ng(), cp->r_w());
  offset += (cp->ng()*cp->bits_w()+7)/8;
  grib::unpackBits(gLength, src + offset, cp->bits_l(), cp->ng()-1, cp->r_l(), cp->inc_l());
  offset += ((cp->ng())*cp->bits_l()+7)/8;
  gLength[cp->ng()-1] = cp->last_l();

  //TODO проверка размеров и индексов, которые пытаемся считать и записать
  //uint64_t total=0;
  // for (uint i=0;i<cp->ng();i++) {
  //   total += gLength[i];
  // }
  // var(total);


  uint8_t shiftBit=0; //уже считанных из байта
  uint32_t shiftByte = 0;
  uint32_t dNum=0;
  for (uint32_t grNum=0; grNum < cp->ng(); grNum++) {
    for (uint32_t i=0; i< gLength[grNum]; i++) {
      uint64_t val = unpackVal(src+offset, gWidth[grNum], &shiftBit, &shiftByte);
      //printf("%d %llu\n", dNum, val);
      if (!fillMissing(gWidth[grNum], gRef[grNum], &val, cp, pd->bits()  )) {
	dst[dNum] = val + gRef[grNum];;
	miss[dNum] = false;
	//	printf("%d %.0f\n", dNum, dst[dNum]);
      } else {
	miss[dNum] = true;
	dst[dNum] = val;
      }
      dNum++;
      if (dNum >= dstSize) break;
    }
  }

  // for (uint i=0;i<dNum; i++) {
  //   printf("%d %.0f\n", i, dst[i]);
  // }

  uint64_t idx = 0;
  while (miss[idx] == true) {
    idx++;
  };

  float penultimate=0, last = 0;
  if (cp->order() == 1) {
    dst[idx] = extra[0];
    last = dst[idx];
    for (uint32_t i = idx+1; i<dNum; i++) {
      if (miss[i] == false) {
	dst[i] += last + min;
	last = dst[i];
      }
    }
  } else if (cp->order() == 2) {
    dst[idx] = extra[0];
    penultimate = dst[idx];
    idx++;

    while (miss[idx] == true) {
      idx++;
    };
    dst[idx] = extra[1];
    last = dst[idx];
    idx++;

    for (uint32_t i = idx; i<dNum; i++) {
      if (miss[i] == false) {
	dst[i] += last + min + last - penultimate;
	penultimate = last;
	last = dst[i];
      }
    }
  }

  //float
  dNum=0;
  // float maxd =0;
  // float mind = 99999999.0;
  for (uint32_t grNum=0; grNum < cp->ng(); grNum++) {
    for (uint32_t i=0; i< gLength[grNum]; i++) {
      if (miss[dNum] == false) {
	dst[dNum] = ref + (dst[dNum])*scale;
	// if (dst[dNum] > maxd) maxd = dst[dNum];
	// if (dst[dNum] < mind) mind = dst[dNum];
	//	printf("%d %f\n", dNum, dst[dNum]);
      }
      dNum++;
    }
  }

  // var(mind);
  // var(maxd);

  delete[] miss;
  delete[] gRef;
  delete[] gWidth;
  delete[] gLength;

  return true;
}


//template 4
bool grib::IeeePoint(float* dst, uint32_t dstSize, uint8_t* src, uint32_t size, google::protobuf::Message* def)
{
  uint8_t prec = (static_cast<TRepresent::IEEEPoint*>(def))->prec();
  if (prec < 1 || prec > 3) {
    error_log << QObject::tr("Точность %1 не поддерживается").arg(prec);
    return false;
  }

  if (prec != 1) { //TODO доделать для 8 байт. 16?
    warning_log<<QObject::tr("Точность %1 будет потеряна").arg(prec);
  }
   
  prec <<= 2; //количество байт на число

  uint32_t cnt = size / prec;

  for (uint32_t num = 0; num < cnt; num++) {
    dst[num] = MnMath::ieee2double(src + num*prec);
    if (num >= dstSize) break;
  }

  return true;
}

// -
//template 40 (checked)
bool grib::JpegPoint(float* dst, uint32_t dstSize, uint8_t* src, uint32_t size, google::protobuf::Message* def)
{
  TRepresent::PointData* base = (static_cast<TRepresent::JpegPoint*>(def))->release_pd();
  if (!base) return false;

  bool ok = false;
  if (base->bits() == 0) {
    ok = grib::unpackBits(dst, dstSize, src, size*8 / base->bits(), base);  
  } else {

    opj_dparameters_t param;
    opj_image_t *image = 0;
    opj_codec_t *codec = 0;
    //opj_stream_t *stream = opj_stream_create_default_file_stream("img.j2k", OPJ_TRUE);

    opj_input_memory_stream l_mem_stream;
    l_mem_stream.dataSize = size;
    l_mem_stream.offset = 0U;
    l_mem_stream.pData = src;
    //   (OPJ_UINT8*)malloc(l_mem_stream.dataSize);
    // memcpy(l_mem_stream.pData, src, l_mem_stream.dataSize);
    
    opj_stream_t *stream = opj_stream_default_create(OPJ_TRUE);
    if (nullptr == stream) {
      return false;
    }
    
    opj_stream_set_read_function(stream, opj_input_memory_stream_read);
    opj_stream_set_seek_function(stream, opj_input_memory_stream_seek);
    opj_stream_set_skip_function(stream, opj_input_memory_stream_skip);
    opj_stream_set_user_data(stream, &l_mem_stream, opj_input_memory_stream_free);
    opj_stream_set_user_data_length(stream, l_mem_stream.dataSize);
        
    //opj_stream_set_user_data(stream, src, opj_stream_free_user_data_fn p_function);
    opj_set_default_decoder_parameters(&param);

    codec = opj_create_decompress(OPJ_CODEC_J2K);
    if (nullptr == codec) {
      opj_stream_destroy(stream);
      return false;
    }
    
    if (!opj_setup_decoder(codec, &param)) {
      opj_destroy_codec(codec);
      opj_stream_destroy(stream);
      return false;
    }
    
    opj_read_header(stream, codec, &image);
    if (nullptr == image) {
      opj_destroy_codec(codec);
      opj_stream_destroy(stream);
      return false;
    }

    if (! opj_decode(codec, stream, image)) {
      opj_image_destroy(image);
      opj_destroy_codec(codec);
      opj_stream_destroy(stream);
      return false;
    }

    if (image->numcomps != 1 || nullptr == image->comps) {
      opj_image_destroy(image);
      opj_destroy_codec(codec);
      opj_stream_destroy(stream);
      return false;
    }    

    int width = image->comps[0].w;
    int height = image->comps[0].h;

    int start = dstSize - height * width;
    float binscale = pow(2, base->e()) * pow(10, -base->d());
    float ref = base->r() * pow(10, -base->d());
    
    for (int i = 0; i < height*width; i++) {
      dst[start + i] = image->comps[0].data[i] * binscale + ref;
      //	if (i<5 && j<5) debug_log<<i<<j<<jas_data->rows_[i][j]<<dst[k+j+i*pcmpt->width_];
    }

    opj_end_decompress(codec, stream);

    opj_image_destroy(image);
    opj_destroy_codec(codec);
    opj_stream_destroy(stream);
    ok = true;
  }
  
  //---
  
  //libjasper
  // bool ok = false;

  // if (base->bits() == 0) {
  //   ok = grib::unpackBits(dst, dstSize, src, size*8 / base->bits(), base);  
  // } else {
  //   char *opts = 0;
  //   jas_stream_t* jpcstream=jas_stream_memopen((char *) src, size);
  //   jas_image_t* image = jpc_decode(jpcstream, opts);
  //   if (!image || image->numcmpts_ != 1) {
  //     error_log<<"Error jpeg image stuct";
  //     jas_image_destroy(image);
  //     jas_stream_close(jpcstream);
  //     return false;
  //   }
    
  //   int height = jas_image_height(image);
  //   int width = jas_image_width(image);
  //   jas_matrix_t* jas_data=jas_matrix_create(height, width);
  //   jas_image_readcmpt(image, 0, 0, 0, width, height, jas_data);
  //   //debug_log<<height<<width<<image->cmpts_[0]->height_<<image->cmpts_[0]->width_;
    
  //   int start = dstSize - height * width;
  //   float binscale = pow(2, base->e()) * pow(10, -base->d());
  //   float ref = base->r() * pow(10, -base->d());

  //   debug_log << height << width << start << binscale << ref;
    
  //   for (int i = 0; i < height; i++) {
  //     for (int j = 0; j < width; j++) {
  // 	dst[start + j + i*width] = jas_data->rows_[i][j] * binscale + ref;
  // 	//	if (i<5 && j<5) debug_log<<i<<j<<jas_data->rows_[i][j]<<dst[k+j+i*pcmpt->width_];
  //     }
  //   }

  //   jas_matrix_destroy(jas_data);
  //   jas_image_destroy(image);
  //   jas_stream_close(jpcstream);
  //   ok = true;
  // }
  
  return ok;
}

// -
//template 41
bool grib::PngPoint(float* dst, uint32_t dstSize, uint8_t* src, uint32_t size, google::protobuf::Message* def)
{
 TRepresent::PointData* base = (static_cast<TRepresent::SimplePoint*>(def))->release_pd();
  if (!base) return false;
  
  bool ok = false;

  if (base->bits() == 0) {
    ok = grib::unpackBits(dst, dstSize, src, size*8 / base->bits(), base);  
  } else {
    uint8_t* png = new(std::nothrow) uint8_t[dstSize * 4];
    if (!png) {
      return false;
    }
    png_uint_32 width, height;
    

    if (decodePng(src, &width, &height, png)) {
      ok = grib::unpackBits(dst, dstSize, png, size*8 / base->bits(), base);  
    }
    delete[] png;
  }

  return ok; 
}

// -
//template 50
bool grib::SimpleSpectral(float* dst, uint32_t dstSize, uint8_t* src, uint32_t size, google::protobuf::Message* def)
{
  TRepresent::Spectral* sp = static_cast<TRepresent::Spectral*>(def);
 TRepresent::PointData* base = sp->release_pd();
  if (!base) return false;
 
  bool ok = grib::unpackBits(dst, dstSize, src, size*8 / base->bits(), base);  
  if (!ok) return false;

  uint32_t cnt = size*8 / base->bits();  
  for (uint32_t i=0; i<cnt; i++) {
    dst[i] = dst[i] + sp->rp();
  }
  return ok;
}

// -
//template 51
bool grib::ComplexSpectral(float* dst, uint32_t dstSize, uint8_t* src, uint32_t size, google::protobuf::Message* def)
{
 TRepresent::PointData* base = (static_cast<TRepresent::Spectral*>(def))->release_pd();
  if (!base) return false;
 
  return grib::unpackBits(dst, dstSize, src, size*8 / base->bits(), base);  
}

//template 61
bool grib::SimpleLogPoint(float* dst, uint32_t dstSize, uint8_t* src, uint32_t size, google::protobuf::Message* def)
{
  TRepresent::SimplePoint* lp = static_cast<TRepresent::SimplePoint*>(def);
  TRepresent::PointData* base = lp->release_pd();
  if (!base) return false;
 
  //TODO можно ускорить, если сразу выделить константу из логарифма (exp(ref))  
  grib::unpackBits(dst, dstSize, src, size*8 / base->bits(), base);  
  uint32_t cnt = size*8 / base->bits();  
  for (uint32_t i=0; i<cnt; i++) {
    dst[i] = exp(dst[i]) - lp->ln_b();
  }

  return true;
}

//template 200
bool grib::LevelValues(float* dst, uint32_t dstSize, uint8_t* src, uint32_t size, google::protobuf::Message* def)
{
 TRepresent::LevelValues* lv = static_cast<TRepresent::LevelValues*>(def);
 
  uint32_t cnt = size*8 / lv->bits();
  double* levels = new double[lv->mvl()];

  for (uint32_t i = 0; i < lv->mvl(); i++) {
    levels[i] = lv->scale(i*2) * lv->d();
  }

  int range = (1 << lv->bits()) - 1 - lv->mv();
  if (range <= 0) {
    error_log<< QObject::tr("Ошибка формата данных");
    delete[] levels;
    return false;
  }

  uint64_t* vals = new uint64_t[cnt];
  unpackBits(vals, src,lv->bits(), cnt);

  uint32_t i=0, j=0, ncheck=0;
  uint64_t v=0;
  uint32_t n=0, factor=0;

  while (i < cnt) {
    if (vals[i] > lv->mv()) {
      error_log << QObject::tr("Ошибка формата данных");
      delete[] vals;
      delete[] levels;
      return false;
    }
    v = vals[i++];
    n = 1;
    factor = 1;
    while (vals[i] > lv->mv() && i < cnt) {
      n += factor * (vals[i]-lv->mv()-1);
      factor = factor * range;
      i++;
    }
    ncheck += n;
    // if (ncheck > npts) { //TODO размер из 3 раздела
    //  error_log;
    // }
    
    for (uint32_t k = 0; k < n; k++) {
      dst[j++] = levels[v];
      if (j >= dstSize) break;
    }
  }

  delete[] vals;
  delete[] levels;
  return true;
}


//-----------------------------------------------------------

bool grib::unpackBits(float* dst, uint32_t dstSize, uint8_t* src, uint32_t cnt, TRepresent::PointData* base)
{
  uint8_t shiftBit=0; //уже считанных из байта
  uint32_t shiftByte = 0;

  float binscale = pow(2, base->e()) * pow(10, -base->d());
  float ref = base->r() * pow(10, -base->d());

  // printvar(QString::fromStdString(base->DebugString()));
  // var(cnt);
  // var(dstSize);
  // printvar(ref);
  // printvar(binscale);

  uint64_t i=0;
  while (i < cnt) {
    if (i >= dstSize) {
      error_log << QObject::tr("Ошибка размера данных");
      break;
    }
    float val = unpackVal(src, base->bits(), &shiftBit, &shiftByte);
    dst[i++] = ref + val*binscale;
  }

  if (i != dstSize) {
    error_log << QObject::tr("Ожидалось %1 байт, заполнено %2 байт").arg(dstSize).arg(i);    
  }

  return true;
}

bool grib::unpackBits(uint64_t* dst, uint8_t* src, uint32_t bits, uint32_t cnt, uint32_t ref/* = 0*/, uint16_t scale /*= 1*/)
{
  //  debug_log<<bits<<cnt<<ref<<scale;
  
  uint8_t shiftBit=0; //уже считанных из байта
  uint32_t shiftByte = 0;

  uint64_t i=0;
  while (i < cnt) {
    uint64_t val = unpackVal(src, bits, &shiftBit, &shiftByte);
    dst[i++] = ref + val*scale;
  }

  return true;
}

uint64_t grib::unpackVal(uint8_t* src, uint32_t bits, uint8_t* shiftBit, uint32_t* shiftByte)
{
  const int mask[9] = {0x0, 0x1, 0x3, 0x7, 0xF, 0x1F, 0x3F, 0x7F, 0xFF};
  uint64_t val = 0;
  uint8_t bitsCnt = 0;
  while (bitsCnt < bits) {
    uint8_t byte = src[*shiftByte];
    if (*shiftBit > 8) {
      error_log << QObject::tr("Ошибка в коде");
      return false;
    }
    if (*shiftBit != 0) {
      byte &= mask[8-*shiftBit];
    }
    uint8_t curbits = 0; //кол-во бит, к-е будем считывать из этого байта
    if ((uint32_t)(bitsCnt + (8-*shiftBit)) > bits) {
      curbits = bits-bitsCnt;
      byte >>= 8 - *shiftBit  - curbits;
      *shiftBit += curbits;
    } else {
      curbits = 8 - *shiftBit;
      *shiftBit = 0;
      (*shiftByte)++;
    }
    bitsCnt += curbits;
    
    val <<= curbits;
    val += byte;
  }
  return val;
}

bool grib::fillMissing(uint32_t gWidth, uint32_t gRefs, uint64_t* val, const TRepresent::ComplexPoint* cp, uint16_t bits)
{
  if (cp->missing() == 0) {
    return false;
  } 

  uint64_t mask1;
  uint32_t missVal;
  if (gWidth == 0) {
    mask1 = (1 << bits) - 1;
    //debug_log<<bits;
    missVal = gRefs;
  } else {
    mask1 = (1 << gWidth) - 1;
    missVal = *val;
  }
  uint64_t mask2 = mask1 - 1;

  if (cp->missing() == 2 && missVal == mask2) {
    *val = cp->second();
    return true;
  }
  
  //  printf("mis=%u m=%llu gr=%u gw=%u val=%llu\n", missVal, mask1, gRefs, gWidth, *val);
  
  if (missVal == mask1) {
    *val = cp->prime();
    return true;
  }

  return false;
}


//------

//для чтения из буффера, а не файла
void grib::customPngRead(png_structp png_ptr,png_bytep data, png_size_t length)
{
  png_stream* mem=(png_stream *)png_get_io_ptr(png_ptr);

  memcpy(data, mem->stream_ptr + mem->stream_len, length);
  mem->stream_len += length;
}

bool grib::decodePng(unsigned char *pngbuf, png_uint_32 *width, png_uint_32 *height, uint8_t *dst)
{
  if ( png_sig_cmp(pngbuf, 0, 8) != 0) { //проверка, что тип PNG
    return false;
  }
  
  png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
  png_infop info = png_create_info_struct(png);
  png_infop infoEnd = png_create_info_struct(png);

  if (!png || !info || !infoEnd) {
    png_destroy_read_struct(&png, &info, &infoEnd);
    return false;
  }
  
  png_stream read_io_ptr;  
  read_io_ptr.stream_ptr=pngbuf;
  read_io_ptr.stream_len=0;

  png_set_read_fn(png, &read_io_ptr, customPngRead);
  png_read_png(png, info, PNG_TRANSFORM_IDENTITY, 0);

  png_bytepp row_pointers = png_get_rows(png, info); //указатель каждой строки

  int interlace,color,compres,filter,bit_depth;
  png_get_IHDR(png, info, width, height, &bit_depth, &color, &interlace, &compres, &filter);

  if ( color == PNG_COLOR_TYPE_RGB ) {
    bit_depth=24;
  } else if ( color == PNG_COLOR_TYPE_RGB_ALPHA ) {
    bit_depth=32;
  }
  
  png_uint_32 n = 0;
  png_uint_32 len = (bit_depth / 8) * (*width);
  for (png_uint_32 j=0; j< *height; j++) {
    for (png_uint_32 k=0; k<len; k++) {
      dst[n] = *(row_pointers[j]+k);
      n++;
    }
  }
  
  png_destroy_read_struct(&png, &info, &infoEnd);
  return true;
}


//----

static OPJ_SIZE_T opj_input_memory_stream_read(void * p_buffer, OPJ_SIZE_T p_nb_bytes, void * p_user_data)
{
  opj_input_memory_stream* l_stream = (opj_input_memory_stream*)p_user_data;
  OPJ_SIZE_T l_nb_bytes_read = p_nb_bytes;

  if (l_stream->offset >= l_stream->dataSize) {
    return (OPJ_SIZE_T)-1;
  }

  if (p_nb_bytes > (l_stream->dataSize - l_stream->offset)) {
    l_nb_bytes_read = l_stream->dataSize - l_stream->offset;
  }

  memcpy(p_buffer, &(l_stream->pData[l_stream->offset]), l_nb_bytes_read);

  l_stream->offset += l_nb_bytes_read;

  return l_nb_bytes_read;
}

static OPJ_OFF_T opj_input_memory_stream_skip(OPJ_OFF_T p_nb_bytes, void * p_user_data)
{
  opj_input_memory_stream* l_stream = (opj_input_memory_stream*)p_user_data;
  
  if (p_nb_bytes < 0) {
    return -1;
  }

  l_stream->offset += (OPJ_SIZE_T)p_nb_bytes;
  
  return p_nb_bytes;
}

static OPJ_BOOL opj_input_memory_stream_seek(OPJ_OFF_T p_nb_bytes, void * p_user_data)
{
  opj_input_memory_stream* l_stream = (opj_input_memory_stream*)p_user_data;

  if (p_nb_bytes < 0) {
    return OPJ_FALSE;
  }
  
  l_stream->offset = (OPJ_SIZE_T)p_nb_bytes;
  
  return OPJ_TRUE;
}

static void opj_input_memory_stream_free (void * p_user_data)
{
  opj_input_memory_stream* l_stream = (opj_input_memory_stream*)p_user_data;
  
  if ((l_stream != NULL) && (l_stream->pData != NULL)) {
    //free(l_stream->pData);
    l_stream->pData = NULL;
    l_stream->dataSize = 0;
  }
}

