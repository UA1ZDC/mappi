#include "mapwindow.h"

#include <quuid.h>
#include <qfileinfo.h>
#include <qfile.h>
#include <qdir.h>

#include <cross-commons/app/paths.h>

#include <sql/psql/psqlquery.h>

#include <commons/geobasis/geopoint.h>
#include <commons/textproto/tprototext.h>
#include <commons/textproto/pbtools.h>

#include <meteo/commons/global/global.h>
#include <meteo/commons/ui/mainwindow/mainwindow.h>
#include <meteo/commons/ui/mainwindow/widgethandler.h>
#include <meteo/commons/global/common.h>
#include <meteo/commons/ui/map/document.h>
#include <meteo/commons/ui/map/loader.h>
#include <meteo/commons/ui/map/map.h>

#include "mapview.h"
#include "mapscene.h"

Q_DECLARE_METATYPE(meteo::menu::MenuItem)

namespace meteo {
namespace map {


MapWindow::MapWindow( app::MainWindow* w, const proto::Document& blank, const QString& menufile )
  : QMainWindow(w),
    window_(w),
    mapview_(nullptr)
{
  if ( false == menufile.isEmpty() ) {
    loadMenuFromConfig(menufile);
  }
  mapview_ = new MapView(this);
  QMainWindow::setCentralWidget(mapview_);

  meteo::GeoPoint mc = pbgeopoint2geopoint( blank.map_center() );
  meteo::GeoPoint dc = pbgeopoint2geopoint( blank.doc_center() );

  QApplication::setOverrideCursor(Qt::WaitCursor);
  Document* doc = new Document( mc, dc, blank.doctype(), kGeneral );
  proto::Document newprop = doc->property();
  newprop.MergeFrom(blank);
  doc->setProperty(newprop);
  doc->init();
  //  QString name = QString::fromUtf8("Починить наименование документа!");
  //  doc->setName(name);
  doc->setScale(blank.scale());
  new MapScene(doc, mapview_);
  loadBaseLayersVisibilitySettings();
  QApplication::restoreOverrideCursor();
}

MapWindow::MapWindow( app::MainWindow* w, const proto::Document& blank, const QString& loader, const QString& menufile )
  : QMainWindow(w),
    window_(w),
    mapview_(nullptr)
{
  if ( false == menufile.isEmpty() ) {
    loadMenuFromConfig(menufile);
  }
  mapview_ = new MapView(this);
  QMainWindow::setCentralWidget(mapview_);

  meteo::GeoPoint mc( blank.map_center().lat_radian(), blank.map_center().lon_radian() );
  meteo::GeoPoint dc( blank.doc_center().lat_radian(), blank.doc_center().lon_radian() );

  QApplication::setOverrideCursor(Qt::WaitCursor);
  Document* doc = new Document( mc, dc, blank.doctype(), blank.projection() );
  proto::Document newprop = doc->property();
  newprop.set_geoloader( loader.toStdString() );
  newprop.MergeFrom(blank);
  doc->setProperty(newprop);
  doc->init();
  if ( meteo::map::proto::kGeoMap == blank.doctype()  ) {
    meteo::map::Loader::instance()->handleStation( meteo::global::kStationLoaderCommon, doc );
    meteo::map::Loader::instance()->handleGrid( meteo::global::kGridLoaderCommon, doc );
    meteo::map::Loader::instance()->handleCities(meteo::global::kCitiesLoaderCommon, doc );
  }
  //  QString name = QString::fromUtf8("Починить наименование документа!");
  //  doc->setName(name);
  doc->setScale(blank.scale());
  new MapScene(doc, mapview_);
  if ( nullptr != doc->eventHandler() ) {
    doc->eventHandler()->installEventFilter(this);
  }
  loadBaseLayersVisibilitySettings();
  QApplication::restoreOverrideCursor();
}

MapWindow::MapWindow( app::MainWindow* w, Document* doc, const QString& menufile )
  : QMainWindow(w),
    window_(w),
    mapview_(nullptr)
{
  if ( false == menufile.isEmpty() ) {
    loadMenuFromConfig(menufile);
  }
  mapview_ = new MapView(this);
  QMainWindow::setCentralWidget(mapview_);

  new MapScene(doc, mapview_);
  if ( nullptr != doc->eventHandler() ) {
    doc->eventHandler()->installEventFilter(this);
  }
  setWindowTitle(" ");
  loadBaseLayersVisibilitySettings();
}

MapWindow::~MapWindow()
{
  MapScene* scene = mapview_->mapscene();
  delete scene;
  delete mapview_; mapview_ = nullptr;
}

MapScene* MapWindow::mapscene() const
{
  return mapview_->mapscene();
}

Document* MapWindow::document() const
{
  return mapview_->mapscene()->document();
}

void MapWindow::loadMenuFromConfig(const QString& dirname)
{
  QDir dir(dirname);
  if( nullptr == menuBar() ) {
    return;
  }
  meteo::menu::MenuConfig* menuconf = new meteo::menu::MenuConfig();
  meteo::menu::MenuConfig* menubuf = new meteo::menu::MenuConfig();
  QStringList filesList = dir.entryList(QDir::Files);
  for (auto file : filesList) {
    if ( false == TProtoText::fromFile( (dirname+"/"+file), menubuf ) ) {
      error_log << QObject::tr("Ошибка загрузки конфигурации: %1").arg(dirname+"/"+file);
    }
    else {
      mergeMenu(menuconf,menubuf);
    }
  }
  menuBar();
  sortMenuconf(menuconf);
  for ( int i = 0, sz = menuconf->menu_size(); i < sz; ++i ) {
    meteo::menu::MenuItem menu = menuconf->menu(i);
    QMenu* qm = new QMenu(pbtools::toQString(menu.title()), menuBar() );
    QAction* a = menuBar()->addMenu(qm);
    menuBar()->addMenu(qm)->setObjectName(pbtools::toQString(menu.id()));
    a->setObjectName(pbtools::toQString(menu.id()));
    qm->setProperty("properties", QVariant::fromValue(menu));
    qm->setObjectName(pbtools::toQString(menu.id()));
    if ( 0 != menu.menu_size() )  {
      parse(menu, qm);
    }
  }
  delete menuconf;
  //  delete idInMenu;
  delete menubuf;
}

void MapWindow::mergeMenu(meteo::menu::MenuConfig* target, meteo::menu::MenuConfig* source)
{
  bool find = false;
  meteo::menu::MenuItem* menuSource = nullptr;
  meteo::menu::MenuItem* menuTarget = nullptr;
  for(int i=0, sz=source->menu_size();i<sz; ++i) {
    find = false;
    menuSource = source->mutable_menu(i);
    for (int j=0, sz=target->menu_size(); j<sz; ++j) {
      menuTarget = target->mutable_menu(j);
      if (menuSource->id() == menuTarget->id()) {
        find = true;
        if ((menuTarget->has_title()==false) && (menuSource->has_title() == true)) {
          menuTarget->set_title(menuSource->title());
        }
        if ((menuTarget->has_window_title()==false) && (menuSource->has_window_title() == true)) {
          menuTarget->set_window_title(menuSource->window_title());
        }
        if ((menuTarget->has_handler()==false) && (menuSource->has_handler() == true)) {
          menuTarget->set_handler(menuSource->handler());
        }
        if ((menuTarget->has_ico()==false) && (menuSource->has_ico() == true)) {
          menuTarget->set_ico(menuSource->ico());
        }
        if ((menuTarget->has_tooltip()==false) && (menuSource->has_tooltip() == true)) {
          menuTarget->set_tooltip(menuSource->tooltip());
        }
        if ((menuTarget->has_app()==false) && (menuSource->has_app() == true)) {
          menuTarget->set_app(menuSource->app());
        }
        if ((menuTarget->has_position()==false) && (menuSource->has_position() == true)) {
          menuTarget->set_position(menuSource->position());
        }
        //        if ((menuTarget->has_arg()==false) && (menuSource->has_arg() == true)) {
        //          menuTarget->set_arg(menuSource->arg());
        //        }
        if ((menuTarget->has_disabled()==false) && (menuSource->has_disabled() == true)) {
          menuTarget->set_disabled(menuSource->disabled());
        }
        if ((menuTarget->has_options()==false) && (menuSource->has_options() == true)) {
          menuTarget->set_options(menuSource->options());
        }
        if ((menuTarget->has_appsu()==false) && (menuSource->has_appsu() == true)) {
          menuTarget->set_appsu(menuSource->appsu());
        }
        if ((menuTarget->has_argsu()==false) && (menuSource->has_argsu() == true)) {
          menuTarget->set_argsu(menuSource->argsu());
        }
        if ((menuTarget->has_cmd()==false) && (menuSource->has_cmd() == true)) {
          menuTarget->set_cmd(menuSource->cmd());
        }
        if ((menuTarget->has_dockwidget()==false) && (menuSource->has_dockwidget() == true)) {
          menuTarget->set_dockwidget(menuSource->dockwidget());
        }
        if ((menuTarget->has_plugin()==false) && (menuSource->has_plugin() == true)) {
          menuTarget->set_plugin(menuSource->plugin());
        }
        if ((menuTarget->has_type()==false) && (menuSource->has_type() == true)) {
          menuTarget->set_type(menuSource->type());
        }
        if ((menuTarget->has_mdi()==false) && (menuSource->has_mdi() == true)) {
          menuTarget->set_mdi(menuSource->mdi());
        }
        if ((menuTarget->has_hidden()==false) && (menuSource->has_hidden() == true)) {
          menuTarget->set_hidden(menuSource->hidden());
        }
        if ((menuTarget->has_separator()==false) && (menuSource->has_separator() == true)) {
          menuTarget->set_separator(menuSource->separator());
        }
        break;
      }
    }
    if(false == find) {
      meteo::menu::MenuItem* newMenu = target->add_menu();
      newMenu->CopyFrom(*menuSource);
    }
    else if (menuSource->menu_size() > 0){
      mergeMenuRecoursive(menuTarget,menuSource);
    }
  }
}

void MapWindow::mergeMenuRecoursive(meteo::menu::MenuItem* target, meteo::menu::MenuItem* source)
{
  bool find = false;
  meteo::menu::MenuItem* menuSource = nullptr;
  meteo::menu::MenuItem* menuTarget = nullptr;
  for ( int i=0, sz=source->menu_size(); i<sz; ++i ) {
    find = false;
    menuSource = source->mutable_menu(i);
    for (int j=0, sz=target->menu_size(); j<sz; ++j) {
      menuTarget = target->mutable_menu(j);
      if ( menuSource->id() == menuTarget->id() ) {
        find = true;
        if ((menuTarget->has_title()==false) && (menuSource->has_title() == true)) {
          menuTarget->set_id(menuSource->id());
        }
        if ((menuTarget->has_window_title()==false) && (menuSource->has_window_title() == true)) {
          menuTarget->set_window_title(menuSource->window_title());
        }
        if ((menuTarget->has_handler()==false) && (menuSource->has_handler() == true)) {
          menuTarget->set_handler(menuSource->handler());
        }
        if ((menuTarget->has_ico()==false) && (menuSource->has_ico() == true)) {
          menuTarget->set_ico(menuSource->ico());
        }
        if ((menuTarget->has_tooltip()==false) && (menuSource->has_tooltip() == true)) {
          menuTarget->set_tooltip(menuSource->tooltip());
        }
        if ((menuTarget->has_app()==false) && (menuSource->has_app() == true)) {
          menuTarget->set_app(menuSource->app());
        }
        if ((menuTarget->has_position()==false) && (menuSource->has_position() == true)) {
          menuTarget->set_position(menuSource->position());
        }
        if ((menuTarget->has_disabled()==false) && (menuSource->has_disabled() == true)) {
          menuTarget->set_disabled(menuSource->disabled());
        }
        if ((menuTarget->has_options()==false) && (menuSource->has_options() == true)) {
          menuTarget->set_options(menuSource->options());
        }
        if ((menuTarget->has_appsu()==false) && (menuSource->has_appsu() == true)) {
          menuTarget->set_appsu(menuSource->appsu());
        }
        if ((menuTarget->has_argsu()==false) && (menuSource->has_argsu() == true)) {
          menuTarget->set_argsu(menuSource->argsu());
        }
        if ((menuTarget->has_cmd()==false) && (menuSource->has_cmd() == true)) {
          menuTarget->set_cmd(menuSource->cmd());
        }
        if ((menuTarget->has_dockwidget()==false) && (menuSource->has_dockwidget() == true)) {
          menuTarget->set_dockwidget(menuSource->dockwidget());
        }
        if ((menuTarget->has_plugin()==false) && (menuSource->has_plugin() == true)) {
          menuTarget->set_plugin(menuSource->plugin());
        }
        if ((menuTarget->has_type()==false) && (menuSource->has_type() == true)) {
          menuTarget->set_type(menuSource->type());
        }
        if ((menuTarget->has_mdi()==false) && (menuSource->has_mdi() == true)) {
          menuTarget->set_mdi(menuSource->mdi());
        }
        if ((menuTarget->has_hidden()==false) && (menuSource->has_hidden() == true)) {
          menuTarget->set_hidden(menuSource->hidden());
        }
        if ((menuTarget->has_separator()==false) && (menuSource->has_separator() == true)) {
          menuTarget->set_separator(menuSource->separator());
        }
        break;
      }
    }
    if ( false == find ) {
      meteo::menu::MenuItem* newMenu = target->add_menu();
      newMenu->CopyFrom(*menuSource);
    }
    else if ( 0 < menuSource->menu_size() ) {
      mergeMenuRecoursive(menuTarget,menuSource);
    }
  }
}


static QAction* actionByIdInMenuSearch(const QString& id, const QMenu* menu)
{
  if ( nullptr == menu ){
    return nullptr;
  }

  for ( auto action: menu->actions() ){
    if ( action->objectName() == id ){
      return  action;
    }
    QAction* result = actionByIdInMenuSearch(id, action->menu());
    if ( nullptr != result ){
      return result;
    }
  }
  return nullptr;
}

QAction* MapWindow::findActionInMenu(const QString& id)
{
  for ( auto action: menuBar()->actions() ){
    if ( action->objectName() == id ){
      return action;
    }
    QAction* result = actionByIdInMenuSearch(id, action->menu());
    if ( nullptr != result ){
      return result;
    }
  }
  return nullptr;
}

QAction *MapWindow::addActionToMenu(const QPair<QString, QString> &title, const QList<QPair<QString, QString> > &menupath)
{
  QList<QAction*> menulist = menuBar()->actions();
  QAction* a = findInMenuActionRecoursive(menulist, title);
  if ( nullptr != a) {
    a->setProperty("isSet",true);
    return a;
  }

  if ( true == menupath.empty() ) {
    for (auto a : menulist) {
      if (a->objectName() == title.first) {
        warning_log << QObject::tr("В главное меню уже добавлен пункт '%1'")
                       .arg(title.first);
        if (nullptr != a->menu() && a->menu()->actions().empty()) {
          a->setMenu(nullptr);
        }
        a->setProperty("isSet",true);
        return a;
      }
    }
    auto act = menuBar()->addAction(title.second);
    act->setObjectName(title.first);
    act->setProperty("isSet",true);
    return act;
  }

  auto addrlist = menupath;
  QPair<QString, QString> menuname = addrlist.takeFirst();
  QMenu* m = nullptr;
  for (auto a : menulist) {
    if ( a->objectName() == menuname.first ) {
      m = a->menu();
      if (nullptr == m) {
        warning_log << QObject::tr("Непонятная ситуация. Найден пункт '%1' на панели меню, который не является меню, но должен им быть.")
                       .arg(a->objectName());
        m = new QMenu(menuBar());
        a->setMenu(m);
      }
      break;
    }
  }

  if (nullptr == m ) {
    QAction* a = menuBar()->addAction(menuname.second);
    a->setObjectName(menuname.first);
    a->setMenu(new QMenu(menuname.second, this));
    m = a->menu();
  }
  return addSlotToMenuRecoursive(m, title, addrlist);
}


QAction* MapWindow::menubarActionByName( const QString& id ) const
{
  QList<QAction*> baractions = menuBar()->actions();
  for ( int i = 0, sz = baractions.size(); i < sz; ++i ) {
    QAction* a = baractions[i];
    if ( id == a->objectName() ) {
      return a;
    }
  }
  return nullptr;
}

QAction* MapWindow::menuActionByName( const QString& id, const QString& baritemid ) const
{
  QList<QAction*> baractions = menuBar()->actions();
  for ( int i = 0, sz = baractions.size(); i < sz; ++i ) {
    QAction* a = baractions[i];
    if ( false == baritemid.isEmpty() && baritemid != a->objectName() ) {
      continue;
    }
    if ( a->objectName() == id ) {
      return a;
    }
    if ( nullptr != a->menu() ) {
      a = menuActionByName( a->menu(), id );
      if ( nullptr != a ) {
        return a;
      }
    }
  }
  return nullptr;
}

QAction* MapWindow::menuActionByName( QMenu* menu, const QString& id ) const
{
  QList<QAction*> actions = menu->actions();
  for ( int i = 0, sz = actions.size(); i < sz; ++i ) {
    QAction* a = actions[i];
    if ( a->objectName() == id ) {
      return a;
    }
    if ( nullptr != a->menu() ) {
      a = menuActionByName( a->menu(), id );
      if ( nullptr != a ) {
        return a;
      }
    }
  }
  return nullptr;
}

void MapWindow::addPluginGroup( const QString& gr )
{
  if ( nullptr != mapview_ && nullptr != mapview_->mapscene() ) {
    mapview_->mapscene()->addPluginGroup(gr);
  }
  QList<QAction*> menulist = menuBar()->actions();
  applyConfSettingsRecoursive(menulist);
  cleanMenu(menulist);
}

void MapWindow::parse(const meteo::menu::MenuItem& menu, QMenu* m)
{
  for ( int i = 0; i < menu.menu_size(); ++i ) {
    meteo::menu::MenuItem item = menu.menu(i);
    if ( 0 == item.menu_size() ) {
      //TAction* act = new TAction(pbtools::toQString(item.title()), m );
      QAction* act = new TAction(pbtools::toQString(item.title()), m);
      act->setProperty("isSet",false);
      act->setProperty("properties", QVariant::fromValue(item));
      act->setObjectName(pbtools::toQString(item.id()));
      //act->setConfig(item);
      if( true != item.hidden() ){
        m->addAction(act);
      }
    }
    else {
      QMenu* subm = new QMenu(pbtools::toQString(item.title()), m);
      m->addMenu( subm )->setObjectName(pbtools::toQString(item.id()));
      subm->setObjectName(pbtools::toQString(item.id()));
      parse(item, subm);
    }
  }
  return;
}

void MapWindow::closeEvent(QCloseEvent* e)
{
  if( 0 == QMessageBox::question( this, tr("Выход"), tr("Закрыть документ %1?").arg(windowTitle()), tr("Да"), tr("Нет"))){
    e->accept();
  }
  else {
    e->ignore();
  }
}

void MapWindow::setCurTitle(){
  if ( nullptr != mapview_ &&
       nullptr != mapview_->mapscene() &&
       nullptr != mapview_->mapscene()->document() ) {
    proto::Map info = mapview_->mapscene()->document()->info();
    QString title = pbtools::toQString( info.title() );
    if ( true == title.isEmpty() ) {
      title = QObject::tr("Пустой документ");
    }
    setWindowTitle(title);
  }
}



bool MapWindow::eventFilter( QObject* o, QEvent* ev )
{
  Q_UNUSED(o);
  Q_UNUSED(ev);

  if ( nullptr == document() ) {
    return false;
  }
  if ( MapEvent::MapChanged == ev->type() ) {
    return false;
  }
  if ( LayerEvent::LayerChanged == ev->type() ) {
    LayerEvent* event = reinterpret_cast<LayerEvent*>(ev);
    switch ( event->changeType() ) {
    case LayerEvent::Activity:
    case LayerEvent::Added:
      setCurTitle();
    break;
    default:
    break;
    }
  }
  if ( DocumentEvent::DocumentChanged == ev->type() ) {
    DocumentEvent* event = reinterpret_cast<DocumentEvent*>(ev);
    switch ( event->changeType() ) {
    case DocumentEvent::NameChanged:
      setCurTitle();
    break;
    case DocumentEvent::IncutChanged:
    case DocumentEvent::LegendChanged:
    case DocumentEvent::DocumentLoaded:
    case DocumentEvent::Changed:
    break;
    }
  }
  return false;
}


QAction* MapWindow::findInMenuActionRecoursive(const QList<QAction*> menulist, const QPair<QString, QString> &title)
{
  for (auto a : menulist) {
    if (a->objectName() == title.first) {
      if (nullptr != a->menu() && a->menu()->actions().empty()) {
        a->setMenu(nullptr);
      }
      return a;
    }
    QMenu* menu = a->menu();
    if (nullptr != menu ) {
      QList<QAction*> childActions = menu->actions();
      QAction* a = findInMenuActionRecoursive(childActions, title);
      if (nullptr != a) {
        return a;
      }
    }
  }
  return nullptr;
}

void MapWindow::applyConfSettingsRecoursive(QList<QAction*> menulist )
{
  for (auto a : menulist) {
    QVariant property = a->property("properties");
    meteo::menu::MenuItem*  protoConf = nullptr;
    if (property.isValid()) {
      protoConf = static_cast<meteo::menu::MenuItem*>( property.data());
    }
    if (nullptr != protoConf) {
      a->setIconVisibleInMenu(true);
      a->setIcon(QIcon(pbtools::toQString(protoConf->ico())));
      a->setDisabled(protoConf->disabled());
      a->setText(pbtools::toQString(protoConf->title()));
    }
    QMenu* menu = a->menu();
    if (nullptr != menu ) {
      QList<QAction*> childActions = menu->actions();
      applyConfSettingsRecoursive(childActions);
    }
  }
}

void MapWindow::cleanMenu(QList<QAction *> menulist)
{
  for (auto a : menulist) {
    menulist.removeOne(a);
    if (nullptr == a->menu()) {
      QVariant isSet = a->property("isSet");
      if(true == isSet.isValid()) {
        if(false == isSet.toBool()){
          QMenu* testmenu = static_cast<QMenu*>(a->parent());
          testmenu->removeAction(a);
        }
      }
      else {
        //TODO при создании пунктов меню из плагина, нет свойства isSet
      }
    }
    QMenu* menu = a->menu();
    if (nullptr != menu ) {
      QList<QAction*> childActions = menu->actions();
      cleanMenu(childActions);
    }
  }
}

QAction *MapWindow::addSlotToMenuRecoursive(QMenu *target, const QPair<QString, QString> &title, const QList<QPair<QString, QString> > &address)
{
  QList<QAction*> menulist = target->actions();
  QMenu* m = nullptr;

  if (address.empty()) {
    for ( auto a : menulist ) {
      if ( a->objectName() == title.first ) {
        info_log << QString("Пунт меню %1 уже существует").arg(title.second);
        a->setProperty("isSet",true);
        return a;
      }
    }
    auto act = target->addAction(title.second);
    act->setObjectName(title.first);
    act->setProperty("isSet",true);
    return act;
  }
  else {
    auto newlist = address;
    QPair<QString, QString> menuname = newlist.takeFirst();
    for(auto a : menulist)
    {
      if (a->objectName() == menuname.first)
      {
        m = a->menu();
        if(nullptr == m) {
          warning_log << QObject::tr("Непонятная ситуация. Найден пункт '%1' на панели меню, который не является меню, но должен им быть.")
                         .arg(a->objectName());
          m = new QMenu(target);
          a->setMenu(m);
        }
        break;
      }
    }

    if ( nullptr == m ) {
      QAction* a = target->addAction(menuname.second);
      a->setObjectName(menuname.first);
      a->setMenu(new QMenu(menuname.second, this));
      m = a->menu();
      //      m = target->addMenu(menuname.second);
      //      m->setObjectName(menuname.first);
    }
    return addSlotToMenuRecoursive(m, title, newlist);
  }
}

void MapWindow::sortMenuconf(menu::MenuConfig* menuconf)
{
  menu::MenuItem* menu = nullptr;
  menu::MenuItem* menunext = nullptr;
  for ( int i=0,sz=menuconf->menu_size();i<sz;i++ ) {
    for (int j=0;j<sz-1;j++) {
      menu = menuconf->mutable_menu(j);
      menunext = menuconf->mutable_menu(j+1);
      if (menu->position() > menunext->position()) {
        menu->Swap(menunext);
      }
    }
  }
  for (int i=0,sz=menuconf->menu_size();i<sz;i++) {
    menu::MenuItem* menu = menuconf->mutable_menu(i);
    if (menu->menu_size() > 1) {
      sortMenuItemRecoursive(menu);
    }
  }
}


void MapWindow::sortMenuItemRecoursive(menu::MenuItem *mainmenu)
{
  menu::MenuItem* menu = nullptr;
  menu::MenuItem* menunext = nullptr;
  for ( int i=0,sz=mainmenu->menu_size();i<sz;i++ ) {
    for (int j=0;j<sz-1;j++) {
      menu = mainmenu->mutable_menu(j);
      menunext = mainmenu->mutable_menu(j+1);
      if (menu->position() > menunext->position()) {
        menu->Swap(menunext);
      }
    }
  }
  for (int i=0,sz=mainmenu->menu_size();i<sz;i++) {
    menu::MenuItem* menu=mainmenu->mutable_menu(i);
    if (menu->menu_size() > 1) {
      sortMenuItemRecoursive(menu);
    }
  }
}

void MapWindow::loadBaseLayersVisibilitySettings()
{
  Layers setts;
  QString fileName;

  switch (mapview()->mapscene()->document()->property().doctype()) {
  case meteo::map::proto::kGeoMap:
    if ( "geo.old" == mapview()->mapscene()->document()->property().geoloader() ) {
      fileName = QDir::homePath() + "/.meteo/baselayers.visibility.map.old.conf";
    }
    else if ( "ptkpp" == mapview()->mapscene()->document()->property().geoloader() ) {
      fileName = QDir::homePath() + "/.meteo/baselayers.visibility.map.ptkpp.conf";
    }
    else {
      fileName = QString();
    }
  break;
  case meteo::map::proto::kVerticalCut:
    fileName = QDir::homePath() + "/.meteo/baselayers.visibility.vprofile.conf";
  break;
  case meteo::map::proto::kAeroDiagram:
    fileName = QDir::homePath() + "/.meteo/baselayers.visibility.ad.conf";
  break;
  case meteo::map::proto::kOceanDiagram:
    fileName = QDir::homePath() + "/.meteo/baselayers.visibility.od.conf";
  break;
  case meteo::map::proto::kMeteogram:
    fileName = QDir::homePath() + "/.meteo/baselayers.visibility.meteogram.conf";
  break;
  case meteo::map::proto::kFormalDocument:
  default:
    fileName = QString();
  break;
  }

  if ( false == QFile::exists(fileName) ) {
    debug_log << QObject::tr("Нет соответствующего файла с настройками видимости базовых слоев");
    return;
  }

  if ( false == TProtoText::fromFile(fileName, &setts) ) {
    debug_log << tr("Не удалось загрузить настройки (%1).").arg(fileName);
    return;
  }

  QList<Layer*> layers =  mapview()->mapscene()->document()->layers();
  for ( auto layer : layers ) {
    for ( auto l : setts.layer() ) {
      if ( layer->name().toStdString() == l.name() ) {
        if ( true == l.visible() ) {
          mapview()->mapscene()->document()->showLayer(layer->uuid());
        }
        else {
          mapview()->mapscene()->document()->hideLayer(layer->uuid());
        }
        break;
      }
    }
  }
}

}
}
