#ifndef CONDITIONDIALOGDOUBLE_H
#define CONDITIONDIALOGDOUBLE_H


#include <qdialog.h>
#include <meteo/commons/ui/conditionmaker/conditiongeneric.h>
#include <meteo/commons/ui/conditionmaker/templates/conditiongenerator.h>
#include <limits.h>

class Ui_ConditionWidgetDouble;

namespace meteo {
namespace commons {


class ConditionDialogDouble: public QDialog, ConditionGenerator
{
public:
  ConditionDialogDouble( const QString& fieldName,
                         const QString& fieldDisplayName,
                         double min = std::numeric_limits<double>::lowest(),
                         double max = std::numeric_limits<double>::max() );
  virtual ~ConditionDialogDouble() override;  

protected:
  virtual Condition getCondition() const override;

private:
  Ui_ConditionWidgetDouble *ui_;
  const QString fieldName_;
  const QString fieldDisplayName_;
  double min_;
  double max_;
};

}
}

#endif // CONDITIONDIALOGDOUBLE_H
