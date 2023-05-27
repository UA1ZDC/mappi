#include "tfield.h"

#include <qbuffer.h>
#include <qbytearray.h>
#include <qmath.h>

#include <google/protobuf/io/coded_stream.h>

#include <cross-commons/debug/tlog.h>

#include "interpolorder.h"
#include "func_obanal.h"
#include "tisolinedata.h"
#include "tfieldparams.h"

namespace obanal{


TField::TField()
{
  unit_ = QObject::tr("м.");
  name_ = QObject::tr("высота");
  min_val_ = -1e10;
  max_val_ = 1e10;
  step_val_ = 1.0;
  meteodescr_ = 0;
  net_type_ = STEP_0500x0500;
  dfi_ = 1.;
  dla_ =1.;
  size_fi_ = 0;
  size_la_ = 0;

  level_= 0;
  level_type_=1;
  hour_=0;
  model_=0;
  center_=0;
  center_name_="";
  level_type_name_="";
  smootch_stepen_ = 0;
  db_id_ =-1;

  size_data_ = net_fi_.size()*net_la_.size();

  resetPointers();
}

TField::TField(const TField& f)
{
  copyFrom(f);
}

TField::~TField()
{
  clear();
}

void TField::resetPointers()
{
  pnet_fi_ = net_fi_.data();
  pnet_la_ = net_la_.data();
  pnetmask_ = netmask_.data();
  pdatamask_ = datamask_.data();
  pdata_ = data_.data();
}

bool TField::loadHeader(QIODevice* dev, int* dataOffset)
{
  quint32 headerSize = 0;

  uint rsz = dev->read(reinterpret_cast<char*>(&headerSize),sizeof(headerSize));
  if ( rsz < sizeof(headerSize) ) {
      var(dev->size());
      return false;
    }

  QByteArray ba(headerSize,'\0');

  rsz = dev->read(ba.data(), headerSize);
  if ( rsz < headerSize ) {
      var(rsz) << var(dev->size()) << var(headerSize);
      return false;
    }

  if ( nullptr != dataOffset ) {
      *dataOffset = sizeof(headerSize) + headerSize;
    }

  proto::TFieldHeader header;
  if ( false == header.ParseFromArray(ba.data(), headerSize) ) {
    error_log << QObject::tr("Не удается разобрать заголовок поля.");
    return false;
  }

  src_point_count_ = header.point_count();
  net_type_ = static_cast<NetType>(header.net_type());

  reg_param_.start = meteo::GeoPoint::fromData(header.region_start_point().c_str());
  reg_param_.end = meteo::GeoPoint::fromData(header.region_end_point().c_str());

  if ( !reg_param_.start.isValid() || !reg_param_.end.isValid() ) {
      return false;
    }

  float fi0 = header.start_fi();
  float la0 = header.start_la();
  float fi1 = header.end_fi();
  float la1 = header.end_la();;
  float stepfi = header.step_fi();
  float stepla = header.step_la();

  if ( GENERAL_NET == net_type_ ) {
    if ( !setDecartNet(fi0,la0,fi1,la1,stepfi,stepla) ) {
      return false;
    }
  }
  else {
    setNet(reg_param_, stepfi, stepla);
  }

  meteodescr_ = header.meteodescr();
  if ( 5510004 == meteodescr_ ) { //!FIXME с какого-то перепугу дескриптор давления на тропопаузе = 5510004
    meteodescr_ = 10051;
  }

  level_ = header.level();
  level_type_ = header.level_type();

  if ( header.has_level_type_name() ) {
    level_type_name_ = QString::fromStdString(header.level_type_name());
  }

  min_val_ = header.min_value();
  max_val_ = header.max_value();

  setValueType(meteodescr_, level_, level_type_);

  if ( true == header.has_date() ) {
    date_ = QDateTime::fromString(QString::fromStdString(header.date()), Qt::ISODate);
  }

  hour_ = header.hour();
  model_ = header.model();
  center_ = header.center();

  if ( header.has_center_name() ) {
    center_name_ = QString::fromStdString(header.center_name());
  }

  dfi_ = pnet_fi_[1] - pnet_fi_[0];
  dla_ = pnet_la_[1] - pnet_la_[0];
  size_fi_ = net_fi_.size();
  size_la_ = net_la_.size();
  size_data_ = net_fi_.size()*net_la_.size();

  data_.resize(size_data_);
  datamask_.resize(size_data_);
  netmask_.resize(size_data_);

  dataOffset_ = sizeof(headerSize) + headerSize;
  dataMaskOffset_ = dataOffset_ + size_data_*sizeof(float);
  netMaskOffset_ = dataMaskOffset_ + size_data_*sizeof(bool);

  return true;
}

proto::TFieldHeader TField::header() const
{
  QByteArray ba;

  proto::TFieldHeader header;
  header.set_point_count(src_point_count_);
  header.set_net_type(net_type_);

  reg_param_.start.getData(&ba);
  header.set_region_start_point(ba.data(),ba.size());
  reg_param_.end.getData(&ba);
  header.set_region_end_point(ba.data(),ba.size());

  header.set_start_fi(getFi(0));
  header.set_start_la(getLa(0));
  header.set_end_fi(getFi(kolFi() - 1));
  header.set_end_la(getLa(kolLa() - 1));
  float vs = 0;
  if ( stepFi(&vs) ) { header.set_step_fi(vs); }
  if ( stepLa(&vs) ) { header.set_step_la(vs); }

  header.set_meteodescr(meteodescr_);

  header.set_date(date_.toString(Qt::ISODate).toStdString());
  header.set_hour(hour_);
  header.set_level(level_);
  header.set_level_type(level_type_);

  if ( !level_type_name_.isEmpty() ) {
      header.set_level_type_name(level_type_name_.toStdString());
    }

  header.set_model(model_);
  header.set_center(center_);

  if ( !center_name_.isEmpty() ) {
      header.set_center_name(center_name_.toStdString());
    }

  float vn =  3.40282347e+38F;
  float vx = -3.40282347e+38F;
  for ( int i = 0, ds = data_.size(); i < ds; ++i ) {
      vn = qMin(pdata_[i], vn);
      vx = qMax(pdata_[i], vx);
    }

  header.set_min_value(vn);
  header.set_max_value(vx);

  return header;
}

bool TField::loadToMemory()
{
  if ( inMemory_ ) { return true; }

  inMemory_ = true;

  if ( nullptr != dataArr_ ) {
      dataFile_.unmap(dataArr_);
      dataFile_.close();
      dataArr_ = nullptr;
    }

  resetPointers();

  // если файл не задан, поле ещё не создано
  if ( dataFile_.fileName().isEmpty() ) {
      return true;
    }

  if ( !dataFile_.open(QFile::ReadOnly) ) {
      return false;
    }

  QByteArray ba = dataFile_.readAll();
  return fromBuffer(&ba);
}

void TField::clear()
{
  if ( nullptr != dataArr_ ) {
      dataFile_.unmap(dataArr_);
      dataFile_.close();
      dataArr_ = nullptr;
    }
  inMemory_ = false;
  iso_.clear();
  data_.clear();
  net_fi_.clear();
  net_la_.clear();
  resetPointers();
}

int TField::cycle_num(int i, int j) const
{
  while(i > size_fi_){
      i = i - size_fi_;
    }
  while(j > size_la_){
      j = j - size_la_;
    }
  return i * size_la_ + j;
}

int TField::getIsolines(QVector<meteo::GeoVector> *isolines, float val)
{
  if(iso_.contains(val)){
      *isolines = iso_.value(val);
      return isolines->count();
    }
  TIsoLineData iso(this);
  iso.makeOne(val, isolines );
  if(0 < isolines->count() ) {
      iso_.insert(val,*isolines);
    }
  return isolines->count();
}


//#pragma GCC diagnostic push
//#pragma GCC diagnostic ignored "-Wmisleading-indentation"
int TField::smootchField(float  stepen)
{
  if ( !loadToMemory() ) { return -1; }

  int ret_val=0;
  int i_nach=0, i_kon, j;

  if( MnMath::isZero(stepen) ) return 0;

  for(;i_nach<kolFi();i_nach++)
    for(j=0;j<kolLa();j++) if(getMask(i_nach,j)) goto m1;
m1:

  for(i_kon=kolFi()-1;-1<i_kon;i_kon--)
    for(j=0;j<kolLa();j++) if(getMask(i_kon,j)) goto m2;
m2:
  if((i_kon-i_nach)<4) return ret_val;

  if(GENERAL_NET == net_type_){
      ret_val= smootchY(&data_,net_fi_,kolLa(),i_nach,i_kon+1,stepen);
      if(ret_val!=0) return ret_val;

      ret_val= smootchX(&data_,net_fi_,kolLa(),i_nach,i_kon+1,stepen);
      if(ret_val!=0) return ret_val;
    } else {
      ret_val= smootchLong(&data_,net_fi_,kolLa(),i_nach,i_kon+1,stepen);
      if(ret_val!=0) return ret_val;

      ret_val= smootchLat(&data_,net_fi_,kolLa(),i_nach,i_kon+1,stepen);
      if(ret_val!=0) return ret_val;
    }
  pdata_ = data_.data();
  smootch_stepen_ = stepen;
  return ret_val;
}

//#pragma GCC diagnostic pop

bool TField::geostrofWind(int i, int j, float *u,float *v) const{
  float dfi_1 = 1/dfi_;
  float dla_1 = 1/dla_;

  float grad_fi = dPx(i,j,dla_1)/6.371e6f;
  float grad_la = dPy(i,j,dfi_1)/6.371e6f;

  // float Rz=MnMath::EARTH_RAD/*6.371e6*/;
  float G=9.80665;
  float w=1.4584e-4f;
  float f = netFi(i);
  float teta = f +M_PI_2f;
  float ll=w*cos(teta);
  //if (fabs(ll) <LITTLE_VALUE) return false;
  if (MnMath::isZero(ll)) return false;
  float b=G/(ll);
  // l=MnMath::fmod2p(l);
  float koef =10.;

  *v = b*grad_fi*koef;
  *u = -b*grad_la*koef;
  return true;
}
//радиус кривизны


bool TField::gradient(int i, int j, float *grad_fi,float *grad_la) const
{

  float dHf, dHl;

  if(data_.isEmpty()) return false;

  int i1 = i+1;
  int j4 = j+1;
  if(i == kolFi()-1|| i == 0){
      return false;
    }
  int i4 = i;
  if(j == kolLa()-1){
      j4 = 1;
    }
  int j1 = j;

  if(!(getMask(i,j)&&getMask(i1,j1)&&
       getMask(i4,j4))) {
      return false;
    }

  float df=1.f/(getFi(1)-getFi(0));
  float dl=1.f/(getLa(1)-getLa(0));

  float sinT   = sinf(getFi(i)+M_PI_2f);
  if(MnMath::isZero(sinT)){
      //   debug_log<< "return false"<<i<<j<<i1<<j1<<i4<<j4;
      return false;
    }

  float vs_fi =  getData(i1, j1) - getData(i, j);// fi
  float vs_la = getData(i4, j4) - getData(i, j);//la
  //sinT=1.;
  dHf = df*(vs_fi)/(6.371e6f);
  dHl = dl*(vs_la)/(6.371e6f*sinT);

  *grad_fi=dHf;
  *grad_la=dHl;
  return true;
}

bool TField::gradient2(int i, int j, float *grad_fi,float *grad_la) const
{

  float dHf, dHl;

  if(data_.isEmpty()) return false;


  int i1 = i+1;
  int j4 = j+1;
  if ( i == kolFi() - 1 ){
    i1 = i-1;
    float l =getLa(j)+M_PIf;
    float dla;
    if(!stepLa(&dla)){
      //debug_log<< "return false";
      return false;
    }
    j4 = MnMath::ftoi_norm((l - pnet_la_[0])/dla)%( size_la_ - 1);
    if ( j4 < 0 ) {
      //debug_log<< "return false";
      return false;
    }
  }
  int i4 = i;
  if(j == kolLa()-1){
      j4 = 0;
      //  debug_log<< getLa(j)*180/3.14<<getLa(j4)*180/3.14;
    }
  int j1 = j;

  if(!(getMask(i,j)&&getMask(i1,j1)&&
       getMask(i4,j4))) return false;

  float df=1/(getFi(1)-getFi(0));
  float dl=1/(getLa(1)-getLa(0));

  float sinT   = sinf(getFi(i)+M_PI_2f);
  if(MnMath::isZero(sinT)){
      return false;
  }

  float agrad_fi, agrad_la;
  if(!gradient(i1,j1, &agrad_fi,&agrad_la)){
    return false;
  }
  float vs_fi = sqrt(agrad_fi*agrad_fi+agrad_la*agrad_la); //модуль градиента в соседней точке по широте


  if(!gradient(i4,j4, &agrad_fi,&agrad_la)){
    return false;
  }
  float vs_la = sqrt(agrad_fi*agrad_fi+agrad_la*agrad_la);  //модуль градиента в соседней точке по долготе
  
  if(!gradient(i,j, &agrad_fi,&agrad_la)){
    return false;
  }
  float vs = sqrt(agrad_fi*agrad_fi+agrad_la*agrad_la);  //модуль градиента в текущей точке

  dHf = -(vs_fi - vs) * df / (6.371e6f);
  dHl = -(vs_la - vs) * dl /(6.371e6f*sinT);

  *grad_fi=dHf;
  *grad_la=dHl;
  return true;
}

//градиент при расстоянии до следующей точки равном step км
bool TField::gradientKm(int ii, int jj, float step, float *grad_fi,float *grad_la) const
{
  float dHf, dHl;
  
  if(data_.isEmpty()) return false;
  if(!getMask(ii, jj)) return false;

  float fi = getFi(ii);
  float la = getLa(jj);

  //TODO проверить km to grad
  float dfi = MnMath::deg2rad(step / (40008.55 / 360));
  float dla = MnMath::deg2rad(step / (111.3 * cos(fi)));

 
  float next_fi = fi + dfi;
  float next_la = MnMath::PiToPi(la + dla);
  if (next_fi > M_PI_2f || next_fi < -M_PI_2f) {
    return false;
  }

  // debug_log << "coord" << "fi=" << fi*180/3.14 << "la=" << la*180/3.14 << next_fi*180/3.14 << next_la*180/3.14 << "step" << dfi*180/3.14 << dla*180/3.14;
  
  bool ok = false;
  float nfi_val = pointValue(meteo::GeoPoint(next_fi, la), &ok);
  if (!ok) return false;
  float nla_val = pointValue(meteo::GeoPoint(fi, next_la), &ok);
  if (!ok) return false;
  
  float sinT   = sinf(fi + M_PI_2f);
  if(MnMath::isZero(sinT)) {
    //   debug_log<< "return false"<<i<<j<<i1<<j1<<i4<<j4;
    return false;
  }

  float vs_fi = nfi_val - getData(ii, jj);// fi
  float vs_la = nla_val - getData(ii, jj);//la
  //sinT=1.;
  // dHf = (vs_fi)/(dfi * 6.371e6f);
  // dHl = (vs_la)/(dla * 6.371e6f * sinT);

  dHf = (vs_fi) / step / 1000; //в м
  dHl = (vs_la) / step / 1000; //в м

  
  *grad_fi = dHf;
  *grad_la = dHl;
  return true;
}

//градиент при расстоянии до следующей точки (fi, la) равном step км
bool TField::gradientKmByCoord(float fi, float la, float step, float *grad_fi,float *grad_la) const
{
  float dHf, dHl;
  
  if(data_.isEmpty()) return false;
  
  //TODO проверить km to grad
  float dfi = MnMath::deg2rad(step / (40008.55 / 360));
  float dla = MnMath::deg2rad(step / (111.3 * cos(fi)));
  
  float next_fi = fi + dfi;
  float next_la = MnMath::PiToPi(la + dla);
  if (next_fi > M_PI_2f || next_fi < -M_PI_2f) {
    return false;
  }

  bool ok = false;
  float nfi_val = pointValue(meteo::GeoPoint(next_fi, la), &ok);
  if (!ok) return false;
  float nla_val = pointValue(meteo::GeoPoint(fi, next_la), &ok);
  if (!ok) return false;
  
  float sinT   = sinf(fi + M_PI_2f);
  if(MnMath::isZero(sinT)) {
    //   debug_log<< "return false"<<i<<j<<i1<<j1<<i4<<j4;
    return false;
  }

  float value = pointValue(meteo::GeoPoint(fi, la), &ok);
  float vs_fi = nfi_val - value;// fi
  float vs_la = nla_val - value;//la
  //sinT=1.;
  // dHf = (vs_fi)/(dfi * 6.371e6f);
  // dHl = (vs_la)/(dla * 6.371e6f * sinT);

  dHf = (vs_fi) / step / 1000; //в м 
  dHl = (vs_la) / step / 1000; //в м 
  
  *grad_fi = dHf;
  *grad_la = dHl;
  return true;
}

//градиент градиента при расстоянии до следующей точки равном step км
bool TField::gradient2Km(int ii, int jj, float step, float *grad_fi,float *grad_la) const
{

  float dHf, dHl;

  if(data_.isEmpty()) return false;
  if(!getMask(ii, jj)) return false;
  
  float fi = getFi(ii);
  float la = getLa(jj);

  //TODO проверить km to grad
  float dfi = MnMath::deg2rad(step / (40008.55 / 360));
  float dla = MnMath::deg2rad(step / (111.3 * cos(fi)));
  
  float next_fi = fi + dfi;
  float next_la = MnMath::PiToPi(la + dla);
  if (next_fi > M_PI_2f || next_fi < -M_PI_2f) {
    return false;
  }
  
  float sinT   = sinf(getFi(ii)+M_PI_2f);
  if(MnMath::isZero(sinT)){
      return false;
  }

  float agrad_fi, agrad_la;
  if(!gradientKmByCoord(next_fi, la, step, &agrad_fi, &agrad_la)) {
    return false;
  }
  float vs_fi = sqrt(agrad_fi*agrad_fi+agrad_la*agrad_la); //модуль градиента в соседней точке по широте


  if(!gradientKmByCoord(fi, next_la, step, &agrad_fi, &agrad_la)) {
    return false;
  }
  float vs_la = sqrt(agrad_fi*agrad_fi+agrad_la*agrad_la);  //модуль градиента в соседней точке по долготе
  
  if(!gradientKm(ii,jj, step, &agrad_fi,&agrad_la)) {
    return false;
  }
  float vs = sqrt(agrad_fi*agrad_fi+agrad_la*agrad_la);  //модуль градиента в текущей точке

  // dHf = -(vs_fi - vs) / (dfi * 6.371e6f);
  // dHl = -(vs_la - vs) / (dla * 6.371e6f * sinT);

  dHf = -(vs_fi - vs) / step / 1000; //в м
  dHl = -(vs_la - vs) / step / 1000; //в м


  *grad_fi = dHf;
  *grad_la = dHl;
  
  return true;
}

// bool TField::gradient2(int i, int j, float *grad_fi,float *grad_la) const
// {

//   float dHf, dHl;

//   if(data_.isEmpty()) return false;


//   int i1 = i+1;
//   int j4 = j+1;
//   if ( i == kolFi() - 1 ){
//     i1 = i-1;
//     float l =getLa(j)+M_PIf;
//     float dla;
//     if(!stepLa(&dla)){
//       //debug_log<< "return false";
//       return false;
//     }
//     j4 = MnMath::ftoi_norm((l - pnet_la_[0])/dla)%( size_la_ - 1);
//     if ( j4 < 0 ) {
//       //debug_log<< "return false";
//       return false;
//     }
//   }
//   int i4 = i;
//   if(j == kolLa()-1){
//       j4 = 0;
//       //  debug_log<< getLa(j)*180/3.14<<getLa(j4)*180/3.14;
//     }
//   int j1 = j;

//   if(!(getMask(i,j)&&getMask(i1,j1)&&
//        getMask(i4,j4))) return false;

//   float df=1/(getFi(1)-getFi(0));
//   float dl=1/(getLa(1)-getLa(0));

//   float sinT   = sinf(getFi(i)+M_PI_2f);
//   if(MnMath::isZero(sinT)){
//       return false;
//     }

//   float agrad_fi, agrad_la;
//   if(!gradient(i1,j1, &agrad_fi,&agrad_la)){
//       return false;
//     }
//   float vs_fi = sqrt(agrad_fi*agrad_fi+agrad_la*agrad_la);
//   if(!gradient(i,j, &agrad_fi,&agrad_la)){
//       return false;
//     }
//   float vs_la = sqrt(agrad_fi*agrad_fi+agrad_la*agrad_la);
//   vs_fi -= vs_la;
//   dHf=df*(vs_fi)/(6.371e6f);

//   if(!gradient(i4,j4, &agrad_fi,&agrad_la)){
//       return false;
//     }
//   vs_la = sqrt(agrad_fi*agrad_fi+agrad_la*agrad_la) - vs_la;
//   dHl=dl*(vs_la)/(6.371e6f*sinT);

//   *grad_fi=dHf;
//   *grad_la=dHl;
//   return true;
// }


bool TField::laplainPoint(const meteo::GeoPoint &geo, float *alapla) const
{
  int i,j;
  if(data_.isEmpty()) return false;
  if(!getNumPoFila(geo,&i,&j)) return false;
  return lapla( i, j, alapla);
}

//skelet0 - скелет (координаты точек) объекта
//vskelet последовательность скелетов
int TField::prognTrackObj(const meteo::GeoVector& skelet0,
                          QVector<meteo::GeoVector>* vskelet,
int time, int step, int* real_time, float koef) const
{
  int result = ERR_NOERR;
  meteo::GeoVector g;
  foreach (meteo::GeoPoint g0, skelet0) {
      g.clear();
      result = prognTrack(g0,&g, time, step, real_time, koef);
      int csize = g.size();
      if(0 < csize){
          vskelet->append(g);
        }
      if(ERR_NOERR != result){
          break;
        }
    }
 return result;
}

//координаты точки адвекции

int TField::prognTrack(const meteo::GeoPoint &g0,meteo::GeoVector *g,
                       int time, int step, int *real_time, float koef) const
{
  int ret_val=ERR_NOERR;
  if(!g0.isValid() || 1 > time || 1> step)
    return DATA_PERENOS_ERROR;
  *real_time=0;
  //  meteo::GeoVector::const_iterator it;
  meteo::GeoPoint vs_gp;
  meteo::GeoVector vs_gv;
  int step_mal = 60;
  int step_vs = 0;
  int i=0;

  vs_gp = g0;
  for ( i=0;i<time;i+=step) {

      if ( vs_gp.lat() > -kPR_DOPUSK && vs_gp.lat() < kPR_DOPUSK){
          *real_time=i;
          return GEOSTROF_ERROR;
        }
      if((i+step) > time ){
          step =  time-i;
        }
      for (step_vs =0; step_vs <= step; step_vs+=step_mal) {
          if (ERR_NOERR != (ret_val=prognToStep(vs_gp,&vs_gp,step_mal,koef))){
              *real_time=i;
              return ret_val;
            }
        }
      g->append(vs_gp);
      // debug_log <<"step"<<i<<"result"<< vs_gp.toString();
    }

  *real_time=i;
  return ret_val;
}

laplaValue TField::lapla(int i,int j, float *lapla) const{

  if(data_.isEmpty()) return NONE_LAPLA;

  if(!getMask(i,j) || !getMask(i-1,j) ||
     !getMask(i+1,j) || !getMask(i,j-1)
     || !getMask(i,j+1))
    {
      return NONE_LAPLA;
    }
  float dfi=(net_fi_[1] -net_fi_[0])*0.5f;
  float fi = net_fi_[i];
  float sinT=sinf((fi+M_PI_2f));
  float sinT1=sinf((fi+M_PI_2f+dfi));
  float sinT2=sinf((fi+M_PI_2f-dfi));
  float vs=sinT1*(getData(i+1,j) -getData(i,j))-
      sinT2*(getData(i,j)-getData(i-1,j));
  float vs1=(getData(i,j-1)+getData(i,j+1)-
              2.f*getData(i,j))/sinT/sinT;
  vs=vs/sinT+vs1;
  if( nullptr != lapla) *lapla = vs/(6.371e6f*6.371e6f);
  if(vs>0) return PLUS_LAPLA;
  return MINUS_LAPLA;

}


float TField::dPx(int i, int j, float dla_1 )const{
  float sin_1 = 1.f/sinf(net_fi_[i]+M_PI_2f);
  return (pdata_[num(i,j+1)] - pdata_[num(i,j-1)])*0.5f*dla_1*sin_1;
}
float TField::dPxx(int i, int j, float d_1)const{
  float sin_1 = 1.f/sinf(net_fi_[i]+M_PI_2f);
  return (dPx( i,  j+1, d_1 )-dPx( i,  j-1, d_1 ))*d_1*sin_1;
}

float TField::dPy(int i, int j, float dfi_1)const{
  return (pdata_[num(i+1,j)] - pdata_[num(i-1,j)])*0.5f*dfi_1;
}

float TField::dPyy(int i, int j, float d_1)const{

  float sin_1 = 1.f/sinf(net_fi_[i]+M_PI_2f);
  return (sinf(net_fi_[i+1]+M_PI_2f)*dPy(i+1,j,d_1)-sinf(net_fi_[i-1]+M_PI_2f)*dPy(i-1,j,d_1) )
      *sin_1*0.5f*d_1;
  //  /sin(net_fi_[i]+M_PI_2)*0.5*d_1;
}

float TField::dPxy(int i, int j, float dfi_1, float dla_1)const{
  float sin_1 = 1.f/sin(net_fi_[i]+M_PI_2f);
  return (dPy(i,j+1,dfi_1)-dPy(i,j-1,dfi_1))*0.5f*dla_1*sin_1;
}


bool TField::radKrivPlos(int i, int j, float *rad)const{
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
  float p[13];
  for(int i = 0; i< 13; ++i){
      if(!pdatamask_[anum[i]] || !pnetmask_[anum[i]]){ return false; }
      p[i] = pdata_[anum[i]];
    }

  float d = 1;
  float pp =((p[1]-p[3])*(p[1]-p[3])+(p[2]-p[4])*(p[2]-p[4]));
  float q = sqrtf( pp*pp*pp );
  float h1 =  (p[9]+p[11]-2*p[0])*(p[2]-p[4])*(p[2]-p[4]);
  float h2 = 2 * (p[5]-p[6]-p[8]+p[7])*(p[1]-p[3])*(p[2]-p[4]);
  float h3 = (p[10]+p[12]- 2* p[0])*(p[1]-p[3]);
  *rad = 2*d*q/(h1-h2+h3);

  return true;
}

bool TField::radKriv(const meteo::GeoPoint &geo, float *rad)const{
  int num_fi, num_la;
  if(!getNumPoFila(geo, &num_fi, &num_la))
    return false;
  return radKriv(num_fi, num_la, rad);
}

bool TField::isValidSheme(int i, int j)const{
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
      if( 0 > anum[ii]|| anum[ii] > size_data_-1
          || !pdatamask_[anum[ii]] || !pnetmask_[anum[ii]]){
          return false;
        }
    }
  return true;
}

bool TField::radKriv(int i, int j, float *rad)const{
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

bool TField::lapltest(int i, int j, float *rad) const{
  if(!isValidSheme(i,j)) return false;

  float rz_1 = 1.f/6371.f;

  float dfi_1 = 1.f/dfi_;
  float dla_1 = 1.f/dla_;
  float dpxx = dPxx(i,j,dla_1);
  float dpyy = dPyy(i,j,dfi_1);

  *rad =rz_1*rz_1*(dpxx+dpyy);

  return true;
}




laplaValue TField::laplaEasy(int i,int j) const
{
  float dfi= (pnet_fi_[1] -pnet_fi_[0])*0.5f;
  float fi = pnet_fi_[i];
  float sinT_=1.f/sinf((fi+M_PI_2f));
  float sinT1=sinf((fi+M_PI_2f+dfi));
  float sinT2=sinf((fi+M_PI_2f-dfi));
  float vs=sinT1*(pdata_[num(i+1,j)]-pdata_[num(i,j)])-
      sinT2*(pdata_[num(i,j)]-pdata_[num(i-1,j)]);
  float vs1=(pdata_[num(i,j-1)]+pdata_[num(i,j+1)]-
      2.f*pdata_[num(i,j)])*sinT_*sinT_;
  vs=vs*sinT_+vs1;
  if(vs>0) return PLUS_LAPLA;
  return MINUS_LAPLA;
}


QVector<fieldExtremum> TField::calcExtremum() const{
  int razmer=2;
  float vs=0.;
  float data_vs = 0.;
  QVector<fieldExtremum> extremum_;
  int size_fi = kolFi();
  int size_la = kolLa();
  extremum_.reserve(size_fi*size_la);
  for(int i=razmer, ssi = size_fi-razmer; i<ssi; ++i)
    {
      int i1=i-1;
      int i2=i+1;
      for(int j=razmer,ssj = size_la-razmer; j<ssj; ++j)
        {
          int j1=j-1;
          int j2=j+1;
          int k =0;
          int l =0;
          laplaValue lapl = laplaEasy(i,j);
          switch(lapl)
            {
            case PLUS_LAPLA:
              for( k= -razmer+1; k<razmer; ++k){
                  for( l= -razmer+1; l<razmer; ++l) {
                      if(PLUS_LAPLA != laplaEasy(i+k,j+l) ) goto met1;
                    }
                }
met1: if((k<razmer)||(l<razmer)) continue;
              data_vs = pdata_[num(i,j)];
              if((data_vs<pdata_[num(i1,j)])&&
                 (data_vs<pdata_[num(i2,j)])&&
                 (data_vs<pdata_[num(i,j1)])&&
                 (data_vs<pdata_[num(i,j2)])){
                  vs = data_vs;
                }
              else continue;
              break;
            case MINUS_LAPLA:
              for( k=-razmer+1; k<razmer; k++){
                  for( l=-razmer+1; l<razmer; l++){
                      if(MINUS_LAPLA != laplaEasy(i+k,j+l)) goto met2;
                    }
                }
met2: if((k<razmer)||(l<razmer)) continue;
              data_vs = pdata_[num(i,j)];
              if((data_vs>pdata_[num(i1,j)])&&
                 (data_vs>pdata_[num(i2,j)])&&
                 (data_vs>pdata_[num(i,j1)])&&
                 (data_vs>pdata_[num(i,j2)])){
                  vs=data_vs;
                }
              else continue;
              break;
            default: continue;
            }
          //vs=vs;
          //  if((vs<min_val_)||(vs>max_val_)) continue;
          extremum_.append(fieldExtremum( lapl ,meteo::GeoPoint(netFi(i),netLa(j)),vs));
        }
    }

  return extremum_;
}

int TField::prognToStep(const meteo::GeoPoint &g0,meteo::GeoPoint *g,int step,float koef) const{
  float Rz=MnMath::EARTH_RAD/*6.371e6*/;
  float G=9.80665f;
  float w=1.4584e-4f;
  float teta=g0.lat()+M_PI_2f;
  float f=g0.lat();
  float l=g0.lon();
  float ll=w*cosf(teta);
  float b=G/(ll*Rz*Rz);
  // l=MnMath::fmod2p(l);
  float sn=sinf(teta);
  //if (fabs(sn)<LITTLE_VALUE) return GEOSTROF_ERROR;
  if (MnMath::isZero(sn)) return GEOSTROF_ERROR;


  float dla;
  float dfi;
  if(!stepFi(&dfi)){
      return -1;
    }
  if(!stepLa(&dla)){
      return -1;
    }

  int i,j;
  koef*=10.f;
  if (!getNumPoFila(g0,&i, &j))
    return DATA_PERENOS_ERROR;
  float dx=(f- pnet_fi_[i])/dfi;
  float dy=(l- pnet_la_[j])/dla;

  if (getMask(i,j)&&getMask(i,j+1)&&getMask(i+1,j)&&getMask((i+1),j+1) ){
      float vs_1=(1.f-dy)* getData(i,j)+dy*getData(i,j+1);
      float vs_2=(1.f-dy)*getData((i+1),j)+dy*getData((i+1),j+1);
      float vs_11=(1.f-dx)*getData(i,j)+dx*getData((i+1),j);
      float vs_22=(1.f-dx)*getData(i,j+1)+dx*getData((i+1),j+1);
      float dHf=(vs_2-vs_1)/(dfi);
      float dHl=(vs_22-vs_11)/(dla*sn);
      float dl=b*dHf*step*koef/sn;
      float df=-b*dHl*step*koef;
      g->setLon( MnMath::PiToPi(l+dl));
      g->setLat(f+df);
      //FIXME временно
      {
        g->setAlt(g0.alt());
      }
      if (g->lat() < -M_PI_2f) {
          g->setLat( -M_PIf - g->lat());
          //g->setLon(MnMath::fmod2p(M_PI + g->lon()));
        }
      else if (g->lat() > M_PI_2f) {
          g->setLat( M_PIf - g->lat());
          // g->setLon(MnMath::fmod2p(M_PI + g->lon()));
        }
      return ERR_NOERR;
    }
  else return DATA_PERENOS_ERROR;
}

float TField::pointValue( const meteo::GeoPoint& point, bool* ok ) const
{
  if ( nullptr != ok ) {
      *ok = false;
    }

  bool valueF_ok = false;
  float calcvalue = pointValueF(point, &valueF_ok);

  if ( valueF_ok && (nullptr != ok)) {
      *ok = true;
    }
  return calcvalue;
}

float TField::pointValueF( const meteo::GeoPoint& point, bool* ok ) const
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

  if ( GENERAL_NET != typeNet() ) {
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
    return 0.0;
  }
  if ( !ensureNetMaskLoaded(i00,i01,i10,i11) ) {
    return 0.0;
  }

  if ( pdatamask_[i00] && pdatamask_[i01] && pdatamask_[i10] && pdatamask_[i11]
       && pnetmask_[i00] && pnetmask_[i01] && pnetmask_[i10] && pnetmask_[i11] ) {
    if ( !ensureDataLoaded(i00,i01,i10,i11) ) {
      return 0.0;
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



bool TField::ensureDataLoaded(int idx) const
{
  Q_UNUSED(idx);
  return true;
}

bool TField::ensureDataLoaded(int i0, int i1, int i2, int i3) const
{
  return ensureDataLoaded(i0) && ensureDataLoaded(i1) && ensureDataLoaded(i2) && ensureDataLoaded(i3);
}

bool TField::ensureNetMaskLoaded(int idx) const
{
  Q_UNUSED(idx);
  return true;
}

bool TField::ensureNetMaskLoaded(int i0, int i1, int i2, int i3) const
{
  return ensureNetMaskLoaded(i0) && ensureNetMaskLoaded(i1) && ensureNetMaskLoaded(i2) && ensureNetMaskLoaded(i3);
}

bool TField::ensureDataMaskLoaded(int idx) const
{
  Q_UNUSED(idx);
  return true;
}

bool TField::ensureDataMaskLoaded(int i0, int i1, int i2, int i3) const
{
  return ensureDataMaskLoaded(i0) && ensureDataMaskLoaded(i1) && ensureDataMaskLoaded(i2) && ensureDataMaskLoaded(i3);
}

bool TField::ensurePageLoaded(int n) const
{
  Q_UNUSED(n);
  return true;
}



float TField::radarValue( const meteo::GeoPoint& point, bool* ok ) const
{
  if ( 0 != ok ) {
      *ok = false;
    }
  if ( 4 > size_data_ || 2 > size_fi_|| 2 > size_la_) {
      return 0.0;
    }
  int i, j;
  float f = -1.0;
  float l = -1.0;

  f = point.lat();
  l = point.lon();

  int size_fi = net_fi_.size();
  int size_la = net_la_.size();
  bool found = false;
  for ( i = 1; i < size_fi; ++i ) { //FIXME
      if ( pnet_fi_[i-1] <= f && pnet_fi_[i] > f ) {
          found = true;
          break;
        }
    }
  if ( false == found ) {
      return 0.0;
    }
  found = false;
  for ( j = 1; j < size_la; ++j ) { //FIXME
      if ( pnet_la_[j-1] <= l && pnet_la_[j] > l ) {
          found = true;
          break;
        }
    }
  if ( false == found ) {
      return 0.0;
    }

  if( true == pdatamask_[i*size_la+j] )
    {
      if ( nullptr != ok ) {
          *ok = true;
        }
      return pdata_[i*size_la+j];
    }
  return 0.0;//точка geo не попала в сетку
}

int TField::setRadarNet(const RadarParam & r_par) {

  meteo::GeoPoint fl_0, fl ;
  float dx = r_par.dx;
  float dy = r_par.dy;
  float rad_dx =dx/6371000.f;
  float ddx = dx*r_par.kol_x*0.5f;
  float ddy = dy*r_par.kol_y*0.5f;
  float toconer = sqrt(ddx*ddx+ddy*ddy)/6371000.f;
  fl_0 = r_par.center.findSecondCoord(toconer,M_PIf*1.25f);
  // debug_log << r_par.center.toString() << fl_0.toString();
  //    net_fi_.resize(r_par.kol_y * r_par.kol_x );
  //    net_la_.resize(r_par.kol_y * r_par.kol_x );
  if ( !init(r_par.kol_y, r_par.kol_x ))
    return ERR_MEMORY;
  fl = fl_0;
  for(int i = 0; i< r_par.kol_x ;++i){//по долготе
      pnet_la_[i] = fl.la();
      fl = fl.findSecondCoord(rad_dx,M_PIf*0.5f);
    }
  fl=fl_0;
  for(int j = 0; j< r_par.kol_y ;++j){//по широте
      pnet_fi_[j]  = fl.fi();
      fl = fl.findSecondCoord(rad_dx,0.);
    }
  /* for(int j = 0; j< r_par.kol_y ;++j){//по широте
  for(int i = 0; i< r_par.kol_x ;++i){//по долготе
        setData(num(i,j),10,true);
  }}*/
  return ERR_NOERR;
}

bool TField::init(const int &asize_fi, const int &asize_la )
{
  int asize = asize_la*asize_fi;
  data_.resize(asize);

  net_fi_.resize(asize_fi);
  net_la_.resize(asize_la);
  netmask_.resize(asize);
  datamask_.resize(asize);

  resetPointers();

  size_fi_ = net_fi_.size();
  size_la_ = net_la_.size();
  size_data_ = net_fi_.size()*net_la_.size();

  net_fi_.fill(0.);
  net_la_.fill(0.);
  dfi_ = 1.;
  dla_ =1.;

  clearData();

  return true;
}

void TField::setSrcPointCount(int sc){
  src_point_count_ = sc;
}

void TField::setMinMaxVal(float amin, float amax)
{
  max_val_ = amax;
  min_val_ = amin;
}

bool TField::setData(int i, int j, float aval, bool amask)
{
  if ( !loadToMemory() ) { return false; }

  return setData(num(i,j), aval, amask);
}

bool TField::setData(int num, float aval, bool amask)
{
  if ( !loadToMemory() ) { return false; }

  if(size_data_ < num){
      return false;
    }

  pdata_[num] = aval;
  pdatamask_[num] = amask;

  return true;
}

void TField::setMasks(int i, int j, bool f){
  setMasks(num(i,j), f);
}

void TField::setMasks(int num, bool f)
{
  if ( !loadToMemory() ) { return; }

  if(size_data_ > num){
      pnetmask_[num] = f;
      pdatamask_[num] = f;
    } //данных нет
}

meteo::GeoPoint TField::begin() const
{
  return reg_param_.start;
}

meteo::GeoPoint TField::end() const
{
  return reg_param_.end;
}

int TField::setNet(const QVector<RegionParam> & areg_pars, NetType type_net ) {
  if(areg_pars.size()<1) return -1;
  RegionParam reg_par(-90.,-180.,90.,180.);
  setNet(reg_par,type_net);
  netmask_.fill(false);
  datamask_.fill(false);

  for(int i =0; i< areg_pars.size(); ++i){
      if(areg_pars.at(i).start.la() > areg_pars.at(i).end.la() ){
          RegionParam reg_par1 = areg_pars.at(i);
          reg_par1.end.setLaDeg(0.);
          setNetMask(reg_par1);
          reg_par1 = areg_pars.at(i);
          reg_par1.start.setLaDeg(0);
          setNetMask(reg_par1);
        }else {
          setNetMask(areg_pars.at(i));
        }
    }

  return true;
}


int TField::setNet(const RegionParam & areg_par, NetType type_net) {
  net_type_ = type_net;
  float step_fi=-9999;
  float step_la=-9999;
  switch(type_net)
    {
    case STEP_500x500:              // через 5 градусов по широте и долготе
      step_fi=5.;
      step_la=5.;
      break;
    case STEP_250x250:              // через 2.5 градуса по широте и долготе
      step_fi=2.5;
      step_la=2.5;
      break;
    case STEP_125x125:              // через 1.25 градуса по широте и долготе
      step_fi=1.25;
      step_la=1.25;
      break;
    case STEP_100x100:              // через 1 градус по широте и долготе
      step_fi=1.;
      step_la=1.;
      break;
    case STEP_0500x0500:              // через 0.5 градус по широте и долготе
      step_fi=0.5;
      step_la=0.5;
      break;
    case STEP_0100x0100:              // через 0.1 градус по широте и долготе
      step_fi=0.1f;
      step_la=0.1f;
      break;
    case STEP_0625x0625:              // через 0.625 градуса по широте и долготе
      step_fi=0.625;
      step_la=0.625;
      break;
    case STEP_2983x100:              // через 0.625 градуса по широте и долготе
      step_fi=2.983f;
      step_la=1.;
      break;
    default: return 2;    // Неизвестная сетка
    }
  return setNet(areg_par, step_fi*M_PIf / 180.0f, step_la*M_PIf / 180.0f);

}


bool TField::setDecartNet(float fi0, float la0, float fi1, float la1, float astep_fi, float astep_la)
{
  setMasks(false);
  //  reg_param_ = areg_par;
  float step_fi = astep_fi;
  float step_la = astep_la;
  float shirfi = fi1 - fi0;
  float shirla = la1 - la0;

  size_fi_ = int(shirfi/ step_fi+(1.2f));
  size_la_ = int(shirla / step_la+(1.2f));
  if ( !init(size_fi_, size_la_ ) ) {
      return false;
    }
  size_data_ = net_fi_.size()*net_la_.size();

  int i = 0;
  QVector<float>::iterator it = net_fi_.begin();
  float last = 0;
  while(it != net_fi_.end()) {
      last = fi0+i*step_fi;
      (*it) = last;
      ++i;
      ++it;
    }
  if(last < fi1){
      net_fi_.append(fi1);
    }
  i = 0;
  it = net_la_.begin();
  while ( it != net_la_.end()) {
      last =  la0 + i*step_la;
      (*it) = last;
      ++i;
      ++it;
    }
  if(last < la1){
      net_la_.append(la1);
    }

  size_fi_ = net_fi_.size();
  size_la_ = net_la_.size();
  size_data_ = size_fi_*size_la_;

  data_.resize(size_data_);
  netmask_.resize(size_data_);
  datamask_.resize(size_data_);
  resetPointers();
  for (int i=0; i<kolFi(); ++i ) {
      for (int j=0; j<kolLa(); ++j ) {
          pnetmask_[num(i,j)] = true;
        }
    }
  dfi_=pnet_fi_[1]-pnet_fi_[0];
  dla_=pnet_la_[1]-pnet_la_[0];


  return true;
}


int TField::setNet(const RegionParam & areg_par, float astep_fi, float astep_la)
{
  reg_param_ = areg_par;
  float step_fi = astep_fi;
  float step_la = astep_la;
  float shirfi = reg_param_.end.fi() - reg_param_.start.fi();
  getNumPoFila(reg_param_.start, &reg_param_.start_i, &reg_param_.start_j);

  size_fi_ = int(shirfi/ step_fi+(1.2f));
  size_la_ = int(2.f*M_PIf / step_la+(1.2f));
  size_data_ = net_fi_.size()*net_la_.size();
  if ( !init(size_fi_, size_la_ ))
    return ERR_MEMORY;

  float i = 0;
  QVector<float>::iterator it = net_fi_.begin();
  while(it != net_fi_.end()) {
      (*it) = reg_param_.start.fi()+i*step_fi;
      i+=1;
      ++it;
    }
  reg_param_.kol_i = i;
  i = 0;
  it = net_la_.begin();
  while ( it != net_la_.end()) {
      (*it) = MnMath::PiToPi( reg_param_.start.la() + i*step_la);
      i+=1;
      ++it;
    }
  reg_param_.kol_j = i;
  return setNetMask(areg_par);
}

void TField::setNetMask(const QVector <bool > &nm)
{
  if ( !loadToMemory() ) { return; }

  //  debug_log<<netmask_.count()<<nm.count();

  if(netmask_.count() != nm.count()) return;
  for (int i = 0; i < netmask_.count();++i){
      if(true == pnetmask_[i] && true == nm[i]){
          pnetmask_[i] = true;
        } else {
          pnetmask_[i] = false;
        }
    }
}


bool TField::setNetMask(const RegionParam &areg_par) {

  float step_fi,step_la;
  if(!stepFi(&step_fi) || !stepLa(&step_la)){
      return false;
    }

  //debug_log<<areg_par.start.toString()<<areg_par.end.toString();
  float shirfi = fabs(areg_par.start.fi() - areg_par.end.fi());
  float shirla = fabs(areg_par.start.la() - areg_par.end.la());
  int size_fi1 = MnMath::ftoi_norm ((shirfi /step_fi+(1.2f)));
  int size_la1 =  MnMath::ftoi_norm((shirla / step_la+(1.2f)));

  float nf = pnet_fi_[0];
  float nl = pnet_la_[0];

  int ii = MnMath::ftoi_norm((areg_par.start.fi() - nf)/step_fi)%( size_fi_ - 1);
  int jj = MnMath::ftoi_norm((areg_par.start.la() - nl)/step_la)%( size_la_ - 1);
  if(size_fi_ < (size_fi1+ii) || size_la_ < (size_la1+jj)){
      error_log << areg_par.start.toString()<< areg_par.end.toString();
      error_log <<"ii "<<ii<<"jj "<<jj<<"size_fi "<< size_fi_ <<"(size_fi1+ii)"<< (size_fi1+ii)<<"size_la"<<size_la_ <<"(size_la1+jj)"<< (size_la1+jj);
      return false;
    }
  for ( int i = ii, isz = size_fi1 + ii; i < isz; ++i ) {
      int idx = i*size_la_;
      for ( int j=jj, jsz = size_la1 + jj; j < jsz; ++j ) {
          *(pnetmask_ + idx + j) = true;
        }
    }
  return true;

}

void TField::clearData()
{
  iso_.clear();
  float* pdata = data_.data();
  bool* pmask = datamask_.data();
  bool* pnmask = netmask_.data();

  for ( int i = 0, isz = data_.size(); i < isz; ++i ) {
      *(pdata + i) = -9999.;
      *(pmask + i) = false;
      *(pnmask + i) = false;
    }
  resetPointers();
}

bool TField::getNumPoFila(const meteo::GeoPoint &geo,int *num_fi, int *num_la) const
{
  float f=geo.lat();
  float l=geo.lon();

  //float shift = MnMath::M_2PI ;
  // if (l<0.) l+=shift;
  float dfi, dla;
  if(!stepFi(&dfi) || ! stepLa(&dla)){
      return false;
    }

  float nf = pnet_fi_[0];
  float nl = pnet_la_[0];

  int i = MnMath::ftoi_norm((f - nf)/dfi)%( size_fi_ - 1);
  int j = MnMath::ftoi_norm((l - nl)/dla)%( size_la_ - 1);
  if (i < 0 || j < 0 ) {
      return false;
    }

  *num_fi = i;
  *num_la = j;

  return true;
}


bool TField::stepLa(float *ret_val) const
{
  if (net_la_.size() < 2) {
      *ret_val = -9999;
      return false;
    }
  (*ret_val) = (net_la_.at(1) - net_la_.at(0));
  return true;
}

bool TField::stepFi(float *ret_val) const
{
  if (net_fi_.size() < 2) {
      *ret_val = -9999;
      return false;
    }
  (*ret_val) = (net_fi_.at(1) - net_fi_.at(0));
  return true;
}

bool TField::stepFiDeg(float* ret_val) const
{
  if(false == stepFi(ret_val)){
      return false;
    }
  (*ret_val) = MnMath::rad2deg(*ret_val);
  return true;
}

bool TField::stepLaDeg(float* ret_val) const{
  if(false == stepLa(ret_val)){
      return false;
    }
  (*ret_val) = MnMath::rad2deg(*ret_val);
  return true;
}


bool TField::isEmpty() const {
  return (data_.isEmpty() || net_fi_.isEmpty() || net_la_.isEmpty());
}

float TField::netFi(int i) const {
  if (i < size_fi_) return pnet_fi_[i];
  return -9999;
}

float TField::netLa(int i) const {
  if (i < size_la_) return pnet_la_[i];
  return -9999;
}

bool TField::getData(int i, int j, float* rv)const {
  *rv = -9999;
  if(i >= kolFi() || j >= kolLa() || !getMask(i,j)) return false;
  *rv = pdata_[num( i, j )];
  return true;
}

bool TField::getData(int i, int j, double* rv)const {
  *rv = -9999;
  if(i >= kolFi() || j >= kolLa() || !getMask(i,j)) return false;
  *rv = pdata_[num( i, j )];
  return true;
}

float TField::getData(int i, int j)const {
  if(i >= kolFi() || j >= kolLa()) return -9999;
  return pdata_[num( i, j )];
}

float TField::getData(int ij)const {
  if(ij >= size_data_) return -9999;
  return pdata_[ij];
}

bool TField::getMask(int ij)const {
  if( ij >= size_data_)  return false;
  return pdatamask_[ij]&&pnetmask_[ij];
}

bool TField::getMask(int i, int j)const {
  int anum =  num( i, j );

  if( anum >= size_data_ )  return false;
  return pdatamask_[anum]&&pnetmask_[anum];
}

bool TField::getNetMask(int i, int j)const {
  int anum =  num( i, j );
  if( anum >= size_data_)  return false;
  return pnetmask_[anum];
}

float TField::getFi(int i)const {
  if(i >= size_fi_ )  return -9999;
  return pnet_fi_[ i ];
}

float TField::getLa(int j)const {
  if(j >= size_la_ )  return -9999;
  return pnet_la_[ j ];
}

int TField::getSrcPointCount()const {
  return src_point_count_;
}

int TField::sizeofBuffer() const
{
  proto::TFieldHeader head = header();

  int size = sizeof(quint32) + head.ByteSize();
  size += data_.size()*sizeof(float) + datamask_.size()*sizeof(bool) + netmask_.size()*sizeof(bool);

  return size;
}

bool TField::fromFile(const QString& fileName)
{
  clear();

  dataFile_.close();
  dataFile_.setFileName(fileName);
  if ( !dataFile_.open(QFile::ReadOnly) ) {
      error_log << QObject::tr("Не удалось открыть файл %1.").arg(dataFile_.fileName())
                << dataFile_.errorString();
      return false;
    }

  int offset = 0;
  if ( !loadHeader(&dataFile_, &offset) ) {
      return false;
    }

  uint dataSize = sizeof(float)*data_.size() + sizeof(bool)*datamask_.size() + sizeof(bool)*netmask_.size();

  dataArr_ = dataFile_.map(offset, dataSize);
  if ( nullptr == dataArr_ ) {
      return false;
    }

  pdata_ = reinterpret_cast<float*>(dataArr_);
  pdatamask_ = reinterpret_cast<bool*>(dataArr_ + sizeof(float)*data_.size());
  pnetmask_ = reinterpret_cast<bool*>(dataArr_ + sizeof(float)*data_.size() + sizeof(bool)*datamask_.size());

  inMemory_ = false;

  return true;
}

bool TField::save(const QString& fileName) const
{
  QFile file(fileName);
  if ( !file.open(QFile::WriteOnly | QFile::Truncate) ) {
      error_log << QObject::tr("Не удалось открыть файл %1.").arg(file.fileName())
                << file.errorString();
      return false;
    }

  QByteArray ba;
  getBuffer(&ba);

  int wsz = file.write(ba);
  if ( wsz != ba.size() ) {
      return false;
    }

  file.close();

  return true;
}


void TField::copyFrom(const TField& f)
{
  inMemory_ = f.inMemory_;
  net_fi_ = f.net_fi_;
  net_la_ = f.net_la_;
  data_ = f.data_;
  datamask_ = f.datamask_;
  netmask_ = f.netmask_;
  reg_param_ = f.reg_param_;
  net_type_ = f.net_type_;
  src_point_count_ = f.src_point_count_;
  max_val_ = f.max_val_;
  min_val_ = f.min_val_;
  step_val_ = f.step_val_;
  unit_ = f.unit_;
  name_ = f.name_;
  meteodescr_ = f.meteodescr_;
  level_ = f.level_;
  level_type_ = f.level_type_;

  date_ = f.date_;
  hour_ = f.hour_;
  model_ = f.model_;
  center_ = f.center_;
  center_name_ = f.center_name_;
  level_type_name_ = f.level_type_name_;

  resetPointers();

  dfi_ = f.dfi_;
  dla_ = f.dla_;
  size_fi_ = f.size_fi_;
  size_la_ = f.size_la_;
  size_data_ = f.size_data_;

  if ( !inMemory_ && f.dataFile_.isOpen() ) {
      dataFile_.close();
      dataFile_.setFileName(f.dataFile_.fileName());
      if ( !dataFile_.open(QFile::ReadOnly) ) {
          error_log << QObject::tr("Не удалось открыть файл %1.").arg(dataFile_.fileName())
                    << dataFile_.errorString();
        }

      quint32 headerSize = 0;
      int offset = 0;

      uint rsz = dataFile_.read(reinterpret_cast<char*>(&headerSize),sizeof(headerSize));
      if ( rsz == sizeof(headerSize) ) {
          offset = sizeof(headerSize) + headerSize;
        }

      uint dataSize = sizeof(float)*data_.size() + sizeof(bool)*datamask_.size() + sizeof(bool)*netmask_.size();

      dataArr_ = dataFile_.map(offset, dataSize);
      if ( nullptr != dataArr_ ) {
          pdata_ = reinterpret_cast<float*>(dataArr_);
          pdatamask_ = reinterpret_cast<bool*>(dataArr_ + sizeof(float)*data_.size());
          pnetmask_ = reinterpret_cast<bool*>(dataArr_ + sizeof(float)*data_.size() + sizeof(bool)*datamask_.size());

          inMemory_ = false;
        }
    }
}

TField&TField::operator=(const TField& f)
{
  copyFrom(f);
  return *this;
}

float TField::max_field_value() const
{
  float maxval = -999999999;
  bool calc = false;
  const float* pdata = data_.data();
  const bool* pdmask = datamask_.data();
  const bool* pnmask = netmask_.data();
  for ( int i = 0, sz = data_.size(); i < sz; ++i ) {
    if ( false == *(pdmask+i) || false == *(pnmask+i) ) {
      continue;
    }
    if ( maxval < *(pdata+i) ) {
      maxval = *(pdata+i);
      calc = true;
    }
  }
  if ( false == calc ) {
    error_log << QObject::tr("Максимальное значение не вычислено!");
  }
  return maxval;
}

float TField::min_field_value() const
{
  float minval = 999999999;
  bool calc = false;
  const float* pdata = data_.data();
  const bool* pdmask = datamask_.data();
  const bool* pnmask = netmask_.data();
  for ( int i = 0, sz = data_.size(); i < sz; ++i ) {
    if ( false == *(pdmask+i) || false == *(pnmask+i) ) {
      continue;
    }
    if ( minval > *(pdata+i) ) {
      minval = *(pdata+i);
      calc = true;
    }
  }
  if ( false == calc ) {
    error_log << QObject::tr("Минимальное значение не вычислено!");
  }
  return minval;
}

void TField::getBuffer(QByteArray* buf) const
{
  QByteArray& ba = *buf;

  proto::TFieldHeader head = header();

  std::string str;
  head.SerializePartialToString(&str);

  ba.resize(sizeof(quint32));

  quint32 sz = str.size();
  memcpy( ba.data(), &sz, sizeof(quint32) );

  ba.reserve(str.size() + data_.size() * sizeof(float) + netmask_.size() * sizeof(bool) * 2);

  ba += QByteArray::fromRawData(str.data(), str.size());
  ba += QByteArray::fromRawData(reinterpret_cast<char*>(pdata_), data_.size() * sizeof(float));
  ba += QByteArray::fromRawData(reinterpret_cast<char*>(pdatamask_), datamask_.size() * sizeof(bool));
  ba += QByteArray::fromRawData(reinterpret_cast<char*>(pnetmask_), netmask_.size() * sizeof(bool));
}

bool TField::fromStdString(const ::std::string& str)
{
  QByteArray arr( str.data(), str.size() );
  return fromBuffer(&arr);
}

bool TField::fromBuffer(QByteArray* buf)
{
  QBuffer s(buf);
  s.open(QBuffer::ReadOnly);

  int offset = 0;
  if ( !loadHeader(&s, &offset) ) {
      return false;
    }

  const QByteArray& ba = *buf;
  uint bufSize = ba.size();

  uint dataSize = sizeof(float)*data_.size() + sizeof(bool)*datamask_.size() + sizeof(bool)*netmask_.size();

  if ( bufSize < offset + dataSize ) {
      var(bufSize) << var(offset) << var(dataSize);
      return false;
    }
  memcpy(data_.data(), ba.data() + dataOffset_, sizeof(float)*data_.size());
  memcpy(datamask_.data(), ba.data() + dataMaskOffset_, sizeof(bool)*datamask_.size());
  memcpy(netmask_.data(), ba.data() + netMaskOffset_, sizeof(bool)*netmask_.size());

  pdata_ = data_.data();
  pdatamask_ = datamask_.data();
  pnetmask_ = netmask_.data();

  inMemory_ = true;

  return true;
}


TField * TField::getCleanCopy()const{
  TField *afield =getCopy();
  afield->clearData();
  return afield;
}


TField * TField::getCopy() const
{
  TField *afield = new TField();
  afield->copyFrom(*this);
  return afield;
}


bool TField::oprMatOz(float *field_MO) const
{
  *field_MO=0.;
  int n=0;

  for(int i=0 , ds = data_.size();i < ds; i++){
      if(getMask(i)){
          (*field_MO)+=pdata_[i];
          n++;
        }
    }
  // debug_log <<"n m" <<n;

  if(n!=0) {(*field_MO)/=float(n); return true;}
  else return false;
}

bool TField::oprMatOz(int start_i,int start_j,int kol_i,int kol_j, float *field_MO) const
{
  *field_MO=0.;
  int n=0;
  int kol_fi = start_i+kol_i;
  int kol_la = start_j+kol_j;

  for( int i=start_i; i < kol_fi; i++ ) {
    for(int j = start_j; j < kol_la; j++){
      int r_abs_num = cycle_num(i,j);
      if(getMask(r_abs_num)){
        (*field_MO)+=pdata_[r_abs_num];
        n++;
      }
    }
  }
  // debug_log <<"n m" <<n;

  if ( n!=0 ) {
    (*field_MO)/=float(n);
    return true;
  }
  return false;
}

bool TField::oprDisp(int start_i,int start_j,int kol_i,int kol_j,float *field_Disp,float *field_MO) const
{

  *field_MO = 0.;
  if(!oprMatOz(start_i,start_j,kol_i,kol_j,field_MO)){
      return false;
    }

  *field_Disp=0.;
  int n=0;

  int kol_fi = start_i+kol_i;
  int kol_la = start_j+kol_j;

  for(int i=start_i; i < kol_fi; i++){
      for(int j = start_j; j < kol_la; j++){
          int r_abs_num = cycle_num(i,j);
          if(getMask(r_abs_num)){
              (*field_Disp) += ((pdata_[r_abs_num] - *field_MO)*(pdata_[r_abs_num] - *field_MO));
              n++;
            }
        }
    }


  if(1<n) {(*field_Disp)/=float(n);  return true;}
  else return false;
}

bool TField::oprDisp(float *field_Disp,float *field_MO) const
{

  *field_MO = 0.;
  if(!oprMatOz(field_MO)){
      return false;
    }

  *field_Disp=0.;
  int n=0;
  for(int i=0, ds = data_.size(); i < ds; i++){
      if(getMask(i)){
          (*field_Disp) += ((pdata_[i] - *field_MO)*(pdata_[i] - *field_MO));
          n++;
        }
    }

  if(1<n) {(*field_Disp)/=float(n);  return true;}
  else return false;
}



bool TField::oprKoefKor(const TField* right, float* result, int di, int dj) const{
  if(reg_param_ != right->getRegPar() ||
     kolFi() != right->kolFi() ||
     kolLa() != right->kolLa() ){
      return false;
    }

  *result = 0.;
  float  kor_fields = 0.;
  float matoz_r = 0.;
  float matoz_ = 0.;
  float disp_r = 0.;
  float disp_ = 0.;

  int n = 0;
  int kol_fi = kolFi();
  int kol_la = kolLa();


  for(int i=0; i < kol_fi; i++){
      for(int j=0; j < kol_la; j++){
          int r_abs_num= num(i,j);
          int abs_num  = cycle_num(i+di,j+dj);

          if(getMask(abs_num)&& right->getMask(r_abs_num)){
              matoz_ += getData(abs_num);
              matoz_r += right->getData(r_abs_num);
              n++;
            }
        }
    }
  if(n <1) return false;

  matoz_ /= float(n);
  matoz_r /= float(n);
  n =0;
  for(int i=0; i < kol_fi; i++){
      for(int j=0; j < kol_la; j++){
          int r_abs_num= num(i,j);
          int abs_num  = cycle_num(i+di,j+dj);
          if(getMask(abs_num)&& right->getMask(r_abs_num)){
              disp_ += (getData(abs_num)-matoz_)*(getData(abs_num)-matoz_);
              disp_r += (right->getData(r_abs_num) - matoz_r)*(right->getData(r_abs_num) - matoz_r);
              n++;
            }
        }
    }
  if(n <1) return false;
  disp_ /= float(n);
  disp_r /= float(n);
  n=0;
  for(int i=0; i < kol_fi; i++){
      for(int j=0; j < kol_la; j++){
          int r_abs_num= num(i,j);
          int abs_num  = cycle_num(i+di,j+dj);
          if(getMask(abs_num)&& right->getMask(r_abs_num)){
              kor_fields += ((getData(abs_num)- matoz_)*(right->getData(r_abs_num) - matoz_r));
              n++;
            }
        }
    }
  if(1<n && disp_>0.f && disp_r >0.f){
      float sko_ = sqrt(disp_);
      float sko_r = sqrt(disp_r);
      kor_fields = kor_fields/float(n)/sko_/sko_r;
      *result = kor_fields;
      return true;
    }
  else return false;

}



bool TField::oprKoefKor(const TField* right, const RegionParam& rpar, float* result) const{
  if(reg_param_ != right->getRegPar() ||
     kolFi() != right->kolFi() ||
     kolLa() != right->kolLa() ){
      return false;
    }

  *result = 0.;
  float  kor_fields = 0.;
  float matoz_r = 0.;
  float matoz_ = 0.;
  float disp_r = 0.;
  float disp_ = 0.;

  int n = 0;

  for(int i=0; i < kolFi(); i++){
      for(int j=0; j < kolLa(); j++){
          if(getMask(i,j)&& right->getMask(i,j) && rpar.isIn(getFi(i),getLa(j))){
              matoz_ += getData(i,j);
              matoz_r += right->getData(i,j);
              n++;
            }
        }
    }

  if(n <1) return false;

  matoz_ /= float(n);
  matoz_r /= float(n);
  n =0;
  for(int i=0; i < kolFi(); i++){
      for(int j=0; j < kolLa(); j++){
          if(getMask(i,j)&& right->getMask(i,j)&& rpar.isIn(getFi(i),getLa(j))){
              disp_ += (getData(i,j)-matoz_)*(getData(i,j)-matoz_);
              disp_r += (right->getData(i,j) - matoz_r)*(right->getData(i,j) - matoz_r);
              n++;
            }
        }
    }
  if(n <1) return false;
  disp_ /= float(n);
  disp_r /= float(n);
  n=0;

  for(int i=0; i < kolFi(); i++){
      for(int j=0; j < kolLa(); j++){
          if(getMask(i,j)&& right->getMask(i,j)&& rpar.isIn(getFi(i),getLa(j))){
              kor_fields += ((getData(i,j)- matoz_)*(right->getData(i,j) - matoz_r));
              n++;
            }
        }
    }

  if(1<n && disp_>0.f && disp_r >0.f){
      float sko_ = sqrt(disp_);
      float sko_r = sqrt(disp_r);
      kor_fields = kor_fields/float(n)/sko_/sko_r;
      *result = kor_fields;
      return true;
    }
  else return false;

}


//!определяет поле result, значения в каждой точке которого
//!равны модулю разницы значений этого поля, умноженного на mnoz1 и right
//! mnoz2 - коэффициент умножения на результат

bool TField::mnoz_plus_mnoz(const TField*right, TField** result, float mnoz1, float mnoz2) const
{
  if(reg_param_ != right->getRegPar() ||
     data_.count() != right->data_.count() ){
      return false;
    }
  *result = getCopy();

  int dsize = data_.count();
  (*result)->setSrcPointCount(dsize);
  for(int i=0; i < dsize; i++){
      if(getMask(i)&& right->getMask(i)){
          float dvs = pdata_[i]*mnoz1 + right->getData(i);
          dvs *= mnoz2;
          (*result)->setData(i,dvs,true);
        } else  (*result)->setMasks(i,false);
    }
  return true;
}


bool TField::minus(const TField* right, TField** result) const
{
  if(reg_param_ != right->getRegPar() ||
     data_.count() != right->data_.count() ){
    return false;
  }

  *result = getCopy();
  int dsize = data_.count();
  (*result)->setSrcPointCount(dsize);

  for(int i=0; i < dsize; i++) {
    if(getMask(i) && right->getMask(i)){
      float dvs = pdata_[i] - right->getData(i);
      (*result)->setData(i,dvs,true);
    } else  {
      (*result)->setMasks(i,false);
    }
  }

  return true;
}

bool TField::plus(const TField* right, TField** result) const
{
  if(reg_param_ != right->getRegPar() ||
     data_.count() != right->data_.count() ){
    return false;
  }
  *result = getCopy();

  int dsize = data_.count();
  (*result)->setSrcPointCount(dsize);
  for(int i=0; i < dsize; i++){
    if(getMask(i)&& right->getMask(i)) {
      float dvs = pdata_[i] + right->getData(i);
      (*result)->setData(i,dvs,true);
    }
    else {
      (*result)->setMasks(i,false);
    }
  }

  return true;
}

//k*sin(this - rigth) + b
bool TField::sinDiff(const TField* right, float k, float b, TField** result) const
{
  if(reg_param_ != right->getRegPar() ||
     data_.count() != right->data_.count() ){
    return false;
  }
  *result = getCopy();

  int dsize = data_.count();
  (*result)->setSrcPointCount(dsize);

  for(int i=0; i < dsize; i++){
    if(getMask(i)&& right->getMask(i)) {
      double dvs = k*sin(pdata_[i] - right->getData(i)) + b;
      (*result)->setData(i,dvs,true);
    } else {
      (*result)->setMasks(i,false);
    }
  }

  return true;
}

//result = sum(k*right+b)
bool TField::sumLinear(const QList<const TField*>& right, const QList<float>& k, const QList<float>& b, TField** result)
{
  if ( right.size() != k.size() || right.size() != b.size() || right.size() < 1) {
    return false;
  }

  for (int idx = 1; idx < right.size(); idx++) {
    if (nullptr == right.at(idx - 1) || nullptr == right.at(idx) ||
    right.at(idx - 1)->getRegPar() != right.at(idx)->getRegPar() ||
    right.at(idx - 1)->data_.count() != right.at(idx)->data_.count()) {
      return false;
    }
  }

  const TField* f1 = right.at(0);
  int dsize = f1->data_.count();
  *result = f1->getCopy();
  // var(dsize);
  (*result)->setSrcPointCount(dsize);

  for(int i=0; i < dsize; i++) {
    float dvs = 0;
    bool exist = true;
    for (int ridx = 0; ridx < right.size(); ridx++) {
      if (right.at(ridx)->getMask(i)) {
    dvs += k.at(ridx)*right.at(ridx)->getData(i) + b.at(ridx);
      } else {
    exist = false;
    break;
      }
    }
    if (exist) {
      (*result)->setData(i,dvs,true);
    } else {
      (*result)->setMasks(i,false);
    }
  }

  return true;
}

void TField::mnoz(float mnoz)
{
  if ( !loadToMemory() ) { return; }

  int dsize = data_.count();
  for(int i=0; i < dsize; i++){
      if(getMask(i)){
          float dvs = pdata_[i]*mnoz;
          setData(i,dvs,true);
        }
    }
}


//!это поле - значение поля
//!oprfield - предыдущее поле с количеством значений, превышающим порог (количество неоправдавшихся прогнозов, например)
//!result - поле с количеством значений, превышающим порог (количество неоправдавшихся прогнозов, например)
//!porog - пороговое значение
bool TField::calcDeltaPorog(const TField* oprfield, TField** result, float porog ) const
{

  if( (nullptr != oprfield) && (reg_param_ != oprfield->getRegPar() ||
     data_.count() != oprfield->data_.count()) ){
      return false;
    }
      *result = getCopy();

  int dsize = data_.count();
  (*result)->setSrcPointCount(dsize);
  for(int i=0; i < dsize; i++){
      if(getMask(i)){
          float dvs = pdata_[i];
          int res = 0;
          if(nullptr != oprfield && oprfield->getMask(i)){
              res = oprfield->getData(i);
            }
          if(dvs >= porog){
              ++res;
            }
          (*result)->setData(i,res,true);
        } else  (*result)->setMasks(i,false);
    }
  return true;
}


//!определяет поле result, значения в каждой точке которого равны модулю разницы значений этого поля и right
//!mnoz - коэффициент умножения на результат
bool TField::delta_fabs(const TField* right, TField** result, float mnoz) const
{
  if(reg_param_ != right->getRegPar() ||
     data_.count() != right->data_.count() ){
      return false;
    }
  *result = getCopy();

  int dsize = data_.count();
  (*result)->setSrcPointCount(dsize);
  for(int i=0; i < dsize; i++){
      if(getMask(i)&& right->getMask(i)){
          float dvs = pdata_[i] - right->getData(i);
          dvs*=mnoz;
          (*result)->setData(i,fabs(dvs),true);
        } else  (*result)->setMasks(i,false);
    }
  return true;
}



int TField::calcSynSit(const meteo::GeoPoint &fl)
{
  int i,j;
  int napr = kUncNapr;
  int type = kUncBar;
  if(fl.fiDeg() > -10.f && fl.fiDeg() < 10.f){
      return  kLowGradient;
    }
  if(!getNumPoFila(fl,&i,&j)) return napr+type;
  return calcSynSit(i,j);
}

int TField::calcSynSit(int i, int j)
{

  int napr = kUncNapr;
  int type = kUncBar;
  if(!isValidSheme(i,j)) return napr+type;

  float px = dPx(i,j,1./dla_)/63.71f;
  float py = dPy(i,j,1./dla_)/63.71f;
  float pxx= dPxx(i,j,1./dla_)/63.71f/63.71f;
  float pyy= dPyy(i,j,1./dla_)/63.71f/63.71f;

  float dp1=0.6f;
  float dp2=0.6f;

  //циклон
  if((pxx > dp1 && pyy > dp1)){
      type = kZyclone;
    }
  //антициклон
  if(( pxx < -dp1 && pyy < -dp1)){
      type = kAntiZyclone;
    }
  //ложбина
  if(( fabs(pxx) <= fabs(dp1) && pyy > dp1) ||
     ( pxx > dp1 && fabs(pyy) <= dp1)){
      type = kLozbina;
    }
  //гребень
  if(( pxx < -dp1 && fabs(pyy) <= fabs(dp1)) ||
     ( fabs(pxx) < fabs(dp1) && pyy < -dp1)){
      type = kGreben;
    }
  //Малоградиентная область
  if((fabs(pxx) < dp1 && fabs(pyy) < -dp1)){
      type = kLowGradient;
    }
  //Седловина
  if(( pxx > dp1 && pyy < -dp1)){
      type = kSaddle;
    }


  //Северо-восток
  if(px >= dp2 &&py >= dp2) {
      napr = kSevVost;
    }
  //Северо-запад
  if(px < dp2 &&py > dp2){
      napr = kSevZap;
    }
  //Юго-восток
  if(px >= dp2 &&py < dp2){
      napr = kUgVost;
    }
  //Юго-запад
  if(px < dp2 &&py <= dp2){
      napr = kUgZap;
    }
  //штиль
  if(fabs( px ) <= dp2 && fabs( py ) <= dp2){
      napr = kCalm;
    }


  return napr+type;
}

void TField::setMasksPoData(const meteo::GeoData & adata, float rad_kor){

  const int kf = int(rad_kor);
  int kol_la = 180/kf;
  int kol_fi = 360/kf;
  int ii[kol_la][kol_fi]; //ii масив числа стнций в квадратах
  bzero(ii,kol_la*kol_fi*sizeof(int));

  int n = adata.size();
  int j = 0;
  int k = 0;

  for ( int i = 0; i < n; ++i ){
      if(adata.at(i).mask != TRUE_DATA) continue;
      float fi = 90.f + adata.at(i).point.fiDeg();
      float la = int(adata.at(i).point.laDeg()+180.f);
      j = int(fi);
      if( j < 0 ) j = 0;
      if(j>179) j=179;
      k=int(la);
      if(k<0) k=0;
      if(k>359) k=359;
      (ii[j/kf][k/kf])++;
    }

  for(int i = 0;i< size_fi_; ++i){
      float fi = 90.f + MnMath::rad2deg(netFi(i));
      int jj = int(fi);
      if( jj < 0 ) jj = 0;
      if( jj > 179 ) jj = 179;
//    debug_log<< "fi"<<fi;
      for(int j = 0; j < size_la_; ++j){
          float la = MnMath::rad2deg(netLa(j))+ 180.f;
          int kk = int(la);
          if( kk < 0) kk = 0;
          if( kk > 359) kk = 359;
          if( ii[jj/kf][kk/kf] == 0){
               setMasks(i, j,false);
          }
       }
    }
}

void TField::setMasks(bool f)
{
  if (false == loadToMemory()) { return; }
  netmask_.fill(f);
  datamask_.fill(f);
}

bool TField::oprSynSit5(const meteo::GeoPoint& fl, QString *str_ret,QString *astr_fenom, int *anum_sit)
{
  if ( !loadToMemory() ) { return false; }

  str_Syn(calcSynSit(fl),str_ret,astr_fenom,anum_sit );
  return true;
}


QString TField::sitName(int anum_sit)
{
  switch(anum_sit){
    case 0: return QObject::tr("Центральная часть циклона или ось ложбины");
    case 1: return QObject::tr("Передняя часть циклона");
    case 2: return QObject::tr("Теплый сектор циклона");
    case 3: return QObject::tr("Тыловая часть циклона или тыловая часть ложбины");
    case 4: return QObject::tr("Центральная часть антициклона или ось барического гребня");
    case 5: return QObject::tr("Северная периферия антициклона");
    case 6: return QObject::tr("Западная периферия антициклона");
    case 7: return QObject::tr("Восточная периферия антициклона");
    case 8: return QObject::tr("Южная периферия антициклона или отрог(ядро)");
    case 9: return QObject::tr("Малоградиентная область пониженного (повышенного) давления");
    case 11: return QObject::tr("Ось ложбины");
    case 12: return QObject::tr("Передняя часть ложбины");
    case 13: return QObject::tr("Тыловая часть ложбины");
    case 14: return QObject::tr("Ось барического гребня");
    case 15: return QObject::tr("Северная часть барического гребня");
    case 16: return QObject::tr("Западная часть барического гребня");
    case 17: return QObject::tr("Восточная часть барического гребня");
    case 18: return QObject::tr("Отрог(ядро)");
    default: return QObject::tr("Барическая седловина");
    }
}

void TField::str_Syn(int tip,QString *str_ret,QString *astr_fenom, int *anum_sit)
{

  QString str_all,str_fenom;
  int num_sit;
  QString  fenom[16]=
  {
    QObject::tr("Слоистообразная облачность"),
    QObject::tr("Кучеводождевая облачность"),
    QObject::tr("Адвективный туман"),
    QObject::tr("Радиационный туман"),
    QObject::tr("Ливневые осадки"),
    QObject::tr("Гроза"),
    QObject::tr("Град"),
    QObject::tr("Шквал"),
    QObject::tr("Метель"),
    QObject::tr("Болтанка"),
    QObject::tr("Обледенение"),
    QObject::tr("Наводнение"),
    QObject::tr("Гололед"),
    QObject::tr("Переход температуры через 0 к отрицательным значениям"),
    QObject::tr("Статическое электричество"),
    QObject::tr("Туман испарения")
  };

  switch(tip) {
    case kZyclone + kCalm:
    case kZyclone:
      {
        str_fenom=fenom[0]+"\n"+fenom[10];
        num_sit=0;//"Центральная часть циклона или ось ложбины";
        break;
      }
    case kZyclone + kSevVost:
      {
        str_fenom=fenom[2]+"\n"+fenom[0]+"\n"+fenom[9]+"\n"+fenom[10];
        num_sit=1;//"Передняя часть циклона или передняя часть ложбины";
        break;
      }
    case kZyclone + kUgVost: case kZyclone + kUgZap:
      {
        str_fenom=fenom[0]+"\n"+fenom[12]+"\n"+fenom[14]+"\n"+fenom[10];
        num_sit=2;
        break;//"Теплый сектор циклона";
      }
    case kZyclone +kSevZap:
      {
        str_fenom=fenom[1]+"\n"+fenom[4]+"\n"+fenom[9]+"\n"+fenom[7];
        num_sit=3;//"Тыловая часть циклона или тыловая часть ложбины";
        break;
      }
    case kAntiZyclone + kCalm:
    case kAntiZyclone:
      {
        str_fenom=fenom[0]+"\n"+fenom[3]+"\n"+fenom[10]+"\n"+fenom[13];
        num_sit=4;//"Центральная часть антициклона или ось барического гребня";
        break;
      }
    case kAntiZyclone + kSevZap:
      {
        str_fenom=fenom[0]+"\n"+fenom[2];
        num_sit=5; //"Северная периферия антициклона или северная часть барического гребня";
        break;
      }
    case kAntiZyclone + kUgZap:
      {
        str_fenom=fenom[0]+"\n"+fenom[2]+"\n"+fenom[7]+"\n"+fenom[9];
        num_sit=6;//"Западная периферия антициклона или тыловая часть барического гребня";
        break;
      }
    case kAntiZyclone + kSevVost:
      {
        str_fenom=fenom[0]+"\n"+fenom[13]+"\n"+fenom[9];
        num_sit=7;//"Восточная периферия антициклона или передняя часть барического гребня";
        break;
      }
    case kAntiZyclone + kUgVost:
      {
        str_fenom=fenom[0]+"\n"+fenom[3];
        num_sit=8;//"Южная периферия антициклона или отрог(ядро)";
        break;
      }
    case kLowGradient:
    case kLowGradient + kSevZap:
    case kLowGradient + kUgZap:
    case kLowGradient + kSevVost:
    case kLowGradient + kUgVost:
    case kLowGradient + kCalm:
      {
        str_fenom=fenom[0]+"\n"+fenom[3]+"\n"+fenom[5]+"\n"+fenom[4];
        num_sit=9;//"Малоградиентная область пониженного (повышенного) давления";
        break;
      }
    case kLozbina + kCalm:
    case kLozbina:
      {
        str_fenom=fenom[0]+"\n"+fenom[10];
        num_sit=11;//""Ось ложбины";
        break;
      }
    case kLozbina + kSevVost:
    case kLozbina + kUgVost:
      {
        str_fenom=fenom[2]+"\n"+fenom[0]+"\n"+fenom[9]+"\n"+fenom[10];
        num_sit=12;//"Передняя часть ложбины";
        break;
      }
    case kLozbina + kUgZap:
    case kLozbina +kSevZap:
      {
        str_fenom=fenom[1]+"\n"+fenom[4]+"\n"+fenom[9]+"\n"+fenom[7];
        num_sit=13;//"Тыловая часть ложбины";
        break;
      }
    case kGreben + kCalm:
    case kGreben:
      {
        str_fenom=fenom[0]+"\n"+fenom[3]+"\n"+fenom[10]+"\n"+fenom[13];
        num_sit=14;//"ось барического гребня";
        break;
      }
    case kGreben + kSevZap:
      {
        str_fenom=fenom[0]+"\n"+fenom[2];
        num_sit=15; //"Северная часть барического гребня";
        break;
      }
    case kGreben + kUgZap:
      {
        str_fenom=fenom[0]+"\n"+fenom[2]+"\n"+fenom[7]+"\n"+fenom[9];
        num_sit=16;//"Западная периферия антициклона или тыловая часть барического гребня";
        break;
      }
    case kGreben + kSevVost:
      {
        str_fenom=fenom[0]+"\n"+fenom[13]+"\n"+fenom[9];
        num_sit=17;//"Восточная часть барического гребня";
        break;
      }
    case kGreben + kUgVost:
      {
        str_fenom=fenom[0]+"\n"+fenom[3];
        num_sit=18;//"Южная периферия антициклона или отрог(ядро)";
        break;
      }

    default:{
        str_fenom=fenom[0]+"\n"+fenom[3]+"\n"+fenom[5]+"\n"+fenom[4];
        num_sit=10;
      }// "Седловина или перемычка пониженного (повышенного) давления";
    }
  *astr_fenom = str_fenom;
  *str_ret = sitName(num_sit);
  *anum_sit = num_sit;
}

void TField::setValueType(int descr, int value_level, int value_type)
{
  meteodescr_ = descr;
  level_ = value_level;
  level_type_ = value_type;
  singleton::TFieldParams::instance()->setFieldParams(this);
}

}
