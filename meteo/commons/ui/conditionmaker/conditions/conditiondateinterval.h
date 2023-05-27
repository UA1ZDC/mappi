#ifndef METEO_COMMONS_UI_CONDITIONMAKER_CONDITIONS_CONDITIONDATEINTERVAL_H
#define METEO_COMMONS_UI_CONDITIONMAKER_CONDITIONS_CONDITIONDATEINTERVAL_H

#include <meteo/commons/ui/conditionmaker/templates/conditiongenericprivate.h>
#include <qdatetime.h>

namespace meteo {
namespace internal {

class ConditionDateInterval : public internal::GenericConditionPrivateBase
{
public:
  ConditionDateInterval( const QString& name, const QString& displayName, const QDateTime& dtBegin, const QDateTime& dtEnd);
  ~ConditionDateInterval() override;

  const QDateTime& dtBegin() const;
  const QDateTime& dtEnd() const;

  virtual void serialize( meteo::proto::ConditionProto *out ) const override;
  virtual QString getStringCondition(const ConnectProp& prop) const override;

private:
  const QDateTime dtBegin_;
  const QDateTime dtEnd_;

  virtual bool isMatch(const QVariant& dt) const override;
  virtual QString displayCondition() const override;
};

}
}

#endif // CONDITIONDATEINTERVAL_H
