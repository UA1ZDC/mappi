#include "weatherfont.h"

#include <qdir.h>
#include <qfontdatabase.h>

#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/paths.h>

namespace meteo {

template<> internal::WeatherFont* WeatherFont::_instance = 0;

namespace internal {

WeatherFont::WeatherFont()
{
  loadFonts();
}

WeatherFont::~WeatherFont()
{
}

void WeatherFont::loadFonts()
{
  QDir dir( MnCommon::sharePath("meteo") + "/fonts/" );
  if ( false == dir.exists() ) {
    error_log << QObject::tr("Директория со шрифтами не существет = %1")
      .arg( dir.absolutePath() );
    return;
  }
  QStringList files = dir.entryList( QStringList() << "*.ttf" );
  if ( 0 == files.size() ) {
    error_log << QObject::tr("Шрифты не найдены в директории %1")
      .arg( dir.absolutePath() );
    return;
  }
  for ( int i = 0, sz = files.size(); i < sz; ++i ) {
    QString filename = dir.absoluteFilePath(files[i]);
    int res = QFontDatabase::addApplicationFont(filename);
    if ( -1 == res ) {
      error_log << QObject::tr("Не удалось загрузить шрифт из файла %1")
        .arg(filename);
      continue;
    }
  }

  QFontDatabase fdb;
  QStringList families = fdb.families();

  if ( true == families.contains(kWthr0) ) {
    QFont fnt( kWthr0, 12, QFont::Normal, false );
    specialfonts_.insert( kWthr0, fnt );
    kFontWthr0 = fnt;
  }
  if ( true == families.contains(kWthr1) ) {
    QFont fnt( kWthr1, 12, QFont::Normal, false );
    specialfonts_.insert( kWthr1, fnt );
    kFontWthr1 = fnt;
  }
  if ( true == families.contains(kSynop) ) {
    QFont fnt( kSynop, 12, QFont::Normal, false );
    specialfonts_.insert( kSynop, fnt );
    kFontSynop = fnt;
  }
  if ( true == families.contains(kWths) ) {
    QFont fnt( kWths, 12, QFont::Normal, false );
    specialfonts_.insert( kWths, fnt );
    kFontWths = fnt;
  }
  if ( true == families.contains(kGms) ) {
    QFont fnt( kGms, 12, QFont::Normal, false );
    specialfonts_.insert( kGms, fnt );
    kFontWths = fnt;
  }

  kFontHelveticaBold = QFont( kHelvetica, 10, QFont::Bold, false );
  kFontHelvetica = QFont( kHelvetica, 14, QFont::Normal, false );
  specialfonts_.insert( kHelveticaBold, kFontHelveticaBold );
  specialfonts_.insert( kHelvetica, kFontHelvetica );
}

void WeatherFont::addSpecial( const QString& name, const QFont& font )
{
  if ( true == specialfonts_.contains(name) ) {
    warning_log << QObject::tr("Специальный шрифт с именем %1 уже установлен и будет заменен");
  }
  specialfonts_.insert( name, font );
}

void WeatherFont::addSpecial( const QFont& font )
{
  addSpecial( font.family(), font );
}

}
}
