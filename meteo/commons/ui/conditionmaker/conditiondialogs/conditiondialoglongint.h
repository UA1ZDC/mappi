#ifndef METEO_COMMONS_UI_CONDITIONMAKER_CONDITIONDIALOGS_CONDITIONDIALOGLONGINT_H
#define METEO_COMMONS_UI_CONDITIONMAKER_CONDITIONDIALOGS_CONDITIONDIALOGLONGINT_H

#include <qdialog.h>
#include <meteo/commons/ui/conditionmaker/conditiongeneric.h>
#include <meteo/commons/ui/conditionmaker/templates/conditiongenerator.h>

class Ui_ConditionWidgetLongInt;

namespace meteo {

class ConditionDialogLongInt: public QDialog, ConditionGenerator
{
public:
  ConditionDialogLongInt( const QString& fieldName,
                         const QString& fieldDisplayName,
                         long int min = INT64_MIN,
                         long int max = INT64_MAX );
  virtual ~ConditionDialogLongInt() override;  

  virtual Condition getCondition() const override;

private:
  Ui_ConditionWidgetLongInt *ui_;
  const QString fieldName_;
  const QString fieldDisplayName_;
  long int min_;
  long int max_;
};


}

#endif
