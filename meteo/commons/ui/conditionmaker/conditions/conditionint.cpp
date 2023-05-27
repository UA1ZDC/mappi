#include "conditionint.h"
#include <qobject.h>
#include <cross-commons/debug/tlog.h>
#include <sql/dbi/dbiquery.h>
#include <meteo/commons/global/global.h>

using namespace meteo::proto;

namespace meteo {
namespace internal {

const QString ConditionInt::conditionDisplayNames[conditionTypesCount] = {  
  QObject::tr("Равно"),
  QObject::tr("Не равно"),
  QObject::tr("Меньше чем"),
  QObject::tr("Больше чем"),
  QObject::tr("Меньше или равно"),
  QObject::tr("Больше или равно")
};

ConditionInt::ConditionInt( const QString& fieldName, meteo::ConditionInt32::ConditionMatchType matchType, int value ):
  GenericConditionPrivateBase(fieldName, fieldName),
  matchType_(matchType),
  value_(value)
{

}

ConditionInt::ConditionInt( const QString& fieldName, const QString& fieldDisplayName, meteo::ConditionInt32::ConditionMatchType matchType, int value ) :
  GenericConditionPrivateBase (fieldName, fieldDisplayName),
  matchType_(matchType),
  value_(value)
{

}

ConditionInt::~ConditionInt()
{

}

bool ConditionInt::isMatch(const QVariant &buf) const
{
  if ( false == buf.canConvert<int>() ){
    warning_log << QObject::tr("Не удается привести тип данных");
    return false;
  }
  int value = buf.toInt();

  switch (matchType_) {
  case meteo::ConditionInt32::kEqual:{
    return (value == value_);
  }
  case meteo::ConditionInt32::kNotEqual:{
    return (value != value_);
  }
  case meteo::ConditionInt32::kLess:{
    return (value_ > value);
  }
  case meteo::ConditionInt32::kMore:{
    return (value_ < value);
  }
  case meteo::ConditionInt32::kLessOrEqual:{
    return (value_ >= value);
  }
  case meteo::ConditionInt32::kMoreOrEqual:{
    return (value_ <= value);
  }
  }
  return false;
}

QString ConditionInt::displayCondition() const
{
  if ( false == displayCondition_.isNull() ){
    return displayCondition_;
  }

  auto fieldName = getFieldDisplayName();
  switch (matchType_) {
  case meteo::ConditionInt32::kEqual:{
    return QObject::tr("%1 = %2").arg(fieldName).arg(value_);
  }
  case meteo::ConditionInt32::kNotEqual:{
    return QObject::tr("%1 ≠ %2").arg(fieldName).arg(value_);
  }
  case meteo::ConditionInt32::kLess:{
    return QObject::tr("%1 < %2").arg(fieldName).arg(value_);
  }
  case meteo::ConditionInt32::kMore:{
    return QObject::tr("%1 > %2").arg(fieldName).arg(value_);
  }
  case meteo::ConditionInt32::kLessOrEqual:{
    return QObject::tr("%1 ≤ %2").arg(fieldName).arg(value_);
  }
  case meteo::ConditionInt32::kMoreOrEqual:{
    return QObject::tr("%1 ≥ %2").arg(fieldName).arg(value_);
  }
  }
  return QString();
}

void ConditionInt::serialize(meteo::proto::ConditionProto* out) const
{
  auto proto = out->mutable_condition_int();
  proto->set_name(getFieldName().toStdString());
  proto->set_display_name(getFieldDisplayName().toStdString());

  proto->set_value(value_);
  proto->set_condition(matchType_);
}

QString ConditionInt::getStringCondition(const ConnectProp& prop) const
{
  QString queryName;
  switch (matchType_) {
  case meteo::ConditionInt32::kEqual:{
    queryName = QObject::tr("condition_eq");
    break;
  }
  case meteo::ConditionInt32::kNotEqual:{
    queryName = QObject::tr("condition_ne");
    break;
  }
  case meteo::ConditionInt32::kLess:{
    queryName = QObject::tr("condition_lt");
    break;
  }
  case meteo::ConditionInt32::kLessOrEqual:{
    queryName = QObject::tr("condition_lte");
    break;
  }
  case meteo::ConditionInt32::kMore:{
    queryName = QObject::tr("condition_gt");
    break;
  }
  case meteo::ConditionInt32::kMoreOrEqual:{
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
