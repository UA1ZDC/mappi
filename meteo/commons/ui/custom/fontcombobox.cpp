#include "fontcombobox.h"

#include <qpainter.h>
#include <qfontdatabase.h>
#include <qevent.h>
#include <qlistview.h>

FontItemDelegate::FontItemDelegate( QWidget* p )
  : QAbstractItemDelegate(p)
{
}

void FontItemDelegate::paint(QPainter* p,
                                const QStyleOptionViewItem& o,
                                const QModelIndex& i ) const
{
  QString text = i.data(Qt::DisplayRole).toString();
  QFont font(o.font);
  font.setPointSize(QFontInfo(font).pointSize() * 3 / 2);
  QFont font2 = font;
  font2.setFamily(text);

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
  p->setFont(font);
  p->drawText(r, Qt::AlignVCenter|Qt::AlignLeading|Qt::TextSingleLine, text);


  int w = p->fontMetrics().width(text + QLatin1String("  "));
  p->save();
  p->setFont(font2);
  QString sample = QFontDatabase().writingSystemSample( QFontDatabase::Latin);
  r.setLeft(r.left() + w);
  p->drawText(r, Qt::AlignVCenter|Qt::AlignLeading|Qt::TextSingleLine, sample);

  p->restore();
  p->restore();

  if (o.state & QStyle::State_Selected) {
    p->restore();
  }

}

QSize FontItemDelegate::sizeHint(const QStyleOptionViewItem& o, const QModelIndex& i ) const
{
  QString text = i.data(Qt::DisplayRole).toString();
  QFont font(o.font);
  font.setPointSize(QFontInfo(font).pointSize() * 3/2);
  QFontMetrics fontMetrics(font);
  return QSize(fontMetrics.width(text), fontMetrics.height());
}

FontComboBox::FontComboBox( QWidget* p )
  : QComboBox(p)
{
}

FontComboBox::~FontComboBox()
{
}

QSize FontComboBox::sizeHint() const
{
    QSize sz = QComboBox::sizeHint();
    QFontMetrics fm(font());
    sz.setWidth(fm.width(QLatin1Char('m'))*14);
    return sz;
}

bool FontComboBox::event( QEvent* e )
{
  if (e->type() == QEvent::Resize) {
      QListView *lview = qobject_cast<QListView*>(view());
      if (lview)
          lview->window()->setFixedWidth(width() * 6 / 3);
  }
  return QComboBox::event(e);
}
