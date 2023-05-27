#include "tfieldmongo.h"

#include <qbuffer.h>
#include <qbytearray.h>
#include <qmath.h>
#include <qelapsedtimer.h>

#include <google/protobuf/io/coded_stream.h>

#include <cross-commons/debug/tlog.h>

#include "interpolorder.h"
#include "func_obanal.h"
#include "tfieldparams.h"

namespace obanal{

  
TFieldMongo::TFieldMongo()
  :TField()
{
}

TFieldMongo::TFieldMongo(const TField& f)
  :TField(f)
{
}

TFieldMongo::~TFieldMongo()
{

  clear();
}

bool TFieldMongo::ensureDataLoaded(int idx) const
{
  if ( false == useGridFile_ ) { return true; }

  int num = qFloor((sizeof(float)*idx + dataOffset_) / double(kPageSize));
  return ensurePageLoaded(num);
}

bool TFieldMongo::ensureDataLoaded(int i0, int i1, int i2, int i3) const
{
  return ensureDataLoaded(i0) && ensureDataLoaded(i1) && ensureDataLoaded(i2) && ensureDataLoaded(i3);
}

bool TFieldMongo::ensureNetMaskLoaded(int idx) const
{
  if ( false == useGridFile_ ) { return true; }

  int num = qFloor((sizeof(bool)*idx + netMaskOffset_) / double(kPageSize));
  return ensurePageLoaded(num);
}

bool TFieldMongo::ensureNetMaskLoaded(int i0, int i1, int i2, int i3) const
{
  return ensureNetMaskLoaded(i0) && ensureNetMaskLoaded(i1) && ensureNetMaskLoaded(i2) && ensureNetMaskLoaded(i3);
}

bool TFieldMongo::ensureDataMaskLoaded(int idx) const
{
  if ( false == useGridFile_ ) { return true; }

  int num = qFloor((sizeof(bool)*idx + dataMaskOffset_) / double(kPageSize));
  return ensurePageLoaded(num);
}

bool TFieldMongo::ensureDataMaskLoaded(int i0, int i1, int i2, int i3) const
{
  return ensureDataMaskLoaded(i0) && ensureDataMaskLoaded(i1) && ensureDataMaskLoaded(i2) && ensureDataMaskLoaded(i3);
}

bool TFieldMongo::ensurePageLoaded(int n) const
{
  if ( false == useGridFile_ ) { return true; }

  if ( n >= pageLoaded_.size() || n < 0 ) { return false; }

  if ( true == pageLoaded_.testBit(n) ) { return true; }

  int offset = n*kPageSize;

  bool ok = false;
  if ( !gridFile_.seek(offset) ) {
    error_log << gridFile_.lastError();
    return false;
  }

  QByteArray ba = gridFile_.read(kPageSize, &ok);
  if ( !ok ) {
    error_log << gridFile_.lastError();
    return false;
  }
  pageLoaded_.setBit(n);

  int dataBeg[3];
  int dataEnd[3];

  quint8* dstData[3];

  const int pageBeg = offset;
  const int pageEnd = offset + ba.size();

  // data
  dstData[0] = (quint8*)pdata_;
  dataBeg[0] = dataOffset_;
  dataEnd[0] = dataOffset_ + data_.size()*sizeof(float);

  // data mask
  dstData[1] = (quint8*)pdatamask_;
  dataBeg[1] = dataMaskOffset_;
  dataEnd[1] = dataMaskOffset_ + datamask_.size()*sizeof(bool);

  // net mask
  dstData[2] = (quint8*)pnetmask_;
  dataBeg[2] = netMaskOffset_;
  dataEnd[2] = netMaskOffset_ + netmask_.size()*sizeof(bool);

  for ( int i = 0; i < 3; ++i ) {
    int sz = qMin(pageEnd,dataEnd[i]) - qMax(pageBeg,dataBeg[i]);
    if ( sz <= 0 ) {
      continue;
    }

    int pageShift = 0;
    int maskShift = 0;
    if ( pageBeg < dataBeg[i] ) {
      pageShift = dataBeg[i] - pageBeg;
    }
    else {
      maskShift = pageBeg - dataBeg[i];
    }
    void* dst = dstData[i] + maskShift;
    const void* src = ba.constData() + pageShift;
    memcpy(dst, src, sz);
  }

  return true;
}

float TFieldMongo::pointValueF( const meteo::GeoPoint& point, bool* ok ) const
{
  if ( nullptr != ok ) {
    *ok = false;
  }
  if ( 4 > size_data_ || 2 > size_la_ || 2 > size_fi_ ) {
    return 0.0;
  }

  int i,j;
  float dfi, dla, dx, dy, vs_1, vs_2;
  float f, l;

  float calcvalue = 0;

  f = point.lat();
  l = point.lon();

  if ( !(net_fi_.first() <= f && f <= net_fi_.last()) ) {
    return 0.0;
  }

  dfi=pnet_fi_[1]-pnet_fi_[0];
  dla=pnet_la_[1]-pnet_la_[0];

  int size_fi = net_fi_.size();
  int size_la = net_la_.size();

  if(GENERAL_NET != typeNet()){
    i=int((f-pnet_fi_[0])/dfi)%(size_fi - 1);
    j=int((l-pnet_la_[0])/dla)%(size_la - 1);
  }
  else {
    i=int((f-pnet_fi_[0])/dfi);
    j=int((l-pnet_la_[0])/dla);
    if(i < 0 || j < 0 ) {
      return 0.0;
    }
    if(i >= kolFi()-1 || j >= kolLa()-1 ) {
      return 0.0;
    }
  }

  if ( i < 0 || j < 0 ) {
    return 0.0;
  }

  dx=(f-pnet_fi_[i])/dfi;
  dy=(l-pnet_la_[j])/dla;

  int i00 = i*size_la+j;
  int i01 = i*size_la+j+1;
  int i10 = (i+1)*size_la+j;
  int i11 = (i+1)*size_la+j+1;

  if ( !ensureDataMaskLoaded(i00,i01,i10,i11) ) {
    return false;
  }
  if ( !ensureNetMaskLoaded(i00,i01,i10,i11) ) {
    return false;
  }

  if ( pdatamask_[i00] && pdatamask_[i01] && pdatamask_[i10] && pdatamask_[i11]
       && pnetmask_[i00] && pnetmask_[i01] && pnetmask_[i10] && pnetmask_[i11]
       )
  {
    if ( !ensureDataLoaded(i00,i01,i10,i11) ) {
      return false;
    }

    vs_1=(1.f-dy)*pdata_[i00]+dy*pdata_[i01];
    vs_2=(1.f-dy)*pdata_[i10]+dy*pdata_[i11];
    calcvalue = (1.f-dx)*vs_1+dx*vs_2;
    if ( nullptr != ok ) {
      *ok = true;
    }
  }

  if ( calcvalue < min_val_ ) {
    return min_val_;
  }
  else if ( calcvalue > max_val_ ) {
    return max_val_;
  }

  return calcvalue;
}

bool TFieldMongo::isValidSheme(int i, int j)const{
  int anum[13];
  anum[0 ] = num(i,  j);
  anum[1 ] = num(i,  j+1);
  anum[2 ] = num(i-1,j);
  anum[3 ] = num(i,  j-1);
  anum[4 ] = num(i+1,j);
  anum[5 ] = num(i-1,j+1);
  anum[6 ] = num(i-1,j-1);
  anum[7 ] = num(i+1,j-1);
  anum[8 ] = num(i+1,j+1);
  anum[9 ] = num(i,  j+2);
  anum[10] = num(i-2,j);
  anum[11] = num(i,  j-2);
  anum[12] = num(i+2,j);
  for(int ii = 0; ii< 13; ++ii){
      int k = anum[ii];
      if ( !ensureDataMaskLoaded(k) ) {
        return false;
      }
      if ( !ensureNetMaskLoaded(k) ) {
        return false;
      }
      if ( !ensureDataLoaded(k) ) {
        return false;
      }
      if( 0 > k || k > size_data_-1
          || !pdatamask_[k] || !pnetmask_[k]){
          return false;
        }
    }
  return true;
}

bool TFieldMongo::radKriv(int i, int j, float *rad)const{

  if(!isValidSheme(i,j)) return false;

  float rz_1 = 6371.;

  float dfi_1 = 1.f/dfi_;
  float dla_1 = 1.f/dla_;
  float dpx = dPx(i,j,dla_1);
  float dpy = dPy(i,j,dfi_1);
  float pp =(dpx*dpx+dpy*dpy);
  float q = sqrtf( pp*pp*pp );
  float dpxx = dPxx(i,j,dla_1);
  float dpyy = dPyy(i,j,dfi_1);
  float dpxy = dPxy(i,j,dfi_1,dla_1);

  float h1 = dpxx*dpy*dpy;
  float h2 = 2.f*dpxy*dpx*dpy;
  float h3 = dpyy*dpx*dpx;
  *rad =rz_1*q/(h1-h2+h3);

  return true;
}



bool TFieldMongo::fromGridFile(const meteo::GridFile& file)
{

  clear();




  int64_t sz = file.size();
  if ( -1 == sz ) {
    error_log << file.lastError();
    return false;
  }

  useGridFile_ = true;
  gridFile_ = file;

  gridPages_ = qCeil(double(sz) / kPageSize);
  pageLoaded_ = QBitArray(gridPages_);

  if ( !gridFile_.seek(0) ) {
    error_log << gridFile_.lastError();
    return false;
  }

  bool ok = false;
  QByteArray ba = gridFile_.read(1024, &ok);
  if ( !ok ) {
    error_log << gridFile_.lastError();
    return false;
  }

  QBuffer s(&ba);
  s.open(QBuffer::ReadOnly);

  int offset = 0;
  if ( !loadHeader(&s, &offset) ) {
    return false;
  }

  pdata_ = data_.data();
  pdatamask_ = datamask_.data();
  pnetmask_ = netmask_.data();

  inMemory_ = false;

  return true;
}



}
