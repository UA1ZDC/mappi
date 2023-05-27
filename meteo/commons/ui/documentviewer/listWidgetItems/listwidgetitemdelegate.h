#ifndef LISTWIDGETITEMDELEGATE_H
#define LISTWIDGETITEMDELEGATE_H

#include "info.h"
#include <QStyledItemDelegate>

class ListWidgetItemDelegate: public QStyledItemDelegate
{
public:
  ListWidgetItemDelegate(const QList<AnyInfo*> docInfo);
  virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
  QSize sizeHint(const QStyleOptionViewItem &option,const QModelIndex &index) const override;

private:
  //void paintHeader( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const;
  void paintDocument(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index, DocumentInfo* object ) const;

const QList<AnyInfo*> docInfo_;

};

#endif // LISTWIDGETITEMDELEGATE_H
