#ifndef METEO_COMMONS_UI_CONDITIONMAKER_CONDITIONS_CONDITIONSTRING_H
#define METEO_COMMONS_UI_CONDITIONMAKER_CONDITIONS_CONDITIONSTRING_H

#include <meteo/commons/ui/conditionmaker/conditiongeneric.h>
#include <meteo/commons/ui/conditionmaker/templates/conditiongenericprivate.h>

namespace meteo {
namespace internal {

class ConditionString : public internal::GenericConditionPrivateBase
{
public:  
  static const int conditionTypesCount = meteo::ConditionString::kNotNUll + 1;
  static const QString conditionNames[conditionTypesCount];
  ConditionString(const QString& fieldName, meteo::ConditionString::ConditionMatchType matchType, bool translit, Qt::CaseSensitivity caseSensitivity, const QString& value);
  ConditionString(const QString& fieldName, const QString& fieldDisplayName, meteo::ConditionString::ConditionMatchType matchType, bool translit, Qt::CaseSensitivity caseSensitivity, const QString& value);
  virtual ~ConditionString() override;
  QString getRegExp() const;
  virtual QString getStringCondition(const ConnectProp& prop) const override;

  virtual bool isMatch(const QVariant &value) const override;
  virtual QString displayCondition() const override;

  virtual void serialize(meteo::proto::ConditionProto *out) const override;

  meteo::ConditionString::ConditionMatchType type() const;
  QString value() const;

private:  
  const  meteo::ConditionString::ConditionMatchType matchType_;
  const QString matchValue_;
  bool transliterate_;
  Qt::CaseSensitivity caseSensitivity_;

};

}
}

#endif
