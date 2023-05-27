#include "navyitem.h"

#include <meteo/commons/ui/map/puanson.h>

namespace meteo {
namespace map {

NavyItem::NavyItem( Puanson* o )
  : origin_(o)
{
}

NavyItem::~NavyItem()
{
}

QRectF NavyItem::boundingRect() const
{
  return QRectF();
}

void NavyItem::paint( QPainter* p, const QStyleOptionGraphicsItem* o, QWidget* w )
{
  Q_UNUSED(p);
  Q_UNUSED(o);
  Q_UNUSED(w);
}

}
}
