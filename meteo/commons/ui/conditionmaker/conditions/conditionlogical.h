#ifndef METEO_COMMONS_UI_CONDITIONMAKER_CONDITIONS_CONDITIONLOGICAL_H
#define METEO_COMMONS_UI_CONDITIONMAKER_CONDITIONS_CONDITIONLOGICAL_H

#include <meteo/commons/ui/conditionmaker/templates/conditiongenericprivate.h>
#include <sql/nosql/nosqlquery.h>
#include <meteo/commons/global/global.h>
#include <qobject.h>

namespace meteo {
namespace internal {

class ConditionLogical : public GenericConditionPrivateBase {
public:
  enum ConditionType {
    kAnd,
    kOr
  };
  ConditionLogical( QList<GenericConditionPrivateBase*> conditions, ConditionType type );

  ConditionLogical( QList<std::shared_ptr<GenericConditionPrivateBase>> conditions, ConditionType type );

  virtual ~ConditionLogical() override;

  ConditionType type() const;

  virtual QString displayCondition( ) const override ;

  virtual void serialize( meteo::proto::ConditionProto* out ) const override;

  virtual QString getStringCondition(const ConnectProp& prop) const override;
  virtual bool isMatch(const QVariant& value) const override;

private:
  QString getStringConditionMongo(const ConnectProp& prop) const;
  QString getStringConditionPsql(const ConnectProp& prop) const;

private:
  QList<std::shared_ptr<GenericConditionPrivateBase>> conditions_;
  ConditionType type_;
};

}
}

#endif
