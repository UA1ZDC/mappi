#include "conditiongeneric.h"
#include <cross-commons/debug/tlog.h>

#include <meteo/commons/ui/conditionmaker/conditions/conditionboolean.h>
#include <meteo/commons/ui/conditionmaker/conditions/conditiondateinterval.h>
#include <meteo/commons/ui/conditionmaker/conditions/conditiondouble.h>
#include <meteo/commons/ui/conditionmaker/conditions/conditionint.h>
#include <meteo/commons/ui/conditionmaker/conditions/conditionlogical.h>
#include <meteo/commons/ui/conditionmaker/conditions/conditionlongint.h>
#include <meteo/commons/ui/conditionmaker/conditions/conditionnull.h>
#include <meteo/commons/ui/conditionmaker/conditions/conditionoid.h>
#include <meteo/commons/ui/conditionmaker/conditions/conditionstring.h>
#include <meteo/commons/ui/conditionmaker/conditions/conditionstringlist.h>

namespace meteo {

Condition::Condition() :
  ptr_(nullptr)
{

}

Condition::Condition( internal::GenericConditionPrivateBase* base )
  : ptr_(base)
{

}

Condition::Condition( const Condition& condition ) :
  ptr_(condition.ptr_)
{

}

Condition::~Condition()
{

}

bool Condition::isMatch(const QVariant& value) const
{
  if ( nullptr == ptr_ ){
    warning_log << QObject::tr("Предупреждение: условие не проверено на isNull");
    return false;
  }
  return ptr_->isMatch(value);
}

QString Condition::Condition::displayCondition() const
{  
  if ( nullptr == ptr_ ) {
    warning_log << QObject::tr("Предупреждение: условие не проверено на isNull");
    return QString();
  }
  return ptr_->displayCondition();

}

meteo::proto::ConditionProto Condition::serialize() const
{
  meteo::proto::ConditionProto proto;
  if ( nullptr == ptr_ ) {
    warning_log << QObject::tr("Предупреждение: условие не проверено на isNull");
    return meteo::proto::ConditionProto();
  }
  ptr_->serialize(&proto);
  return proto;
}

void Condition::serialize(meteo::proto::ConditionProto* out) const
{
  if ( nullptr == ptr_ ) {
    warning_log << QObject::tr("Предупреждение: условие не проверено на isNull");
    return;
  }
  ptr_->serialize(out);
}

QString Condition::getStringCondition( const ConnectProp& prop ) const
{
  if ( nullptr != ptr_ ) {
    return ptr_->getStringCondition(prop);
  }
  else {
    warning_log << QObject::tr("Предупреждение: условие не проверено на isNull");
    return QString();
  }
}

QString Condition::getJsonCondition() const
{
  if ( nullptr == ptr_ )  {
    warning_log << QObject::tr("Условие не проверено на isNull!");
    return QString();
  }

  return ptr_->getJsonCondition();
}

Condition Condition::deserialize( meteo::proto::ConditionProto proto)
{
  return Condition(internal::GenericConditionPrivateBase::deserialize(proto));
}

bool Condition::isNull() const
{
  return nullptr == this->ptr_;
}



ConditionNull::ConditionNull( const QString& fieldName, const QString& fieldDisplayName, bool isNull ) :
  Condition ( new internal::ConditionNull( fieldName,fieldDisplayName, isNull))
{
}

ConditionOid::ConditionOid(const QString& fieldName, const QString& fieldDisplayName, const QString& value):
  Condition (new internal::ConditionOid(fieldName, fieldDisplayName, value))
{

}

ConditionStringList::ConditionStringList( const QString& fieldName, const QString& fieldDisplayName, const QStringList& values ):
  Condition (new internal::ConditionStringList(fieldName, fieldDisplayName, values))
{
}

ConditionBool::ConditionBool( const QString& fieldName, const QString& fieldDisplayName, bool value ):
  Condition (new internal::ConditionBoolean(fieldName, fieldDisplayName, value))
{
}

ConditionString::ConditionString( const QString& fieldName, ConditionString::ConditionMatchType match, bool translit, Qt::CaseSensitivity sens, const QString& value ):
  Condition (new internal::ConditionString(fieldName, match, translit, sens, value))
{  
}

ConditionString::ConditionString( const QString& fieldName, const QString& fieldDisplayName,
                                  ConditionString::ConditionMatchType match, bool translit,
                                  Qt::CaseSensitivity sens, const QString& value ):
  Condition ( new internal::ConditionString( fieldName, fieldDisplayName, match, translit, sens, value ) ) {
}

ConditionDouble::ConditionDouble():
  Condition (nullptr)
{

}

ConditionDouble::ConditionDouble( const QString& fieldName, const QString& fieldDisplayName,
                                                    ConditionDouble::ConditionMatchType match, double value ):
  Condition( new internal::ConditionDouble(fieldName, fieldDisplayName, match, value))
{
}


ConditionDateTimeInterval::ConditionDateTimeInterval( const QString& fieldName, const QString& fieldDisplayName,
                                                              const QDateTime& dtBegin, const QDateTime& dtEnd ):
  Condition(new internal::ConditionDateInterval( fieldName, fieldDisplayName, dtBegin, dtEnd ))
{
}

ConditionInt64::ConditionInt64():
  Condition (nullptr)
{

}

ConditionInt64::ConditionInt64( const QString& fieldName, const QString& fieldDisplayName, ConditionInt64::ConditionMatchType match, long int value ):
  Condition ( new internal::ConditionLongInt(fieldName, fieldDisplayName, match, value) )
{
}

ConditionInt32::ConditionInt32( const QString& fieldName, ConditionInt32::ConditionMatchType match, int value ):
  Condition( new internal::ConditionInt(fieldName, QString(), match, value) )
{
}


ConditionInt32::ConditionInt32( const QString& fieldName, const QString& fieldDisplayName, ConditionInt32::ConditionMatchType match, int value ):
  Condition( new internal::ConditionInt(fieldName, fieldDisplayName, match, value) )
{
}

void Condition::setDisplayCondition(const QString& condition)
{
  if ( nullptr == this->ptr_ ){
    return;
  }
  this->ptr_->setDisplayCondition(condition);
}

ConditionStringList::ConditionStringList(const Condition& other):
  Condition(nullptr)
{
  internal::ConditionStringList* cond = dynamic_cast<internal::ConditionStringList*>(other.ptr_.get());
  if ( nullptr == cond ){
    return;
  }
  ptr_ = other.ptr_;
}

QStringList ConditionStringList::values()
{
  internal::ConditionStringList* cond = dynamic_cast<internal::ConditionStringList*>(ptr_.get());
  if ( nullptr == cond ){
    error_log << QObject::tr("Ошибка приведения типа")    ;
    return QStringList();
  }
  return cond->getValues();
}



ConditionOid::ConditionOid(const Condition& other):
  Condition (nullptr)
{
  internal::ConditionOid* cond = dynamic_cast<internal::ConditionOid*>(other.ptr_.get());
  if ( nullptr == cond ){
    return;
  }
  ptr_ = other.ptr_;
}

ConditionString::ConditionString(const Condition& other):
  Condition (nullptr)
{
  internal::ConditionString* cond = dynamic_cast<internal::ConditionString*>(other.ptr_.get());
  if ( nullptr == cond ){
    return;
  }
  ptr_ = other.ptr_;
}

ConditionDouble::ConditionDouble(const Condition& other):
  Condition (nullptr)
{
  internal::ConditionDouble* cond = dynamic_cast<internal::ConditionDouble*>(other.ptr_.get());
  if ( nullptr == cond ){
    return;
  }
  ptr_ = other.ptr_;
}

ConditionDateTimeInterval::ConditionDateTimeInterval(const Condition& other):
  Condition (nullptr)
{
  internal::ConditionDateInterval* cond = dynamic_cast<internal::ConditionDateInterval*>(other.ptr_.get());
  if ( nullptr == cond ){
    return;
  }
  ptr_ = other.ptr_;
}


ConditionInt64::ConditionInt64(const Condition& other):
  Condition (nullptr)
{
  internal::ConditionLongInt* cond = dynamic_cast<internal::ConditionLongInt*>(other.ptr_.get());
  if ( nullptr == cond ){
    return;
  }
  ptr_ = other.ptr_;
}


ConditionNull::ConditionNull(const Condition& other):
  Condition (nullptr)
{
  internal::ConditionNull* cond = dynamic_cast<internal::ConditionNull*>(other.ptr_.get());
  if ( nullptr == cond ){
    return;
  }
  ptr_ = other.ptr_;
}

ConditionBool::ConditionBool(const Condition& other):
  Condition (nullptr)
{
  internal::ConditionBoolean* cond = dynamic_cast<internal::ConditionBoolean*>(other.ptr_.get());
  if ( nullptr == cond ){
    return;
  }
  ptr_ = other.ptr_;
}


ConditionInt32::ConditionInt32(const Condition& other):
  Condition (nullptr)
{
  internal::ConditionInt* cond = dynamic_cast<internal::ConditionInt*>(other.ptr_.get());
  if ( nullptr == cond ){
    return;
  }
  ptr_ = other.ptr_;
}

QString ConditionString::value() const
{
  internal::ConditionString* cond = dynamic_cast<internal::ConditionString*>(ptr_.get());
  if ( nullptr == cond ){
    return QString();
  }
  return cond->value();
}

ConditionString::ConditionMatchType ConditionString::type() const
{
  internal::ConditionString* cond = dynamic_cast<internal::ConditionString*>(ptr_.get());
  if ( nullptr == cond ){
    return kMatch;
  }
  return cond->type();
}

QDateTime ConditionDateTimeInterval::dtBegin() const
{
  internal::ConditionDateInterval* cond = dynamic_cast<internal::ConditionDateInterval*>(ptr_.get());
  if ( nullptr == cond ){
    return QDateTime();
  }
  return cond->dtBegin();
}

QDateTime ConditionDateTimeInterval::dtEnd() const
{
  internal::ConditionDateInterval* cond = dynamic_cast<internal::ConditionDateInterval*>(ptr_.get());
  if ( nullptr == cond ){
    return QDateTime();
  }
  return cond->dtEnd();
}

ConditionString::ConditionString():
  Condition (nullptr)
{

}

ConditionInt32::ConditionInt32():
  Condition (nullptr)
{

}

ConditionLogicalAnd::ConditionLogicalAnd():
  Condition(nullptr){

}

ConditionLogicalAnd::ConditionLogicalAnd( QList<Condition> conditions )
{
  QList<std::shared_ptr<internal::GenericConditionPrivateBase>> baseConditions;
  for ( auto condition : conditions ){
    baseConditions << condition.ptr_;
  }
  this->ptr_ = std::shared_ptr<internal::GenericConditionPrivateBase>( new internal::ConditionLogical( baseConditions, internal::ConditionLogical::kAnd) );
}

ConditionLogicalAnd::ConditionLogicalAnd( const Condition& other){
  internal::ConditionLogical* cond = dynamic_cast<internal::ConditionLogical*>(other.ptr_.get());
  if ( nullptr == cond ){
    return;
  }
  if ( internal::ConditionLogical::kAnd != cond->type() ){
    return;
  }
  this->ptr_ = other.ptr_;
}

ConditionLogicalOr::ConditionLogicalOr():
  Condition (nullptr)
{

}

ConditionLogicalOr::ConditionLogicalOr(  QList<Condition> conditions )
{
  QList<std::shared_ptr<internal::GenericConditionPrivateBase>> baseConditions;
  for ( auto condition : conditions ){
    baseConditions << condition.ptr_;
  }
  this->ptr_ = std::shared_ptr<internal::GenericConditionPrivateBase>( new internal::ConditionLogical( baseConditions, internal::ConditionLogical::kOr) );
}

ConditionLogicalOr::ConditionLogicalOr(const Condition& other)
{
  internal::ConditionLogical* cond = dynamic_cast<internal::ConditionLogical*>(other.ptr_.get());
  if ( nullptr == cond ){
    return ;
  }
  if ( cond->type() != internal::ConditionLogical::kOr ){
    return;
  }
  ptr_ = other.ptr_;
}

}
