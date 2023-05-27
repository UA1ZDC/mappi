#include "options.h"

#include <iostream>

#include <cross-commons/debug/tlog.h>

using namespace commons;

ArgParser* ArgParser::instance_ = 0;

Arg::Arg( const QString& shortname, const QString& longname, bool val )
 : short_(shortname),
  long_(longname),
  needval_(val),
  installed_(false)
{
  ArgParser::instance()->addOption(*this);
}

Arg::Arg( const QString& shortname, bool val )
  : short_(shortname),
  needval_(val),
  installed_(false)
{
  ArgParser::instance()->addOption(*this);
}

bool Arg::operator==( const Arg& arg ) const
{
  if ( true == this->isEmpty() && true == arg.isEmpty() ) {
    return true;
  }
  if ( true == this->isEmpty() || true == arg.isEmpty() ) {
    return false;
  }
  if ( false == short_.isEmpty() ) {
    if ( short_ == arg.short_ || short_ == arg.long_ ) {
      return true;
    }
  }
  if ( false == long_.isEmpty() ) {
    if ( long_ == arg.short_ || long_ == arg.long_ ) {
      return true;
    }
  }
  return false;
}

void Arg::setValue( const QString& val )
{
  if ( true == val.isEmpty() ) {
    return;
  }
  QStringList list = val.split(',', QString::SkipEmptyParts );
  for ( int i = 0, sz = list.size(); i < sz; ++i ) {
    values_ << list[i].simplified();
  }
}

QString Arg::help() const {
  QString h;
  if ( false == valid() ) {
    return h;
  }
  if ( false == short_.isEmpty() && false == long_.isEmpty() ) {
    h = long_ + QObject::tr(" или ") + short_;
  }
  else if ( false == short_.isEmpty() ) {
    h = short_;
  }
  else {
    h = long_;
  }

  if ( true == needval_ ) {
    h += QObject::tr(" <значение>");
  }
  return h;
}

std::string ArgParser::paramhelp( const Arg& arg, const char* descr )
{
  Q_UNUSED(descr);
  QString str = QObject::tr("    %1")
      .arg( arg.help() ).leftJustified(40);
  return (str+descr).toStdString();
}

ArgParser::ArgParser()
{
}

bool ArgParser::parse( int argc, char* argv[] )
{
  for ( int i = 1; i < argc; ++i ) {
    if ( false == hasOption( argv[i] ) ) {
      std::cerr << QObject::tr("Неизветный параметр <%1>\n").arg(argv[i]).toLocal8Bit().constData();
      return false;
    }
    Arg& arg = option( argv[i] );
    if ( true == arg.needval() ) {
      if ( i+1 >= argc ) {
        std::cerr << QObject:: tr("Неверно задано значение параметра <%1>\n").arg(argv[i]).toLocal8Bit().constData();
        return false;
      }
      arg.setValue(argv[++i]);
      arg.setInstalled();
    }
    else {
      arg.setInstalled();
    }
  }
  return true;
}

void ArgParser::clear(){
  options_.clear();
  return;
}

Arg& ArgParser::at( const Arg& arg )
{
  int indx = options_.indexOf(arg);
  if ( -1 == indx ) {
    std::cerr << QObject::tr("Не найден параметр [%1:%2]\n")
      .arg(arg.shortname())
      .arg(arg.longname()).toLocal8Bit().constData();
    return empty_;
  }
  return options_[indx];
}

bool ArgParser::hasValues( const Arg& arg ) const
{
  int indx = options_.indexOf(arg);
  if ( -1 == indx ) {
    return false;
  }
  return options_[indx].hasValues();
}

bool ArgParser::installed( const Arg& arg ) const
{
  int indx = options_.indexOf(arg);
  if ( -1 == indx ) {
    return false;
  }
  return options_[indx].installed();
}

ArgParser* ArgParser::instance()
{
  if ( 0 == instance_ ) {
    instance_ = new ArgParser;
  }
  return instance_;
}

QList<Arg> ArgParser::installedOptions() const
{
  QList<Arg> list;
  for ( const auto a : options_ ) {
    if ( true == a.installed() ) {
      list.append(a);
    }
  }
  return list;
}

void ArgParser::addOption( const Arg& opt )
{
  if ( true == options_.contains(opt) ) {
    return;
    std::cerr << QObject::tr("Аргумент [%1:%2] уже добавлен")
      .arg( opt.shortname() )
      .arg( opt.longname() ).toUtf8().constData() << '\n';
    return;
  }
  if ( false == opt.valid() ) {
    std::cerr << QObject::tr("аргумент [%1:%2]")
      .arg( opt.shortname() )
      .arg( opt.longname() ).toUtf8().constData() << '\n';
    return;
  }
  options_.append(opt);
}

bool ArgParser::hasOption( const char* arg ) const
{
  for ( int i = 0, sz = options_.size(); i < sz; ++i ) {
    if ( arg == options_[i].shortname() || arg == options_[i].longname() ) {
      return true;
    }
  }
  return false;
}

Arg& ArgParser::option( const char* arg )
{
  for ( int i = 0, sz = options_.size(); i < sz; ++i ) {
    if ( arg == options_[i].shortname() || arg == options_[i].longname() ) {
      return options_[i];
    }
  }
  return empty_;
}
