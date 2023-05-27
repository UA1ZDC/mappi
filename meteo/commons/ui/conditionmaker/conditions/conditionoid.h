#include <meteo/commons/ui/conditionmaker/templates/conditiongenericprivate.h>

namespace meteo {
namespace internal {

class ConditionOid : public internal::GenericConditionPrivateBase
{
public:
  ConditionOid(const QString& fieldName, const QString& fieldDisplayName, const QString& value);

  virtual ~ConditionOid() override;

  virtual QString getStringCondition( const ConnectProp& prop ) const override;
  virtual bool isMatch(const QVariant &value) const override;
  virtual QString displayCondition() const override;

  virtual void serialize(meteo::proto::ConditionProto* out) const override;
private:
  bool isNull_;

  QString oid_;
};

}
}
