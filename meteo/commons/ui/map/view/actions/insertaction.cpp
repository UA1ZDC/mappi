#include "insertaction.h"
#include <cross-commons/debug/tlog.h>
#include <meteo/commons/ui/map/objectmimedata.h>
#include <meteo/commons/ui/map/view/mapscene.h>
#include <meteo/commons/ui/map/view/mapview.h>
#include <meteo/commons/ui/map/view/menu.h>
#include <meteo/commons/ui/map/geotext.h>
#include <meteo/commons/ui/map/geopixmap.h>
#include <meteo/commons/ui/map/geogroup.h>
#include <meteo/commons/ui/map/geopolygon.h>

#include <qapplication.h>
#include <qclipboard.h>
#include <qaction.h>

namespace meteo {
namespace map {

InsertAction::InsertAction(MapScene* scene)
  : Action(scene, "insertaction" )
{
  showAction_ = new QAction( QObject::tr("Вставить"), this );
  clipboard_ = QApplication::clipboard();
  const ObjectMimeData* data =  qobject_cast< const ObjectMimeData*>(clipboard_->mimeData());
  if ( nullptr != data && true == data->hasSerialObjectsList() ) {
    show_ = true;
  }
  QObject::connect( showAction_, SIGNAL(triggered()), SLOT(slotPaste()) );
  QObject::connect( clipboard_, SIGNAL(dataChanged()), SLOT(slotDataChanged()) );
}

InsertAction::~InsertAction()
{
  clipboard_ = nullptr;
  delete showAction_;
  showAction_ = nullptr;
}
void InsertAction::addActionsToMenu( Menu* menu ) const
{
  if ( true == show_ ) {
    menu->addObjectAction( showAction_ );
  }
}

void InsertAction::slotDataChanged()
{
  const ObjectMimeData* data = qobject_cast< const ObjectMimeData*>( clipboard_->mimeData() );
  if ( nullptr == data ) {
    show_ = false;
    return;
  }
  else {
    show_ = true;
  }
}

void InsertAction::slotPaste()
{
  if ( clipboard_ == nullptr) {
    error_log << QObject::tr("Ошибка. Невозможно выполнить операцию \"Вставить\"");
    return;
  }
  const ObjectMimeData* data =  qobject_cast< const ObjectMimeData*>(clipboard_->mimeData());
  if ( nullptr != data && true == data->hasSerialObjectsList() ) {
    QList< QPair< PrimitiveTypes, std::string > > list = data->serialObjectsList();
    Layer* l = getLayer();
    foreach (auto pair, list) {
      GeoText text(l->projection());
      GeoPixmap pixmap(l->projection());
      GeoPolygon polygon(l->projection());
      GeoGroup geoGroup(l->projection());
      switch (pair.first) {
      case PrimitiveTypes::kText:
        text.parseFromArray(pair.second.c_str());
        text.copy(getLayer());
        break;
      case PrimitiveTypes::kPixmap:
        pixmap.parseFromArray(pair.second.c_str());
        pixmap.copy(getLayer());
        break;
      case PrimitiveTypes::kPolygon:
        polygon.parseFromArray(pair.second.c_str());
        polygon.copy(getLayer());
        break;
      case PrimitiveTypes::kGroup:
        geoGroup.parseChildsFromArray(pair.second.c_str());
        geoGroup.copy(getLayer());
        break;
      default:
        break;
      }
    }
  }
}

void InsertAction::keyPressEvent(QKeyEvent* event)
{
  if ( event->key() == Qt::Key_V ) {
    if ( Qt::KeyboardModifier::ControlModifier == event->modifiers() ) {
      slotPaste();
    }
  }
}

Layer* InsertAction::createLayer()
{
  QString name = tr("Слой пользователя: вставка (%1)")
                   .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm"));
  Layer* l = new Layer(scene_->document(), name);
  l->setActive();
  layerUuid_ = l->uuid();
  return l;
}

Layer* InsertAction::getLayer()
{
  if ( true == scene_->document()->activeLayer()->isBase() ) {
    if ( layerUuid_.isEmpty() ) {
      return createLayer();
    }
    else if ( scene_->document()->hasLayer(layerUuid_) ) {
      scene_->document()->setActiveLayer(layerUuid_);
    }
    return scene_->document()->activeLayer();
  }
  else {
    return scene_->document()->activeLayer();
  }
  return createLayer();
}

}
}
