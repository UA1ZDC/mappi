#ifndef METEO_COMMONS_UI_CONDITIONMAKER_CONDITIONS_CONDITIONLONGINT_H
#define METEO_COMMONS_UI_CONDITIONMAKER_CONDITIONS_CONDITIONLONGINT_H

#include <meteo/commons/ui/conditionmaker/templates/conditiongenericprivate.h>
#include <qhash.h>
#include <meteo/commons/ui/conditionmaker/conditiongeneric.h>

namespace meteo {
namespace internal {

class ConditionLongInt : public internal::GenericConditionPrivateBase
{
public:
  static const int conditionTypesCount = meteo::ConditionInt64::kMoreOrEqual + 1;
  static const QString conditionDisplayNames[conditionTypesCount];
  ConditionLongInt( const QString& fieldName,  meteo::ConditionInt64::ConditionMatchType matchType, long int value );
  ConditionLongInt( const QString& fieldName, const QString& fieldDisplayName,  meteo::ConditionInt64::ConditionMatchType matchType, long int value );
  virtual ~ConditionLongInt() override;
  virtual QString getStringCondition(const ConnectProp& prop) const override;

  virtual bool isMatch(const QVariant &value) const override;
  virtual QString displayCondition() const override;
  virtual void serialize(meteo::proto::ConditionProto* out) const override;


private:
  const  meteo::ConditionInt64::ConditionMatchType matchType_;
  const long int value_;
};

}
}

#endif
