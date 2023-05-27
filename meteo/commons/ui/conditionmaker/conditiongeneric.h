#ifndef METEO_COMMONS_UI_CONDITIONMAKER_CONDITIONGENERIC_H
#define METEO_COMMONS_UI_CONDITIONMAKER_CONDITIONGENERIC_H

#include <meteo/commons/proto/customviewer.pb.h>
#include <meteo/commons/ui/conditionmaker/templates/conditiongenericprivate.h>
#include <meteo/commons/ui/conditionmaker/conditions/conditionlogical.h>

namespace meteo {

class Condition {
public:
  static Condition deserialize( meteo::proto::ConditionProto proto);
  void setDisplayCondition(const QString& condition);
  Condition(const Condition& other);

  Condition();
  virtual ~Condition();

  virtual QString displayCondition() const ;
  virtual meteo::proto::ConditionProto serialize() const ;
  virtual void serialize(meteo::proto::ConditionProto* out) const ;
  virtual QString getJsonCondition() const;
  virtual QString getStringCondition( const ConnectProp& prop) const ;
  bool isNull() const;
  bool isMatch(const QVariant& value) const;

  public:
  std::shared_ptr<internal::GenericConditionPrivateBase> ptr_;

  protected:
  explicit Condition( internal::GenericConditionPrivateBase* base ) ;
};

class ConditionOid: public Condition {
public:
  ConditionOid(const QString& fieldName, const QString& fieldDisplayName, const QString& value);
  explicit ConditionOid(const Condition& other);
};

class ConditionString: public Condition {
public:
  enum ConditionMatchType {    
    kMatch = proto::ConditionType::kMatch,
    kNotMatch = proto::ConditionType::kNotMatch,
    kContains = proto::ConditionType::kContains,
    kNotContains = proto::ConditionType::kNotContains,
    kNull = proto::ConditionType::kNull,
    kNotNUll = proto::ConditionType::kNotNull
  };
  ConditionString();
  ConditionString( const QString& fieldName, ConditionString::ConditionMatchType match, bool translit, Qt::CaseSensitivity sens, const QString& value );
  ConditionString( const QString& fieldName, const QString& fieldDisplayName, ConditionString::ConditionMatchType match, bool translit, Qt::CaseSensitivity sens, const QString& value );
  explicit ConditionString(const Condition& other);

  QString value() const;
  ConditionMatchType type() const;
};

class ConditionDouble: public Condition {
public:
  enum ConditionMatchType {
    kEqual = proto::ConditionType::kEqual,
    kNotEqual = proto::ConditionType::kNotEqual,
    kLess = proto::ConditionType::kLess,
    kMore = proto::ConditionType::kMore,
    kLessOrEqual = proto::ConditionType::kLessOrEqual,
    kMoreOrEqual = proto::ConditionType::kMoreOrEqual
  };

  ConditionDouble();
  ConditionDouble( const QString& fieldName, const QString& fieldDisplayName, ConditionDouble::ConditionMatchType match, double value );
  explicit ConditionDouble(const Condition& other);
};


class ConditionStringList: public Condition {
public:
  ConditionStringList( const QString& fieldName, const QString& fieldDisplayName, const QStringList& values );
  explicit ConditionStringList(const Condition& other);
  QStringList values();
};

class ConditionDateTimeInterval: public Condition {
public:
  ConditionDateTimeInterval( const QString& fieldName, const QString& fieldDisplayName, const QDateTime& dtBegin, const QDateTime& dtEnd );
  explicit ConditionDateTimeInterval(const Condition& other);
  QDateTime dtBegin() const;
  QDateTime dtEnd() const;
};

class ConditionInt64: public Condition{
public:
  enum ConditionMatchType {
    kEqual = proto::ConditionType::kEqual,
    kNotEqual = proto::ConditionType::kNotEqual,
    kLess = proto::ConditionType::kLess,
    kMore = proto::ConditionType::kMore,
    kLessOrEqual = proto::ConditionType::kLessOrEqual,
    kMoreOrEqual = proto::ConditionType::kMoreOrEqual
  };

  ConditionInt64();
  ConditionInt64( const QString& fieldName, const QString& fieldDisplayName, ConditionMatchType match, long int value );
  explicit ConditionInt64(const Condition& other);
};

class ConditionNull: public Condition
{
  public:
  ConditionNull( const QString& fieldName, const QString& fieldDisplayName, bool isNull );
  explicit ConditionNull(const Condition& other);
};

class ConditionBool: public Condition {
public:
  ConditionBool( const QString& fieldName, const QString& fieldDisplayName, bool value );
  explicit ConditionBool(const Condition& other);
};

class ConditionInt32: public Condition {
public:
  enum ConditionMatchType {    
    kEqual = proto::ConditionType::kEqual,
    kNotEqual = proto::ConditionType::kNotEqual,
    kLess = proto::ConditionType::kLess,
    kMore = proto::ConditionType::kMore,
    kLessOrEqual = proto::ConditionType::kLessOrEqual,
    kMoreOrEqual = proto::ConditionType::kMoreOrEqual
  };
  ConditionInt32();
  ConditionInt32( const QString& fieldName, ConditionMatchType match, int value );
  ConditionInt32( const QString& fieldName, const QString& fieldDisplayName, ConditionMatchType match, int value );
  explicit ConditionInt32(const Condition& other);
};


class ConditionLogicalAnd: public Condition
{
public:
  ConditionLogicalAnd();
  ConditionLogicalAnd(  QList<Condition> conditions );
  explicit ConditionLogicalAnd(const Condition& other);
};

class ConditionLogicalOr: public Condition
{
public:
  ConditionLogicalOr();
  ConditionLogicalOr(  QList<Condition> conditions );
  explicit ConditionLogicalOr(const Condition& other);
};


}

#endif
