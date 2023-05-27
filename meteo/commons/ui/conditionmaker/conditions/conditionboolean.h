#include <meteo/commons/ui/conditionmaker/templates/conditiongenericprivate.h>

namespace meteo {
namespace internal {

class ConditionBoolean : public internal::GenericConditionPrivateBase
{
public:
  ConditionBoolean(const QString& fieldName, const QString& fieldDisplayName, bool value);

  virtual ~ConditionBoolean() override;

  virtual QString getStringCondition(const ConnectProp& prop) const override;
  virtual bool isMatch(const QVariant &value) const override;
  virtual QString displayCondition() const override;

  virtual void serialize(meteo::proto::ConditionProto* out) const override;
private:
  bool value_;

};

}
}
