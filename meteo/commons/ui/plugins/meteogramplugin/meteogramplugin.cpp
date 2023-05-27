#include "meteogramplugin.h"

#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/paths.h>

#include <commons/geobasis/generalproj.h>

#include <meteo/commons/global/global.h>
#include <meteo/commons/global/log.h>
#include <meteo/commons/ui/map/graph/funcs.h>
#include <meteo/commons/ui/graph/coordaction/graphcoordaction.h>
#include <meteo/commons/ui/graph/valueaction/graphvalueaction.h>
#include <meteo/commons/ui/mainwindow/mainwindow.h>
#include <meteo/commons/ui/mainwindow/mdisubwindow.h>
#include <meteo/commons/ui/mainwindow/widgethandler.h>
#include <meteo/commons/ui/map/view/actions/scaleaction.h>
#include <meteo/commons/ui/map/view/mapwindow.h>

#include "meteogramaction.h"

#include <meteo/commons/rpc/channel.h>
#include <meteo/commons/proto/surface_service.pb.h>
#include <commons/textproto/pbtools.h>
#include <meteo/commons/ui/custom/stationwidget.h>

#include <sql/nosql/nosqlquery.h>

namespace meteo {

const QString kRootPath = MnCommon::projectPath();
QString MeteogramPlugin::basePath = QDir::homePath() + "/.meteo/meteogram_window/";
MeteogramPlugin* MeteogramPlugin::instance_ = nullptr;

/*!
 * \brief kMenuPath - Путь к пункту меню { id: "meteograms", title: "Метеограммы"} ->
 * Для добавления вложенности нужно добавить пару в список
*/
static const QString& meteograms = QObject::tr("meteograms");
static const QString& meteogramsT = QObject::tr("meteogram-t");
static const QString& meteogramsTr = QObject::tr("meteogram-tr");
static const QString& meteogramsP = QObject::tr("meteogram-p");
static const QString& meteogramsH = QObject::tr("meteogram-h");
static const QString& meteogramsPQNH = QObject::tr("meteogram-pqnh");
static const QString& meteogramsW = QObject::tr("meteogram-w");

MeteogramPlugin* MeteogramPlugin::instance(){
  return instance_;
}


MeteogramPlugin::MeteogramPlugin()
  : app::MainWindowPlugin("meteogramv2plugin")
{
  if(nullptr == mainwindow()) {
    return;
  }
  if (nullptr != instance_) {
    warning_log << QObject::tr("Перезапись instance у плагина");
  }
  instance_ = this;

  for ( const QString& actionId :  { meteogramsT, meteogramsTr, meteogramsP, meteogramsH, meteogramsPQNH, meteogramsW } ){
    QAction* action = mainwindow()->findAction(actionId);
    if ( nullptr == action ){
      warning_log.msgBox() << msglog::kTargetMenyNotFound.arg(actionId);
    }
    else {
      action->setDisabled(false);
      QObject::connect( action, &QAction::triggered, this, &MeteogramPlugin::slotCreateDocument );
    }
  }

  meteo::dbusConnect(this, SLOT(slotDBusHandler(quint32,QString)));
}

MeteogramPlugin::~MeteogramPlugin()
{
}

MeteogramWindow* MeteogramPlugin::slotCreateDocument()
{
  WidgetHandler::instance()->mainwindow()->setCursor(Qt::WaitCursor);

  map::MapWindow* mapWindow = MeteogramWindow::createWindow(window_);
  map::Document* document = MeteogramWindow::createDocument(mapWindow);

  mapWindow->show();

  MeteogramAction* act = new MeteogramAction(mapWindow->mapscene(), document);
  mapWindow->mapscene()->addAction(act, kTopRight, QPoint(1,0));
  act->slotShow(true);
  MeteogramWindow* w = act->window();

  w->slotLoadState();
  WidgetHandler::instance()->mainwindow()->unsetCursor();
  mapWindow->loadBaseLayersVisibilitySettings();

  QAction* actionId = qobject_cast<QAction*>(QObject::sender());
  if ( nullptr != actionId ){
    bool isT = false, isTr = false, isP = false, isH = false, isPQNH = false, isW = false;
    const QString& actionName = actionId->objectName();
    if ( actionName == meteogramsT ) {
      isT = true;
    }
    else if ( actionName == meteogramsTr ){
      isTr = true;
    }
    else if ( actionName == meteogramsP ){
      isP = true;
    }
    else if ( actionName == meteogramsH ){
      isH = true;
    }
    else if ( actionName == meteogramsPQNH ) {
      isPQNH = true;
    }
    else if ( actionName == meteogramsW ){
      isW = true;
    }
    w->setButtonsState(isT, isTr, isP, isH, isPQNH, isW);
  }

  return w;
}

MeteogramWindow* MeteogramPlugin::findMeteogramWindow(){
  auto mainWindow = mainwindow();
  for (auto subWindow : mainWindow->mdi()->subWindowList()){
    auto mapWindow = qobject_cast<meteo::map::MapWindow*>(subWindow->widget());
    if (nullptr == mapWindow) continue;

    for (auto children: mapWindow->children()){
      MeteogramWindow* window = qobject_cast<MeteogramWindow*>(children);
      if ( nullptr != window && !window->isContainsMap() ) return window;
    }
  }
  return nullptr;
}


void MeteogramPlugin::slotDBusHandler(quint32 , const QString &key){
  if ( QString("default") == key ) { // hot fix
    return;
  }
  auto callbackData = getCallback(key);
  if (!callbackData.IsInitialized()) return;

  MeteogramWindow* window = findMeteogramWindow();
  if (nullptr == window){
    window = slotCreateDocument();
  }

  window->setBeginDateTime(NosqlQuery::datetimeFromString(callbackData.begindatetime()));
  window->setEndDateTime(NosqlQuery::datetimeFromString(callbackData.enddatetime()));

  window->stationEdit()->findStation(QString::fromStdString(callbackData.stationindex()), callbackData.stationtype());
  window->setButtonsState(callbackData.isbuildt(), callbackData.isbuilddewpoint(), callbackData.isbuildp(), callbackData.isbuildu(), callbackData.isbuildpqnh(), callbackData.isbuildwind());
  window->slotRun();
}



meteo::ui::proto::MeteogramWindowState MeteogramPlugin::getCallback(const QString &key){
  QString filename= basePath + key;
  QFile file( filename );
  meteo::ui::proto::MeteogramWindowState windowState;
  if ( file.open(QIODevice::ReadWrite) )
  {
      QTextStream stream( &file );
      QString data = stream.readAll();
      windowState.ParseFromString(data.toStdString());
      file.remove();
  }
  return windowState;
}

void MeteogramPlugin::addCallback(const QString &key, const meteo::ui::proto::MeteogramWindowState &args){
  if ( !QDir("/").exists(basePath) ) {
    QDir("/").mkpath(basePath);
  }
  QString filename= basePath + key;
  QFile file( filename );
  if ( file.open(QIODevice::WriteOnly) )
  {
      QTextStream stream( &file );
      stream << QString::fromStdString( args.SerializeAsString() );
  }
}

} // meteo


