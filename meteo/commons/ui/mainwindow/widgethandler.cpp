#include "widgethandler.h"
#include "mainwindow.h"

#include <cross-commons/app/paths.h>
#include <cross-commons/debug/tlog.h>

#include <commons/textproto/tprototext.h>
#include <commons/textproto/pbtools.h>

#include <meteo/commons/formsh/thandler.h>
#include <meteo/commons/global/global.h>
#include <meteo/commons/ui/custom/stationwidget.h>
#include <meteo/commons/ui/mainwindow/embedwindow/appwatcher.h>

#include <meteo/commons/proto/sprinf.pb.h>

#include <meteo/commons/ui/mainwindow/embedwindow/appcontainer_windows.h>


template<>mapauto::WidgetHandler* WidgetHandler::_instance = nullptr;

namespace
{
//  const QString weatherPlugins() { return MnCommon::pluginPath() +".weather"; }
//  const QString actionPlugins() { return MnCommon::pluginPath() +".map.common"; }

  bool registerAuto()
  {
    mapauto::WidgetHandler* hndl = WidgetHandler::instance();
    return TFormAdaptor::instance()->registerHandler(hndl);
  }

  static bool regresult = registerAuto();
}

namespace mapauto {

WidgetHandler::WidgetHandler()
  : THandler( "auto" ),
  mainwindow_(nullptr),
  appwatcher_(nullptr),
  stations_tool_bar_(nullptr),
  stations_widget_(nullptr)
{
}

WidgetHandler::~WidgetHandler()
{
}

bool WidgetHandler::registerWidget( const QString& name, Callback call )
{
  if ( true == wgtfuncs_.contains( name ) ) {
    return false;
  }
  wgtfuncs_.insert( name, call );
  return true;
}

QMap<QString, Callback>&WidgetHandler::autowidgets()
{
  return wgtfuncs_;
}

void WidgetHandler::setMainwindow( meteo::app::MainWindow* w )
{
  mainwindow_ = w;

  if( nullptr == mainwindow_){
    return;
  }

  appwatcher_ = new meteo::app::AppWatcher(mainwindow_);
  appwatcher_->setMainWindow(mainwindow_);

 //loadMenuFromConfig( mainwindow_, applicationMenuFilename() );
 //addMenuFromConfigToMenu(mainwindow_, mapMenuFilename(), QObject::tr("Документ") );
}

meteo::app::MainWindow* WidgetHandler::mainwindow() const
{
  return mainwindow_;
}


bool WidgetHandler::isCurrentTabGeoMap()
{  
  auto mdi = this->mainwindow()->mdi();
  auto currentSubWindow = mdi->currentSubWindow();
  for (int i = 0; i < currentSubWindow->children().count(); ++i)
  {
    meteo::map::MapWindow* mapCast = qobject_cast<meteo::map::MapWindow*>(currentSubWindow->children()[i]);
    if ((nullptr != mapCast) && (mapCast->document()->property().doctype() ==  meteo::map::proto::kGeoMap))
    {
      return true;
    }
  }
  return false;
}

QList<QMdiSubWindow*> WidgetHandler::geoMapSearch()
{
  auto mdi = this->mainwindow()->mdi();
  auto allMdiWindowsList = mdi->subWindowList();
  QList<QMdiSubWindow*> result;
  for (int i = 0; i < allMdiWindowsList.size(); ++i)
  {
    for (int j = 0; j < allMdiWindowsList[i]->children().count(); ++j)
    {
      QObject* currentChildren =  allMdiWindowsList[i]->children()[j];
      auto currentTryMap = qobject_cast<meteo::map::MapWindow*>(currentChildren);
      if ( nullptr != currentTryMap ) {
        auto doctype = currentTryMap->document()->property().doctype();
        if (meteo::map::proto::kGeoMap == doctype) {
          result << allMdiWindowsList[i];
        }
      }
    }
  }
  return result;
}

QStatusBar* WidgetHandler::statusBar() const
{
  if ( nullptr == mainwindow_ ) {
    return nullptr;
  }
  return mainwindow_->statusBar();
}

void WidgetHandler::showStatusMessage( const QString& txt, int msecs )
{
  if ( nullptr == mainwindow_ || nullptr == mainwindow_->statusBar() ) {
    return;
  }
  mainwindow_->statusBar()->showMessage( txt, msecs );
}

void WidgetHandler::clearStatusMessage()
{
  if ( nullptr == mainwindow_ || nullptr == mainwindow_->statusBar() ) {
    return;
  }
  mainwindow_->statusBar()->clearMessage();
}

QMenuBar* WidgetHandler::menuBar() const
{
  if ( nullptr == mainwindow_ ) {
    return nullptr;
  }
  return mainwindow_->menuBar();
}


QWidget* WidgetHandler::handleEntity(TAction* act)
{
  if( nullptr == act ) {
    return nullptr;
  }
  QString name = pbtools::toQString(act->config().id());
  QString title = pbtools::toQString(act->config().window_title());
  QString options = pbtools::toQString(act->config().options());
  QApplication::setOverrideCursor(Qt::WaitCursor);
  QWidget* wgt = createWidget( name, title, options );
  QApplication::restoreOverrideCursor();
  if( nullptr == wgt ){
    return nullptr;
  }
  QList<QMdiSubWindow*> list = mainwindow_->mdi()->subWindowList();
  for ( int i = 0, sz = list.size(); i < sz; ++i ) {
    if ( list[i]->widget() == wgt && mainwindow_->mdi()->activeSubWindow() != list[i] ) {
      mainwindow_->mdi()->setActiveSubWindow(list[i]);
      return wgt;
    }
  }
  if( wgt != nullptr ) {
    wgt->setObjectName(name);
//    debug_log << "ebu dalsa";
//    wgt->show();
  }
  return wgt;
}

QWidget* WidgetHandler::handleEntity(const QString& name)
{
  TAction* act = action(name);
  if(nullptr == act){
    error_log << QObject::tr("Action %1 не зарегистрирован").arg(name);
    return nullptr;
  }
  return handleEntity(act);
}

QAction* WidgetHandler::actionByName(QList<QAction*> list, const QString& name)
{
  for(auto act : list){
    if(name == act->objectName()) {
      return act;
    }
  }
  return nullptr;
}

TAction* WidgetHandler::action(const QString& name)
{
  if ( false == tactions_.contains(name) ) {
    warning_log << QObject::tr("Не найден TAction %1 в списке всех TAction'ов")
      .arg(name);
    return nullptr;
  }
  return tactions_[name];
}

void WidgetHandler::closeX11()
{
  if ( nullptr == appwatcher_ ) { return; }

  foreach ( QWidget* w, appwatcher_->procx11().values() ) {
    if( nullptr != w ){
      w->close();
    }
  }
}

void WidgetHandler::loadMenuFromConfig(QMainWindow* mainwindow, const QString& dirname)
{
  QDir dir(dirname);
  meteo::menu::MenuConfig* menuconf = new meteo::menu::MenuConfig();
  meteo::menu::MenuConfig* menubuf = new meteo::menu::MenuConfig();
  if( nullptr == mainwindow ) {
    delete menuconf;
    delete menubuf;
    return;
  }
  if( nullptr == mainwindow->menuBar() ) {
    delete menuconf;
    delete menubuf;
    return;
  }
  QStringList filesList = dir.entryList(QDir::Files);
  QMap<std::string,int>* idInMenu = new QMap<std::string,int>();
  for (const auto &file : qAsConst(filesList)) {
    if ( false == TProtoText::fromFile( (dirname+"/"+file), menubuf ))  {
      error_log << QObject::tr("Ошибка загрузки конфигурации: %1").arg(dirname+"/"+file);
    }
    else {
      //TODO устранение дубликатов меню
      checkMenuConfDuplicate(idInMenu,*menubuf);
      //menuconf->MergeFrom(*menubuf);
      mergeMenu(menuconf, menubuf);
    }
  }
  sortMenuconf(menuconf);
  for ( int i = 0, sz = menuconf->menu_size(); i < sz; ++i ) {
    meteo::menu::MenuItem menu = menuconf->menu(i);
//    if ( 0 == menu.menu_size() ) {
//      continue;
//    }
    QMenu* qm = new QMenu(pbtools::toQString(menu.title()), mainwindow->menuBar());
    mainwindow->menuBar()->addMenu(qm)->setObjectName(pbtools::toQString(menu.id()));
    qm->setObjectName(pbtools::toQString(menu.id()));
    parse(menu, qm);
  }
  delete idInMenu;
  delete menuconf;
  delete menubuf;
}

void WidgetHandler::mergeMenu(meteo::menu::MenuConfig* target, meteo::menu::MenuConfig* source)
{
  bool find = false;
  meteo::menu::MenuItem* menuSource;
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

void WidgetHandler::mergeMenuRecoursive(meteo::menu::MenuItem* target, meteo::menu::MenuItem* source)
{
  bool find = false;
  meteo::menu::MenuItem* menuSource;
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

void WidgetHandler::checkMenuConfDuplicate(QMap<std::string, int> *idMenu, meteo::menu::MenuConfig menubuf)
{
  for (int i=0, sz=menubuf.menu_size();i<sz;i++) {
    meteo::menu::MenuItem* menu = menubuf.mutable_menu(i);
    if (idMenu->end() != idMenu->find(menu->id())) {
      idMenu->insert(menu->id(),1);
    }
    else {
      //debug_log << "В конфигурационных файлах меню обнаружены повторы id" << menu->id();
      menu->Clear();
    }
    if (menu->menu_size() > 0) {
      checkMenuItemDuplicateRecoursive(idMenu,menu);
    }
  }
}

void WidgetHandler::checkMenuItemDuplicateRecoursive(QMap<std::string, int> *idMenu, meteo::menu::MenuItem *mainmenu)
{
  for (int i=0, sz=mainmenu->menu_size();i<sz;i++) {
    meteo::menu::MenuItem* menu = mainmenu->mutable_menu(i);
    if (idMenu->end() != idMenu->find(menu->id())) {
      idMenu->insert(menu->id(),1);
    }
    else {
      debug_log << "В конфигурационных файлах меню обнаружены повторы id" << menu->id();
      menu->Clear();
    }
    if (menu->menu_size() > 0) {
      checkMenuItemDuplicateRecoursive(idMenu,menu);
    }
  }
}
/*!
 * \brief WidgetHandler::addMenuFromConfigToMenu - Добавляем пункт меню
 * \param mainwindow - окно
 * \param config     - строка с конфигурацией пункта меню
 * \param menupath   - путь к пункту меню
 * \return
 */
QAction* WidgetHandler::addMenuFromConfigToMenu(QMainWindow *mainwindow, const QString& config, const QList<QPair<QString, QString>>& menupath)
{
  if(nullptr == mainwindow || nullptr == mainwindow->menuBar()) {
    return nullptr;
  }

  // Ищем корневой пункт меню для добавляемого меню
  auto path = menupath; // [{ id, title }, ...]
  QString id = path.first().first;
  QMenu* head = mainwindow->menuBar()->findChild<QMenu*>(id);
  if(nullptr != head) {
    path.pop_front();
    while(false == path.empty())
    {
      id = path.first().first;
      QMenu* qm = head->findChild<QMenu*>(id);
      if(qm != nullptr) {
        path.pop_front();
        head = qm;
      }
      else {
        break;
      }
    }
  }

  // Добавляем недастающие вложения
  for(auto m = path.cbegin(); m != path.cend(); ++m)
  {
    QMenu* qm;
    if(nullptr == head) {
      qm = new QMenu(m->second, mainwindow->menuBar());
    }
    else {
      qm = new QMenu(m->second, head);
    }
    qm->setObjectName(m->first);
    if(nullptr == head) {
      mainwindow->menuBar()->addMenu(qm)->setObjectName(m->first);
    }
    else {
      head->addMenu(qm)->setObjectName(m->first);
    }
    head = qm;
  }

  QAction* act = nullptr;
  // Считываем меню
  meteo::menu::MenuConfig menuConfig;
  TProtoText::fillProto(config, &menuConfig);
  for(const meteo::menu::MenuItem &menu : menuConfig.menu())
  {
    id = pbtools::toQString(menu.id());
    QString title = pbtools::toQString(menu.title());

    // Ищем считаный пункт
    if(nullptr == head) {
      head = mainwindow->menuBar()->findChild<QMenu*>(id);
    }
    else {
      QMenu* qm = head->findChild<QMenu*>(id);
      if(nullptr != qm) {
        head = qm;
      }
    }
    //Добовляем пункт меню верхнего уровня
    if(nullptr == head) {
      head = new QMenu(title, mainwindow->menuBar());
      head->setObjectName(id);
      act = mainwindow->menuBar()->addMenu(head);
      act->setObjectName(id);
    }

    if (0 == menu.menu_size())
    {
      if(menu.has_app())
      {
        QStringList args;
        if((false == meteo::global::isRoot()) && (menu.has_argsu()))
        {
          args.append( pbtools::toQString(menu.argsu()).replace("$BUILD_DIR", MnCommon::projectPath()) );
          args.append( pbtools::toQString(menu.app()).replace("$BUILD_DIR", MnCommon::projectPath()) );
        }
        for(const auto& arg : menu.arg()) {
          args.append(pbtools::toQString(arg).replace("$BUILD_DIR", MnCommon::projectPath()));
        }
        if((false == meteo::global::isRoot()) && (menu.has_appsu()))
        {
          registerWidget(id, pbtools::toQString(menu.appsu()).replace("$BUILD_DIR", MnCommon::projectPath()), args);
        }
        else
        {
          registerWidget(id, pbtools::toQString(menu.app()).replace("$BUILD_DIR", MnCommon::projectPath()), args);
        }
      }

      TAction* tact = new TAction(title);
      tact->setObjectName(id);
      tact->setConfig(menu);
      tact->setIconVisibleInMenu(true);
      tact->setIcon(QIcon(pbtools::toQString(menu.ico())));
      tact->setDisabled(menu.disabled());

      tactions_.insert(id, tact);
      if( true == menu.has_handler() ){
        TFormAdaptor::instance()->registerAction(tact);
      }
      act = tact;
    }
    else
    {
      parse(menu, head);
    }
  }
  return act;
}

void WidgetHandler::addMenuFromConfigToMenu(QMainWindow* mainwindow, const QString& filename, const QString& title)
{
  if(nullptr == mainwindow || nullptr == mainwindow->menuBar() ){
    return;
  }

  QList<QAction*> menulist = mainwindow->menuBar()->actions();
  for(auto a : mainwindow->menuBar()->actions())
  {
    if(a->text() == title)
    {
      meteo::menu::MenuConfig menuconf;
      if(false == Conf::load(&menuconf, filename)) {
        error_log << QObject::tr("Ошибка загрузки конфигурации: %1").arg(filename);
      }
      for(const meteo::menu::MenuItem &menu : menuconf.menu())
      {
        QMenu* qm = a->menu();
        if(nullptr == qm) {
          return;
        }
        QString id = pbtools::toQString(menu.id());
        QMenu* subm = new QMenu(pbtools::toQString(menu.title()), qm);
        qm->addMenu(subm);
        subm->setObjectName(id);
        parse(menu, subm);
      }
    }
  }
}

void WidgetHandler::addMenu(const meteo::menu::MenuItem& menu)
{
  if(nullptr == mainwindow_ || nullptr == mainwindow_->menuBar()) {
    return;
  }

  QAction* act = actionByName(mainwindow_->menuBar()->actions(), pbtools::toQString(menu.id()));

  QMenu* qm = nullptr;
  if(nullptr != act) {
    qm = act->menu();
  }
  else {
    qm = mainwindow_->menuBar()->addMenu(pbtools::toQString(menu.title()));
  }
  parse(menu, qm);
}

bool WidgetHandler::registerWidget(const QString& name, const QString& executable, const QStringList& arguments)
{
  if(wgtapps_.contains(name)) {
    return false;
  }

  wgtapps_.insert(name, executable);
  wgtargs_.insert(name, arguments);
  return true;
}

QWidget*WidgetHandler::createWidget(const QString& name, const QString& title, const QString& options )
{
  QWidget* wgt = widget(name);
  if(nullptr != wgt) {
    return wgt;
  }

  if(true == wgtfuncs_.contains(name))
  {
    QWidget* p = mainwindow_;
    wgt = (wgtfuncs_[name])(p, options);
    if(nullptr == wgt) {
      return nullptr;
    }
    wgt->setWindowTitle(title);
    wgt->setObjectName(name);
    QObject::connect(wgt, &QWidget::destroyed, this, &WidgetHandler::slotCreatedDestroyed);
    created_.insert(name, wgt);
    mainwindow_->toMdi(wgt);
    return wgt;
  }
  else if(wgtapps_.contains(name))
  {
    if(nullptr != appwatcher_) {
      return appwatcher_->create(wgtapps_[name], wgtargs_[name], title);
    }
    return nullptr;
  }
  else
  {
    error_log
        << QObject::tr("Имя виджета =") << name
        << QObject::tr("Не найдена функция создания виджета.");
    return nullptr;
  }
}

void WidgetHandler::parse(const meteo::menu::MenuItem& menu, QMenu* m)
{
  if(nullptr == m) {
    warning_log << QObject::tr("Нулевой указатель QMenu* m");
    return;
  }

  m->menuAction()->setVisible(false == menu.hidden());

  if(menu.has_ico()) {
    m->setIcon(QIcon(pbtools::toQString(menu.ico())));
  }

  QMap<QString, QAction*> actions;

  QString id, title, position;
  for(const meteo::menu::MenuItem& item : menu.menu())
  {
    id = pbtools::toQString(item.id());
    title = pbtools::toQString(item.title());   
    if (0 == item.menu_size())
    {
      if(item.has_app())
      {
        QStringList args;
        if((false == meteo::global::isRoot()) && (item.has_argsu()))
        {
          args.append( pbtools::toQString(item.argsu()).replace("$BUILD_DIR", MnCommon::projectPath()) );
          args.append( pbtools::toQString(item.app()).replace("$BUILD_DIR", MnCommon::projectPath()) );
        }
        for(const auto& arg : item.arg()) {
          args.append(pbtools::toQString(arg).replace("$BUILD_DIR", MnCommon::projectPath()));
        }
        if((false == meteo::global::isRoot()) && (item.has_appsu()))
        {
          registerWidget(id, pbtools::toQString(item.appsu()).replace("$BUILD_DIR", MnCommon::projectPath()), args);
        }
        else
        {
          registerWidget(id, pbtools::toQString(item.app()).replace("$BUILD_DIR", MnCommon::projectPath()), args);
        }
      }

      TAction* act = new TAction(title);
      act->setObjectName(id);
      act->setConfig(item);
      act->setIconVisibleInMenu(true);
      act->setIcon(QIcon(pbtools::toQString(item.ico())));
      act->setDisabled(item.disabled());

      if(false == item.hidden())
      {
        if(false == position.isEmpty())
        {
          QAction* before = tactions_.value(position);
          if(nullptr == before) {
            before = qactions_.value(position);
          }
          if(nullptr == before) {
            m->addAction(act);
          }
          else {
            m->insertAction(before, act);
          }
        }
        else
        {
          m->addAction(act);
        }
      }

      if(item.separator())
      {
        if(false == position.isEmpty())
        {
          QAction* before = tactions_.value(position);
          if(nullptr == before) {
            before = qactions_.value(position);
          }
          if(nullptr == before) {
            m->addSeparator();
          }
          else {
            m->insertSeparator(before);
          }
        }
        else
        {
          m->addSeparator();
        }
      }

      actions.insert(id, qobject_cast<QAction*>(act));
      tactions_.insert(id, act);
      if( true == item.has_handler() ){
        TFormAdaptor::instance()->registerAction(act);
      }
    }
    else {
      QMenu* subm = new QMenu(title);
      QAction* act = m->addMenu(subm);
      act->setObjectName(id);
      subm->setObjectName(id);
      qactions_.insert(id, act);

      parse(item, subm);

      if(item.separator())
      {
        if(false == position.isEmpty())
        {
          QAction* before = tactions_.value(position);
          if(nullptr == before) {
            before = qactions_.value(position);
          }
          if(nullptr == before) {
            m->addSeparator();
          }
          else {
            m->insertSeparator(before);
          }
        }
        else
        {
          m->addSeparator();
        }
      }
    }
    //item.set_handler("auto");
  }
}

void WidgetHandler::slotCreatedDestroyed(QObject* o)
{
  created_.remove(o->objectName());
}

QMap<QString, QWidget*> WidgetHandler::widgets()
{
  return created_;
}

QWidget* WidgetHandler::widget( const QString& name )
{
  return created_.value(name, nullptr);
}

void WidgetHandler::sortMenuconf(meteo::menu::MenuConfig* menuconf)
{
  meteo::menu::MenuItem* menu = nullptr;
  meteo::menu::MenuItem* menunext = nullptr;
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
    meteo::menu::MenuItem* menu = menuconf->mutable_menu(i);
    if (menu->menu_size() > 1) {
      sortMenuItemRecoursive(menu);
    }
  }
}

void WidgetHandler::sortMenuItemRecoursive(meteo::menu::MenuItem *mainmenu)
{
  meteo::menu::MenuItem* menu = nullptr;
  meteo::menu::MenuItem* menunext = nullptr;
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
    meteo::menu::MenuItem* menu=mainmenu->mutable_menu(i);
    if (menu->menu_size() > 1) {
      sortMenuItemRecoursive(menu);
    }
  }
}

}


bool Conf::load(meteo::menu::MenuConfig* conf, const QString& filename)
{
  if(nullptr == conf) {
    return false;
  }

  QFile file(filename);
  if (false == file.exists()) {
    error_log << QObject::tr("Нет файла настроек");
    return false;
  }

  if (false == file.open(QIODevice::ReadOnly)) {
    error_log << QObject::tr("Не удалось открыть для чтения файл параметров меню: %1").arg(file.errorString());
    return false;
  }

  QString strconf = QString::fromUtf8(file.readAll());
  file.close();
  return TProtoText::fillProto(strconf, conf);
}

namespace meteo {
namespace global {

void showStatusMessage( const QString& message, int msecs )
{
  return WidgetHandler::instance()->showStatusMessage( message, msecs );
}

void clearStatusMessage()
{
  return WidgetHandler::instance()->clearStatusMessage();
}



} // global
} // meteo
