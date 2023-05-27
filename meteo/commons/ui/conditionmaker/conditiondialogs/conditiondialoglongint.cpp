#include "conditiondialoglongint.h"
#include <qpushbutton.h>

#include "conditions/conditionlongint.h"
#include "conditions/conditionlogical.h"

#include "ui_conditionwidgetlongint.h"
#include "condition_widgets/longintvalidator.h"

namespace meteo {

ConditionDialogLongInt::ConditionDialogLongInt(const QString& fieldName, const QString& fieldDisplayName, long int min, long int max)
  : ui_(new Ui_ConditionWidgetLongInt()),
  fieldName_(fieldName),
  fieldDisplayName_(fieldDisplayName),
  min_(min),
  max_(max)
{
  ui_->setupUi(this);

  ui_->lbFieldName->setText(QObject::tr("Показать записи, где %1:").arg(fieldDisplayName));

  ui_->leValue->setValidator(new LongIntValidator(min, max) );
  ui_->leValue2->setValidator(new LongIntValidator(min, max) );

  for ( int i = 0; i < internal::ConditionLongInt::conditionTypesCount; ++i ){
    ConditionInt64::ConditionMatchType type = static_cast<ConditionInt64::ConditionMatchType>(i);
    ui_->cbMatchType->addItem( internal::ConditionLongInt::conditionDisplayNames[type], type );
    ui_->cbMatchType2->addItem( internal::ConditionLongInt::conditionDisplayNames[type], type );
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

ConditionDialogLongInt::~ConditionDialogLongInt()
{

}

static Condition condition(QString fieldname, QString fieldDisplayName, QComboBox* cbMatch, QLineEdit* leValue)
{
  int data = cbMatch->currentData().toInt();
  auto type = static_cast<ConditionInt64::ConditionMatchType>(data);
  if ( true == leValue->text().isEmpty() ) {
    return ConditionInt64();
  }
  return ConditionInt64( fieldname, fieldDisplayName, type, leValue->text().toLongLong() );
}

Condition ConditionDialogLongInt::getCondition() const
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
