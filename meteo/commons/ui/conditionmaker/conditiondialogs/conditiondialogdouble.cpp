#include "conditiondialogdouble.h"

#include <qpushbutton.h>
#include <qvalidator.h>
#include "conditions/conditionlogical.h"
#include "ui_conditionwidgetdouble.h"

#include "conditions/conditiondouble.h"
#include <qvalidator.h>

namespace meteo {
namespace commons {

ConditionDialogDouble::ConditionDialogDouble(const QString& fieldName, const QString& fieldDisplayName, double min, double max)
  : ui_(new Ui_ConditionWidgetDouble()),
  fieldName_(fieldName),
  fieldDisplayName_(fieldDisplayName),
  min_(min),
  max_(max)
{
  ui_->setupUi(this);
  ui_->lbFieldName->setText(QObject::tr("Показать записи, где %1:")
                            .arg(fieldDisplayName));

  ui_->leValue->setValidator(new QDoubleValidator(min, max, 5) );
  ui_->leValue2->setValidator(new QDoubleValidator(min, max, 5) );
  for ( int i = 0; i < internal::ConditionDouble::conditionTypesCount; ++i ){
    ConditionDouble::ConditionMatchType type = static_cast<ConditionDouble::ConditionMatchType>(i);
    ui_->cbMatchType->addItem( internal::ConditionDouble::conditionDisplayNames[type], type );
    ui_->cbMatchType2->addItem( internal::ConditionDouble::conditionDisplayNames[type], type );
  }
  ui_->bBox->clear();
  auto pbAccept = new QPushButton();
  pbAccept->setText("Применить");
  auto pbReject = new QPushButton();
  pbReject->setText("Отмена");
  ui_->bBox->addButton(pbAccept, QDialogButtonBox::ButtonRole::AcceptRole);
  ui_->bBox->addButton(pbReject, QDialogButtonBox::ButtonRole::RejectRole);
  QObject::connect(pbAccept, &QPushButton::clicked, this, &QDialog::accept);
  QObject::connect(pbReject, &QPushButton::clicked, this, &QDialog::reject);
}

ConditionDialogDouble::~ConditionDialogDouble()
{
  if ( nullptr != ui_ ){
    delete ui_;
  }
}

static Condition condition(QString fieldname, QString fieldDisplayName, QComboBox* cbMatch, QLineEdit* leValue)
{
  int data = cbMatch->currentData().toInt();
  auto type = static_cast<ConditionDouble::ConditionMatchType>(data);
  if ( true == leValue->text().isEmpty() ) {
    return ConditionDouble();
  }
  return ConditionDouble( fieldname, fieldDisplayName, type, leValue->text().toDouble() );
}

Condition ConditionDialogDouble::getCondition() const
{
  bool isAnd = ui_->rbAnd->isChecked();
  auto firstCondition = condition(fieldName_,
                                                        fieldDisplayName_,
                                                        ui_->cbMatchType,
                                                        ui_->leValue);

  auto secondCondition = condition(fieldName_,
                                                         fieldDisplayName_,
                                                         ui_->cbMatchType2,
                                                         ui_->leValue2);



  if ( true == firstCondition.isNull() ){
    return secondCondition;
  }
  if ( true == secondCondition.isNull() ){
    return firstCondition;
  }

  if ( true == isAnd ){
    return ConditionLogicalAnd({ firstCondition, secondCondition });
  }
  return ConditionLogicalOr({firstCondition, secondCondition});
}


}
}
