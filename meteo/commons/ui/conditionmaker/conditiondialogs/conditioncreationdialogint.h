#ifndef METEO_COMMONS_UI_CONDITIONMAKER_CONDITIONDIALOGS_CONDITIONCREATIONDIALOGINT_H
#define METEO_COMMONS_UI_CONDITIONMAKER_CONDITIONDIALOGS_CONDITIONCREATIONDIALOGINT_H


#include <qdialog.h>
#include <meteo/commons/ui/conditionmaker/conditiongeneric.h>
#include <meteo/commons/ui/conditionmaker/templates/conditiongenerator.h>

class Ui_ConditionWidgetInt;

namespace meteo {

class ConditionCreationDialogInt : public QDialog, ConditionGenerator
{
public:
  ConditionCreationDialogInt(const QString& fieldName, const QString& fieldDisplayName, int min = INT32_MIN, int max = INT32_MAX);
  virtual ~ConditionCreationDialogInt() override;  

  virtual Condition getCondition() const override;

private:
  Ui_ConditionWidgetInt* ui_;
  const QString fieldName_;
  const QString fieldDisplayName_;
  int min_;
  int max_;  
};

}

#endif // CONDITIONCREATIONDIALOGINT_H
