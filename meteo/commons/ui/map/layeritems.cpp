#include "layeritems.h"

#include <qdebug.h>
#include <quuid.h>
#include <qpainter.h>
#include <qgraphicsitem.h>
#include <qstyleoption.h>

#include <cross-commons/debug/tlog.h>
#include <sql/psql/psqlquery.h>
#include <commons/geobasis/projection.h>
#include <meteo/commons/global/global.h>

#include "map.h"
#include "document.h"
#include "object.h"
#include "geomrl.h"
#include "geotext.h"
#include "event.h"
#include <meteo/commons/global/weatherloader.h>
#include "puanson.h"
#include <meteo/commons/global/radarparams.h>

namespace meteo {
namespace map {

LayerItems::LayerItems ( Document* m )
  : Layer ( m, QObject::tr("Слой пользователя: задачи") )
{
}

LayerItems::~LayerItems()
{
  while ( 0 != items_.size() ) {
    delete items_[0];
  }
}

void LayerItems::addItem( QGraphicsItem* item )
{
  if ( nullptr == item ) {
    return;
  }
  if ( true == items_.contains(item) ) {
    return;
  }
  items_.append(item);
}

void LayerItems::removeItem( QGraphicsItem* item )
{
  if ( items_.contains(item) ) {
    items_.removeAll(item);
  }
}

bool LayerItems::hasItem( QGraphicsItem* item ) const
{
  return hasItem( item, items_ );
}

bool LayerItems::hasItem( QGraphicsItem* item, const QList<QGraphicsItem*>& childs ) const
{
  for ( int i = 0, sz = childs.size(); i < sz; ++i ) {
    if ( item == childs[i] ) {
      return true;
    }
    QList<QGraphicsItem*> ch = childs[i]->childItems();
    if ( 0 != ch.size() ) {
      if ( true == hasItem( item, ch ) ) {
        return true;
      }
    }
  }
  return false;
}

namespace {
  bool zLevelOrder( QGraphicsItem* item1, QGraphicsItem* item2 )
  {
    return item1->zValue() < item2->zValue();
  }
}

void LayerItems::render( QPainter* painter, const QRect& target, Document* document )
{
  Q_UNUSED(target);
  if ( false == visible() ) {
    return;
  }
  Document* olddoc = document_;
  document_ = document;
  QStyleOptionGraphicsItem style;
  qStableSort( items_.begin(), items_.end(), zLevelOrder );
  QList<QGraphicsItem*> already;
  for ( int i = 0, sz = items_.size(); i < sz; ++i ) {
    QGraphicsItem* item = items_[i];
    renderItem( painter, item, &style, &already );
  }
  document_ = olddoc;
}

void LayerItems::renderItem( QPainter* painter, QGraphicsItem* item, QStyleOptionGraphicsItem* style, QList<QGraphicsItem*>* already )
{
  if ( nullptr == item || false == item->isVisible() ) {
    return;
  }
  if ( true != already->contains(item) ) {
    item->paint( painter, style );
    already->append(item);
  }
  QList<QGraphicsItem*> childs = item->childItems();
  for ( int i = 0, sz = childs.size(); i < sz; ++i ) {
    renderItem( painter, childs[i], style, already );
  }
}

}
}
