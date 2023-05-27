#ifndef METEO_COMMONS_FONT_WEATHERFONT_H
#define METEO_COMMONS_FONT_WEATHERFONT_H

#include <qstring.h>
#include <qfont.h>
#include <qmap.h>
#include <qlist.h>
#include <qrect.h>

#include <cross-commons/singleton/tsingleton.h>
#include <commons/meteo_data/meteo_data.h>
#include <commons/geobasis/geopoint.h>

namespace meteo {

const QString kHelvetica        ("helvetica");
const QString kWthr0            ("wthrNew0");
const QString kWthr1            ("wthrNew1");
const QString kSynop            ("Synop");
const QString kWths             ("wths");
const QString kHelveticaBold    ("helvetica Bold");
const QString kGms              ("gms");

const int kPrintFontSizeCrdnet          = 5;
const int kPrintFontSizeCity            = 8;
const int kPrintFontSizeStation         = 5;
const int kPrintFontSizeStationHydro    = 5;
const int kPrintFontSizeData            = 8;
const int kPrintFontSizeExtremum        = 16;

const int kDrawFontSizeCity             = 9;
const int kDrawFontSizeStation          = 6;
const int kDrawFontSizeStationHydro     = 6;
const int kDrawFontSizeExtremum         = 20;
const int kDrawFontSizePuanson          = 35;

const QRect kPuansonSize                = QRect(0,0,280,192);
namespace internal {

class WeatherFont
{
  private:
    WeatherFont();
    ~WeatherFont();

  public:
    QFont kFontHelvetica;
    QFont kFontSymbol;
    QFont kFontWthr0;
    QFont kFontWthr1;
    QFont kFontSynop;
    QFont kFontWths;
    QFont kFontHelveticaBold;

    void loadFonts();

    void addSpecial( const QString& name, const QFont& font );
    void addSpecial( const QFont& font );
    const QMap< QString, QFont > specialfonts() const { return specialfonts_; }

  private:
    QMap< QString, QFont > specialfonts_;

  friend class TSingleton<internal::WeatherFont>;
};

}

typedef TSingleton<internal::WeatherFont> WeatherFont;

}

#endif
