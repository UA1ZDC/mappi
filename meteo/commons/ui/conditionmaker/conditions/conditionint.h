#ifndef METEO_COMMONS_UI_CONDITIONMAKER_CONDITIONS_CONDITIONINT_H
#define METEO_COMMONS_UI_CONDITIONMAKER_CONDITIONS_CONDITIONINT_H

#include <meteo/commons/ui/conditionmaker/conditiongeneric.h>
#include <meteo/commons/ui/conditionmaker/templates/conditiongenericprivate.h>
#include <qhash.h>

namespace meteo {
namespace internal {

class ConditionInt : public internal::GenericConditionPrivateBase
{
public:
  static const int conditionTypesCount = meteo::ConditionInt32::kMoreOrEqual + 1;
  static const QString conditionDisplayNames[conditionTypesCount];
  ConditionInt( const QString& fieldName, meteo::ConditionInt32::ConditionMatchType matchType, int value );
  ConditionInt( const QString& fieldName, const QString& fieldDisplayName, meteo::ConditionInt32::ConditionMatchType matchType, int value );
  virtual ~ConditionInt() override;
  virtual QString getStringCondition(const ConnectProp& prop) const override;

  virtual bool isMatch(const QVariant &value) const override;
  virtual QString displayCondition() const override;
  virtual void serialize(meteo::proto::ConditionProto* ) const override;

private:
  const meteo::ConditionInt32::ConditionMatchType matchType_;
  const int value_;
};

}
}
#endif
