#include "punchrules.h"

#include <qfont.h>

#include <meteo/commons/global/common.h>
#include <cross-commons/debug/tlog.h>

namespace meteo {
namespace map {

QString stringFromRuleValue( float val, const QString& id, const puanson::proto::Puanson& punch, const QString& code )
{
  puanson::proto::Id idproto;
  idproto.set_name( id.toStdString() );
  return stringFromRuleValue( val, idproto, punch, code );
}

QString stringFromRuleValue( float val, const puanson::proto::Id& id, const puanson::proto::Puanson& punch, const QString& code )
{
  for ( int i = 0, sz = punch.rule_size(); i < sz; ++i ) {
    const puanson::proto::CellRule& rule = punch.rule(i);
    if ( rule.id().name() == id.name() ) {
      return stringFromRuleValue( val, rule, code );
    }
  }
  if ( true == MnMath::isEqual( BAD_METEO_ELEMENT_VAL, val ) ) {
    return "";
  }
  return QString::number( val, 'f', 1 );
}

QString stringFromRuleValue( float val, const puanson::proto::CellRule& rule, const QString& code )
{
  if ( true == MnMath::isEqual( BAD_METEO_ELEMENT_VAL, val ) && true == rule.has_absent_rule() ) {
    QString str = QObject::tr( QByteArray::fromHex( QByteArray( rule.absent_rule().patternba().data(), rule.absent_rule().patternba().size() ) ) );
    return str;
  }
  if ( true == MnMath::isEqual( BAD_METEO_ELEMENT_VAL, val ) && false == rule.has_symbol() && false == rule.symbol().codedigit() ) {
    return "";
  }
  switch ( rule.type() ) {
    case puanson::proto::kNumber:
      if ( false == rule.has_number() ) {
//        error_log << QObject::tr("Правило форматирования числа дескриптора %1 не установлено")
//          .arg( QString::fromStdString( rule.id().Utf8DebugString() ) );
      }
      return stringFromRuleValue( val, rule.number() );
      break;
    case puanson::proto::kSymbol:
      if ( false == rule.has_symbol() ) {
        warning_log << QObject::tr("Правило текста для значения дескриптора %1 не установлено")
          .arg( QString::fromStdString( rule.id().Utf8DebugString() ) );
      }
      return stringFromRuleValue( val, rule.symbol(), code );
      break;
    case puanson::proto::kSymNumber: {
        QString str = stringFromRuleValue( val, rule.symbol(), code );
        if ( 0 == str.size() ) {
          str = stringFromRuleValue( val, rule.number() );
        }
        return str;
      }
      break;
  }
  return QString::number( val, 'f', 1 );
}

QString stringFromRuleValue( float val, const puanson::proto::CellRule::NumbeRule& rule )
{
  if ( true == MnMath::isZero(val ) && true == rule.zerohide() ) {
    return QString();
  }
  val *= rule.factor();
  char sign = ( 0 > val ) ? '-' : '+' ;
  QString strval = QString::number( val, 'f', rule.fracount() );
  strval = strval.simplified();
  QString intstr;
  QString frastr;
  if ( 0 != rule.fracount() ) {
    QStringList list = strval.split(".", QString::SkipEmptyParts );
    if ( 2 != list.size() ) {
      error_log << QObject::tr("Неизвестная ошибка");
      return strval;
    }
    intstr = list[0];
    frastr = list[1];
  }
  else {
    intstr = strval;
  }
  if ( intstr[0] == sign ) {
    intstr = intstr.right( intstr.length() - 1 );
  }
  if ( rule.intcount() < intstr.length() ) {
    switch ( rule.cutorder() ) {
      case puanson::proto::kUpper:
        intstr = intstr.left( rule.intcount() );
        frastr = "";
        break;
      case puanson::proto::kLower:
        intstr = intstr.right( rule.intcount() );
        break;
      default:
        break;
    }
  }

  if ( 0 < rule.fracount() && rule.fracount() < frastr.length() ) {
    frastr = frastr.left( rule.fracount() );
  }
  else if ( 0 > rule.fracount() ) {
    frastr.remove( QRegExp("[0]*$") );
  }

  QString sep;
  switch ( rule.septype() ) {
    case puanson::proto::kComma:
      if ( 0 != frastr.size() ) {
        strval = intstr + "," + frastr;
      }
      else {
        strval = intstr;
      }
      break;
    case puanson::proto::kDot:
      if ( 0 != frastr.size() ) {
        strval = intstr + "." + frastr;
      }
      else {
        strval = intstr;
      }
      break;
    case puanson::proto::kNoSep:
      if ( 0 != frastr.size() ) {
        strval = intstr + frastr;
      }
      else {
        strval = intstr;
      }
      break;
  }
  switch (rule.signview()) {
    case puanson::proto::kMinus:
      if ( 0 > val ) {
        strval = sign + strval;
      }
      break;
    case puanson::proto::kBoth:
      strval = sign + strval;
      break;
    case puanson::proto::kNoSign:
      break;
  }
  if ( false == strval.isEmpty() && true == rule.has_postfix() ) {
    strval += QString::fromStdString( rule.postfix() );
  }
  return strval;
}

QString stringFromRuleValue( float val, const puanson::proto::CellRule::SymbolRule& rule, const QString& code )
{
  if ( true == rule.codedigit() && false == code.isEmpty() && "''" != code && "NULL" != code ) {
    QString loc = code;
    return loc.replace("'", "");
  }
  for ( int i = 0, sz = rule.symbol_size(); i < sz; ++i ) {
    const puanson::proto::CellRule::Symbol& s = rule.symbol(i);
    if ( true == s.has_codeval() ) {
      if ( QString::fromStdString( s.codeval() ) == code ) {
        if ( true == s.has_patternba() ) {
          return QString( QByteArray::fromHex( QByteArray( s.patternba().data(), s.patternba().size() ) ) );
        }
        else if ( true == s.has_pattern() ) {
          return QString::fromStdString( s.pattern() );
        }
        else {
          return QString();
        }
      }
    }
    else if ( s.minval() <= val && s.maxval() >= val ) {
      if ( true == s.has_number() ) {
        return stringFromRuleValue( val, s.number() );
      }
      else if ( true == s.has_patternba() ) {
        return QString( QByteArray::fromHex( QByteArray( s.patternba().data(), s.patternba().size() ) ) );
      }
      else if ( true == s.has_pattern() ) {
        return QString::fromStdString( s.pattern() );
      }
      else {
        return QString();
      }
//      return QString::fromStdString( s.pattern() );
    }
  }
//  error_log << QObject::tr("Значение %1 не описано правилом %2")
//    .arg( val, 0, 'f' )
//    .arg( QString::fromStdString(rule.Utf8DebugString()) );
  return QString();
}

QFont fontFromRuleValue( float val, const puanson::proto::Id& id, const puanson::proto::Puanson& punch )
{
  for ( int i = 0, sz = punch.rule_size(); i < sz; ++i ) {
    const puanson::proto::CellRule& rule = punch.rule(i);
    if ( rule.id().name() == id.name() ) {
      return fontFromRuleValue( val, rule );
    }
  }
//  warning_log << QObject::tr("Не найдено правило для дескриптора %1")
//    .arg( QString::fromStdString( id.Utf8DebugString() ) );
  return QFont();
}

QFont fontFromRuleValue( float val, const puanson::proto::CellRule& rule )
{
  if (MnMath::isEqual( BAD_METEO_ELEMENT_VAL, val) && true == rule.has_absent_rule() ) {
    QFont fnt = font2qfont( rule.absent_rule().font() );
    return fnt;
  }
  switch ( rule.type() ) {
    case puanson::proto::kNumber:
      if ( false == rule.has_number() ) {
//        warning_log << QObject::tr("Правило шрифта для числа дескриптора %1 не установлено")
//          .arg( QString::fromStdString( rule.id().Utf8DebugString() ) );
      }
      return fontFromRuleValue( val, rule.number() );
      break;
    case puanson::proto::kSymbol:
      if ( false == rule.has_symbol() ) {
        warning_log << QObject::tr("Правило текста для значения дескриптора %1 не установлено")
          .arg( QString::fromStdString( rule.id().Utf8DebugString() ) );
      }
      return fontFromRuleValue( val, rule.symbol() );
      break;
    case puanson::proto::kSymNumber: {
        QString str = stringFromRuleValue( val, rule.symbol() );
        if ( 0 != str.size() ) {
          return fontFromRuleValue( val, rule.symbol() );
        }
        else {
          return fontFromRuleValue( val, rule.number() );
        }
      }
      break;
  }
  return QFont();
}

QFont fontFromRuleValue( float val, const puanson::proto::CellRule::NumbeRule& rule )
{
  Q_UNUSED(val);
  QFont font;
  if ( true == rule.has_font() ) {
    font = font2qfont( rule.font() );
  }
  return font;
}

QFont fontFromRuleValue( float val, const puanson::proto::CellRule::SymbolRule& rule )
{
  QFont font;
  for ( int i = 0, sz = rule.symbol_size(); i < sz; ++i ) {
    const puanson::proto::CellRule::Symbol& s = rule.symbol(i);
    if ( s.minval() <= val && s.maxval() >= val ) {
      if ( true == s.has_number() ) {
        font = fontFromRuleValue( val, s.number() );
      }
      else {
        font = font2qfont( s.font() );
      }
      return font;
    }
  }
//  error_log << QObject::tr("Значение %1 не описано правилом %2")
//    .arg( val, 0, 'f' )
//    .arg( QString::fromStdString(rule.Utf8DebugString()) );
  return font;
}

QColor colorFromRuleValue( float val, const puanson::proto::Id& id, const puanson::proto::Puanson& punch )
{
  for ( int i = 0, sz = punch.rule_size(); i < sz; ++i ) {
    const puanson::proto::CellRule& rule = punch.rule(i);
    if ( rule.id().name() == id.name() ) {
      return colorFromRuleValue( val, rule );
    }
  }
//  warning_log << QObject::tr("Не найдено правило для дескриптора %1")
//    .arg( QString::fromStdString( id.Utf8DebugString() ) );
  return QColor();
}

QColor colorFromRuleValue( float val, const puanson::proto::CellRule& rule )
{
  if ( MnMath::isEqual(BAD_METEO_ELEMENT_VAL, val) && true == rule.has_absent_rule() ) {
    QColor clr = QColor::fromRgba( rule.absent_rule().color() );
    return clr;
  }
  switch ( rule.type() ) {
    case puanson::proto::kNumber:
      if ( false == rule.has_number() ) {
//        warning_log << QObject::tr("Правило цвета для числа дескриптора %1 не установлено")
//          .arg( QString::fromStdString( rule.id().Utf8DebugString() ) );
      }
      return colorFromRuleValue( val, rule.number() );
      break;
    case puanson::proto::kSymbol:
      if ( false == rule.has_symbol() ) {
        warning_log << QObject::tr("Правило текста для значения дескриптора %1 не установлено")
          .arg( QString::fromStdString( rule.id().Utf8DebugString() ) );
      }
      return colorFromRuleValue( val, rule.symbol() );
      break;
    case puanson::proto::kSymNumber: {
        QString str = stringFromRuleValue( val, rule.symbol() );
        if ( 0 != str.size() ) {
          return colorFromRuleValue( val, rule.symbol() );
        }
        else {
          return colorFromRuleValue( val, rule.number() );
        }
      }
      break;
  }
  return QColor();
}

QColor colorFromRuleValue( float val, const puanson::proto::CellRule::NumbeRule& rule )
{
  QColor clr;
  if ( 0 <= val ) {
    if ( true == rule.has_pluscolor() ) {
      clr = QColor::fromRgba( QRgb(rule.pluscolor()) );
    }
  }
  else {
    if ( true == rule.has_minuscolor() ) {
      clr = QColor::fromRgba( QRgb(rule.minuscolor()) );
    }
    else if ( true == rule.has_pluscolor() ) {
      clr = QColor::fromRgba( QRgb(rule.pluscolor()) );
    }
  }
  return clr;
}

QColor colorFromRuleValue( float val, const puanson::proto::CellRule::SymbolRule& rule )
{
  QColor clr;
  for ( int i = 0, sz = rule.symbol_size(); i < sz; ++i ) {
    const puanson::proto::CellRule::Symbol& s = rule.symbol(i);
    if ( s.minval() <= val && s.maxval() >= val ) {
      if ( true == s.has_number() ) {
        clr = colorFromRuleValue( val, s.number() );
      }
      else {
        clr = QColor::fromRgba( s.color() );
      }
      return clr;
    }
  }
//  error_log << QObject::tr("Значение %1 не описано правилом %2")
//    .arg( val, 0, 'f' )
//    .arg( QString::fromStdString(rule.Utf8DebugString()) );
  return clr;
}

QMap<QString, QString> stringForCloud( const meteo::surf::MeteoDataProto* data, const puanson::proto::Puanson& punch )
{
  if ( nullptr == data) {
    error_log << "Метеопараметры или шаблон пуансона отсутствуют";
    return QMap<QString,QString>();
  }
  QStringList descrList{"C","CH","CM","CL","Cn","h","N","Nh"};
  QMap<std::string, Param> paramMap;
  for ( int i = 0, sz = data->param_size(); i < sz; ++i) {
    if (!descrList.contains(QString::fromStdString(data->param(i).descrname()))) {
      continue;
    }
    Param param;
    if ( paramMap.contains(data->param(i).uuid())) {
      param = paramMap[data->param(i).uuid()];
    }
    else {
      param.setUuid(data->param(i).uuid());
    }
    param.appendParam(QString::fromStdString(data->param(i).descrname()),data->param(i).value());
    paramMap[param.uuid()] = param;
  }
  ParamList paramList;
  paramList.init(punch);
  for ( auto& key : paramMap.keys()) {
    paramList.append(paramMap[key]);
  }
  return paramList.getMap();
}

void Param::appendParam(const QString &param, const float &value)
{
  if ( param == "h" && value == 0 ) {
    return;
  }
  params_.insert(param, value);
}

void ParamList::append(const Param &param)
{
  if ( QList<Param>::isEmpty() ) {
     QList<Param>::append(param);
     return;
  }

  if ( param.contains("h") ) {
    for ( int i = 0, sz = QList<Param>::size(); i < sz; ++i  ) {
      if ( QList<Param>::at(i).contains("h") ) {
        if ( param["h"] == QList<Param>::at(i)["h"] ) {
          if ( param == QList<Param>::at(i) ) {
            return;
          }
          else {
            continue;
          }
        }
        else if ( param["h"] > QList<Param>::at(i)["h"] ) {
          QList<Param>::insert(i, param);
          return;
        }
      }
      else {
        QList<Param>::insert(i, param);
        return;
      }
    }
  }
  else {
    if ( QList<Param>::last().contains("h") ) {
      QList<Param>::append(param);
      return;
    }
    for ( int i = QList<Param>::size() - 1; i >= 0; --i  ) {
      if ( param == QList<Param>::at(i) ) {
        return;
      }
      if (QList<Param>::at(i).contains("h")) {
        QList<Param>::insert(i, param);
        return;
      }
    }
  }
}

void ParamList::init( const puanson::proto::Puanson& punch )
{
  punch_.CopyFrom(punch);
  for ( int i = 0, isz = punch_.rule_size(); i < isz; ++i ) {
    QString param = QString::fromStdString(punch_.rule(i).id().name());
    paramsRules_[param] = punch_.rule(i);
  }
}

QString ParamList::getH()
{
  QStringList out;
  for ( auto& param : *this ) {
    if ( param.contains("h") ) {
      auto p = stringFromRuleValue(param["h"], paramsRules_["h"]);
      out << p;
    }
  }
  return out.join("/");
}

QString ParamList::getRawH()
{
  QStringList out;
  for ( auto& param : *this ) {
    if ( param.contains("h") ) {
      out << QString::number(param["h"]);
    }
  }
  return out.join("/");
}

QMap<QString, QString> ParamList::getMap()
{
  QMap<QString, QString> map;
  map["C"] = getC();
  map["CH"] = getCH();
  map["CM"] = getCM();
  map["CL"] = getCL();
  map["N"] = getN();
  map["h"] = getH();
  map["rawC"] = getRawC();
  map["rawN"] = getRawN();
  map["rawH"] = getRawH();
  return map;
}

QString ParamList::getCH()
{
  QStringList out;
  for ( auto& param : *this ) {
    auto p = stringFromRuleValue(param["CH"], paramsRules_["CH"]);
    if ( !p.isEmpty() ) {
      out << p;
    }
    if ( param.contains("h") && param["h"] >= 6000 ) {
      p = stringFromRuleValue(param["Cn"], paramsRules_["Cn"]);
      out << p;
    }
  }
  return out.join("/");
}

QString ParamList::getCM()
{
  QStringList out;
  for ( auto& param : *this ) {
    auto p = stringFromRuleValue(param["CM"], paramsRules_["CM"]);
    if ( !p.isEmpty() ) {
      out << p;
    }
    if ( !param.contains("h") || ( param["h"] < 6000  && param["h"] >= 2500)) {
      p = stringFromRuleValue(param["Cn"], paramsRules_["Cn"]);
      out << p;
    }
  }
  return out.join("/");
}

QString ParamList::getCL()
{
  QStringList out;
  for ( auto& param : *this ) {
    auto p = stringFromRuleValue(param["CL"], paramsRules_["CL"]);
    if ( !p.isEmpty() ) {
      out << p;
    }
    if ( param.contains("h") && param["h"] < 2500 ) {
      p = stringFromRuleValue(param["Cn"], paramsRules_["Cn"]);
      out << p;
    }
  }
  return out.join("/");
}

QString ParamList::getN()
{
  QStringList out;
  QString N;
  for ( auto& param : *this ) {
    auto p = stringFromRuleValue(param["Nh"], paramsRules_["Nh"]);
    if ( !p.isEmpty() ) {
      out << p;
    }
    if ( param.contains("N")) {
      p = stringFromRuleValue(param["N"], paramsRules_["N"]);
      out.prepend(p);
    }
  }
  return out.join("/");
}

QString ParamList::getRawN()
{
  QStringList out;
  QString N;
  for ( auto& param : *this ) {
    auto p = QString::number(param["Nh"]);
    if ( !p.isEmpty() ) {
      out << p;
    }
    if ( param.contains("N")) {
      p = QString::number(param["N"]);
      out.prepend(p);
    }
  }
  return out.join("/");
}

QString ParamList::getC()
{
  QStringList out;
  QStringList ch;
  QStringList cm;
  QStringList cl;
  for ( auto& param : *this ) {
    QString p;
    if ( param.contains("CH") ) {
      p = stringFromRuleValue(param["CH"], paramsRules_["CH"]);
      ch << p;
    }
    if ( param.contains("CM") ) {
      p = stringFromRuleValue(param["CM"], paramsRules_["CM"]);
      cm << p;
    }
    if ( param.contains("CM") ) {
      p = stringFromRuleValue(param["CL"], paramsRules_["CL"]);
      cl << p;
    }
    if ( param.contains("Cn") ) {
      p = stringFromRuleValue(param["Cn"], paramsRules_["Cn"]);
      if ( param.contains("h") ) {
//        debug_log << "h=" << param["h"] << " Cn = " << p;
//        debug_log << param;
        if (param["h"] < 2500) {
          cl << p;
        }
        else if (param["h"] < 6000) {
          cm << p;
        }
        else {
          ch << p;
        }
      }
      else {
        cm << p;
      }
   }
  }
  for ( auto& str : QList<QString>{ch.join("/"), cm.join("/"), cl.join("/")} ) {
    if ( !str.isEmpty() ) {
      out << str;
    }
  }
  return out.join("/");
}

QString ParamList::getRawC()
{
  QStringList out;
  for ( auto& param : *this ) {
    auto p = QString::number(param["CH"]);
    if ( !p.isEmpty() ) {
      out << p;
    }
    p = QString::number(param["CM"]);
    if ( !p.isEmpty() ) {
      out << p;
    }
    p = QString::number(param["CL"]);
    if ( !p.isEmpty() ) {
      out << p;
    }
    p = QString::number(param["Cn"]);
    if ( !p.isEmpty() ) {
      out << p;
   }
  }
  return out.join("/");
}

bool Param::operator==(const Param &param) const
{
  bool result = true;
  result = (params_["h"] == param.params_["h"]) && (params_["Nh"] == param.params_["Nh"]);
  return result;
}

TLog& operator<<(TLog& out, const Param& p)
{
  for (auto& key: p.params_.keys() ) {
      out << "key = " << key << " param " << p.params_[key] << "\n";
  }
  return out;
}

}
}
