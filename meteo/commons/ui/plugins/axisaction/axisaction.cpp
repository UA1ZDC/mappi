#include "axisaction.h"

#include <qcursor.h>
#include <qevent.h>

#include <cross-commons/debug/tlog.h>

#include <commons/geobasis/geopoint.h>

#include <meteo/commons/global/common.h>
#include <meteo/commons/global/global.h>
#include <meteo/commons/ui/graphitems/geoaxisitem.h>
#include <meteo/commons/ui/graphitems/markeritem.h>
#include <meteo/commons/ui/mainwindow/widgethandler.h>
#include <meteo/commons/ui/map/document.h>
#include <meteo/commons/ui/map/geopolygon.h>
#include <meteo/commons/ui/map/graph/layergraph.h>
#include <meteo/commons/ui/map/layeritems.h>
#include <meteo/commons/ui/map/profile/layerprofile.h>
#include <meteo/commons/ui/map/ramka.h>
#include <meteo/commons/ui/map/view/mapscene.h>
#include <meteo/commons/ui/map/geoaxis.h>
#include <meteo/commons/ui/map/view/mapview.h>
#include <meteo/commons/ui/map/view/mapwindow.h>
#include <meteo/commons/ui/map/view/menu.h>
#include <meteo/commons/ui/map/view/widgetitem.h>
#include <meteo/commons/global/weatherloader.h>
#include <meteo/commons/ui/map/geogroup.h>


namespace meteo {
namespace map {

static QVector<float> mkSequence(float beg, float end, float step)
{
  QVector<float> vec;
  for ( float v = beg; v <= end; v += step ) {
    vec << v;
  }
  return vec;
}

static const double kTopLevel = 100;
static const double kBotLevel = 1000;

const QString AxisAction::kName = "axis_action";

AxisAction::AxisAction(MapScene* scene)
  : Action(scene, AxisAction::kName)
{
  removeAct_ = new QAction(tr("Удалить профиль"), this);
  connect( removeAct_, SIGNAL(triggered(bool)), SLOT(slotDeleteClicked()) );

  auto path = QList<QPair<QString, QString>> ({QPair<QString, QString>("view", "Вид")});
  auto title = QPair<QString, QString>(kName, tr("Значение под курсором (профиль)"));

  menuAct_ = scene_->mapview()->window()->addActionToMenu(title,path);

  if ( nullptr == menuAct_ ) {
    menuAct_ = new QAction(tr("Значение под курсором (вертикально)"), this);
  }
  menuAct_->setCheckable(true);
  menuAct_->setEnabled(false);
  connect( menuAct_, SIGNAL(toggled(bool)), SLOT(slotActionToggled(bool)) );

  if ( scene_->document()->eventHandler() != nullptr ) {
    scene_->document()->eventHandler()->installEventFilter(this);
  }
}

AxisAction::~AxisAction()
{
}

void AxisAction::mouseMoveEvent(QMouseEvent* e)
{
  if ( nullptr == scene_ ) { return; }
  if ( false == ensureItemCreated() ) { return; }

  const QPoint cursorPos = view()->mapToScene(e->pos()).toPoint();
  const GeoPoint cursorGp = scene_->screen2coord(cursorPos);

  const LayerProfile* l = findLayerProfile();
  if ( nullptr == l ) { return; }

  map::Document* doc = l->document();
  if ( nullptr == doc ) { return; }

  // проверка на попадание курсора в bounding rect объекта типа GeoAxis
  Object* o = axisUnderCursor(cursorPos);
  bool hasAxisUnderCursor = false;

  if ( nullptr != o && userProfiles_.contains(o->uuid()) ) {
    hasAxisUnderCursor = true;
  }

  if ( !isActive() ) { return; }

  axisItem_->setGeoPos(GeoPoint(cursorGp.lat(),kBotLevel,0,LA_GENERAL));
  axisItem_->setVisible(!hasAxisUnderCursor);
}

void AxisAction::mousePressEvent(QMouseEvent* e)
{
  if ( !moveDistance_.isNull() ) { moveDistance_.setP1(e->pos()); } else { moveDistance_ = QLine(e->pos(), e->pos()); }
}

void AxisAction::mouseReleaseEvent(QMouseEvent* e)
{
  if ( nullptr == scene_ ) { return; }

  moveDistance_.setP2(e->pos());

  if ( false == ensureItemCreated() ) { return; }

  if ( moveDistance_.length() > 5) { return; }
  if ( !isActive() ) { return; }
  if ( isWidgetUnderMouse(e->pos()) ) { return; }

  updateAxisPos();

  LayerProfile* l = findLayerProfile();
  if ( nullptr == l ) { return; }

  map::Document* doc = l->document();
  if ( nullptr == doc ) { return; }

  QPoint cursorPos = view()->mapToScene(e->pos()).toPoint();

  GeoPoint cursorGp = scene_->screen2coord(cursorPos);

  if ( axisItem_->isVisible() ) {
    QString uuid = l->addProfile(cursorGp.lat(), mkSequence(kTopLevel,kBotLevel,100));
    userProfiles_.insert(uuid);
    axisItem_->setVisible(false);
  }
}

void AxisAction::wheelEvent(QWheelEvent* e)
{
  Q_UNUSED( e );

  if ( nullptr == scene_ ) { return; }
  if ( !isActive() ) { return; }

  updateAxisPos();
}

void AxisAction::addActionsToMenu(Menu* menu) const
{
  if ( nullptr == menu ) { return; }

  if ( !hasLayerProfile() ) { return; }

  menu->addLayerAction(menuAct_);

  if ( nullptr == scene_ ) { return; }

  QPoint pos = view()->mapFromGlobal(QCursor::pos());
  pos = view()->mapToScene(pos).toPoint();


  Object* o = axisUnderCursor(pos);
  if ( nullptr != o && userProfiles_.contains(o->uuid()) ) {
    removeAct_->setData(o->uuid());
    menu->addLayerAction(removeAct_);
  }
}

bool AxisAction::eventFilter(QObject* obj, QEvent* e)
{
  if ( scene_->document()->eventHandler() == obj ) {
    if ( e->type() == map::DocumentEvent::DocumentChanged ) {
      DocumentEvent* event = static_cast<DocumentEvent*>(e);
      if ( nullptr != event ) {
        delete axisItem_;
        axisItem_ = nullptr;
      }
    }
  }

  return Action::eventFilter(obj, e);
}

void AxisAction::deactivate()
{
  if ( nullptr == menuAct_ ) { return; }

  menuAct_->setChecked(false);
}

bool AxisAction::isActive() const
{
  if ( nullptr != menuAct_ ) {
    return menuAct_->isChecked();
  }
  return false;
}

LayerProfile* AxisAction::findLayerProfile() const
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

bool AxisAction::hasLayerProfile() const
{
  return findLayerProfile() != nullptr;
}

bool AxisAction::isWidgetUnderMouse(const QPointF& screenPos) const
{
  QPointF scenePos = view()->mapToScene(screenPos.toPoint());
  QList<QGraphicsItem*> items = scene_->items(scenePos);

  for ( int i=0,isz=items.size(); i<isz; ++i ) {
    if ( nullptr != qgraphicsitem_cast<QGraphicsProxyWidget*>(items.at(i)) ) {
      return true;
    }
  }

  return false;
}

void AxisAction::updateAxisPos()
{
  if ( false == ensureItemCreated() ) { return; }

  QPoint scenePos = view()->mapToScene(QCursor::pos()).toPoint();
  GeoPoint cursorGp = scene_->screen2coord(scenePos);

  axisItem_->setGeoPos(GeoPoint(cursorGp.lat(),kBotLevel,0,LA_GENERAL));
}

bool AxisAction::ensureItemCreated()
{
  if ( nullptr == findLayerProfile() ) {
    return false;
  }

  if ( nullptr == axisItem_ ) {
    axisItem_ = new graph::GeoAxisItem;
    axisItem_->setZValue(20);
    view()->mapscene()->addItem(axisItem_);

    auto map = map::WeatherLoader::instance()->punchlibraryspecial();
    axisItem_->setPuanson(map["vcutl_levels"]);
    axisItem_->setLength(kBotLevel-kTopLevel);
    axisItem_->setRange(kTopLevel,kBotLevel);
    axisItem_->setTickVector(mkSequence(kTopLevel,kBotLevel,100), 20);
    axisItem_->setTickVector(mkSequence(kTopLevel,kBotLevel, 50), 18);
    axisItem_->setTickVector(mkSequence(kTopLevel,kBotLevel, 10), 15);
    axisItem_->setTickVector(mkSequence(kTopLevel,kBotLevel,  5), 12);
    axisItem_->setVisible(false);

    menuAct_->setEnabled(true);
  }

  return true;
}

Object*AxisAction::axisUnderCursor(const QPoint& scenePos) const
{
  const LayerProfile* l = findLayerProfile();
  if ( nullptr == l ) { return nullptr; }
  const std::unordered_set<Object*> objList = l->objects();
  for ( Object* o : objList ) {
    GeoGroup* gr = mapobject_cast<GeoGroup*>(o);
    if ( nullptr == gr ) { continue; }
    const std::unordered_set<Object*> geoaxisList = gr->objectsByType(kGeoAxisType);
    for ( Object* obj : geoaxisList ) {
      if ( obj->minimumScreenDistance(scenePos) <= 0 ) {
        return o;
      }
    }
  }
  return nullptr;
}

void AxisAction::slotActionToggled(bool toggled)
{
  if ( nullptr == axisItem_ ) { return; }

  axisItem_->setVisible(toggled);
}

void AxisAction::slotDeleteClicked()
{
  if ( nullptr == scene_ ) { return; }

  map::LayerProfile* l = findLayerProfile();
  if ( nullptr == l ) { return; }

  QString uuid = removeAct_->data().toString();
  if ( !uuid.isEmpty() ) {
    delete l->objectByUuid(uuid);
  }

  userProfiles_.remove(uuid);
}


} // map
} // meteo
