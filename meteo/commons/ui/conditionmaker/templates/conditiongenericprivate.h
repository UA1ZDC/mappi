#ifndef METEO_COMMONS_UI_CONDITIONMAKER_TEMPLATES_CONDITIONGENERIC_H
#define METEO_COMMONS_UI_CONDITIONMAKER_TEMPLATES_CONDITIONGENERIC_H

#include <qstring.h>
#include <qvariant.h>
#include <meteo/commons/proto/documentviewer.pb.h>
#include <cross-commons/debug/tlog.h>
#include <meteo/commons/global/global.h>

namespace meteo {

namespace internal {

class GenericConditionPrivateBase {
public:
  GenericConditionPrivateBase();
  GenericConditionPrivateBase(const QString& fieldName, const QString& fieldDisplayName );
  virtual ~GenericConditionPrivateBase();
  virtual QString displayCondition() const = 0;
  virtual void serialize(meteo::proto::ConditionProto* output) const = 0;
  virtual QString getStringCondition( const ConnectProp& prop ) const = 0;
  virtual QString getJsonCondition() const;
  static GenericConditionPrivateBase* deserialize( const meteo::proto::ConditionProto& proto);
  virtual bool isMatch(const QVariant& value) const = 0;

  void setDisplayCondition(const QString& condition) { displayCondition_ = condition; }


  const QString& getFieldName() const {
    return this->fieldName_;
  }
  const QString& getFieldDisplayName() const {
    return this->fieldDisplayName_;
  }
protected:
  QString displayCondition_;
  const QString fieldName_;
  const QString fieldDisplayName_;
};

}

}

#endif
