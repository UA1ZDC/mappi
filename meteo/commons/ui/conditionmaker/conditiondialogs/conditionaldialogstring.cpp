#include "conditionaldialogstring.h"
#include "conditions/conditionstring.h"
#include <qvalidator.h>
#include "ui_conditionwidgetstring.h"

#include "conditions/conditionlogical.h"

namespace meteo {

static const QString registryIcon(QObject::tr(":/meteo/icons/registr.png"));
static const QString removeIcon(QObject::tr(":/meteo/icons/minus.png"));
static const QString translitIcon(QObject::tr(":/meteo/icons/translit.png"));

ConditionalDialogString::ConditionalDialogString(const QString& fieldName, const QString& fieldDisplayName, QWidget* parent) :
  QDialog (parent),
  ui_(new Ui_ConditionWidgetString()),
  fieldName_(fieldName),
  fieldDisplayName_(fieldDisplayName)
{
  ui_->setupUi(this);
  for ( int i = 0; i < internal::ConditionString::conditionTypesCount; ++i ){
    this->ui_->cbConditionTypes->addItem(internal::ConditionString::conditionNames[i], i);
    this->ui_->cbConditionTypes2->addItem(internal::ConditionString::conditionNames[i], i);
  }

  this->ui_->pbCaseSensetive->setIcon(QIcon(registryIcon));
  this->ui_->pbCaseSensetive->setToolTip(QObject::tr("При нажатой кнопке, фильтр не различает заглавные и строчные буквы"));
  this->ui_->pbTranslit->setIcon(QIcon(translitIcon));
  this->ui_->pbTranslit->setToolTip("При нажатой кнопке, фильтр переводит буквы кириллицы в латинские");

  this->ui_->pbCaseSensetive2->setIcon(QIcon(registryIcon));
  this->ui_->pbCaseSensetive2->setToolTip(QObject::tr("При нажатой кнопке, фильтр не различает заглавные и строчные буквы"));
  this->ui_->pbTranslit2->setIcon(QIcon(translitIcon));
  this->ui_->pbTranslit2->setToolTip("При нажатой кнопке, фильтр переводит буквы кириллицы в латинские");

  auto displayTitle = QObject::tr("Показать записи, где %1:").arg(fieldDisplayName);
  this->ui_->lbFieldName->setText(displayTitle);

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

ConditionalDialogString::~ConditionalDialogString()
{

}

static Condition condition(QString fieldName, QString fieldDisplayName, QPushButton* pbCase, QPushButton* translitBtn, QComboBox* cbType, QLineEdit* textField)
{
  ConditionString::ConditionMatchType type = static_cast<ConditionString::ConditionMatchType>( cbType->currentIndex() );
  Qt::CaseSensitivity sens = pbCase->isChecked()? Qt::CaseInsensitive : Qt::CaseSensitive;

  const QString& value = textField->text();
  if ( ConditionString::kMatch == type && true == value.isEmpty() ){
    return ConditionString();
  }
  return ConditionString( fieldName, fieldDisplayName, type, translitBtn->isChecked(), sens, value );
}


Condition ConditionalDialogString::getCondition() const
{
  bool isAnd = ui_->rbAnd->isChecked();
  auto firstCondition = condition(fieldName_,
                                  fieldDisplayName_,
                                  ui_->pbCaseSensetive,
                                  ui_->pbTranslit,
                                  ui_->cbConditionTypes,
                                  ui_->leValue);

  Condition secondCondition = condition(fieldName_,
                                        fieldDisplayName_,
                                        ui_->pbCaseSensetive2,
                                        ui_->pbTranslit2,
                                        ui_->cbConditionTypes2,
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
