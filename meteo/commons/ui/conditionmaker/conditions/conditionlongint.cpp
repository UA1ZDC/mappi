#include "conditionlongint.h"
#include <qobject.h>
#include <cross-commons/debug/tlog.h>
#include <meteo/commons/global/global.h>

using namespace meteo::proto;

namespace meteo {
namespace internal {


const QString ConditionLongInt::conditionDisplayNames[conditionTypesCount] = {
  QObject::tr("Равно"),
  QObject::tr("Не равно"),
  QObject::tr("Меньше чем"),
  QObject::tr("Больше чем"),
  QObject::tr("Меньше или равно"),
  QObject::tr("Больше или равно")
};

ConditionLongInt::ConditionLongInt( const QString& fieldName, meteo::ConditionInt64::ConditionMatchType matchType, long int value ):
  GenericConditionPrivateBase(fieldName, fieldName),
  matchType_(matchType),
  value_(value)
{

}

ConditionLongInt::ConditionLongInt( const QString& fieldName, const QString& fieldDisplayName,  meteo::ConditionInt64::ConditionMatchType matchType, long int value ) :
  GenericConditionPrivateBase (fieldName, fieldDisplayName),
  matchType_(matchType),
  value_(value)
{

}

ConditionLongInt::~ConditionLongInt()
{

}

bool ConditionLongInt::isMatch(const QVariant &buf) const
{
  if ( false == buf.canConvert<qlonglong>() ){
    warning_log << QObject::tr("Ошибка при приведении типа данных");
    return false;
  }
  long int value = buf.toLongLong();

  switch (matchType_) {
  case meteo::ConditionInt64::kEqual:{
    return (value == value_);
  }
  case meteo::ConditionInt64::kNotEqual:{
    return (value != value_);
  }
  case meteo::ConditionInt64::kLess:{
    return (value_ > value);
  }
  case meteo::ConditionInt64::kMore:{
    return (value_ < value);
  }
  case meteo::ConditionInt64::kLessOrEqual:{
    return (value_ >= value);
  }
  case meteo::ConditionInt64::kMoreOrEqual:{
    return (value_ <= value);
  }
  }
  return false;
}

QString ConditionLongInt::displayCondition() const
{
  if ( false == displayCondition_.isNull() ){
    return displayCondition_;
  }

  auto fieldName = getFieldDisplayName();
  switch (matchType_) {
  case meteo::ConditionInt64::kEqual:{
    return QObject::tr("%1 = %2").arg(fieldName).arg(value_);
  }
  case meteo::ConditionInt64::kNotEqual:{
    return QObject::tr("%1 ≠ %2").arg(fieldName).arg(value_);
  }
  case meteo::ConditionInt64::kLess:{
    return QObject::tr("%1 < %2").arg(fieldName).arg(value_);
  }
  case meteo::ConditionInt64::kMore:{
    return QObject::tr("%1 > %2").arg(fieldName).arg(value_);
  }
  case meteo::ConditionInt64::kLessOrEqual:{
    return QObject::tr("%1 ≤ %2").arg(fieldName).arg(value_);
  }
  case meteo::ConditionInt64::kMoreOrEqual:{
    return QObject::tr("%1 ≥ %2").arg(fieldName).arg(value_);
  }
  }
  return QString();
}

void ConditionLongInt::serialize(meteo::proto::ConditionProto* out) const {
  auto proto = out->mutable_condition_long_int();

  proto->set_name(getFieldName().toStdString());
  proto->set_display_name(getFieldDisplayName().toStdString());

  proto->set_value(value_);
  proto->set_condition(matchType_);
}

QString ConditionLongInt::getStringCondition(const ConnectProp& prop) const
{
  QString queryName;
  switch (matchType_) {
  case meteo::ConditionInt64::kEqual:{
    queryName = QObject::tr("condition_eq");
    break;
  }
  case meteo::ConditionInt64::kNotEqual:{
    queryName = QObject::tr("condition_ne");
    break;
  }
  case meteo::ConditionInt64::kLess:{
    queryName = QObject::tr("condition_lt");
    break;
  }
  case meteo::ConditionInt64::kLessOrEqual:{
    queryName = QObject::tr("condition_lte");
    break;
  }
  case meteo::ConditionInt64::kMore:{
    queryName = QObject::tr("condition_gt");
    break;
  }
  case meteo::ConditionInt64::kMoreOrEqual:{
    queryName = QObject::tr("condition_gte");
    break;
  }
  }

  std::unique_ptr<DbiQuery> query(meteo::global::dbqueryByName(prop, queryName));
  if ( nullptr == query ){
    error_log << meteo::msglog::kDbRequestNotFound.arg(queryName);
    return QString();
  }

  query->argFieldName("param", getFieldName());
  query->arg("value",value_);
  return query->query();
}


}
}
