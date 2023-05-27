#include "conditionlogical.h"
#include <sql/dbi/dbiquery.h>

namespace meteo {
namespace internal {

ConditionLogical::ConditionLogical( QList<GenericConditionPrivateBase*> conditions, ConditionType type ) :
  GenericConditionPrivateBase (),
  type_(type) {
  for ( auto condition: conditions ){
    this->conditions_ << std::shared_ptr<GenericConditionPrivateBase>(condition);
  }
}

ConditionLogical::ConditionLogical( QList<std::shared_ptr<GenericConditionPrivateBase>> conditions, ConditionType type ):
  GenericConditionPrivateBase(),
  conditions_(conditions),
  type_(type)
{

}

ConditionLogical::~ConditionLogical()
{

}


QString ConditionLogical::displayCondition( ) const
{
  if ( false == this->displayCondition_.isNull() ){
    return this->displayCondition_;
  }
  QStringList displayConditions;
  for ( auto condition: conditions_ ){
    displayConditions << condition->displayCondition();
  }
  switch (type_) {
  case kAnd:{
    return displayConditions.join(QObject::tr(" И "));
  }
  case kOr:{
    return displayConditions.join(QObject::tr(" ИЛИ "));
  }
  }
  error_log << QObject::tr("Ошибка: данные повреждены");
  return QString();
}

void ConditionLogical::serialize( meteo::proto::ConditionProto* out ) const
{
  auto packed = out->mutable_condition_logical();
  packed->set_condition_type(type_);

  for ( auto condition: conditions_ ){
    auto subitem = packed->add_subitems();
    condition->serialize(subitem);
  }
}

QString ConditionLogical::getStringConditionMongo(const ConnectProp& prop) const
{
  QString queryName;
  switch (type_) {
  case kAnd:
    queryName = QObject::tr("condition_logical_and");
    break;
  case kOr:
    queryName = QObject::tr("condition_logical_or");
    break;
  }

  std::unique_ptr<DbiQuery> query( global::dbqueryByName(prop, queryName) );
  if ( nullptr == query ){
    error_log << meteo::msglog::kDbRequestNotFound.arg(queryName);
    return QString();
  }

  QStringList sqlConditions;
  for (auto condition: conditions_) {
    sqlConditions << condition->getStringCondition(prop);
  }
  query->argJson( "conditions", sqlConditions );
  return query->query();
}

QString ConditionLogical::getStringConditionPsql(const ConnectProp& prop) const
{
  Q_UNUSED(prop);
  QStringList conditions;
  QString separator = kAnd == type_? QObject::tr(" AND ")
                                   : QObject::tr(" OR ");

  for ( auto condition: this->conditions_ ){
    conditions << condition->getStringCondition(prop);
  }

  if ( 0 == conditions.size() ){
    error_log << QObject::tr("Ошибка! Пустой список условий;");
    return QString();
  }

  if ( 1 == conditions.size() ){
    return conditions.first();
  }

  return conditions.join(separator);
}

QString ConditionLogical::getStringCondition(const ConnectProp& prop) const
{
  if ( settings::kPsql == prop.driver() ){
    return getStringConditionPsql(prop);
  }
  return getStringConditionMongo(prop);
}

ConditionLogical::ConditionType ConditionLogical::type() const
{
  return type_;
}

bool ConditionLogical::isMatch(const QVariant& value) const
{
  switch(type_){
  case kOr:{
    for ( auto condition: conditions_ ){
      bool res = condition->isMatch(value);
      if ( true == res ){
        return true;
      }
    }
    return false;
  }
  case kAnd:{
    for ( auto condition: conditions_ ){
      bool res = condition->isMatch(value);
      if ( false == res ){
        return false;
      }
    }
    return true;
  }
  }

  warning_log << QObject::tr("Сюда не должно было дойти");
  return false;
}


}
}

