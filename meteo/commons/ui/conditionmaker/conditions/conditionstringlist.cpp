#include "conditionstringlist.h"
#include <sql/dbi/dbiquery.h>

namespace meteo {
namespace internal {

ConditionStringList::ConditionStringList(const QString& fieldName, const QString& fieldDisplayName, const QStringList& values):
  internal::GenericConditionPrivateBase(fieldName, fieldDisplayName),
  values_(values)
{

}

ConditionStringList::~ConditionStringList()
{

}

QString ConditionStringList::getStringCondition(const ConnectProp& prop) const
{
  static auto queryName = QObject::tr("condition_in");

  std::unique_ptr<DbiQuery> query(global::dbqueryByName(prop, queryName));
  if ( nullptr == query ){
    error_log << meteo::msglog::kDbRequestNotFound.arg(queryName);
    return QString();
  }

  query->arg("param", fieldName_);
  query->arg("value", values_);
  return query->query();
}

bool ConditionStringList::isMatch(const QVariant &value) const
{
  if ( false == value.canConvert<QString>() ){
    warning_log << QObject::tr("Не удается преобразовать тип данных");
    return false;
  }
  auto check = value.toString();

  return values_.contains(check);
}

QString ConditionStringList::displayCondition() const
{
  if ( false == displayCondition_.isNull() ){
    return displayCondition_;
  }
  return QObject::tr("%1 - %2")
      .arg(getFieldDisplayName())
      .arg(values_.join(" , "));
}

void ConditionStringList::serialize( meteo::proto::ConditionProto *out ) const
{
  auto proto = out->mutable_condition_string_list();
  proto->set_name(getFieldName().toStdString());
  proto->set_display_name(getFieldDisplayName().toStdString());
  for ( auto value: values_ ){
    proto->add_values(value.toStdString());
  }
}

QStringList ConditionStringList::getValues() const
{
  return values_;
}

}
}
