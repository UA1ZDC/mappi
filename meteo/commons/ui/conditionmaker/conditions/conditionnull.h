#include <meteo/commons/ui/conditionmaker/templates/conditiongenericprivate.h>

namespace meteo {
namespace internal {

class ConditionNull : public internal::GenericConditionPrivateBase
{
public:
  ConditionNull(const QString& fieldName, const QString& fieldDisplayName, bool isNull);

  virtual ~ConditionNull() override;

  virtual QString getStringCondition(const ConnectProp& prop) const override;
  virtual bool isMatch(const QVariant &value) const override;
  virtual QString displayCondition() const override;

  virtual void serialize(meteo::proto::ConditionProto* out) const override;
private:
  bool isNull_;

};

}

}
