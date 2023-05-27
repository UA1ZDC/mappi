#ifndef METEO_COMMONS_UI_CONDITIONMAKER_CONDITIONDIALOGS_CONDITIONALDIALOGSTRING_H
#define METEO_COMMONS_UI_CONDITIONMAKER_CONDITIONDIALOGS_CONDITIONALDIALOGSTRING_H

#include <qdialog.h>
#include <meteo/commons/ui/conditionmaker/conditiongeneric.h>
#include <meteo/commons/ui/conditionmaker/templates/conditiongenerator.h>

class Ui_ConditionWidgetString;

namespace meteo {

class ConditionalDialogString :  public QDialog, public ConditionGenerator
{
  Q_OBJECT
public:
  ConditionalDialogString(const QString& fieldName, const QString& fieldDisplayName, QWidget* parent = nullptr);
  virtual ~ConditionalDialogString() override;

public:
  virtual Condition getCondition() const override;

private:
  Ui_ConditionWidgetString* ui_;
  const QString fieldName_;
  const QString fieldDisplayName_;
};

}
#endif
