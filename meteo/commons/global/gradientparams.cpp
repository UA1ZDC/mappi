#include <meteo/commons/global/gradientparams.h>

#include <qdir.h>
#include <qmap.h>

#include <cross-commons/debug/tlog.h>
#include <meteo/commons/global/global.h>
#include <commons/textproto/tprototext.h>

namespace meteo {
namespace map {

GradientParams::GradientParams( const QString& path )
  : path_(path)
{
  loadParamsFromDir();
}

GradientParams::~GradientParams()
{
}

const proto::FieldColor& GradientParams::protoParams( int descr ) const
{
  for ( int i = 0, sz = params_.color_size(); i < sz; ++i ) {
    const proto::FieldColor& fcolor = params_.color(i);
    if ( fcolor.descr() == descr  ) {
      return fcolor;
    }
  }
  return emptycolor_;
}

const proto::FieldColor& GradientParams::protoParams( const QString& name ) const
{
  std::string stdname = name.toStdString();
  for ( int i = 0, sz = params_.color_size(); i < sz; ++i ) {
    const proto::FieldColor& fcolor = params_.color(i);
    if ( fcolor.name() == stdname  ) {
      return fcolor;
    }
  }
  return emptycolor_;
}

bool GradientParams::contains( int descr ) const
{
  for ( int i = 0, sz = params_.color_size(); i < sz; ++i ) {
    const proto::FieldColor& fcolor = params_.color(i);
    if ( fcolor.descr() == descr  ) {
      return true;
    }
  }
  return false;
}

bool GradientParams::contains( const QString& name ) const
{
  std::string stdname = name.toStdString();
  for ( int i = 0, sz = params_.color_size(); i < sz; ++i ) {
    const proto::FieldColor& fcolor = params_.color(i);
    if ( fcolor.name() == stdname  ) {
      return true;
    }
  }
  return false;
}

TColorGrad GradientParams::isoParams( int level, int type_level, int descr )
{
  for ( int i = 0, sz = params_.color_size(); i < sz; ++i ) {
    const proto::FieldColor& fcolor = params_.color(i);
    if ( fcolor.descr() == descr ) {
      return isoParams( level, type_level, fcolor );
    }
  }
  return TColorGrad();
}

TColorGrad GradientParams::isoParams( int level, int type_level, const QString& name )
{
  std::string stdname = name.toStdString();
  for ( int i = 0, sz = params_.color_size(); i < sz; ++i ) {
    const proto::FieldColor& fcolor = params_.color(i);
    if ( fcolor.name() == stdname ) {
      return isoParams( level, type_level, fcolor );
    }
  }
  return TColorGrad();
}

TColorGradList GradientParams::gradParams( int level, int type_level, int descr )
{
  for ( int i = 0, sz = params_.color_size(); i < sz; ++i ) {
    const proto::FieldColor& fcolor = params_.color(i);
    if ( fcolor.descr() == descr ) {
      return gradParams( level, type_level, fcolor );
    }
  }
  return TColorGradList();
}

TColorGradList GradientParams::gradParams( int level, int type_level, const QString& name )
{
  std::string stdname = name.toStdString();
  for ( int i = 0, sz = params_.color_size(); i < sz; ++i ) {
    const proto::FieldColor& fcolor = params_.color(i);
    if ( fcolor.name() == stdname ) {
      return gradParams( level, type_level, fcolor );
    }
  }
  return TColorGradList();
}

void GradientParams::setParams( const proto::FieldColor& clr )
{
  if ( false == clr.has_descr() ) {
    error_log << QObject::tr("Невозможно добавить настройку цветов для изолиний. Не уазан дескиптор.");
    return;
  }
  proto::FieldColor* destclr = 0;
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

void GradientParams::removeParams( int descr )
{
  int num = -1;
  for ( int i = 0, sz = params_.color_size(); i < sz; ++i ) {
    if ( descr == params_.color(i).descr() ) {
      num = i;
      break;
    }
  }
  if ( -1 == num ) {
    error_log << QObject::tr("Невозможно удалить параметры изолиний %1, потому что их нет в списке параметров.");
    return;
  }
  for ( int i = num + 1, sz = params_.color_size(); i < sz; ++i ) {
    params_.mutable_color(i-1)->CopyFrom(params_.color(i));
  }
  params_.mutable_color()->RemoveLast();
}

bool GradientParams::saveParams()
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
    proto::FieldColors clrs;
    proto::FieldColor* destclr = clrs.add_color();
    const proto::FieldColor& clr = params_.color(i);
    destclr->CopyFrom(clr);
    QString name = QString("%1.conf").arg( clr.descr() );
    TProtoText::toFile( clrs, dir.absoluteFilePath(name) );
  }
  return true;
}

proto::LevelColor GradientParams::levelProto( int level, int type_level, const proto::FieldColor& color )
{
  proto::LevelColor lvlclr;
  for ( int i = 0, sz = color.level_size(); i < sz; ++i ) {
    const proto::LevelColor& srclvl = color.level(i);
    if ( srclvl.level() == level && srclvl.type_level() == type_level ) {
      lvlclr.CopyFrom(srclvl);
      return lvlclr;
    }
  }
  lvlclr.set_level(level);
  lvlclr.set_type_level(type_level);
  lvlclr.set_step_iso( color.step_iso() );
  lvlclr.mutable_color()->CopyFrom( color.def_iso() );
  return lvlclr;
}

void GradientParams::setLevelProto( int level, int type_level, const proto::LevelColor& clr, proto::FieldColor* color )
{
  proto::LevelColor* dst = 0;
  for ( int i = 0, sz = color->level_size(); i < sz; ++i ) {
    proto::LevelColor* lvlclr = color->mutable_level(i);
    if ( lvlclr->level() == level && lvlclr->type_level() == type_level ) {
      dst = lvlclr;
      break;
    }
  }
  if ( 0 == dst ) {
    dst = color->add_level();
  }
  dst->CopyFrom(clr);
}

TColorGrad GradientParams::isoParams( int level, int type_level, const proto::FieldColor& fieldcolor )
{
  TColorGrad grad;
  for ( int i = 0, sz = fieldcolor.level_size(); i < sz; ++i ) {
    const proto::LevelColor& srclvl = fieldcolor.level(i);
    if ( srclvl.level() == level && srclvl.type_level() == type_level ) {
      const proto::ColorGrad& srcgrad = srclvl.color();
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
      return grad;
    }
  }
  const proto::ColorGrad& srcgrad = fieldcolor.def_iso();
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
  return TColorGrad( minval, maxval, minclr, maxclr );
}

TColorGradList GradientParams::gradParams( int level, int type_level, const proto::FieldColor& fieldcolor )
{
  TColorGradList gradlist;
  //если есть, индивидуальная заливка
  for ( int i = 0, sz = fieldcolor.level_size(); i < sz; ++i ) {
    const proto::LevelColor& srclvl = fieldcolor.level(i);
    if ( srclvl.level() == level && srclvl.type_level() == type_level ) {
      if (srclvl.gradient_size() != 0) {
	for ( int j = 0, jsz = srclvl.gradient_size(); j < jsz; ++j ) {
	  const proto::ColorGrad& srcgrad = srclvl.gradient(j);
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
    }
  }
  //настроенная заливка для всех уровней линии
  if ( 0 != fieldcolor.def_grad_size() ) {
    for ( int i = 0, sz = fieldcolor.def_grad_size(); i < sz; ++i ) {
      const proto::ColorGrad& srcgrad = fieldcolor.def_grad(i);
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

  //построение градиентной заливки по мин/макс линии
  float step = fieldcolor.step_grad();
  TColorGrad grad = isoParams(level, type_level, fieldcolor);
  float val = grad.begval();
  if (step != 0) {
    while (val + step < grad.endval()) {
      gradlist.append( TColorGrad( val, val+step, grad.color(val), grad.color(val) ) );
      val += step;
    }
  }
  gradlist.append( TColorGrad( val, grad.endval(), grad.color(val), grad.color(val) ) );
  
  return gradlist;
}

float GradientParams::gradMin( const proto::FieldColor& color )
{
  if ( 0 != color.def_grad_size() ) {
    return color.def_grad().begin()->min_value();
  }
  return 0.0;
}

float GradientParams::gradMin( int lvl, int tplvl, const proto::FieldColor& color )
{
  proto::LevelColor srclvl = levelProto( lvl, tplvl, color );
  if ( 0 != srclvl.gradient_size() ) {
    return srclvl.gradient().begin()->min_value();
  }
  return 0.0;
}

float GradientParams::gradMax( const proto::FieldColor& color )
{
  if ( 0 != color.def_grad_size() ) {
    return (color.def_grad().end()-1)->max_value();
  }
  return 0.0;
}

float GradientParams::gradMax( int lvl, int tplvl, const proto::FieldColor& color )
{
  proto::LevelColor srclvl = levelProto( lvl, tplvl, color );
  if ( 0 != srclvl.gradient_size() ) {
    return (srclvl.gradient().end()-1)->min_value();
  }
  return 0.0;
}

float GradientParams::isoMin( const proto::FieldColor& color )
{
  return color.def_iso().min_value();
}

float GradientParams::isoMin( int lvl, int tplvl, const proto::FieldColor& color )
{
  proto::LevelColor srclvl = levelProto( lvl, tplvl, color );
  return srclvl.color().min_value();
}

float GradientParams::isoMax( const proto::FieldColor& color )
{
  return color.def_iso().max_value();
}

float GradientParams::isoMax( int lvl, int tplvl, const proto::FieldColor& color )
{
  proto::LevelColor srclvl = levelProto( lvl, tplvl, color );
  return srclvl.color().max_value();
}

QColor GradientParams::isoColorMin( const proto::FieldColor& color )
{ 
  QColor clr = QColor::fromRgba( color.def_iso().min_color() );
  return clr;
}

QColor GradientParams::isoColorMax( const proto::FieldColor& color )
{
  QColor clr = QColor::fromRgba( color.def_iso().max_color() );
  return clr;
}

QColor GradientParams::isoColorMin( int level, int type_level, const proto::FieldColor& color )
{ 
  proto::LevelColor srclvl = levelProto( level, type_level, color );
  QColor clr = QColor::fromRgba( srclvl.color().min_color() );
  return clr;
}

QColor GradientParams::isoColorMax( int level, int type_level, const proto::FieldColor& color )
{
  proto::LevelColor srclvl = levelProto( level, type_level, color );
  QColor clr = QColor::fromRgba( srclvl.color().max_color() );
  return clr;
}

float GradientParams::gradStep( const proto::FieldColor& color )
{
  return color.step_grad();
}

float GradientParams::gradStep( int level, int type_level, const proto::FieldColor& color )
{
  proto::LevelColor srclvl = levelProto( level, type_level, color );
  return srclvl.step_grad();
}

float GradientParams::isoStep( const proto::FieldColor& color )
{
  return color.step_iso();
}

float GradientParams::isoStep( int level, int type_level, const proto::FieldColor& color )
{
  proto::LevelColor srclvl = levelProto( level, type_level, color );
  return srclvl.step_iso();
}

proto::LevelColor GradientParams::gradlist2gradientcolor( const TColorGradList& gradlist, int level, int type_level )
{
  proto::LevelColor lvlclr;
  lvlclr.set_level(level);
  lvlclr.set_type_level(type_level);
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
  lvlclr.set_step_grad(step);
  return lvlclr;
}

proto::LevelColor GradientParams::grad2levelcolor( const TColorGrad& grad, int level, int type_level )
{
  proto::LevelColor lvlclr;
  lvlclr.set_level(level);
  lvlclr.set_type_level(type_level);
  proto::ColorGrad* gr = lvlclr.mutable_color();
  gr->set_min_value( grad.begval() );
  gr->set_max_value( grad.endval() );
  gr->set_min_color( grad.begcolor().rgba() );
  gr->set_max_color( grad.endcolor().rgba() );
  lvlclr.set_step_iso(1.0);
  return lvlclr;
}

void GradientParams::setIsoColor( int level, int type_level, const TColorGrad& grad, proto::FieldColor* color )
{
  proto::LevelColor lvlclr = grad2levelcolor( grad, level, type_level );
  proto::LevelColor destclr = levelProto( level, type_level, *color );
  destclr.mutable_color()->CopyFrom( lvlclr.color() );
  setLevelProto( level, type_level, destclr, color );
}

void GradientParams::setIsoStep( int level, int type_level, float step, proto::FieldColor* color )
{
  proto::LevelColor lvlclr = levelProto( level, type_level, *color );
  lvlclr.set_step_iso(step);
  setLevelProto( level, type_level, lvlclr, color );
}

void GradientParams::setIsoMin( int level, int type_level, float min, proto::FieldColor* color )
{
  proto::LevelColor lvlclr = levelProto( level, type_level, *color );
  lvlclr.mutable_color()->set_min_value(min);
  setLevelProto( level, type_level, lvlclr, color );
}

void GradientParams::setIsoMax( int level, int type_level, float max, proto::FieldColor* color )
{
  proto::LevelColor lvlclr = levelProto( level, type_level, *color );
  lvlclr.mutable_color()->set_max_value(max);
  setLevelProto( level, type_level, lvlclr, color );
}

void GradientParams::setIsoColorMin( int level, int type_level, const QColor& clr, proto::FieldColor* color )
{
  proto::LevelColor lvlclr = levelProto( level, type_level, *color );
  lvlclr.mutable_color()->set_min_color( clr.rgba() );
  setLevelProto( level, type_level, lvlclr, color );
}

void GradientParams::setIsoColorMax( int level, int type_level, const QColor& clr, proto::FieldColor* color )
{
  proto::LevelColor lvlclr = levelProto( level, type_level, *color );
  lvlclr.mutable_color()->set_max_color( clr.rgba() );
  setLevelProto( level, type_level, lvlclr, color );
}

void GradientParams::setIsoColor( const TColorGrad& grad, proto::FieldColor* color )
{
  proto::LevelColor lvlclr = grad2levelcolor( grad, 0, 0 );
  color->mutable_def_iso()->CopyFrom( lvlclr.color() );
}

void GradientParams::setIsoStep( float step, proto::FieldColor* color )
{
  color->set_step_iso(step);
}

void GradientParams::setIsoMin( float min, proto::FieldColor* color )
{
  color->mutable_def_iso()->set_min_value(min);
}

void GradientParams::setIsoMax( float max, proto::FieldColor* color )
{
  color->mutable_def_iso()->set_max_value(max);
}

void GradientParams::setIsoColorMin( const QColor& clr, proto::FieldColor* color )
{
  color->mutable_def_iso()->set_min_color( clr.rgba() );
}

void GradientParams::setIsoColorMax( const QColor& clr, proto::FieldColor* color )
{
  color->mutable_def_iso()->set_max_color( clr.rgba() );
}

void GradientParams::setGradColor( int level, int type_level, const TColorGradList& grad, proto::FieldColor* color )
{
  proto::LevelColor lvlclr = gradlist2gradientcolor( grad, level, type_level );
  proto::LevelColor destclr = levelProto( level, type_level, *color );
  destclr.clear_gradient();
  for ( int i = 0, sz = lvlclr.gradient_size(); i < sz; ++i ) {
    proto::ColorGrad* grd = destclr.add_gradient();
    grd->CopyFrom( lvlclr.gradient(i) );
  }
//  destclr.mutable_gradient()->CopyFrom( lvlclr.gradient() );
  setLevelProto( level, type_level, destclr, color );
}

void GradientParams::setGradStep( int level, int type_level, float step, proto::FieldColor* color )
{
  proto::LevelColor lvlclr = levelProto( level, type_level, *color );
  lvlclr.set_step_grad(step);
  setLevelProto( level, type_level, lvlclr, color );
}

void GradientParams::setGradColor( const TColorGradList& grad, proto::FieldColor* color )
{
  proto::LevelColor lvlclr = gradlist2gradientcolor( grad, 0, 0 );
  color->clear_def_grad();
  for ( int i = 0, sz = lvlclr.gradient_size(); i < sz; ++i ) {
    proto::ColorGrad* grd = color->add_def_grad();
    grd->CopyFrom( lvlclr.gradient(i) );
  }
//  color->mutable_def_grad()->CopyFrom( lvlclr.gradient() );
}

void GradientParams::setGradStep( float step, proto::FieldColor* color )
{
  color->set_step_grad(step);
}


void GradientParams::loadParamsFromDir()
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
  QMap<QString, meteo::map::proto::FieldColors> setts;
  bool res = TProtoText::loadProtosFromDirectory( path_, &setts );
  if ( false == res ) {
    error_log << QObject::tr("Не удалось загрузить файлы из директории %1")
      .arg(path_);
    return;
  }
  QMapIterator< QString, meteo::map::proto::FieldColors> it(setts);
  while ( true == it.hasNext() ) {
    it.next();
    params_.MergeFrom( it.value() );
  }
}

}
}
