#include "conditionstring.h"
#include <qregexp.h>
#include <meteo/commons/global/global.h>
#include <qobject.h>
#include <cross-commons/debug/tlog.h>
#include <meteo/commons/global/global.h>
#include <sql/dbi/dbiquery.h>

using namespace meteo::proto;

namespace meteo {
namespace internal {

const QString ConditionString::conditionNames[conditionTypesCount] = {  
  QObject::tr("Равно"),
  QObject::tr("Не равно"),
  QObject::tr("Содержит"),
  QObject::tr("Не содержит"),
  QObject::tr("Пустая строка"),
  QObject::tr("Не пустая строка")
};

ConditionString::ConditionString(const QString& fieldName, meteo::ConditionString::ConditionMatchType matchType,bool translit, Qt::CaseSensitivity caseSensitivity, const QString& value):
  GenericConditionPrivateBase(fieldName, fieldName),
  matchType_(matchType),
  matchValue_(value),
  transliterate_(translit),
  caseSensitivity_(caseSensitivity)
{

}

ConditionString::ConditionString(const QString& fieldName, const QString& fieldDisplayName, meteo::ConditionString::ConditionMatchType matchType, bool translit, Qt::CaseSensitivity caseSensitivity, const QString& value):
  GenericConditionPrivateBase (fieldName, fieldDisplayName),
  matchType_(matchType),
  matchValue_(value),
  transliterate_(translit),
  caseSensitivity_(caseSensitivity)
{

}

ConditionString::~ConditionString()
{
}

bool ConditionString::isMatch(const QVariant &buf) const
{
  if ( false == buf.canConvert<QString>() ){
    warning_log << QObject::tr("Не удается преобразовать данные к типу %1");
    return false;
  }

  auto original = buf.toString();

  static const QString& containsPattern = QObject::tr("^.*%1.*$");
  const QString &value = ( true == this->transliterate_ )?
        meteo::global::kTranslitFunc(original) :
        original;
  const QString &matchValue = ( true == this->transliterate_ ) ? meteo::global::kTranslitFunc( this->matchValue_ ) : this->matchValue_;

  switch (this->matchType_) {  
  case meteo::ConditionString::kContains:{
    QRegExp regexp(containsPattern.arg(matchValue), this->caseSensitivity_);
    return ( -1 != regexp.indexIn(value) );
  }
  case meteo::ConditionString::kNotContains:{
    QRegExp regexp(containsPattern.arg(matchValue), this->caseSensitivity_);
    return ( -1 == regexp.indexIn(value) );
  }

  case meteo::ConditionString::kMatch:{
    return ( 0 == matchValue.compare(value, caseSensitivity_) );
  }
  case meteo::ConditionString::kNotMatch:{
    return ( 0 != matchValue.compare(value, caseSensitivity_) );
  }
  case meteo::ConditionString::kNotNUll:{
    return ( false == value.isEmpty() );
  }
  case meteo::ConditionString::kNull:{
    return ( true == value.isEmpty() );
  }
  }
  return false;
}

QString ConditionString::displayCondition() const
{
  if ( false == displayCondition_.isNull() ){
    return displayCondition_;
  }
  auto fieldName = this->getFieldDisplayName();
  switch (this->matchType_) {  
  case meteo::ConditionString::kContains:{
    return QObject::tr("%1 = *%2* ").arg(fieldName).arg(matchValue_);
  }
  case meteo::ConditionString::kNotContains:{
    return QObject::tr("%1 ≠ *%2*").arg(fieldName).arg(matchValue_);
  }
  case meteo::ConditionString::kMatch:{
    return QObject::tr("%1 = %2").arg(fieldName).arg(matchValue_);
  }
  case meteo::ConditionString::kNotMatch:{
    return QObject::tr("%1 ≠ %2").arg(fieldName).arg(matchValue_);
  }
  case meteo::ConditionString::kNotNUll:{
    return QObject::tr("%1 ≠ \"\"").arg(fieldName);
  }
  case meteo::ConditionString::kNull:{
    return QObject::tr("%1 = \"\"").arg(fieldName);
  }
  }
  return QString();
}

void ConditionString::serialize(meteo::proto::ConditionProto *out) const
{
  auto proto = out->mutable_condition_string();
  proto->set_name(this->getFieldName().toStdString());
  proto->set_display_name(this->getFieldDisplayName().toStdString());

  proto->set_value(matchValue_.toUtf8().toStdString());
  proto->set_case_sensetive(this->caseSensitivity_);
  proto->set_transliterate(this->transliterate_);
  proto->set_condition(this->matchType_);
}

QString ConditionString::getRegExp() const {
  const QString& value = (true == this->transliterate_) ? meteo::global::kTranslitFunc(this->matchValue_) : this->matchValue_;

  switch (this->matchType_) {  
  case meteo::ConditionString::kMatch:{
    return  QObject::tr("^(?=%1$)").arg(value);
  }
  case meteo::ConditionString::kNotMatch:{
    return  QObject::tr("^(?!%1$)").arg(value);
  }
  case meteo::ConditionString::kContains:{
    return  QObject::tr("^(?=.*%1)").arg(value);
  }
  case meteo::ConditionString::kNotContains:{
    return QObject::tr("^(?!.*%1.*$)").arg(value);
  }
  case meteo::ConditionString::kNull:{
    return QObject::tr("^(?=$)");
  }
  case meteo::ConditionString::kNotNUll:{
    return QObject::tr("^(?=.+$)");
  }
  }
  return QString();
}

QString ConditionString::getStringCondition(const ConnectProp& prop) const
{
  static const QString& patternCaseNotSens = QObject::tr("conditionmaker_condition_string_case_insensetive");
  static const QString& patternCaseSens = QObject::tr("conditionmaker_condition_string_case_sensetive");
  static const QString& patternNull = QObject::tr("conditionmaker_string_null");
  static const QString& patternNotNull = QObject::tr("conditionmaker_string_ne_null");

  switch (this->matchType_) {  
  case meteo::ConditionString::kNotContains:
  case meteo::ConditionString::kContains:
  case meteo::ConditionString::kMatch:
  case meteo::ConditionString::kNotMatch:{
    const QString& currentQueryName = Qt::CaseSensitive == this->caseSensitivity_? patternCaseSens: patternCaseNotSens;
    std::unique_ptr<DbiQuery> query( global::dbqueryByName(prop, currentQueryName));
    if ( nullptr == query ){
      error_log << meteo::msglog::kDbRequestNotFound.arg(currentQueryName);
      return QString();
    }
    query->argFieldName("param",this->getFieldName());
    query->arg("regexp",this->getRegExp());
    return query->query();
  }
  case meteo::ConditionString::kNull: {    
    std::unique_ptr<DbiQuery> query( global::dbqueryByName(prop, patternNull));
    if ( nullptr == query ){
      error_log << meteo::msglog::kDbRequestNotFound.arg(patternNull);
      return QString();
    }
    query->argFieldName("param",this->getFieldName());
    return query->query();
  }
  case meteo::ConditionString::kNotNUll:{
    std::unique_ptr<DbiQuery> query( global::dbqueryByName(prop, patternNotNull));
    if ( nullptr == query ){
      error_log << meteo::msglog::kDbRequestNotFound.arg(patternNotNull);
      return QString();
    }
    query->argFieldName("param",this->getFieldName());
    return query->query();
  }
  }
  return QString();
}

meteo::ConditionString::ConditionMatchType ConditionString::type() const
{
  return matchType_;
}

QString ConditionString::value() const
{
  return matchValue_;
}

}
}
