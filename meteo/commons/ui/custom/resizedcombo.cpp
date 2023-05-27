#include "resizedcombo.h"

#include <qevent.h>
#include <qfontdatabase.h>
#include <qlistview.h>
#include <qmenu.h>
#include <qpainter.h>

ResizedComboItemDelegate::ResizedComboItemDelegate( QWidget* p )
  : QAbstractItemDelegate(p)
{
}

void ResizedComboItemDelegate::paint(QPainter* p,
                                const QStyleOptionViewItem& o,
                                const QModelIndex& i ) const
{
  QString text = i.data(Qt::DisplayRole).toString();
  QRect r = o.rect;

  if (o.state & QStyle::State_Selected) {
    
    p->save();
    p->setBrush(o.palette.highlight());
    p->setPen(Qt::NoPen);
    p->drawRect(o.rect);
    p->setPen(QPen(o.palette.highlightedText(), 0));
  }

  r.setLeft(r.left() + 4);

  p->save();
  p->drawText(r, Qt::AlignLeft|Qt::TextDontClip, text);
  p->restore();

  if (o.state & QStyle::State_Selected) {
    p->restore();
  }

}

QSize ResizedComboItemDelegate::sizeHint(const QStyleOptionViewItem& o, const QModelIndex& i ) const
{
  QString text = i.data(Qt::DisplayRole).toString();
  QFont font(o.font);
  QFontMetrics fontMetrics(font);
  return QSize(fontMetrics.width(text), fontMetrics.height());
}

ResizedComboBox::ResizedComboBox( QWidget* p )
  : QComboBox(p)
{
  
}

ResizedComboBox::~ResizedComboBox()
{
}


QSize ResizedComboBox::sizeHint() const
{
    QSize sz = QComboBox::sizeHint();
  //  QFontMetrics fm(font());
  //  sz.setWidth(fm.width(QLatin1Char('m'))*14);
    return sz;
}

bool ResizedComboBox::event( QEvent* e )
{
  if (e->type() == QEvent::Resize) {
      QListView *lview = qobject_cast<QListView*>(view());
      if (lview){
        QFontMetrics fontMetrics(font());
        lview->window()->setFixedWidth(fontMetrics.width(currentText())*1.2);
      }
  }
  return QComboBox::event(e);
}


