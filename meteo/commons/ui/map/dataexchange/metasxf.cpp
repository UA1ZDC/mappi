#include "metasxf.h"

#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/paths.h>
#include <commons/mathtools/mnmath.h>
#include <meteo/commons/proto/sxfcodes.pb.h>
#include <commons/textproto/tprototext.h>

#include <qfont.h>

template<> meteo::sxf::inner::MetaSxf* meteo::sxf::MetaSxf::_instance = 0;

#define SXF_CODES_FILE MnCommon::etcPath("meteo") + "/sxfcodes.conf"

using namespace meteo;
using namespace sxf;

inner::MetaSxf::MetaSxf():
  codes_(nullptr)
{
  init();
}

inner::MetaSxf::~MetaSxf()
{
  if (nullptr != codes_) {
    delete codes_;
    codes_ = nullptr;
  }
}

bool inner::MetaSxf::init()
{
  codes_ = new meteo::sxf::SxfCodes;
  if (nullptr == codes_) return false;

  QFile file(SXF_CODES_FILE);
  if ( !file.open(QIODevice::ReadOnly) ) {
    error_log << QObject::tr("Ошибка при загрузке конфигурации '%1'").arg(SXF_CODES_FILE);
    return EXIT_FAILURE;
  }

  QString text = QString::fromUtf8(file.readAll());
  file.close();

  if ( !TProtoText::fillProto(text, codes_) ) {
    error_log << QObject::tr("Ошибка в структуре файла конфигурации");
    return false;
  }

  //  var(codes_->DebugString());

  return true;
}

  //! Код ветра в классификаторе
int inner::MetaSxf::windCode(int ff, int length) const
{
  int code = -1;

  if (nullptr == codes_ || 0 == ff) return code;

  for (int idx = 0; idx < codes_->objs_size(); idx++) {
    if ((codes_->objs(idx).type() == sxf::kWind && length > 10) ||
	(codes_->objs(idx).type() == sxf::kWindShort && length <= 10)) {
      if (codes_->objs(idx).code_size() != codes_->objs(idx).value_size()) {
	error_log << QObject::tr("Ошибка в структуре файла конфигурации");
	break;
      }
      for (int i = 0; i < codes_->objs(idx).value_size(); i++) {//TODO значения теоретически отсортированы, можно поиск
	//    побыстрее линейного
	if (ff <= codes_->objs(idx).value(i)) {
	  code = codes_->objs(idx).code(i);
	  //	  var(code);
	  break;
	}
      }
      
      break;
    }
  }

  return code;
}


//! Значение ветра по коду в классификаторе
float inner::MetaSxf::wind(int code) const
{
  float ff = -1;

  if (nullptr == codes_) {
    return ff;
  }
  
  for (int idx = 0; idx < codes_->objs_size(); idx++) {
    if (codes_->objs(idx).type() == sxf::kWind) {
      if (codes_->objs(idx).code_size() != codes_->objs(idx).value_size()) {
	error_log << QObject::tr("Ошибка в структуре файла конфигурации");
	break;
      }
      for (int i = 0; i < codes_->objs(idx).value_size(); i++) {
	if (code == codes_->objs(idx).code(i)) {
	  ff = codes_->objs(idx).value(i);
	  break;
	}
      }
      
      break;
    }
  }

  return ff;  
}

//! Параметры шрифта в классификаторе
const sxf::FontCode& inner::MetaSxf::getFont(const QFont& font) const
{
  if (nullptr == codes_) {
    return sxf::FontCode::default_instance();
  }
  
  for (int idx = 0; idx < codes_->fonts_size(); idx++) {
    if (QString::fromStdString(codes_->fonts(idx).family()) == font.family()) {
      return codes_->fonts(idx);
    }
  }
  
  return sxf::FontCode::default_instance();
}

//! масштаб карты по значению масштаба в sxf
float inner::MetaSxf::mapScale(int sxfScale)
{
  for (int idx = 0; idx < codes_->scale_size(); idx++) {
    if (codes_->scale(idx).sxf() == sxfScale) {
      return codes_->scale(idx).map();
    }
  }

  return -1;
}

//! Код объекта в классификаторе по названию орнамента
int inner::MetaSxf::ornamentCode(const std::string& name)
{
  int code = -1;
  
  if (nullptr == codes_) {
    return code;
  }
  
  for (int idx = 0; idx < codes_->ornament_size(); idx++) {
    if (codes_->ornament(idx).name() == name) {
      return codes_->ornament(idx).code();
    }
  }
  
  return code;
}

//! Код объекта в классификаторе по названию орнамента
std::string inner::MetaSxf::ornamentName(int code)
{
  std::string name;
  
  if (nullptr == codes_) {
    return name;
  }
  
  for (int idx = 0; idx < codes_->ornament_size(); idx++) {
    if (codes_->ornament(idx).code() == code) {
      return codes_->ornament(idx).name();
    }
  }
  
  return name;
}

QString inner::MetaSxf::rscfile()
{
  if (nullptr == codes_) {
    return QString();
  }

  return QString::fromStdString(codes_->rscfile());
}

//! Классификационный код по имени метеопараметра и его символу
int inner::MetaSxf::meteoCode(const QString& name, const QString& avalue) const
{
  int code = -1;
  if (nullptr == codes_) return code;
  int type = meteoType(name);
  if (kUnkClassif == type) return code;

  std::string value = avalue.toStdString();

  for (int idx = 0; idx < codes_->objs_size(); idx++) {
    if (codes_->objs(idx).type() == type) {
      if (codes_->objs(idx).code_size() != codes_->objs(idx).text_size()) {
	error_log << QObject::tr("Ошибка в структуре файла конфигурации");
	break;
      }
      for (int i = 0; i < codes_->objs(idx).text_size(); i++) {
	//    побыстрее линейного
	if (value ==  codes_->objs(idx).text(i)) {
	  code = codes_->objs(idx).code(i);
	  //	  var(code);
	  break;
	}
      }
      
      break;
    }
  }

  return code;
}

int inner::MetaSxf::meteoType(const QString& name) const
{
  int type = kUnkClassif;

  if (name == "N") {
    type = kNMeteo;
  } else if (name == "a") {
    type = kaMeteo;
  } else if (name == "CL" || name == "CM" || name == "CH") {
    type = kCMeteo;
  } else if (name == "w" || name == "phen") {
    type = kwMeteo;
  } else if (name == "W1") {
    type = kW1Meteo;
  } else if (name == "W2") {
    type = kW2Meteo;
  } else if (name == "B" || name == "storm" || name == "level_type" || name == "zn" || 
	     name == "h_v" || name == "Ic") {
    type = kSign;
  }
 
  return type;
}

int inner::MetaSxf::meteoType(int code) const
{
  for (int idx = 0; idx < codes_->objs_size(); idx++) {
    for (int i = 0; i < codes_->objs(idx).text_size(); i++) {
      if (code == codes_->objs(idx).code(i)) {
	return codes_->objs(idx).type();
      }
    }
  }
 
  return 0;
}
