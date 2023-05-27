#ifndef COMMONS_UI_MAP_DATAEXCHANGE_METASXF_H
#define COMMONS_UI_MAP_DATAEXCHANGE_METASXF_H

#include <cross-commons/singleton/tsingleton.h>
#include <iostream>

class QFont;
class QString;

namespace meteo {
  namespace sxf {
    class SxfCodes;
    class FontCode;
  } // sxf
}

namespace meteo {
  namespace sxf {
    namespace inner {
      class MetaSxf {
      public:
	MetaSxf();
	~MetaSxf();

	int windCode(int ff, int length) const;
	float wind(int code) const;
	const ::meteo::sxf::FontCode& getFont(const QFont& font) const;
	float mapScale(int sxfScale);
	int ornamentCode(const std::string& name);
	std::string ornamentName(int code);
	QString rscfile();
	int meteoCode(const QString& name, const QString& value) const;

	int meteoType(int code) const;
	int meteoType(const QString& name) const;
	

      private:	
	bool init();

      private:
	::meteo::sxf::SxfCodes* codes_;
    };
    }
    
    typedef TSingleton<inner::MetaSxf> MetaSxf;
  }
}

#endif
