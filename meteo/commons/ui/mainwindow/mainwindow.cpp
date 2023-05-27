#include "mainwindow.h"
#include <qaction.h>
#include <qmenu.h>
#include <qdir.h>
#include <qfileinfo.h>

#include <QtWidgets>

#include <cross-commons/app/paths.h>

#include <commons/geobasis/geopoint.h>
#include <commons/geobasis/projection.h>
#include <commons/textproto/pbtools.h>
#include <commons/textproto/tprototext.h>

#include <meteo/commons/global/global.h>
#include <meteo/commons/formsh/taction.h>
#include <meteo/commons/formsh/tformadaptor.h>

#include "widgethandler.h"
#include "mdisubwindow.h"

namespace {
const QString applicationMenuPlugins() { return MnCommon::pluginPath("");}
static const QString kMainWindowConfDir( QDir::homePath() + QString("/.meteo/mainwindow") );
const QString toolbarPath() { return MnCommon::etcPath() +"/toolbar.conf"; }
void mkMainwindowConfDir()
{
  QFileInfo fi(kMainWindowConfDir);
  if ( false == fi.exists() ) {
    QDir dir;
    dir.mkpath(kMainWindowConfDir);
  }
}
bool confShowTabs()
{
  mkMainwindowConfDir();
  bool res = false;
  QFileInfo fi( kMainWindowConfDir + "/showtabs" );
  if ( false == fi.exists() ) {
    return res;
  }
  QFile file( fi.absoluteFilePath() );
  if ( false == file.open( QIODevice::ReadOnly ) ) {
    return res;
  }
  QByteArray loc = file.readAll();
  QString str = QString::fromUtf8(loc);
  if ( -1 != str.indexOf("true") ) {
    res = true;
  }
  file.close();
  return res;
}
void saveConfShowTabs( bool fl )
{
  mkMainwindowConfDir();
  QFileInfo fi( kMainWindowConfDir + "/showtabs" );
  QFile file( fi.absoluteFilePath() );
  if ( false == file.open( QIODevice::WriteOnly | QIODevice::Truncate ) ) {
    return;
  }
  QString str("false");
  if ( true == fl ) {
    str = "true";
  }
  file.write( str.toUtf8() );
  file.flush();
  file.close();

}
}

namespace meteo {
namespace app {

const QString applicationMenuDir() { return QString(MnCommon::etcPath()) + QString("/mainwindow.menu"); }
static const QString kSettingFileName = QDir::homePath() + "/.meteo/mainwindow/" + "showToolbar" + ".ini";

MainWindow::MainWindow()
  : QMainWindow(),
    mdi_(new QMdiArea(this))
{
  mdi_->setOption( QMdiArea::DontMaximizeSubWindowOnActivation, true );
  mdi_->setTabsClosable(true);
  WidgetHandler::instance()->setMainwindow(this);
  WidgetHandler::instance()->loadMenuFromConfig( this, applicationMenuDir() );
  // Add exit
  {
    QAction* exit = findAction("exit");
    if(nullptr == exit) {
      warning_log.msgBox() << QObject::tr("Предупреждение: в меню приложения нет пункта меню 'Выход' или этот пункт меню неверно настроен. Проверьте конфигурацию меню приложения.");
    }
    else {
      QObject::connect(exit, &QAction::triggered, this, &MainWindow::slotCloseSubwindowsAndExit);
    }
  }
  // Add window submenu
  {
    QAction* a = findAction("window");
    if(a == nullptr) {
      QMenu* m = new QMenu(QObject::tr("Окно"), this);
      m->setObjectName("window");
      a = menuBar()->addMenu(m);
      a->setObjectName("window");
    }
    menuWindow_ = a->menu();
    a = menuWindow_->addAction(QIcon(":/meteo/icons/misc/cascade.png"), QObject::tr("Расположить &каскадом"),
                               mdi_, SLOT(cascadeSubWindows()));
    a->setObjectName("wincascade"); a->setIconVisibleInMenu(true);

    a = menuWindow_->addAction(QIcon(":/meteo/icons/misc/tile.png"), QObject::tr("Расположить &плиткой"),
                               mdi_, SLOT(tileSubWindows()));
    a->setObjectName("wintile"); a->setIconVisibleInMenu(true);

    a = menuWindow_->addAction(QIcon(":/meteo/icons/misc/maximize.png"), QObject::tr("Восстановить &окна"),
                               this, SLOT(restoreSubWindow()));
    a->setObjectName("winrestore"); a->setIconVisibleInMenu(true);

    a = menuWindow_->addAction(QObject::tr("Отображать &вкладки"), this, SLOT(slotTabModeToggled(bool)));
    a->setObjectName("wintabs");

    bool res = confShowTabs();
    a->setCheckable(true);
    a->setChecked(res);
    slotTabModeToggled(res);
  }

  QObject::connect(menuWindow_, &QMenu::aboutToShow, this, &MainWindow::slotWindowMenuAboutShow);
  QObject::connect(menuWindow_, &QMenu::aboutToHide, this, &MainWindow::slotWindowMenuAboutHide);

  QMainWindow::setCentralWidget(mdi_);

  setObjectName("MainWindow");
  setWindowTitle(app_name_);
  setContextMenuPolicy(Qt::NoContextMenu);
  connect(mdi_, &QMdiArea::subWindowActivated, this, &MainWindow::slotSubWindowActivated);
}

MainWindow::~MainWindow()
{
}

void MainWindow::setAppName(const QString & apname)
{
  app_name_ = apname;
}


void MainWindow::setupToolbar()
{
  QAction* a = findAction("panels");
  if (nullptr == a) {
    a = addActionToMenu({"panels", QObject::tr("Панели инструментов")}, {{"parameters", QObject::tr("Параметры")}});
    //    error_log << QObject::tr("Неизвестная ошибка. Пункт меню == nullptr");
    //    a->setDisabled(false);
    //    return;
  }
  if(nullptr == a->menu()) {
    QMenu *qm = new QMenu(QObject::tr("Панели инструментов"), menuBar());
    qm->setObjectName("panels");
    a->setMenu(qm);
  }
  a->setDisabled(false);
  menuToolbar_ = a->menu();

  meteo::menu::ToolbarConfig conf;
  TProtoText::fromFile(::toolbarPath(), &conf);
  for(const meteo::menu::Toolbar& toolbar : conf.toolbar()) {
    QAction* a = menuToolbar_->addAction(pbtools::toQString(toolbar.title()), this, SLOT(slotToolbar()));
    a->setObjectName(pbtools::toQString(toolbar.id()));
    a->setCheckable(true);
    QToolBar* bar = addToolBar(pbtools::toQString(toolbar.title()));
    bar->setObjectName(pbtools::toQString(toolbar.id()));
    bar->setVisible(toolbar.checked());
    a->setChecked(toolbar.checked());
    for(const auto& action : toolbar.action()) {
      QAction* act = findAction(pbtools::toQString(action));
      if (nullptr != act && act->icon().isNull()){
        debug_log << "Добавление элементов на toolbar без иконок ЗАПРЕЩЕНО"<< "Элемент: " << act->text();
        continue;
      }
      if(nullptr != act) {
        bar->addAction(act);
      }
    }
  }
}

void MainWindow::saveToolbar()
{
  if(nullptr == menuToolbar_) {
    return;
  }
  meteo::menu::ToolbarConfig conf;
  TProtoText::fromFile(::toolbarPath(), &conf);
  for(QAction* act : menuToolbar_->findChildren<QAction*>()) {
    for(int i = 0, sz = conf.toolbar_size(); i < sz; ++i) {
      if(pbtools::toQString(conf.toolbar(i).id()) == act->objectName()) {
        conf.mutable_toolbar(i)->set_checked(act->isChecked());
        break;
      }
    }
  }
  TProtoText::toFile(conf, ::toolbarPath());
}

void MainWindow::loadPlugins( )
{
  for(const auto& gr : plugroups_)
  {
    QDir dir(applicationMenuPlugins() + gr);
    if ( false == dir.exists() ) {
      warning_log << QObject::tr("Директория %1 с плагинами не обнаружена")
                     .arg( dir.absolutePath() );
      continue;
    }

    QStringList files = dir.entryList(QStringList() << "*.so" << "*.dll");
    for(const auto& file : files) {
      QString pname(dir.absoluteFilePath(file));
      QPluginLoader loader(pname, this);
      if(false == loader.isLoaded()) {
        if(false == loader.load()) {
          error_log << QObject::tr("Не удалось загрузить плагин %1.\n\tОшибка = %2")
                       .arg(pname)
                       .arg(loader.errorString());
        }
      }
      MainWindowPlugin* plugin = qobject_cast<MainWindowPlugin*>(loader.instance());
      if(nullptr != plugin) {
        if(false == plugins_.contains(plugin->name())) {
          plugins_.insert(plugin->name(), plugin);
        }
      }
    }
  }
}

void MainWindow::addPluginGroup(const QString &gr)
{
  if (false == plugroups_.contains(gr)) {
    plugroups_.append(gr);
    loadPlugins();
  }
  cleanMenu();
}

QAction* MainWindow::addMenuFromConfigToMenu(const QString &config, const QList<QPair<QString, QString>>& path)
{
  QAction* action = nullptr;
  QList<QAction*> actions;  // Возвращать когда будет заполнятся список меню
  // Ищем добовляемые пункты меню в дереве menuBar()
  meteo::menu::MenuConfig menuConfig;
  TProtoText::fillProto(config, &menuConfig);
  for(const meteo::menu::MenuItem& menu : menuConfig.menu()) {
    QPair<QString, QString> id = {pbtools::toQString(menu.id()), pbtools::toQString(menu.title())};
    action = findAction(id.first);

    if(nullptr != action) { // пункт меню существует
      // TODO : добавить недастающие свойства
      actions.append(action);
      return action;
    }
    else { // Добавляем новый пункт меню в соответствие с расположением
      auto addrlist = path;
      QPair<QString, QString> menuname = addrlist.takeFirst();
      QMenu* m = nullptr;
      for(auto a : menuBar()->actions()) {
        if(a->objectName() == menuname.first) {
          m = a->menu();
          if(nullptr == m) {
            m = new QMenu(menuname.second, menuBar());
            m->setObjectName(menuname.first);
            a->setMenu(m);
          }
          break;
        }
      }
      if(nullptr == m) {
        m = new QMenu(menuname.second, menuBar());
        m->setObjectName(menuname.first);
        menuBar()->addMenu(m)->setObjectName(menuname.first);
      }
      action = addSlotToMenuRecoursive(m, id, addrlist);
      actions.append(action);
      // TODO : добавить недастающие свойства

      QIcon icon(QString::fromStdString(menu.ico()));
      if (!icon.isNull()) {
        action->setIcon(icon);
      } else {
        debug_log << "Добавление элементов на toolbar без иконок ЗАПРЕЩЕНО"<< "Элемент: " << action->text();
      }
      return action;
    }
  }
  return action;
}

QAction* MainWindow::addActionToMenu(const QPair<QString, QString>& title, const QList<QPair<QString, QString>>& menupath)
{
  QAction* action = findAction(title.first);
  if(nullptr != action) {
    return action;
  }

  QList<QAction*> menulist = menuBar()->actions();
  //Добавляем Action, если путь пуст
  if(menupath.empty())
  {
    for(auto a : menuBar()->actions())
    {
      if (a->objectName() == title.first) {
        warning_log << QObject::tr("В главное меню уже добавлен пункт '%1'")
                       .arg(title.first);
        if (nullptr != a->menu() && a->menu()->actions().empty()) {
          a->setMenu(nullptr);
        }
        return a;
      }
    }
    auto act = menuBar()->addAction(title.second);
    act->setObjectName(title.first);
    return act;
  }

  auto addrlist = menupath;
  QPair<QString, QString> menuname = addrlist.takeFirst();
  QMenu* m = nullptr;
  for (auto a : menuBar()->actions())
  {
    if (a->objectName() == menuname.first)
    {
      m = a->menu();
      if (nullptr == m) {
        warning_log << QObject::tr("Непонятная ситуация. Найден пункт '%1' на панели меню, который не является меню, но должен им быть.")
                       .arg(a->objectName());
        m = new QMenu(menuname.second, menuBar());
        m->setObjectName(menuname.first);
        a->setMenu(m);
      }
      break;
    }
  }

  if (nullptr == m ) {
    m = new QMenu(menuname.second, menuBar());
    m->setObjectName(menuname.first);
    menuBar()->addMenu(m)->setObjectName(menuname.first);
  }
  return addSlotToMenuRecoursive(m, title, addrlist);
}


MdiSubWindow* MainWindow::toMdi(QWidget* wgt)
{
  MdiSubWindow* sub = new MdiSubWindow(mdi_,wgt);
  //wgt->setFocusPolicy(Qt::NoFocus);
  wgt->setWindowIcon(QIcon(":/meteo/icons/misc/windowico.png"));
  QObject::connect(sub, &MdiSubWindow::mdiSubWindowClosed, this, &MainWindow::slotRemoveDocument);
  QObject::connect(wgt, SIGNAL(windowTitleChanged(const QString &)), sub,SLOT(slotSetTitle(const QString &)));
  QObject::connect(sub, SIGNAL(windowTitleChanged(const QString &)), this,SLOT(slotSetTitle(const QString &)));
  sub->showMaximized();
  setWindowTitle(wgt->windowTitle());

  return sub;
}

void MainWindow::setActiveWindow( QWidget* wgt )
{
  if ( nullptr == wgt ) {
    error_log << QObject::tr("Нулевой указатель!");
    return;
  }
  QMdiSubWindow* sub = qobject_cast<QMdiSubWindow*>(wgt);
  while ( nullptr == sub && nullptr != wgt->parentWidget() ) {
    wgt = wgt->parentWidget();
    sub = qobject_cast<QMdiSubWindow*>(wgt);
  }
  if ( nullptr != sub ) {
    mdi()->setActiveSubWindow(sub);
  }
}

void MainWindow::slotSetTitle(const QString & t){
  setWindowTitle(t);
 // emit(windowTitleChanged(t));
}

bool MainWindow::askUser( const QString& message){
  return meteo::global::askUser(this, message);
}

QAction* MainWindow::addSlotToMenuRecoursive(QMenu* target, const QPair<QString, QString>& title, const QList<QPair<QString, QString>>& address)
{
  // Мы в меню в котором нужно разместить экшин
  if (address.empty()) {
    // Ищем созданные экшены
    for(auto act : target->actions()) {
      if(act->objectName() == title.first) {
        target->setObjectName(title.first);
        return act;
      }
    }
    // Создаем новый экшен
    auto act = target->addAction(title.second);
    target->setObjectName(title.first);
    act->setObjectName(title.first);
    return act;
  }

  // Рекурсивно спускаемся по дереву меню
  auto newlist = address;
  QPair<QString, QString> menuname = newlist.takeFirst();
  QList<QAction*> menulist = target->actions();
  QMenu* m = nullptr;
  for(auto a : menulist)
  {
    if (a->objectName() == menuname.first)
    {
      m = a->menu();
      if(nullptr == m) {
        //warning_log << QObject::tr("Непонятная ситуация. Найден пункт '%1' на панели меню, который не является меню, но должен им быть.")
        //.arg(a->objectName());
        m = new QMenu(target);
        a->setMenu(m);
      }
      break;
    }
  }

  if(nullptr == m) {
    m = new QMenu(menuname.second, target);
    m->setObjectName(menuname.first);
    target->addMenu(m)->setObjectName(menuname.first);
  }
  return addSlotToMenuRecoursive(m, title, newlist);
}

void MainWindow::showEvent( QShowEvent* ev )
{
  QMainWindow::showEvent(ev);
}

void MainWindow::closeEvent( QCloseEvent* event )
{
  for ( QMdiSubWindow* w : mdi_->subWindowList() ) {
    if( nullptr != w ){
      if( true == w->close() ) {
        event->accept();
      }
      else {
        event->ignore();
        return;
      }
    }
  }
  WidgetHandler::instance()->closeX11();
}

void MainWindow::resizeEvent(QResizeEvent* e)
{
  //  WidgetHandler::instance()->saveGeometry(objectName(), saveGeometry());
  QMainWindow::resizeEvent(e);
}

void MainWindow::moveEvent(QMoveEvent* e)
{
  //  WidgetHandler::instance()->saveGeometry(objectName(), saveGeometry());
  QMainWindow::moveEvent(e);
}



void MainWindow::slotTabModeToggled(bool fl)
{
  if(nullptr == mdi_) {
    return;
  }
  if(true == fl) {
    mdi_->setViewMode(QMdiArea::TabbedView);
  }
  else {
    mdi_->setViewMode(QMdiArea::SubWindowView);
  }

  QList<QMdiSubWindow*> list = mdi_->subWindowList();
  for(auto sub : list) {
    sub->setWindowState(Qt::WindowMaximized);
  }
  QMdiSubWindow * asw = mdi_->activeSubWindow();
  if(nullptr != asw) setWindowTitle(asw->windowTitle());

  saveConfShowTabs(fl);
}

void MainWindow::slotToolbar()
{
  QAction* act = qobject_cast<QAction*>(sender());
  if(nullptr == act) {
    return;
  }

  QList<QToolBar*> lst = findChildren<QToolBar*>();
  for(auto b : lst ) {
    if(b->objectName() == act->objectName()) {
      b->setVisible(act->isChecked());
    }
  }
  saveToolbar();
}

void MainWindow::slotCloseSubwindowsAndExit()
{
  if(nullptr == mdi_) {
    qApp->quit();
  }
  for(auto w : mdi_->subWindowList()) {
    if(false == w->close()) {
      return;
    }
  }
  qApp->quit();
}


QStringList MainWindow::itemPath(const meteo::menu::MenuItem& menu, const QString& itemId) const
{
  QStringList path;
  for(const meteo::menu::MenuItem& mn : menu.menu())
  {
    const QString id = pbtools::toQString(mn.id());
    if(0 != mn.menu_size()) {
      path = itemPath(mn, itemId);
      if(false == path.empty()) {
        path.prepend(id);
        break;
      }
      else if(id == itemId) {
        path << id;
        break;
      }
    }
  }
  return path;
}

QStringList MainWindow::itemPathByTitle(const meteo::menu::MenuItem& menu, const QString& itemTitle) const
{
  QStringList path;
  for(const meteo::menu::MenuItem& mn : menu.menu())
  {
    const QString title = pbtools::toQString(mn.title());
    if(0 != mn.menu_size()) {
      path = itemPathByTitle(mn, itemTitle);
      if(false == path.empty()) {
        path.prepend(title);
        break;
      }
    }
    else if(title == itemTitle) {
      path << title;
      break;
    }
  }

  return path;
}

void MainWindow::slotWindowMenuAboutShow()
{
  if(mdi_->subWindowList().empty()) {
    return;
  }
  menuactions_.append(menuWindow_->addSeparator());
  menuactions_.append(menuWindow_->addAction(QObject::tr("Закрыть всё"), mdi_, SLOT(closeAllSubWindows())));
  menuactions_.append(menuWindow_->addAction(QObject::tr("Закрыть"), mdi_, SLOT(closeActiveSubWindow())));
  menuactions_.append(menuWindow_->addAction(QObject::tr("Закрыть другие"), this, SLOT(slotExcept())));
  menuactions_.append(menuWindow_->addSeparator() );
  menuactions_.append(menuWindow_->addAction(QIcon(":/meteo/icons/arrow/arrowleft.png"), QObject::tr("Следующий документ"), mdi_,
                                             SLOT(activatePreviousSubWindow() ) ) );
  menuactions_.last()->setIconVisibleInMenu(true);
  menuactions_.append( menuWindow_->addAction(QIcon(":/meteo/icons/arrow/arrowright.png"),QObject::tr("Предыдущий документ"), mdi_,
                                              SLOT(activateNextSubWindow() ) ) );
  menuactions_.last()->setIconVisibleInMenu(true);
  //  menuWindow_->addAction(QObject::tr("Открыть в новом окне"), this, SLOT(openNewWindow()), Qt::CTRL + Qt::Key_O);
  menuactions_.append( menuWindow_->addSeparator() );
  QList<QMdiSubWindow*> list = mdi_->subWindowList();
  for ( int i = 0, sz = list.size(); i < sz; ++i ) {
    QAction* a = menuWindow_->addAction( list[i]->widget()->windowTitle().simplified() );
    //debug_log<<list[i]->widget()->windowTitle();
    a->setData(i);
    a->setCheckable(true);
    if ( mdi_->activeSubWindow() == list[i] ) {
      a->setChecked(true);
    }

    QObject::connect( a, SIGNAL( triggered() ), this, SLOT( slotActivateWindow() ) );
    menuactions_.append(a);
  }
}

void MainWindow::slotWindowMenuAboutHide()
{
  while (false == menuactions_.empty()) {
    menuactions_.takeFirst()->deleteLater();
  }
}

void MainWindow::slotExcept()
{
  if(nullptr == mdi_) {
    return;
  }

  QList<QMdiSubWindow*> sublist = mdi_->subWindowList();
  if(sublist.empty()) {
    return;
  }

  QMdiSubWindow* asub = mdi_->activeSubWindow();
  if(nullptr == asub) {
    return;
  }

  for(QMdiSubWindow* sub : sublist) {
    if(nullptr != sub) {
      if(sub != asub) {
        sub->close();
      }
    }
  }
}

void MainWindow::slotRemoveDocument(MdiSubWindow* sub)
{
  if(nullptr == mdi_||0 == mdi_->subWindowList().size()) {
    setWindowTitle(app_name_);
    return;
  }

  if(nullptr != sub) {
    mdi_->removeSubWindow(sub);
  }
  if(0 == mdi_->subWindowList().size()) {
    setWindowTitle(app_name_);
  }

  sub->deleteLater();
}

void MainWindow::slotActivateWindow()
{
  QAction* a = qobject_cast<QAction*>(sender() );
  if (nullptr == a) {
    return;
  }

  int i = a->data().toInt();
  QList<QMdiSubWindow*> list = mdi_->subWindowList();
  if ( i >= list.size() || 0 > i ) {
    return;
  }

  if ( mdi_->activeSubWindow() != list[i] ) {
    mdi_->setActiveSubWindow(list[i]);
    slotSetTitle(list[i]->windowTitle());
  }
}

void MainWindow::restoreSubWindow()
{
  if(nullptr == mdi_) {
    return;
  }

  QMdiSubWindow* sub = mdi_->activeSubWindow();

  if(nullptr != sub) {
    sub->showMaximized();
  }
}

void MainWindow::slotSubWindowActivated(QMdiSubWindow* window)
{
  if(nullptr == window) {
    return;
  }
  MdiSubWindow* sub = qobject_cast<MdiSubWindow*>(window);
  if(nullptr != sub) {
    sub->setVisibleChildWindow(true);
  }

  QList<QMdiSubWindow*> lst = mdi_->subWindowList();
  for(QMdiSubWindow* w : lst) {
    if (w == window) {
      continue;
    }
    sub = qobject_cast<MdiSubWindow*>(w);
    if(nullptr != sub) {
      sub->setVisibleChildWindow(false);
    }
  }
  setWindowTitle(window->windowTitle());
}


/*!
 * \brief MainWindow::findAction рекурсивный поиск действия в лесе действий actions
 * \param actions - лес действий
 * \param id      - идентификатор искомого действия
 * \return nullptr - если действие не найдено
 */
QAction* MainWindow::findAction(const QList<QAction*> actions, const QString& id) //static
{
  QAction* action = nullptr;
  for(const auto act : actions) {
    if(act->objectName() == id) {
      action = act;
      break;
    }
    else {
      if(nullptr != act->menu()) {
        action = findAction(act->menu()->actions(), id);
        if(nullptr != action) {
          break;
        }
      }
    }
  }
  return action;
}

/*!
 * \brief MainWindow::findAction рекурсивный поиск действия во всем дереве действий menuBar
 * \param id - идентификатор искомого действия
 * \return nullptr - если действие не найдено
 */
QAction* MainWindow::findAction(const QString& id) const
{
  return findAction( menuBar()->actions(), id );
  /*
  QAction* action = nullptr;

  for(const auto act : menuBar()->actions())
  {
    if(act->objectName() == id) {
      action = act;
      break;
    }
    else {
      if(nullptr != act->menu()) {
        action = findAction(act->menu()->actions(), id);
        if(nullptr != action) {
          break;
        }
      }
    }
  }
  return action;
  */
}

void MainWindow::cleanMenu()
{
  cleanMenu(menuBar()->actions());
}

void MainWindow::cleanMenu(QList<QAction *> menulist)
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

} // app
} // meteo
