#include "conditiondouble.h"
#include <qobject.h>
#include <cross-commons/debug/tlog.h>

namespace meteo {
namespace internal {

const QString ConditionDouble::conditionDisplayNames[conditionTypesCount] = {
  QObject::tr("Равно"),
  QObject::tr("Не равно"),
  QObject::tr("Меньше чем"),
  QObject::tr("Больше чем"),
  QObject::tr("Меньше или равно"),
  QObject::tr("Больше или равно")
};

ConditionDouble::ConditionDouble( const QString& fieldName, meteo::ConditionDouble::ConditionMatchType matchType, double value ):
  GenericConditionPrivateBase (fieldName, fieldName),
  matchType_(matchType),
  value_(value)
{

}

ConditionDouble::ConditionDouble( const QString& fieldName, const QString& fieldDisplayName, meteo::ConditionDouble::ConditionMatchType matchType, double value ) :
  GenericConditionPrivateBase (fieldName, fieldDisplayName),
  matchType_(matchType),
  value_(value)
{

}

ConditionDouble::~ConditionDouble()
{

}

bool ConditionDouble::isMatch(const QVariant &buf) const
{
  bool ok;
  double value = buf.toDouble(&ok);
  if ( false == ok ){
    warning_log << QObject::tr("Не удается преобразовать буфер");
    return false;
  }

  switch (matchType_) {
  case meteo::ConditionDouble::kEqual:{
    return (value == value_);
  }
  case meteo::ConditionDouble::kNotEqual:{
    return (value != value_);
  }
  case meteo::ConditionDouble::kLess:{
    return (value_ > value);
  }
  case meteo::ConditionDouble::kMore:{
    return (value_ < value);
  }
  case meteo::ConditionDouble::kLessOrEqual:{
    return (value_ >= value);
  }
  case meteo::ConditionDouble::kMoreOrEqual:{
    return (value_ <= value);
  }
  }
  return false;
}

QString ConditionDouble::displayCondition() const
{
  if ( false == displayCondition_.isNull() ){
    return displayCondition_;
  }
  auto fieldName = getFieldDisplayName();
  switch (matchType_) {
  case meteo::ConditionDouble::kEqual:{
    return QObject::tr("%1 = %2").arg(fieldName).arg(value_);
  }
  case meteo::ConditionDouble::kNotEqual:{
    return QObject::tr("%1 ≠ %2").arg(fieldName).arg(value_);
  }
  case meteo::ConditionDouble::kLess:{
    return QObject::tr("%1 < %2").arg(fieldName).arg(value_);
  }
  case meteo::ConditionDouble::kMore:{
    return QObject::tr("%1 > %2").arg(fieldName).arg(value_);
  }
  case meteo::ConditionDouble::kLessOrEqual:{
    return QObject::tr("%1 ≤ %2").arg(fieldName).arg(value_);
  }
  case meteo::ConditionDouble::kMoreOrEqual:{
    return QObject::tr("%1 ≥ %2").arg(fieldName).arg(value_);
  }
  }
  return QString();
}

void ConditionDouble::serialize(meteo::proto::ConditionProto* out) const
{
  auto proto = out->mutable_condition_double();
  proto->set_name(getFieldName().toStdString());
  proto->set_display_name(getFieldDisplayName().toStdString());
  proto->set_value(value_);
  proto->set_condition(matchType_);
}

QString ConditionDouble::getStringCondition(const ConnectProp& prop) const
{
  QString queryName;
  switch (matchType_) {
  case meteo::ConditionDouble::kEqual:{
    queryName = QObject::tr("condition_eq");
    break;
  }
  case meteo::ConditionDouble::kNotEqual:{
    queryName = QObject::tr("condition_ne");
    break;
  }
  case meteo::ConditionDouble::kLess:{
    queryName = QObject::tr("condition_lt");
    break;
  }
  case meteo::ConditionDouble::kLessOrEqual:{
    queryName = QObject::tr("condition_lte");
    break;
  }
  case meteo::ConditionDouble::kMore:{
    queryName = QObject::tr("condition_gt");
    break;
  }
  case meteo::ConditionDouble::kMoreOrEqual:{
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
