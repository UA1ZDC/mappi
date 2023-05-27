#include "navigatoraction.h"
#include "navigatorwidget.h"

#include <commons/geobasis/geopoint.h>
#include <commons/obanal/tfield.h>

#include <cross-commons/debug/tlog.h>

#include <meteo/commons/global/global.h>

#include <meteo/commons/proto/field.pb.h>
#include <meteo/commons/proto/sprinf.pb.h>
#include <meteo/commons/proto/surface_service.pb.h>

#include <meteo/commons/ui/map/customevent.h>
#include <meteo/commons/ui/map/document.h>
#include <meteo/commons/ui/map/ramka.h>

#include <meteo/commons/ui/map/view/actionbutton.h>
#include <meteo/commons/ui/map/view/mapview.h>
#include <meteo/commons/ui/map/view/mapwindow.h>
#include <meteo/commons/ui/map/view/menu.h>
#include <meteo/commons/rpc/rpc.h>

#include <QIcon>
#include <QGraphicsProxyWidget>
#include <QString>

#include <qapplication.h>
#include <qlayout.h>

namespace
{
  const QIcon actionButtonIcon() { return QIcon(":/meteo/icons/map/navigation.png"); }
  const QString actionButtonToolTip() { return QString::fromUtf8("Навигатор"); }

  int syncallTimeout() { return 10000; }
}

namespace meteo {
namespace map {

static const auto kMenuItem = QPair<QString, QString>("navigatoraction", QObject::tr("Навигатор"));
static const auto kMenuPath = QList<QPair<QString, QString>>
    ({QPair<QString, QString>("view", QObject::tr("Вид"))});

NavigatorAction::NavigatorAction(MapScene* scene) :
  Action(scene, "navigatoraction"),
  navigator_(nullptr),
  activateButton_(nullptr),
  navmenu_(new QMenu(QString::fromUtf8("Навигатор"))),
  hideaction_(nullptr),
  proxy_(nullptr),
  navPosition_(kTopRight),
  drag_(false)
{
  navmenu_->addAction(QIcon(":/meteo/icons/text/text-pos-top-left.png"),
                      QString::fromUtf8("Слева вверху"),
                      this, SLOT(slotNavigatorOnTopLeft()));
  navmenu_->addAction(QIcon(":/meteo/icons/text/text-pos-top-right.png"),
                      QString::fromUtf8("Справа вверху"),
                      this, SLOT(slotNavigatorOnTopRight()));
  navmenu_->addAction(QIcon(":/meteo/icons/text/text-pos-bottom-left.png"),
                      QString::fromUtf8("Слева внизу"),
                      this, SLOT(slotNavigatorOnBottomLeft()));
  navmenu_->addAction(QIcon(":/meteo/icons/text/text-pos-bottom-right.png"),
                      QString::fromUtf8("Справа внизу"),
                      this, SLOT(slotNavigatorOnBottomRight()));
  navmenu_->addAction(QIcon(":/meteo/icons/text/text-pos-top-center.png"),
                      QString::fromUtf8("Наверху"),
                      this, SLOT(slotNavigatorOnTop()));
  navmenu_->addAction(QIcon(":/meteo/icons/text/text-pos-bottom-center.png"),
                      QString::fromUtf8("Внизу"),
                      this, SLOT(slotNavigatorOnBottom()));
  navmenu_->addAction(QIcon(":/meteo/icons/text/text-pos-mid-left.png"),
                      QString::fromUtf8("Слева"),
                      this, SLOT(slotNavigatorOnLeft()));
  navmenu_->addAction(QIcon(":/meteo/icons/text/text-pos-mid-right.png"),
                      QString::fromUtf8("Справа"),
                      this, SLOT(slotNavigatorOnRight()));

  hideaction_ = navmenu_->addAction(QString::fromUtf8("Скрыть"),
                                    this, SLOT(slotHideNavigator()));

//  QAction* act = scene_->mapview()->window()->addActionToMenuFromFile("navigator");
  QAction* act = scene_->mapview()->window()->addActionToMenu(kMenuItem, kMenuPath);
  if ( nullptr != act ) {
    act->setMenu(navmenu_);
    QMenu* pm = qobject_cast<QMenu*>( act->parentWidget() );
    if ( nullptr != pm ) {
      QObject::connect( pm, SIGNAL( aboutToShow() ), this, SLOT( slotAboutToShowParentMenu() ) );
    }
  }
}

NavigatorAction::~NavigatorAction()
{
  if (proxy_ == nullptr) {
    delete navigator_;
    navigator_ = nullptr;
  }

  delete navmenu_;
  navmenu_ = nullptr;
}

void NavigatorAction::addActionsToMenu(Menu* menu) const
{
  if (navigator_ == nullptr) {
    hideaction_->setDisabled(true);
  }
  else {
    hideaction_->setEnabled(true);
  }
  menu->addDocumentAction(navmenu_->menuAction());
}

void NavigatorAction::resizeEvent(QResizeEvent* event)
{
  if (proxy_ != nullptr) {
    proxy_->setPos(navigatorWidgetPosition());
  }
  Action::resizeEvent(event);
}

bool NavigatorAction::eventFilter(QObject* object, QEvent* event)
{
  bool result = Action::eventFilter(object, event);

  if (object == scene_->document()->eventHandler() && navigator_ != nullptr) {
    if (   event->type() == MapEvent::MapChanged
        || event->type() == DocumentEvent::DocumentChanged) {
      navigator_->initFromDocument(scene_->document());
      navigator_->setDisplayRamka(displayRamka());
    }
    else if (event->type() == CustomEvent::Custom) {
      GeoVector selectedPoints;
      CustomEvent* ce = static_cast<CustomEvent*>(event);
      switch (ce->customType()) {
        case CustomEvent::SelectGeoPointsByField: {
            field::SimpleDataRequest* request = dynamic_cast<field::SimpleDataRequest*>(ce->ownedMessage());
            if (request != nullptr) {
              selectedPoints = getPointsFromFieldService(*request);
            }
          }
          break;
        case CustomEvent::SelectGeoPointsBySurface: {
            surf::DataRequest* request = dynamic_cast<surf::DataRequest*>(ce->ownedMessage());
            if (request != nullptr) {
              selectedPoints = getStationsFromSurfaceService(*request);
            }
          }
          break;
        case CustomEvent::ClearGeoPoints:
        default:
          break;
      }

      navigator_->clearAllPoints();
      navigator_->setGeoVector(selectedPoints);
    }
  }
  return result;
}

GeoVector NavigatorAction::getPointsFromFieldService(const field::SimpleDataRequest& request) const
{
  GeoVector result;
  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
  field::DataReply* reply = nullptr;
  {
    meteo::rpc::Channel* ch = meteo::global::serviceChannel(settings::proto::kField);
    reply = ch->remoteCall(&field::FieldService::GetFieldDataPoID, request, ::syncallTimeout());
    delete ch;
  }
  if (reply != nullptr) {
    if (reply->result() == true) {
      QByteArray data(reply->fielddata().data(), reply->fielddata().size());
      obanal::TField fld;
      if (fld.fromBuffer(&data)) {
        result = fieldPoints(fld);
      }
    }
    else {
      error_log << QString::fromUtf8("Ошибка получения данных: %1")
                   .arg(reply->error().c_str());
    }
  }
  delete reply;

  QApplication::restoreOverrideCursor();
  return result;
}

GeoVector NavigatorAction::fieldPoints(const obanal::TField& srcfield) const
{
  GeoVector result;
  result.reserve(srcfield.kolData());
  for (int lat = 0, latCount = srcfield.kolFi(); lat < latCount; ++lat) {
    for (int lon = 0, lonCount = srcfield.kolLa(); lon < lonCount; ++lon) {
      float v = 0.0;
      if (srcfield.getData(lat, lon, &v)) {
        GeoPoint gp(srcfield.getFi(lat), srcfield.getLa(lon));
        result.append(gp);
      }
    }
  }
  return result;
}

GeoVector NavigatorAction::getStationsFromSurfaceService(const surf::DataRequest& request) const
{
  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
  surf::StationsDataReply* reply = nullptr;
  {
      meteo::rpc::Channel* ch = meteo::global::serviceChannel(settings::proto::kSrcData);
      reply = ch->remoteCall(&surf::SurfaceService::GetGmiSenderStations, request, ::syncallTimeout());
      delete ch;
  }

  GeoVector result;
  if (reply != nullptr) {
    if (reply->result() == true) {
      result.reserve(reply->coords_size());
      for (int i = 0, sz = reply->coords_size(); i < sz; ++i) {
        result.append(GeoPoint::fromDegree(reply->coords(i).fi(),reply->coords(i).la()));
      }
    }
    else {
      error_log << QString::fromUtf8("Ошибка получения данных: %1")
                   .arg(reply->comment().c_str());
    }
  }
  delete reply;

  QApplication::restoreOverrideCursor();
  return result;
}

void NavigatorAction::mousePressEvent(QMouseEvent* event)
{
  if (event->button() == Qt::LeftButton) {
    if (proxy_ != nullptr) {
      drag_ = true;
    }
  }
  Action::mousePressEvent(event);
}

void NavigatorAction::mouseMoveEvent(QMouseEvent* event)
{
  if (drag_ && proxy_ != nullptr) {
    proxy_->setPos(navigatorWidgetPosition());
    navigator_->setDisplayRamka(displayRamka());
  }
  Action::mouseMoveEvent(event);
}

void NavigatorAction::mouseReleaseEvent(QMouseEvent* event)
{
  if (event->button() == Qt::LeftButton) {
    if (drag_ && proxy_ != nullptr) {
      drag_ = false;
      proxy_->setPos(navigatorWidgetPosition());
    }
  }
  Action::mouseReleaseEvent(event);
}

GeoVector NavigatorAction::displayRamka() const
{
  return scene_->document()->screen2coord(scene_->document()->ramka()->calcRamka(0,0));
}

QPoint NavigatorAction::navigatorWidgetPosition() const
{
  QPoint result;
  if (   !hasView()
      || navigator_ == nullptr
      || scene_->document() == nullptr) {
    return result;
  }

  const int w = navigator_->width();
  const int h = navigator_->height();
  const int offset = scene_->document()->ramkaWidth() + 5;

  switch (navPosition_) {
    case kTopLeft:
        result = scene_->document()->documentTopLeft() + QPoint(offset,offset);
      break;
    case kTopRight:
        result = scene_->document()->documentTopRight() - QPoint(w,0) + QPoint(-offset,offset);
      break;
    case kBottomLeft:
        result = scene_->document()->documentBottomLeft() - QPoint(0,h) + QPoint(offset,-offset);
      break;
    case kBottomRight:
        result = scene_->document()->documentBottomRight() - QPoint(w,h) + QPoint(-offset,-offset);
      break;
    case kTopCenter:
        result = scene_->document()->documentTop() - QPoint(w/2,0) + QPoint(0,offset);
      break;
    case kBottomCenter:
        result = scene_->document()->documentBottom() - QPoint(w/2,h) + QPoint(0,-offset);
      break;
    case kLeftCenter:
        result = scene_->document()->documentLeft() - QPoint(0,h/2) + QPoint(offset,0);
      break;
    case kRightCenter:
        result = scene_->document()->documentRight() - QPoint(w,h/2) + QPoint(-offset,0);
      break;
    default:
        error_log << QString::fromUtf8("Позиция %1 не поддерживается").arg(navPosition_);
      break;
  }
  return result;
}

void NavigatorAction::slotTurnNavigator(bool active)
{
  if (active == true) {
    turnOnNavigator();
  }
  else {
    turnOffNavigator();
  }
}

void NavigatorAction::turnOnNavigator()
{
  if (hasView() == false ||
      scene_ == nullptr ||
      scene_->document() == nullptr) {
    return;
  }

  if (navigator_ == nullptr) {
    navigator_ = new NavigatorWidget(view()->window());
    navigator_->setObjectName("navigator");
    if (scene_->document()->projection()->type() != MERCAT) {
      navigator_->setStyleSheet("QLabel { border: 1px solid rgb(56,123,248); }");
    }
    navigator_->layout()->setContentsMargins(QMargins());
    navigator_->initFromDocument(scene_->document());
    navigator_->setDisplayRamka(displayRamka());
    navigator_->turnSelectingPosition(true);
    connect(navigator_, SIGNAL(selectedPosition(const meteo::GeoPoint&)), SLOT(slotUpdateCurrentPosition(const meteo::GeoPoint&)));
  }

  if (proxy_ == nullptr) {
    proxy_ = new QGraphicsProxyWidget();
    proxy_->setWidget(navigator_);
    scene_->addItem(proxy_);
    scene_->addVisibleItem(proxy_);
  }
  proxy_->setPos(navigatorWidgetPosition());

  EventHandler* ev = scene_->document()->eventHandler();
  if (ev != nullptr) {
    ev->installEventFilter(this);
  }
}

void NavigatorAction::turnOffNavigator()
{
  if (proxy_ != nullptr) {
    proxy_->setWidget(nullptr);
    scene_->removeVisibleItem(proxy_);
    delete proxy_;
    proxy_ = nullptr;
  }

  EventHandler* ev = scene_->document()->eventHandler();
  if (ev != nullptr) {
    ev->removeEventFilter(this);
  }
}

ActionButton* NavigatorAction::createActionButton(QWidget* parent) const
{
  ActionButton* result = new ActionButton(parent);
  result->setIcon(::actionButtonIcon());
  result->setToolTip(::actionButtonToolTip());
  return result;
}

void NavigatorAction::slotUpdateCurrentPosition(const meteo::GeoPoint& pos)
{
  if (hasView() == false ||
      scene_ == nullptr ||
      scene_->document() == nullptr) {
    return;
  }

  if (pos.isValid() == true) {
    scene_->document()->setScreenCenter(pos);
    view()->setCacheMode( QGraphicsView::CacheNone );
    scene_->invalidate();
    scene_->setDrawState(MapScene::kRenderLayers);
    view()->setCacheMode( QGraphicsView::CacheBackground );
  }
}

void NavigatorAction::slotNavigatorOnTopLeft()
{
  showNavigatorOnPos(kTopLeft);
}

void NavigatorAction::slotNavigatorOnTopRight()
{
  showNavigatorOnPos(kTopRight);
}

void NavigatorAction::slotNavigatorOnBottomLeft()
{
  showNavigatorOnPos(kBottomLeft);
}

void NavigatorAction::slotNavigatorOnBottomRight()
{
  showNavigatorOnPos(kBottomRight);
}

void NavigatorAction::slotNavigatorOnTop()
{
  showNavigatorOnPos(kTopCenter);
}

void NavigatorAction::slotNavigatorOnBottom()
{
  showNavigatorOnPos(kBottomCenter);
}

void NavigatorAction::slotNavigatorOnLeft()
{
  showNavigatorOnPos(kLeftCenter);
}

void NavigatorAction::slotNavigatorOnRight()
{
  showNavigatorOnPos(kRightCenter);
}

void NavigatorAction::showNavigatorOnPos(Position pos)
{
  navPosition_ = pos;
  turnOnNavigator();
}

void NavigatorAction::slotHideNavigator()
{
  turnOffNavigator();
}

void NavigatorAction::slotAboutToShowParentMenu()
{
  if (navigator_ == nullptr) {
    hideaction_->setDisabled(true);
  }
  else {
    hideaction_->setEnabled(true);
  }
}

} // map
} // meteo
