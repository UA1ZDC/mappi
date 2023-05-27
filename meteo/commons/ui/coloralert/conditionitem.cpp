#include "conditionitem.h"
#include "conditionedit.h"

namespace meteo {
namespace tablo {

ConditionItem::ConditionItem(QTreeWidget* parent)
  : QTreeWidgetItem(parent)
{
  setCheckState(kColumnGreen, Qt::Unchecked);
  setCheckState(kColumnYellow, Qt::Unchecked);
  setCheckState(kColumnRed, Qt::Unchecked);
  setText(kColumnValue, QObject::tr("< условие не задано >"));

  setData(kColumnValue, kRoleOperation, kNoOperation);
  setData(kColumnValue, kRoleOperandA, 0);
  setData(kColumnValue, kRoleOperandB, 0);
  setData(kColumnValue, kRoleOperandCode, "");
}

void ConditionItem::fromProto(const Condition& cond)
{
  setColor(cond.color());
  setData(kColumnValue, Qt::DisplayRole, ConditionEdit::toText(cond));
  setData(kColumnValue, kRoleOperation, int(cond.operation()));
  setData(kColumnValue, kRoleOperandA, cond.operand_a());
  setData(kColumnValue, kRoleOperandB, cond.operand_b());
  setData(kColumnValue, kRoleOperandCode, QString::fromStdString( cond.operandcode() ));
}

Condition ConditionItem::toProto() const
{
  Condition cond;
  cond.set_color(color());
  cond.set_operation(static_cast<tablo::Operation>(data(kColumnValue, kRoleOperation).toInt()));
  cond.set_operand_a(data(kColumnValue, kRoleOperandA).toDouble());
  cond.set_operand_b(data(kColumnValue, kRoleOperandB).toDouble());
  cond.set_operandcode(data(kColumnValue, kRoleOperandCode).toString().toStdString());
  return cond;
}

Color ConditionItem::color() const
{
  if ( Qt::Checked == checkState(kColumnGreen) ) {
    return kGreen;
  }
  if ( Qt::Checked == checkState(kColumnYellow) ) {
    return kYellow;
  }
  if ( Qt::Checked == checkState(kColumnRed) ) {
    return kRed;
  }

  return kNoColor;
}

void ConditionItem::setColor(Color color)
{
  int col = -1;
  switch ( color ) {
    case kGreen:  col = kColumnGreen;  break;
    case kYellow: col = kColumnYellow; break;
    case kRed:    col = kColumnRed;    break;
    case kNoColor:
      break;
  }
  if ( -1 != col ) {
    setCheckState(col, Qt::Checked);
  }
  else {
    setCheckState(kColumnGreen, Qt::Unchecked);
    setCheckState(kColumnYellow, Qt::Unchecked);
    setCheckState(kColumnRed, Qt::Unchecked);
  }
}

} // tablo
} // meteo
