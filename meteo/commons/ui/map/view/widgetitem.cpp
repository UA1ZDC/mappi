#include "widgetitem.h"

#include <qwidget.h>
#include <qgraphicsscene.h>

#include <cross-commons/debug/tlog.h>

namespace meteo {
namespace map {

WidgetItem::WidgetItem( QWidget* w )
  : QGraphicsItem(),
  widget_(w)
{
}

WidgetItem::~WidgetItem()
{
  delete widget_; widget_ = 0;
}

QRectF WidgetItem::boundingRect() const
{
  return QRect( QPoint(0,0), widget_->size() );
}

void WidgetItem::paint( QPainter* p, const QStyleOptionGraphicsItem* o, QWidget* w )
{
  Q_UNUSED(o);
  Q_UNUSED(w);
  widget_->render(p, QPoint(), QRegion(), 0 );
}

}
}
