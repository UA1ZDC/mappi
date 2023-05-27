#include <QtGui>

#include "delegate.h"

#include <cross-commons/debug/tlog.h>

#include <QStandardItem>


//! [0]
ComboBoxDelegate::ComboBoxDelegate(QObject *parent,const QList< QStandardItem * > list)
    : QStyledItemDelegate(parent),
      list_(list)
{
  trc;
}
//! [0]

//! [1]
QWidget *ComboBoxDelegate::createEditor(QWidget *parent,
    const QStyleOptionViewItem &/* option */,
    const QModelIndex &/* index */) const
{
  trc;
    QComboBox *editor = new QComboBox(parent);
    for (int i = 0, sz = list_.count(); i < sz; ++i)
    {
      QStandardItem* item = list_.at(i);
      if (0 != item)
      {
        editor->addItem(item->data(Qt::EditRole).toString(), item->data(Qt::UserRole));
      }
    }
    editor->installEventFilter( const_cast<ComboBoxDelegate*>( this ) );

    return editor;
}

//! Предоставление виджета-редактора
void ComboBoxDelegate::setEditorData(QWidget *editor,
                                    const QModelIndex &index) const
{
  trc;
//    int value = index.model()->data(index, Qt::EditRole).toInt();

//    QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
//    spinBox->setValue(value);
//  int value = index.model()->data(index, Qt::EditRole).toString();




//  QVariant value = index.model()->data(index, Qt::UserRole);
//  QComboBox* comboBox = static_cast<QComboBox*>(editor);
//  comboBox->setCurrentIndex(comboBox->findData(value, Qt::UserRole));

  QComboBox *comboBox = static_cast<QComboBox*>(editor);
  QVariant value = index.model()->data(index, Qt::UserRole);
  debug_log << value.toString();
  int idx = comboBox->findData(value, Qt::UserRole);
  if ( idx < 0 )
  {
    trc;
    return;
  }
  comboBox->setCurrentIndex(idx);
  connect(comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(slotChanged(int)));

//  QString value = index.model()->data(index, Qt::EditRole).toString();

//      QComboBox *comboBox = static_cast<QComboBox*>(editor);
//      comboBox->addItem(value);

}

void ComboBoxDelegate::slotChanged(int number)
{
  debug_log << number;
  emit changed(number);
}

//! Установка значения поля после завершения редактирования
void ComboBoxDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                   const QModelIndex &index) const
{
  trc;
//    QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
//    spinBox->interpretText();
//    int value = spinBox->value();

//    model->setData(index, value, Qt::EditRole);


//  QComboBox *comboBox = static_cast<QComboBox*>(editor);
//      QString value = comboBox->itemText(comboBox->currentIndex());
//      model->setData(index, value, Qt::EditRole);



//    QComboBox *comboBox = static_cast<QComboBox*>(editor);
//    QVariant value = comboBox->itemData(comboBox->currentIndex(), Qt::UserRole);
//    model->setData(index, value, Qt::UserRole);

  QComboBox *comboBox = static_cast<QComboBox*>(editor);
  model->setData(index, comboBox->itemData(comboBox->currentIndex(), Qt::EditRole), Qt::EditRole);

}

//void ComboBoxDelegate::paint( QPainter * painter, const QStyleOptionViewItem &option,
//              const QModelIndex &index ) const
//{
//  QStyleOptionViewItem opt = option;
//  int value = index.data().toInt();


//  QString s = list_.value( value )->data(Qt::EditRole);
//  QVariant color = index.data( Qt::TextColorRole );

//  if ( color.isValid() && qvariant_cast<QColor>(color).isValid() )
//    opt.palette.setColor( QPalette::Text, qvariant_cast<QColor>(color) );

//  opt.displayAlignment = Qt::AlignVCenter | Qt::AlignLeft;

//  drawDisplay(painter, opt, opt.rect, s);
//  drawFocus(painter, opt, opt.rect);
//}

void ComboBoxDelegate::updateEditorGeometry(QWidget *editor,
    const QStyleOptionViewItem &option, const QModelIndex &/* index */) const
{
    editor->setGeometry(option.rect);
}
//! [4]
