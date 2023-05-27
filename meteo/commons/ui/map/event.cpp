#include "event.h"

#include "layer.h"

#include <qcoreapplication.h>

#include <cross-commons/debug/tlog.h>

namespace meteo {
namespace map {

EventHandler::EventHandler()
  : muted_(false)
{
}

EventHandler::~EventHandler()
{
}

void EventHandler::notifyMapChanges()
{
  if ( true == muted_ ) {
    return;
  }
  MapEvent* event = new MapEvent;
//  QCoreApplication::removePostedEvents ( this, MapEvent::MapChanged );
  QCoreApplication::postEvent( this, event );
  emit mapChanged();
}

void EventHandler::notifyDocumentChanges( DocumentEvent::ChangeType ch )
{
  if ( true == muted_ ) {
    return;
  }
  DocumentEvent* event = new DocumentEvent(ch);
//  QCoreApplication::removePostedEvents ( this, LayerEvent::LayerChanged );
  QCoreApplication::postEvent( this, event );
  emit mapChanged();
}

void EventHandler::notifyLayerChanges( Layer* layer, LayerEvent::ChangeType ch )
{
  if ( true == muted_ ) {
    return;
  }
  if ( nullptr == layer ) {
    return;
  }
  emit layerChanged(layer, ch);
  LayerEvent* event = new LayerEvent( layer->uuid(), ch);
//  QCoreApplication::removePostedEvents ( this, LayerEvent::LayerChanged );
  QCoreApplication::postEvent( this, event );
}

void EventHandler::postEvent( QEvent* ev )
{
  if ( true == muted_ ) {
    delete ev;
    return;
  }
  QCoreApplication::postEvent( this, ev );
}

bool EventHandler::event( QEvent* ev )
{
  if ( true == muted_ ) {
    return true;
  }
  int t = ev->type();
  if (   MapEvent::MapChanged     != t
      && LayerEvent::LayerChanged != t
      && DocumentEvent::DocumentChanged != t ) {
//    error_log << QObject::tr("Неизвестная ошибка. Тип события = %1").arg(ev->type());
    return QObject::event(ev);
  }
  return true;
}

void EventHandler::muteEvents()
{
  muted_ = true;
//  installEventFilter(this);
}

void EventHandler::turnEvents()
{
  muted_ = false;
//  removeEventFilter(this);
}

bool EventHandler::eventFilter( QObject* watched, QEvent* e )
{
  Q_UNUSED(e);
  if ( this == watched ) {
    return true;
  }
  else {
    error_log << QObject::tr("Непредвиденная ситуация. В фильтре получено событие от постороннего объекта");
  }
  return false;
}

const QEvent::Type MapEvent::MapChanged = static_cast<QEvent::Type>( QEvent::registerEventType() );
const QEvent::Type LayerEvent::LayerChanged = static_cast<QEvent::Type>( QEvent::registerEventType() );
const QEvent::Type DocumentEvent::DocumentChanged = static_cast<QEvent::Type>( QEvent::registerEventType() );

MapEvent::MapEvent()
  :QEvent(MapEvent::MapChanged)
{
}

MapEvent::~MapEvent()
{
}

DocumentEvent::DocumentEvent( ChangeType ch )
  :QEvent(DocumentEvent::DocumentChanged),
  changetype_(ch)
{
}

DocumentEvent::~DocumentEvent()
{
}

LayerEvent::LayerEvent( const QString& uuid, ChangeType ch, const QRect& rect )
  :QEvent(LayerEvent::LayerChanged),
  uuid_(uuid),
  changetype_(ch),
  changerect_(rect)
{
}

LayerEvent::~LayerEvent()
{
}

}
}
