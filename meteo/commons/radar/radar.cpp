#include "radar.h"

#include <sql/psql/psqlquery.h>
#include <commons/mathtools/mnmath.h>

namespace meteo {

Radar::Radar( const GeoPoint& c, int cw, int ch, int rs, int cs, int empty )
  : center_(c),
  cell_width_(cw),
  cell_height_(ch),
  row_size_(rs),
  clmn_size_(cs),
  empty_val_(empty)
{
}

Radar::~Radar()
{
}

void Radar::setData( int d, const QVector<double>& dt, const GeoVector& crds )
{
  descr_ = d;
  data_ = dt;
  coords_ = crds;
}

double Radar::cellValue( int x, int y, bool* ok ) const
{
  *ok = false;
  double val = -9999.0;
  if ( 0 > x || 0 > y || clmn_size_ <= x || row_size_ <= y ) {
    return val;
  }
  int num = y*clmn_size_ + x;
  if ( num < data_.size() ) {
    if ( true == hasval_[num] ) {
      *ok = true;
      return data_[num];
    }
  }
  return val;
}

void Radar::setProto( const surf::OneMrlValueReply& proto )
{
  center_ = GeoPoint::fromDegree( proto.center().fi(), proto.center().la(), proto.center().height() );
  cell_width_ = proto.dx();
  cell_height_ = proto.dy();
  row_size_ = proto.kol_y();
  clmn_size_ = proto.kol_x();
  descr_ = proto.meteo_descr();
  datetime_ = PsqlQuery::datetimeFromString( proto.date() );
  station_ = proto.station();
  direction_ = proto.synopdirection();
  speed_ = proto.synopspeed();
  int netsize = 0;
  if ( 0 < row_size_ && 0 < clmn_size_ ) {
    netsize = row_size_*clmn_size_;
  }
  if ( proto.mdata_size() != proto.rdata_size() ) {
    warning_log << QObject::tr("Количество данных в rdata не ссовпалдает с количеством в mdata %1 != %2")
      .arg( proto.rdata_size() )
      .arg( proto.mdata_size() );
  }
  int sz = proto.rdata_size();
  if ( proto.mdata_size() < sz ) {
    sz = proto.mdata_size();
  }
  if ( 0 < netsize ) {
    data_.resize(netsize);
    coords_.resize(netsize);
    hasval_.resize(netsize);
    hasval_.fill(false);
  }
  else {
    data_.clear();
    coords_.clear();
    hasval_.clear();
    return;
  }
  for ( int i = 0; i < sz; ++i ) {
    const surf::MrlData& mrl = proto.rdata(i);
    const surf::MeteoData& meteodata = proto.mdata(i);
    int numrow = mrl.kvadrat()/clmn_size_;
    int numclmn = mrl.kvadrat()%clmn_size_;
    if ( numrow < row_size_ && numclmn < clmn_size_ ) {
      data_[numrow*clmn_size_+numclmn] = mrl.value().value();
      coords_[numrow*clmn_size_+numclmn] = GeoPoint( meteodata.point().fi(), meteodata.point().la(), meteodata.point().height() );
      if ( false == MnMath::isEqual( empty_val_, mrl.value().value() ) ) {
        hasval_[numrow*clmn_size_+numclmn] = true;
      }
    }
    else {
      error_log << QObject::tr("Выход за границы сетки. Кол-во строк = %1. Кол-во столбцов = %2. Номер строки = %3. Номер столбца = %4")
        .arg(row_size_)
        .arg(clmn_size_)
        .arg(numrow)
        .arg(numclmn);
    }
  }
}

surf::OneMrlValueReply Radar::proto() const
{
  surf::OneMrlValueReply mrl;

  mrl.set_date( datetime_.toString( Qt::ISODate ).toStdString() );
  mrl.set_station(station_);
  mrl.mutable_center()->set_fi( center_.latDeg() );
  mrl.mutable_center()->set_la( center_.lonDeg() );
  mrl.mutable_center()->set_height( center_.alt() );
  mrl.set_is_abs_h(2);
  mrl.set_kol_x( row_size_ );
  mrl.set_kol_y( clmn_size_ );
  mrl.set_dx( row_size_ );
  mrl.set_dy( clmn_size_ );
  mrl.set_proj(2);
  mrl.set_synopdirection(direction_);
  mrl.set_synopspeed(speed_);
  mrl.set_meteo_descr(descr_);
  for ( int i = 0, isz = row_size_; i < isz; ++i ) {
    for ( int j = 0, jsz = clmn_size_; j < jsz; ++j ) {
      if ( false == hasval_[i*clmn_size_+j] ) {
        continue;
      }
      surf::MrlData* one = mrl.add_rdata();
      surf::MeteoData* mdata = mrl.add_mdata();
      one->set_kvadrat(i*clmn_size_+j);
      one->mutable_value()->set_quality(0);
      one->mutable_value()->set_value(data_[i*clmn_size_+j]);
      mdata->mutable_value()->set_quality(0);
      mdata->mutable_value()->set_value(data_[i*clmn_size_+j]);
    }
  }

  return mrl;
}

Radar& Radar::fromProto( const surf::OneMrlValueReply& proto, Radar* r )
{
  r->setProto(proto);
  return *r;
}

}
