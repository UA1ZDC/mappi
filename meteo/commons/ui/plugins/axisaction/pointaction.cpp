#include "pointaction.h"

#include <qcursor.h>
#include <qevent.h>

#include <cross-commons/debug/tlog.h>

#include <commons/geobasis/geopoint.h>

#include <meteo/commons/global/common.h>
#include <meteo/commons/global/global.h>
#include <meteo/commons/ui/graphitems/puansonitem.h>
#include <meteo/commons/ui/mainwindow/widgethandler.h>
#include <meteo/commons/ui/map/document.h>
#include <meteo/commons/ui/map/puanson.h>
#include <meteo/commons/ui/map/geopolygon.h>
#include <meteo/commons/ui/map/graph/layergraph.h>
#include <meteo/commons/ui/map/layeritems.h>
#include <meteo/commons/ui/map/profile/layerprofile.h>
#include <meteo/commons/ui/map/ramka.h>
#include <meteo/commons/ui/map/view/mapscene.h>
#include <meteo/commons/ui/map/view/mapview.h>
#include <meteo/commons/ui/map/view/mapwindow.h>
#include <meteo/commons/ui/map/view/menu.h>
#include <meteo/commons/ui/map/view/widgetitem.h>
#include <meteo/commons/global/weatherloader.h>
#include <meteo/commons/ui/map/geogroup.h>


namespace meteo {
namespace map {

const QString PointAction::kName = "point_puanson_action";

PointAction::PointAction(MapScene* scene)
  : Action(scene, PointAction::kName)
{
  QToolButton* btn = new QToolButton;
  btn->setMaximumSize(QSize(16,16));
  btn->setMinimumSize(QSize(16,16));
  btn->setIcon(QIcon(":/meteo/icons/map/close.png"));
  btn->setIconSize(QSize(8,8));
  removeBtnItem_ = new map::WidgetItem(btn);
  removeBtnItem_->setZValue(100);

  auto path = QList<QPair<QString, QString>> ({QPair<QString, QString>("view", "Вид")});
  auto title = QPair<QString, QString>(kName, tr("Значение под курсором (пуансон)"));

  menuAct_ = scene_->mapview()->window()->addActionToMenu(title,path);

  if ( nullptr == menuAct_ ) {
    menuAct_ = new QAction(tr("Значение под курсором (пуансон)"), this);
  }
  menuAct_->setCheckable(true);
  menuAct_->setEnabled(false);
  connect( menuAct_, SIGNAL(toggled(bool)), SLOT(slotActionToggled(bool)) );
}

PointAction::~PointAction()
{
  if ( nullptr != this->removeBtnItem_ ){
    delete this->removeBtnItem_;
    this->removeBtnItem_ = nullptr;
  }
}

void PointAction::mouseMoveEvent(QMouseEvent* e)
{
  if ( 0 == scene_ ) { return; }
  if ( false == ensureItemCreated() ) { return; }

  const QPoint cursorPos = view()->mapToScene(e->pos()).toPoint();
  const GeoPoint cursorGp = scene_->screen2coord(cursorPos);

  const LayerProfile* l = findLayerProfile();
  if ( nullptr == l ) { return; }

  map::Document* doc = l->document();
  if ( nullptr == doc ) { return; }

  // проверка на попадание курсора в bounding rect объекта типа GeoAxis
  QRect objRect;
  QString objUuid;
  bool hasItemUnderCursor = false;
  Puanson* puan = puansonUnderCursor(cursorPos);
  if ( nullptr != puan ) {
    hasItemUnderCursor = true;
    objUuid = puan->uuid();
    objRect = puan->boundingRect(doc->transform()).first();
  }

  if ( hasItemUnderCursor ) {
    puansonItem_->setGeoPos(puan->skelet().first());
    puansonItem_->update(puansonItem_->boundingRect());
    puansonItem_->setVisible(true);
    removeBtnItem_->setData(kRemoveUuid,objUuid);
    setSelected(true);
  }
  else {
    setSelected(false);
    puansonItem_->update(puansonItem_->boundingRect());
    puansonItem_->setGeoPos(cursorGp);
    puansonItem_->setVisible(isActive());
  }

  if ( !isActive() ) { return; }
}

void PointAction::mousePressEvent(QMouseEvent* e)
{
  if ( !moveDistance_.isNull() ) { moveDistance_.setP1(e->pos()); } else { moveDistance_ = QLine(e->pos(), e->pos()); }
}

void PointAction::mouseReleaseEvent(QMouseEvent* e)
{
  if ( 0 == scene_ ) { return; }

  moveDistance_.setP2(e->pos());

  // удаляем элемент "подпись" если под курсором мыши была кнопка "закрыть"
  if ( removeBtnItem_->isUnderMouse() ) {
    slotDeleteClicked();
    return;
  }

  if ( false == ensureItemCreated() ) { return; }

  updateScenePos();

  QPoint cursorPos = view()->mapToScene(e->pos()).toPoint();

  // обновляем (при необходимости) позицию кнопки, на случай если была передвинута карта
  if ( nullptr != removeBtnItem_->scene() ) {
    QRect rect = puansonItem_->boundingRect().toRect();
    QPoint pos = puansonItem_->pos().toPoint();
    pos.rx() += rect.right() - removeBtnItem_->boundingRect().width() - 3;
    pos.ry() += rect.top() + 3;
    removeBtnItem_->setPos(pos);
  }

  if ( moveDistance_.length() > 5) { return; }
  if ( !isActive() ) { return; }
  if ( isWidgetUnderMouse(e->pos()) ) { return; }

  if ( puansonItem_->borderWidth() > 0 ) { return; }

  LayerProfile* l = findLayerProfile();
  if ( nullptr == l ) { return; }

  GeoPoint cursorGp = scene_->screen2coord(cursorPos);
  auto mdVec = l->getDataByCoords({ cursorGp });
  if ( !mdVec.isEmpty() ) {
    Puanson* puan = new Puanson(l);
    puan->setPunch(puansonItem_->puanson());
    puan->setMeteodata(mdVec.first());
    puan->setSkelet(cursorGp);

    removeBtnItem_->setData(kRemoveUuid,puan->uuid());
    setSelected(true);
    puansonItem_->setGeoPos(puan->skelet().first());
    puansonItem_->update(puansonItem_->boundingRect());
  }
}

void PointAction::wheelEvent(QWheelEvent* e)
{
  Q_UNUSED( e );

  if ( 0 == scene_ ) { return; }
  if ( !isActive() ) { return; }

  updateScenePos();
}

void PointAction::addActionsToMenu(Menu* menu) const
{
  if ( 0 == menu ) { return; }

  if ( !hasLayerProfile() ) { return; }

  menu->addLayerAction(menuAct_);

  if ( 0 == scene_ ) { return; }

  QPoint pos = view()->mapFromGlobal(QCursor::pos());
  pos = view()->mapToScene(pos).toPoint();

  QGraphicsItem* item = scene_->itemAt(pos, view()->transform() );

  if ( 0 == item ) { return; }

  while ( item->parentItem() != 0 ) {
    item = item->parentItem();
  }
}

void PointAction::deactivate()
{
  if ( nullptr == menuAct_ ) { return; }

  menuAct_->setChecked(false);
}

bool PointAction::isActive() const
{
  if ( 0 != menuAct_ ) {
    return menuAct_->isChecked();
  }
  return false;
}

LayerProfile* PointAction::findLayerProfile() const
{
  if ( nullptr == scene_ ) { return nullptr; }

  foreach ( Layer* l, scene_->document()->layers() ) {
    LayerProfile* layer = maplayer_cast<LayerProfile*>(l);
    if ( nullptr != layer ) {
      return layer;
    }
  }
  return nullptr;
}

bool PointAction::hasLayerProfile() const
{
  return findLayerProfile() != nullptr;
}

bool PointAction::isWidgetUnderMouse(const QPointF& screenPos) const
{
  QPointF scenePos = view()->mapToScene(screenPos.toPoint());
  QList<QGraphicsItem*> items = scene_->items(scenePos);

  for ( int i=0,isz=items.size(); i<isz; ++i ) {
    if ( 0 != qgraphicsitem_cast<QGraphicsProxyWidget*>(items.at(i)) ) {
      return true;
    }
  }

  return false;
}

void PointAction::updateScenePos()
{
  if ( false == ensureItemCreated() ) { return; }

  puansonItem_->setGeoPos(puansonItem_->geoPos());
}

bool PointAction::ensureItemCreated()
{
  if ( nullptr == findLayerProfile() ) {
    return false;
  }

  if ( nullptr == puansonItem_ ) {
    puansonItem_ = new graph::PuansonItem;
    puansonItem_->setZValue(20);
    view()->mapscene()->addItem(puansonItem_);

    auto map = map::WeatherLoader::instance()->punchlibraryspecial();
    puansonItem_->setPuanson(map["vcutl_levels"]);
    puansonItem_->setVisible(false);

    menuAct_->setEnabled(true);
  }

  return true;
}

Puanson* PointAction::puansonUnderCursor(const QPoint& screenPos) const
{
  const LayerProfile* l = findLayerProfile();
  if ( nullptr == l ) { return nullptr; }

  map::Document* doc = l->document();
  if ( nullptr == doc ) { return nullptr; }

  const QList<Object*> objList = l->objectsUnderGeoPoint(doc->screen2coord(screenPos));
  for ( Object* o : objList ) {
    Puanson* puan = mapobject_cast<Puanson*>(o);
    if ( nullptr != puan ) {
      return puan;
    }
  }
  return nullptr;
}

void PointAction::setSelected(bool flag)
{
  if ( true == flag ) {
    puansonItem_->setBorderStyle(QColor(255,100,100), 1);
    puansonItem_->setBackgroundStyle(QColor(255,255,255,120), Qt::SolidPattern);

    if ( nullptr == removeBtnItem_->scene() ) {
      scene_->addItem(removeBtnItem_);

      QRect rect = puansonItem_->boundingRect().toRect();
      QPoint pos = puansonItem_->pos().toPoint();
      pos.rx() += rect.right() - removeBtnItem_->boundingRect().width() - 3;
      pos.ry() += rect.top() + 3;
      removeBtnItem_->setPos(pos);
    }
  }
  else {
    puansonItem_->setBorderStyle(QColor(Qt::black), 0);
    puansonItem_->setBackgroundStyle(QColor(Qt::black), Qt::NoBrush);

    if ( nullptr != removeBtnItem_->scene() ) {
      scene_->removeItem(removeBtnItem_);
      removeBtnItem_->setData(kRemoveUuid, QVariant());
    }
  }
}

void PointAction::slotActionToggled(bool toggled)
{
  if ( nullptr == puansonItem_ ) { return; }

  puansonItem_->setVisible(toggled);
}

void PointAction::slotDeleteClicked()
{
  if ( 0 == scene_ ) { return; }

  map::LayerProfile* l = findLayerProfile();
  if ( nullptr == l ) { return; }

  QString uuid = removeBtnItem_->data(kRemoveUuid).toString();
  if ( !uuid.isEmpty() ) {
    delete l->objectByUuid(uuid);

    updateScenePos();
    setSelected(false);
    puansonItem_->update(puansonItem_->boundingRect());
  }

  if ( 0 != removeBtnItem_->scene() ) {
    scene_->removeItem(removeBtnItem_);
  }
}


}
}
