#include "delegate.h"

#include <qdialog.h>

#include <cross-commons/debug/tlog.h>

namespace meteo {
namespace tablo {

Delegate::Delegate(QObject* parent)
  : QStyledItemDelegate(parent)
{
  size_ = ConditionEdit().sizeHint();
}

QWidget*Delegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  Q_UNUSED( option );
  Q_UNUSED( index );

  ConditionEdit* ed = new ConditionEdit(parent);
  connect( ed, SIGNAL(visibilityChanged(bool)), SLOT(slotVisibilityChanged(bool)) );
  return ed;
}

void Delegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
  ConditionEdit* e = qobject_cast<ConditionEdit*>(editor);
  if ( nullptr == e ) { return; }

  Condition cond;
  int op = index.data(tablo::kRoleOperation).toInt();
  if ( tablo::Operation_IsValid(op) ) {
    cond.set_operation(static_cast<tablo::Operation>(op));
  }
  cond.set_operand_a(index.data(tablo::kRoleOperandA).toDouble());
  cond.set_operand_b(index.data(tablo::kRoleOperandB).toDouble());
  cond.set_operandcode(index.data(tablo::kRoleOperandCode).toString().toStdString());
  e->fromProto(cond);
}

void Delegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
  ConditionEdit* e = qobject_cast<ConditionEdit*>(editor);
  if ( nullptr == e ) { return; }

  if ( QDialog::Rejected == e->result() ) {
    return;
  }

  Condition cond = e->toProto();
  model->setData(index, int(cond.operation()), tablo::kRoleOperation);
  model->setData(index, cond.operand_a(), tablo::kRoleOperandA);
  model->setData(index, cond.operand_b(), tablo::kRoleOperandB);
  model->setData(index, QString::fromStdString(cond.operandcode()), tablo::kRoleOperandCode);
  model->setData(index, ConditionEdit::toText(cond), Qt::DisplayRole);
}

QSize Delegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  Q_UNUSED( option );
  Q_UNUSED( index );

  return size_;
}

void Delegate::slotVisibilityChanged(bool visible)
{
  if ( visible ) { return; }

  QWidget* editor = qobject_cast<QWidget*>(sender());
  if ( nullptr == editor ) { return; }

  ConditionEdit* e = qobject_cast<ConditionEdit*>(editor);
  if ( nullptr == e ) { return; }

  if ( QDialog::Accepted == e->result() ) {
    emit commitData(editor);
  }

  emit closeEditor(editor);
}

} // tablo
} // meteo
