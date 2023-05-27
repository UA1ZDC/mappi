#include "conditionedit.h"
#include "ui_conditionedit.h"

#include <qdialog.h>

#include <cross-commons/debug/tlog.h>

namespace meteo {

QString ConditionEdit::toText(const tablo::Condition& cond)
{
  QString text;

  switch ( cond.operation() ) {
    case tablo::kNoOperation:
      text = tr("< условие не задано >");
      break;
    case tablo::kRange:
      text = tr("от %1 до %2");
      text = text.arg(cond.operand_a());
      text = text.arg(cond.operand_b());
      break;
    case tablo::kMore:
      text = tr("> %1");
      text = text.arg(cond.operand_a());
      break;
    case tablo::kLess:
      text = tr("< %1");
      text = text.arg(cond.operand_a());
      break;
    case tablo::kEqual:
      text = tr("= %1");
      text = text.arg(cond.operand_a());
      break;
    case tablo::kNotEqual:
      text = tr("!= %1");
      text = text.arg(cond.operand_a());
      break;
    case tablo::kCifrCode:
      text = tr("= %1");
      text = text.arg( QString::fromStdString( cond.operandcode() ) );
      break;
  }

  return text;
}

//
//
//

ConditionEdit::ConditionEdit(QWidget *parent) :
  QWidget(parent),
  ui_(new Ui::ConditionEdit)
{
  ui_->setupUi(this);

  result_ = QDialog::Rejected;

  QComboBox* cbox = ui_->operationCombo;
  cbox->clear();
  cbox->addItem(tr("< не выбрано >"), tablo::kNoOperation);
  cbox->addItem(tr("от ... до ..."), tablo::kRange);
  cbox->addItem(tr("больше"), tablo::kMore);
  cbox->addItem(tr("меньше"), tablo::kLess);
  cbox->addItem(tr("равно"), tablo::kEqual);
  cbox->addItem(tr("не равно"), tablo::kNotEqual);
  cbox->addItem(tr("цифра кода"), tablo::kCifrCode);

  connect( cbox, SIGNAL(currentIndexChanged(int)), SLOT(slotOperationChanged(int)) );

  connect( ui_->okBtn, SIGNAL(clicked()), SLOT(slotOkClicked()) );

  // init
  setupUi(tablo::kNoOperation);
}

ConditionEdit::~ConditionEdit()
{
  delete ui_;
}

void ConditionEdit::fromProto(const tablo::Condition& condition)
{
  QComboBox* cbox = ui_->operationCombo;
  int idx = cbox->findData(condition.operation());
  if ( -1 != idx ) {
    cbox->setCurrentIndex(idx);
  }

  ui_->operandASpin->setValue(condition.operand_a());
  ui_->operandBSpin->setValue(condition.operand_b());
  ui_->operandText->setText(QString::fromStdString(condition.operandcode()));
}

tablo::Condition ConditionEdit::toProto() const
{
  QComboBox* cbox = ui_->operationCombo;
  int idx = cbox->currentIndex();

  tablo::Condition cond;
  cond.set_operation(static_cast<tablo::Operation>(cbox->itemData(idx).toInt()));
  cond.set_operand_a(ui_->operandASpin->value());
  cond.set_operand_b(ui_->operandBSpin->value());
  cond.set_operandcode(ui_->operandText->text().toStdString());
  return cond;
}

void ConditionEdit::setVisible(bool visible)
{
  QWidget::setVisible(visible);
  emit visibilityChanged(visible);
}

void ConditionEdit::slotOperationChanged(int idx)
{
  setupUi(static_cast<tablo::Operation>(ui_->operationCombo->itemData(idx).toInt()));
}

void ConditionEdit::slotOkClicked()
{
  result_ = QDialog::Accepted;
  close();
}

void ConditionEdit::setupUi(tablo::Operation op)
{
  switch ( op ) {
  case tablo::kNoOperation: {
    ui_->label->hide();
    ui_->operandASpin->hide();
    ui_->operandBSpin->hide();
    ui_->operandText->hide();
  }
    break;
  case tablo::kRange: {
    ui_->label->show();
    ui_->operandASpin->show();
    ui_->operandBSpin->show();
    ui_->operandText->hide();
  }
    break;
  case tablo::kLess:
  case tablo::kMore:
  case tablo::kEqual:
  case tablo::kNotEqual: {
    ui_->label->hide();
    ui_->operandASpin->show();
    ui_->operandBSpin->hide();
    ui_->operandText->hide();
  }
    break;
  case tablo::kCifrCode: {
    ui_->operandText->show();
    ui_->label->hide();
    ui_->operandASpin->hide();
    ui_->operandBSpin->hide();
  }
    break;

  }
}

} // meteo
