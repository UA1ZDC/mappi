#ifndef CONDITIONDOUBLE_H
#define CONDITIONDOUBLE_H

#include <meteo/commons/ui/conditionmaker/templates/conditiongenericprivate.h>
#include <meteo/commons/ui/conditionmaker/conditiongeneric.h>
#include <qhash.h>

namespace meteo {
namespace internal {

class ConditionDouble : public internal::GenericConditionPrivateBase
{
public:
  static const int conditionTypesCount = meteo::ConditionDouble::kMoreOrEqual + 1;
  static const QString conditionDisplayNames[conditionTypesCount];
  ConditionDouble( const QString& fieldName, meteo::ConditionDouble::ConditionMatchType matchType, double value );
  ConditionDouble( const QString& fieldName, const QString& fieldDisplayName, meteo::ConditionDouble::ConditionMatchType matchType, double value );
  virtual ~ConditionDouble() override;
  virtual QString getStringCondition(const ConnectProp& prop) const override;

  virtual bool isMatch(const QVariant &buf) const override;
  virtual QString displayCondition() const override;
  virtual void serialize(meteo::proto::ConditionProto* out) const override;

private:
  const meteo::ConditionDouble::ConditionMatchType matchType_;
  const double value_;
};

}
}

#endif
