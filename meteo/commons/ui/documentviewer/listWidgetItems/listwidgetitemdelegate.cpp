#include "listwidgetitemdelegate.h"

#include <QPainter>
#include <QApplication>
#include <QTextLayout>
#include <QVariant>

#include <cross-commons/debug/tlog.h>

#define ITEM_HEIGHT 100
#define OFFSET_H 16
#define DETAIL_OFFSET 30


ListWidgetItemDelegate::ListWidgetItemDelegate(const QList<AnyInfo*> docInfo):
  docInfo_(docInfo)
{

}

void ListWidgetItemDelegate::paint( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const
{
   painter->save();

   DocumentInfo* doc = qobject_cast<DocumentInfo*> (docInfo_[index.row()]);
   if (nullptr != doc){
     paintDocument(painter, option, index, doc);
   }

   painter->restore();
   painter->save();
   painter->setPen(QColor(0xD7, 0xD7, 0xD7));
   painter->drawLine(option.rect.bottomLeft(), option.rect.bottomRight());
   painter->restore();
}

void ListWidgetItemDelegate::paintDocument(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index, DocumentInfo* doc ) const
{
  Q_UNUSED(index);
   QRect tr;
   QString name = doc->getName();
   QString description = doc->dt().toString(Qt::ISODate);
   painter->setClipRect(option.rect);

   painter->translate(option.rect.topLeft());
   painter->translate(10, 10);
   painter->drawText(0, tr.height()-5, name);
   /*

   p.setPen(QColor(210, 210, 210));
   p.setBrush(QColor(240, 240, 240));

   p.setPen(QColor(77, 77, 77));
   p.translate(option.rect.topLeft());

   p.translate(0, sizeHint(option, index).height() - ITEM_HEIGHT);

   p.translate(OFFSET_H, OFFSET_H);

   // рисуем текст
   QFont f = option.font;
   f.setPointSize(10);
   f.setWeight(QFont::Bold);
   QFontMetrics fm(f);
   tr = fm.boundingRect(name);
   p.setFont(f);
   p.drawText(0, tr.height()-5, name);

   // рисуем описание

   p.setFont(option.font);
   fm = QFontMetrics(option.font);

   QString date_ = doc->dt().toString(Qt::ISODate);
   QString versionStr_ = doc->station();



       tr = fm.boundingRect(versionStr_);
       tr.moveTo(option.rect.width() - 2*OFFSET_H - tr.width(), 0 );
       painter->drawText(tr, Qt::TextSingleLine, versionStr_);


   int maxWidth(option.rect.width() -  OFFSET_H - DETAIL_OFFSET );

   description = fm.elidedText(description, Qt::ElideRight, maxWidth);
   tr = fm.boundingRect(description);
   p.drawText(0, tr.height(), description);
*/

}

QSize ListWidgetItemDelegate::sizeHint( const QStyleOptionViewItem & option, const QModelIndex & index ) const
{
  Q_UNUSED(option);
  Q_UNUSED(index);


    return QSize(-1, 150);

}
