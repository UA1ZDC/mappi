#include "conditioncreationdialogint.h"

#include <qpushbutton.h>
#include "ui_conditionwidgetint.h"
#include "conditions/conditionlogical.h"
#include <meteo/commons/ui/conditionmaker/conditiongeneric.h>
#include <meteo/commons/ui/conditionmaker/conditions/conditionint.h>

namespace meteo {

ConditionCreationDialogInt::ConditionCreationDialogInt(const QString& fieldName, const QString& fieldDisplayName, int min, int max)
  : ui_(new Ui_ConditionWidgetInt()),
  fieldName_(fieldName),
  fieldDisplayName_(fieldDisplayName),
  min_(min),
  max_(max)
{
  ui_->setupUi(this);
  ui_->lbFieldName->setText(QObject::tr("Показать записи, где %1:").arg(fieldDisplayName));

  ui_->leValue->setValidator( new QIntValidator(min, max));
  ui_->leValue2->setValidator( new QIntValidator(min, max));

  for ( int i = 0; i < internal::ConditionInt::conditionTypesCount; ++i ){
    ConditionInt32::ConditionMatchType type = static_cast<ConditionInt32::ConditionMatchType>(i);
    ui_->cbMatchType->addItem( internal::ConditionInt::conditionDisplayNames[type], type );
    ui_->cbMatchType2->addItem( internal::ConditionInt::conditionDisplayNames[type], type );
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

ConditionCreationDialogInt::~ConditionCreationDialogInt()
{

}

static Condition condition(QString fieldname, QString fieldDisplayName, QComboBox* cbMatch, QLineEdit* leValue)
{
  int data = cbMatch->currentData().toInt();
  auto type = static_cast<ConditionInt32::ConditionMatchType>(data);
  if ( true == leValue->text().isEmpty() ) {
    return ConditionInt32();
  }
  return ConditionInt32( fieldname, fieldDisplayName, type, leValue->text().toInt() );
}

Condition ConditionCreationDialogInt::getCondition() const
{
  bool isAnd = ui_->rbAnd->isChecked();
  Condition firstCondition = condition(fieldName_,
                                                     fieldDisplayName_,
                                                     ui_->cbMatchType,
                                                     ui_->leValue);

  Condition secondCondition = condition(fieldName_,
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
