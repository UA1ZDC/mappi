#include "mapincut.h"

#include <qdebug.h>

#include <cross-commons/debug/tlog.h>
#include <meteo/commons/ui/map/document.h>

namespace meteo {

namespace map {

MapIncut::MapIncut( Document* document, Incut* incut )
 : document_(document),
  incut_(incut)
{
}

QRectF MapIncut::boundingRect() const
{
  if ( 0 != document_ ) {
    return document_->incutBoundingRect(incut_);
  }
  return QRectF();
}

void MapIncut::paint( QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget )
{
  Q_UNUSED(option);
  Q_UNUSED(widget);
  if ( 0 != document_ ) {
    document_->drawIncut(painter);
  }
}

}
}
