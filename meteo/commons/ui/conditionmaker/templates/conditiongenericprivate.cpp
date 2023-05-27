#include "conditiongenericprivate.h"
#include <cross-commons/debug/tlog.h>
#include "conditions/conditionstring.h"
#include "conditions/conditionlogical.h"
#include "conditions/conditiondateinterval.h"
#include "conditions/conditiondouble.h"
#include "conditions/conditionlongint.h"
#include "conditions/conditionint.h"
#include "conditions/conditionboolean.h"
#include "conditions/conditionoid.h"
#include "conditions/conditionstringlist.h"
#include "conditions/conditionnull.h"


namespace meteo {
namespace internal {

GenericConditionPrivateBase::GenericConditionPrivateBase()
{
}

GenericConditionPrivateBase::GenericConditionPrivateBase(const QString& fieldName, const QString& fieldDisplayName ):
  fieldName_(fieldName),
  fieldDisplayName_(fieldDisplayName)
{

}

GenericConditionPrivateBase::~GenericConditionPrivateBase()
{
}

GenericConditionPrivateBase* GenericConditionPrivateBase::deserialize( const meteo::proto::ConditionProto& proto)
{
  switch (proto.subproto_case()) {
  case proto::ConditionProto::SUBPROTO_NOT_SET:{
    return nullptr;
  }
  case proto::ConditionProto::kConditionBool:{
    auto packed = proto.condition_bool();
    return new ConditionBoolean(QString::fromStdString(packed.name()),
                                QString::fromStdString(packed.display_name()),
                                packed.value());
  }
  case proto::ConditionProto::kConditionInt:{
    auto packed = proto.condition_int();
    return new ConditionInt(QString::fromStdString(packed.name()),
                            QString::fromStdString(packed.display_name()),
                            static_cast<meteo::ConditionInt32::ConditionMatchType>(packed.condition()),
                            packed.value());
  }
  case proto::ConditionProto::kConditionOid:{
    auto packed = proto.condition_oid();
    return new ConditionOid(QString::fromStdString(packed.name()),
                            QString::fromStdString(packed.display_name()),
                            QString::fromStdString(packed.value()));
  }
  case proto::ConditionProto::kConditionDouble:{
    auto packed = proto.condition_double();
    return new ConditionDouble(QString::fromStdString(packed.name()),
                               QString::fromStdString(packed.display_name()),
                               static_cast<meteo::ConditionDouble::ConditionMatchType>(packed.condition()),
                               packed.value());
  }
  case proto::ConditionProto::kConditionLogical: {
    auto packed = proto.condition_logical();
    QList<GenericConditionPrivateBase*> baseItems;
    QSet<proto::ConditionProto::SubprotoCase> cases;
    for ( auto subcondition: packed.subitems() ) {
      baseItems << deserialize(subcondition);
      cases << subcondition.subproto_case();
    }
    return new ConditionLogical(baseItems,
                                static_cast<ConditionLogical::ConditionType>(packed.condition_type()));
  }

  case proto::ConditionProto::kConditionLongInt:{
    auto packed = proto.condition_long_int();
    return new ConditionLongInt(QString::fromStdString(packed.name()),
                                QString::fromStdString(packed.display_name()),
                                static_cast<meteo::ConditionInt64::ConditionMatchType>(packed.condition()),
                                packed.value());
  }

  case proto::ConditionProto::kConditionStringList:{
    auto packed = proto.condition_string_list();
    QStringList values;
    for ( auto value: packed.values() ){
      values << QString::fromStdString(value);
    }

    return new ConditionStringList(QString::fromStdString(packed.name()),
                                   QString::fromStdString(packed.display_name()),
                                   values
                                   );
  }
  case proto::ConditionProto::kConditionString:{
    auto packed = proto.condition_string();
    return new ConditionString(QString::fromStdString(packed.name()),
                               QString::fromStdString(packed.display_name()),
                               static_cast<meteo::ConditionString::ConditionMatchType>(packed.condition()),
                               packed.transliterate(),
                               static_cast<Qt::CaseSensitivity>(packed.case_sensetive()),
                               QString::fromStdString(packed.value()));
  }
  case proto::ConditionProto::kConditionDateInterval:{
    auto packed = proto.condition_date_interval();
    return new ConditionDateInterval(QString::fromStdString(packed.name()),
                                     QString::fromStdString(packed.display_name()),
                                     QDateTime::fromString(QString::fromStdString(packed.dt_begin()), Qt::ISODate),
                                     QDateTime::fromString(QString::fromStdString(packed.dt_end()), Qt::ISODate));

  }
  case proto::ConditionProto::kConditionNull:{
    auto packed = proto.condition_null();
    return new ConditionNull(QString::fromStdString(packed.name()),
                             QString::fromStdString(packed.display_name()),
                             packed.value());
  }

  }

  error_log << QObject::tr("Десериализация не удалась");
  return nullptr;
}

QString GenericConditionPrivateBase::getJsonCondition() const
{
  ConnectProp prop;
  prop.setDriver(settings::DbDriver::kMongo);

  return getStringCondition(prop);
}

}

}

