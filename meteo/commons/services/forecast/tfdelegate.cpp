#include "tfdelegate.h"
#include "tfitem.h"
#include "tforecast.h"

#include <qlineedit.h>
#include <qcombobox.h>
#include <qdatetime.h>
#include <qdatetimeedit.h>
#include <cross-commons/debug/tlog.h>
#include <qevent.h>
#include <QApplication>
#include <qtextbrowser.h>
#include <qpainter.h>
#include <qlabel.h>
#include <QDoubleValidator>

TFItemDelegate::TFItemDelegate(QObject *parent)
    :QStyledItemDelegate(parent)
{

}

void TFItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                           const QModelIndex &index) const
{
  Q_UNUSED(painter);
  Q_UNUSED(option);
  Q_UNUSED(index);
  QStyledItemDelegate::paint(painter, option, index);
  return;
}

QWidget *TFItemDelegate::createEditor(QWidget *parent,
    const QStyleOptionViewItem &/* option */,
    const QModelIndex &index ) const
{
  TFItem* item = static_cast<TFItem*>(index.internalPointer());
  if(!item) return nullptr;

  if(1 == index.column()){
    switch (item->role()) {
      case meteo::forecast::ForecastMethodItemRole::kCoordData:
      case meteo::forecast::ForecastMethodItemRole::kNoData:
      case meteo::forecast::ForecastMethodItemRole::kAdvFieldData:
        break;
      case meteo::forecast::ForecastMethodItemRole::kDatetimeData:
        {
          QTimeEdit *dte = new QTimeEdit(parent);
          return dte;
        }
        break;
      case meteo::forecast::ForecastMethodItemRole::kFieldData:
      {
        QLineEdit *le = new QLineEdit(parent);
        le->setValidator(new QRegExpValidator( item->getRegexpByParam() ));
        return le;
      }
      case meteo::forecast::ForecastMethodItemRole::kComboData:
      case meteo::forecast::ForecastMethodItemRole::kScriptableComboData:
      {
        QComboBox * cb = new QComboBox(parent);
        connect(cb, SIGNAL(currentIndexChanged(int)), this, SLOT(emitCommitData(int)));
        return cb;
      }
      break;
      default:
      {
        QLineEdit *le = new QLineEdit(parent);
        le->setValidator(new QRegExpValidator( item->getRegexpByParam() ));
        return le;
      }
      break;
    }
  }
  if(2 == index.column()){
    switch (item->role()) {
      case meteo::forecast::ForecastMethodItemRole::kFieldData:
      case meteo::forecast::ForecastMethodItemRole::kAdvFieldData:
      case meteo::forecast::ForecastMethodItemRole::kSrcData:
      {
        QComboBox * cb = new QComboBox(parent);
        connect(cb, SIGNAL(currentIndexChanged(int)), this, SLOT(emitCommitData(int)));
        return cb;
      }
      break;
      default:
      //  QLabel * cl = new QLabel(parent);
     //   return cl;
      break;
    }
  }
  return nullptr;

}

void TFItemDelegate::emitCommitData(int){
  emit commitData(qobject_cast<QWidget *>(sender()));
}

void TFItemDelegate::setEditorData(QWidget *editor,
                                    const QModelIndex &index) const
{
  TFItem* item = static_cast<TFItem*>(index.internalPointer());
  if(!item) return;
  if(1 == index.column()){
    switch (item->role()) {
      case meteo::forecast::ForecastMethodItemRole::kCoordData:
        break;
      case meteo::forecast::ForecastMethodItemRole::kDatetimeData:
        {QTimeEdit *dte = qobject_cast<QTimeEdit*>(editor);
        if(dte) {
            dte->setTime(item->getItemDataM().toTime());
        }}
        break;
      case meteo::forecast::ForecastMethodItemRole::kScriptableComboData:
      case meteo::forecast::ForecastMethodItemRole::kComboData:
      {
        QComboBox *cb = qobject_cast<QComboBox*>(editor);
        if(cb)
        {
          cb->addItems(item->listData());
          cb->setCurrentIndex(item->currentIndex());
        }
      }
      break;
      case meteo::forecast::ForecastMethodItemRole::kFieldData:
      case meteo::forecast::ForecastMethodItemRole::kAdvFieldData:
      default:
      {
        QLineEdit *le = qobject_cast<QLineEdit*>(editor);
        if(le) {
            if (meteo::forecast::ForecastMethodItemRole::kResultData == item->role()){
              }
            le->setText(item->getItemDataM().toString());
            if (meteo::forecast::ForecastMethodItemRole::kResultData == item->role()){
                le->setReadOnly(true);
              }
        }
      }
      break;
    }
  }
  if(2 == index.column()){
    switch (item->role()) {
      case meteo::forecast::ForecastMethodItemRole::kSrcData:
      case meteo::forecast::ForecastMethodItemRole::kFieldData:
      case meteo::forecast::ForecastMethodItemRole::kAdvFieldData:
       {
        QComboBox *cb = qobject_cast<QComboBox*>(editor);
        if(cb)
        {
          cb->addItems(item->listData());
          cb->setCurrentIndex(item->currentIndex());
        }
      }
      break;
      default:
       // QLabel * cl = qobject_cast<QLabel*>(editor);
      //  if(cl) { cl->setText(item->getItemInfo()); }

      break;
    }
  }

}

void TFItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                   const QModelIndex &index) const
{
  TFItem* item = static_cast<TFItem*>(index.internalPointer());
  if(!item) return;
  QString value;
  if(2 == index.column()){
    switch (item->role()) {
      case meteo::forecast::ForecastMethodItemRole::kFieldData:
      case meteo::forecast::ForecastMethodItemRole::kAdvFieldData:
      case meteo::forecast::ForecastMethodItemRole::kSrcData:
      {
        QComboBox *cb = qobject_cast<QComboBox*>(editor);

        if(cb){
            if(cb->currentIndex() != item->currentIndex()){
                value = QString::number(cb->currentIndex());
                model->setData(index, value, Qt::UserRole);
            }
        }
      }
      break;
      default:
        return;
        //value = item->getItemDataM().toString();
      break;
    }
  }
  if(1 == index.column()){
    switch (item->role()) {
      case meteo::forecast::ForecastMethodItemRole::kCoordData:
        break;
      case meteo::forecast::ForecastMethodItemRole::kDatetimeData:
        {
          QTimeEdit *dte = qobject_cast<QTimeEdit*>(editor);
          if(dte) {
              //dte->setTime(item->getItemDataM().toTime());
              model->setData(index, dte->time(), Qt::UserRole);
            }
        }
        break;
      case meteo::forecast::ForecastMethodItemRole::kScriptableComboData:
      case meteo::forecast::ForecastMethodItemRole::kComboData:
      {
        QComboBox *cb = qobject_cast<QComboBox*>(editor);
        if(cb) {
            value = QString::number(cb->currentIndex());
            model->setData(index, value, Qt::UserRole);
        }
        return;
    }
      break;
      case meteo::forecast::ForecastMethodItemRole::kFieldData:
      case meteo::forecast::ForecastMethodItemRole::kAdvFieldData:
      default:
      {
        QLineEdit *le = qobject_cast<QLineEdit*>(editor);
        //
        if(le){
            value = le->text();
            model->setData(index, value, Qt::EditRole);
        }
        return;
      }
      break;
    }
  }

}

void TFItemDelegate::updateEditorGeometry(QWidget *editor,
    const QStyleOptionViewItem &option, const QModelIndex &/* index */) const
{
  editor->setGeometry(option.rect);
}

bool TFItemDelegate::editorEvent ( QEvent * /*event*/,
                                          QAbstractItemModel * /*model*/,
                                          const QStyleOptionViewItem & /*option*/,
                                          const QModelIndex & /*index*/ )
{
  return false;
}

QSize TFItemDelegate::sizeHint( const QStyleOptionViewItem& option, const QModelIndex& /*index*/ ) const
{
  return QSize(option.fontMetrics.size( Qt::TextExpandTabs ," Точка адвекции 850 гПа (12 ч.) \n"));
}

