#include <meteo/commons/ui/conditionmaker/templates/conditiongenericprivate.h>

namespace meteo {
namespace internal {

class ConditionStringList : public internal::GenericConditionPrivateBase
{
public:
  ConditionStringList(const QString& fieldName, const QString& fieldDisplayName, const QStringList& values);

  virtual ~ConditionStringList() override;

  virtual QString getStringCondition(const ConnectProp& prop) const override;
  virtual bool isMatch(const QVariant &value) const override;

  virtual QString displayCondition() const override;

  QStringList getValues() const;
  virtual void serialize( meteo::proto::ConditionProto *out ) const override;
private:
  QStringList values_;

};

}
}
