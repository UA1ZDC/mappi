#include "conditionboolean.h"
#include <sql/dbi/dbiquery.h>

namespace meteo {
namespace internal {

ConditionBoolean::ConditionBoolean(const QString& fieldName, const QString& fieldDisplayName, bool value):
  internal::GenericConditionPrivateBase(fieldName, fieldDisplayName),
  value_(value)
{

}

ConditionBoolean::~ConditionBoolean()
{

}


QString ConditionBoolean::getStringCondition(const ConnectProp& prop) const
{
  static auto queryName = QObject::tr("condition_eq");

  std::unique_ptr<DbiQuery> query( meteo::global::dbqueryByName(prop, queryName) );
  if ( nullptr == query ){
    error_log << meteo::msglog::kDbRequestNotFound.arg(queryName);
    return QString();
  }

  query->argFieldName("param", getFieldName());
  query->arg("value", value_);
  return query->query();
}

bool ConditionBoolean::isMatch(const QVariant &buf) const
{
  if ( false == buf.canConvert<bool>() ){
    warning_log << QObject::tr("Не удается преобразовать значение");
    return false;
  }

  bool value = buf.value<bool>();
  return value == value_;
}

QString ConditionBoolean::displayCondition() const
{
  if ( false == displayCondition_.isNull() ){
    return displayCondition_;
  }

  if ( true == value_ ){
    return QString("{ %1 : ИСТИНА }").arg( getFieldDisplayName() );
  }
  else {
    return QString("{ %1 : ЛОЖЬ }").arg( getFieldDisplayName() );
  }
}

void ConditionBoolean::serialize(meteo::proto::ConditionProto* out) const
{
  auto packed = out->mutable_condition_bool();
  packed->set_name(getFieldName().toStdString());
  packed->set_display_name(getFieldDisplayName().toStdString());
  packed->set_value(value_);
}


}
}
