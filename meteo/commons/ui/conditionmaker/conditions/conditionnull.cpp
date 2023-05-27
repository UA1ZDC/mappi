#include "conditionnull.h"

#include <sql/dbi/dbiquery.h>
namespace meteo {
namespace internal {

ConditionNull::ConditionNull(const QString& fieldName, const QString& fieldDisplayName, bool isNull):
  internal::GenericConditionPrivateBase(fieldName, fieldDisplayName),
  isNull_(isNull)
{

}

ConditionNull::~ConditionNull()
{

}

QString ConditionNull::getStringCondition(const ConnectProp& prop) const
{
  static auto queryName = QObject::tr("condition_null");
  std::unique_ptr<DbiQuery> query( meteo::global::dbqueryByName(prop, queryName) );
  if ( nullptr == query ){
    error_log << meteo::msglog::kDbRequestNotFound.arg(queryName);
    return QString();
  }
  query->argFieldName("param", getFieldName());
  return query->query();
}

bool ConditionNull::isMatch(const QVariant &value) const
{
  if ( value.isNull() == isNull_ ){
    return true;
  }

  if ( false == value.canConvert<QString>() ){
    return false;
  }
  return true == value.toString().isNull();
}

QString ConditionNull::displayCondition() const
{
  if ( false == displayCondition_.isNull() ){
    return displayCondition_;
  }
  if ( true == isNull_ ){
    return QObject::tr("%1 - пусто")
        .arg(getFieldDisplayName());
  }
  else {
    return QObject::tr("%1 - существует")
        .arg(getFieldDisplayName());
  }
}

void ConditionNull::serialize(meteo::proto::ConditionProto* out) const
{
  auto packed = out->mutable_condition_null();
  packed->set_name(getFieldName().toStdString());
  packed->set_display_name(getFieldDisplayName().toStdString());
  packed->set_value(isNull_);
}

}
}
