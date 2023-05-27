#include "conditionoid.h"
#include <sql/dbi/dbiquery.h>

namespace meteo {
namespace internal {

ConditionOid::ConditionOid(const QString& fieldName,  const QString& fieldDisplayName, const QString& value):
  internal::GenericConditionPrivateBase(fieldName, fieldDisplayName),
  oid_(value)
{

}

ConditionOid::~ConditionOid()
{

}

QString ConditionOid::getStringCondition( const ConnectProp& prop ) const
{
  static auto queryName = QObject::tr("condition_eq");
  std::unique_ptr<DbiQuery> query(meteo::global::dbqueryByName(prop, queryName));
  if ( nullptr == query ){
    error_log << meteo::msglog::kDbRequestNotFound.arg(queryName);
    return QString();
  }
  query->argFieldName("param", getFieldName());
  query->argOid("value", oid_);
  return query->query();
}

bool ConditionOid::isMatch(const QVariant &buf) const
{
  if ( false == buf.canConvert<QString>() ){
    warning_log << QObject::tr("Ошибка, ожидалось что можно привести к QString");
    return false;
  }
  auto value = buf.value<QString>();
  return 0 == oid_.compare(value) ;
}

QString ConditionOid::displayCondition() const
{
  if ( false == displayCondition_.isNull() ){
    return displayCondition_;
  }
  return QObject::tr("%1 = %2")
      .arg(getFieldDisplayName())
      .arg(oid_);
}

void ConditionOid::serialize(meteo::proto::ConditionProto* out) const
{
  auto packed = out->mutable_condition_oid();
  packed->set_name(getFieldName().toStdString());
  packed->set_display_name(getFieldName().toStdString());
  packed->set_value(oid_.toStdString());
}

}
}
