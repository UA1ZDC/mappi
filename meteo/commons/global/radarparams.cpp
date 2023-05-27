#include <meteo/commons/global/radarparams.h>

#include <qdir.h>
#include <qmap.h>

#include <cross-commons/debug/tlog.h>
#include <meteo/commons/global/global.h>
#include <commons/textproto/tprototext.h>

namespace meteo {
namespace map {

RadarParams::RadarParams( const QString& path )
  : path_(path)
{
  loadParamsFromDir();
}

RadarParams::~RadarParams()
{
}

const proto::RadarColor& RadarParams::protoParams( int descr ) const
{
  for ( int i = 0, sz = params_.color_size(); i < sz; ++i ) {
    const proto::RadarColor& fcolor = params_.color(i);
    if ( fcolor.descr() == descr  ) {
      return fcolor;
    }
  }
  return emptycolor_;
}

const proto::RadarColor& RadarParams::protoParams( const QString& name ) const
{
  std::string stdname = name.toStdString();
  for ( int i = 0, sz = params_.color_size(); i < sz; ++i ) {
    const proto::RadarColor& fcolor = params_.color(i);
    if ( fcolor.name() == stdname  ) {
      return fcolor;
    }
  }
  return emptycolor_;
}

bool RadarParams::contains( int descr ) const
{
  for ( int i = 0, sz = params_.color_size(); i < sz; ++i ) {
    const proto::RadarColor& fcolor = params_.color(i);
    if ( fcolor.descr() == descr  ) {
      return true;
    }
  }
  return false;
}

bool RadarParams::contains( const QString& name ) const
{
  std::string stdname = name.toStdString();
  for ( int i = 0, sz = params_.color_size(); i < sz; ++i ) {
    const proto::RadarColor& fcolor = params_.color(i);
    if ( fcolor.name() == stdname  ) {
      return true;
    }
  }
  return false;
}

void RadarParams::setParams( const proto::RadarColor& clr )
{
  if ( false == clr.has_descr() ) {
    error_log << QObject::tr("Невозможно добавить настройку цветов для радаров. Не уазан дескиптор.");
    return;
  }
  proto::RadarColor* destclr = 0;
  for ( int i = 0, sz = params_.color_size(); i < sz; ++i ) {
    if ( clr.descr() == params_.color(i).descr() ) {
      destclr = params_.mutable_color(i);
      break;
    }
  }
  if ( 0 == destclr ) {
    destclr = params_.add_color();
  }
  destclr->CopyFrom(clr);
}

void RadarParams::setParams( const proto::RadarColors& clrs )
{
  params_.CopyFrom(clrs);
}

void RadarParams::removeParams( int descr )
{
  int num = -1;
  for ( int i = 0, sz = params_.color_size(); i < sz; ++i ) {
    if ( descr == params_.color(i).descr() ) {
      num = i;
      break;
    }
  }
  if ( -1 == num ) {
    error_log << QObject::tr("Невозможно удалить параметры радаров %1, потому что их нет в списке параметров.");
    return;
  }
  for ( int i = num + 1, sz = params_.color_size(); i < sz; ++i ) {
    params_.mutable_color(i-1)->CopyFrom(params_.color(i));
  }
  params_.mutable_color()->RemoveLast();
}

bool RadarParams::saveParams()
{
  QDir dir(path_);
  if ( false == dir.exists() ) {
    if ( false == dir.mkpath( dir.absolutePath() ) ) {
      error_log << QObject::tr("Не удалось создать директорию %1").arg( dir.absolutePath() );
      return false;
    }
  }
  QStringList list = dir.entryList( QDir::Files | QDir::NoDotAndDotDot );
  for ( int i = 0, sz = list.size(); i < sz; ++i ) {
    dir.remove( dir.absoluteFilePath( list[i] ) );
  }
  for ( int i = 0, sz = params_.color_size(); i < sz; ++i ) {
    proto::RadarColors clrs;
    proto::RadarColor* destclr = clrs.add_color();
    const proto::RadarColor& clr = params_.color(i);
    destclr->CopyFrom(clr);
    QString name = QString("%1.conf").arg( clr.descr() );
    TProtoText::toFile( clrs, dir.absoluteFilePath(name) );
  }
  return true;
}

TColorGradList RadarParams::gradParams( const proto::RadarColor& fieldcolor )
{
  TColorGradList gradlist;
  for ( int i = 0, sz = fieldcolor.gradient_size(); i < sz; ++i ) {
    const proto::ColorGrad& srcgrad = fieldcolor.gradient(i);
    float minval = srcgrad.min_value();
    float maxval = minval;
    if ( true == srcgrad.has_max_value() ) {
      maxval = srcgrad.max_value();
    }
    QColor minclr = QColor::fromRgba( srcgrad.min_color() );
    QColor maxclr = minclr;
    if ( true == srcgrad.has_max_color() ) {
      maxclr = QColor::fromRgba( srcgrad.max_color() );
    }
    TColorGrad grad( minval, maxval, minclr, maxclr );
    gradlist.append(grad);
  }
  return gradlist;
}

float RadarParams::gradMin( const proto::RadarColor& color )
{
  if ( 0 != color.gradient_size() ) {
    return color.gradient().begin()->min_value();
  }
  return 0.0;
}

float RadarParams::gradMax( const proto::RadarColor& color )
{
  if ( 0 != color.gradient_size() ) {
    return (color.gradient().end()-1)->max_value();
  }
  return 0.0;
}

float RadarParams::gradStep( const proto::RadarColor& color )
{
  return color.step();
}

proto::RadarColor RadarParams::gradlist2gradientcolor( const TColorGradList& gradlist )
{
  proto::RadarColor lvlclr;
  float step(1.0);
  for ( int i = 0, sz = gradlist.size(); i < sz; ++i ) {
    proto::ColorGrad* grad = lvlclr.add_gradient();
    const TColorGrad& srcgrad = gradlist[i];
    grad->set_min_value( srcgrad.begval() );
    grad->set_max_value( srcgrad.endval() );
    grad->set_min_color( srcgrad.begcolor().rgba() );
    grad->set_max_color( srcgrad.endcolor().rgba() );
    step = srcgrad.endval() - srcgrad.begval();
  }
  lvlclr.set_step(step);
  return lvlclr;
}

void RadarParams::setGradColor( const TColorGradList& grad, proto::RadarColor* color )
{
  proto::RadarColor lvlclr = gradlist2gradientcolor( grad );
  color->clear_gradient();
  for ( int i = 0, sz = lvlclr.gradient_size(); i < sz; ++i ) {
    proto::ColorGrad* grd = color->add_gradient();
    grd->CopyFrom( lvlclr.gradient(i) );
  }
//  color->mutable_gradient()->CopyFrom( lvlclr.gradient() );
}

void RadarParams::setGradStep( float step, proto::RadarColor* color )
{
  color->set_step(step);
}


void RadarParams::loadParamsFromDir()
{
  params_.Clear();
  QDir dir(path_);
  if ( false == dir.exists() ) {
    warning_log << QObject::tr("Дироектория %1 не существует, но будет создана")
      .arg(path_);
    if ( false == dir.mkpath(path_) ) {
      error_log << QObject::tr("Не удалось создать директорию %1")
        .arg(path_);
      return;
    }
  }
  QMap<QString, meteo::map::proto::RadarColors> setts;
  bool res = TProtoText::loadProtosFromDirectory( path_, &setts );
  if ( false == res ) {
    error_log << QObject::tr("Не удалось загрузить файлы из директории %1")
      .arg(path_);
    return;
  }
  QMapIterator< QString, meteo::map::proto::RadarColors> it(setts);
  while ( true == it.hasNext() ) {
    it.next();
    params_.MergeFrom( it.value() );
  }
}

}
}
