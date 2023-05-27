#ifndef METEO_COMMONS_UI_MAINWINDOW_WIDGETHANDLER_H
#define METEO_COMMONS_UI_MAINWINDOW_WIDGETHANDLER_H

#include <qmap.h>
#include <qstring.h>
#include <qmainwindow.h>
#include <qprocess.h>
#include <qstatusbar.h>

#include <QtWidgets>

#include <cross-commons/singleton/tsingleton.h>
#include <meteo/commons/formsh/thandler.h>
#include <meteo/commons/formsh/tformadaptor.h>
#include <meteo/commons/formsh/taction.h>
#include <meteo/commons/proto/meteomenu.pb.h>
#include <meteo/commons/ui/map/view/mapview.h>
#include <meteo/commons/ui/map/map.h>
#include <meteo/commons/ui/map/view/mapwindow.h>

class StationWidget;
typedef QWidget* (*Callback)( QWidget* parent,  const QString& options );

class Conf
{
public:
  Conf();
  ~Conf();
  static bool load( meteo::menu::MenuConfig* conf, const QString& filename );
};

namespace meteo {
namespace app {
class MainWindow;
class AppWatcher;
}
}

namespace mapauto {

typedef QMap<QString, QMenu*> DocMenu;
typedef QMap<QString, QWidget*> DocumentWidget;

class WidgetHandler : public THandler
{
  Q_OBJECT
  public:
    WidgetHandler();
    virtual ~WidgetHandler();

    /**
     * @brief Возвращает true, если в текущий момент в области MDI активное окно является метеокартой
     * @return
     */
    bool isCurrentTabGeoMap();

    /**
     * @brief Возвращает все окна в области MDI, которые являляются геокартой
     * @return
     */
    QList<QMdiSubWindow*> geoMapSearch();

    bool registerWidget( const QString& name, Callback call );
    bool registerWidget( const QString& name, const QString& executable, const QStringList& arguments );

    QMap<QString, Callback>& autowidgets();

    QWidget* handleEntity( TAction* act = nullptr );
    QWidget* handleEntity( const QString& name);

    void setMainwindow( meteo::app::MainWindow* w );
    meteo::app::MainWindow* mainwindow() const;
    QMenuBar* menuBar() const ;
    QWidget* widget( const QString& name );
    QMap<QString, QWidget*> widgets();

    void mergeMenu(meteo::menu::MenuConfig* target, meteo::menu::MenuConfig* source);
    void mergeMenuRecoursive(meteo::menu::MenuItem* target, meteo::menu::MenuItem* source);
    void loadMenuFromConfig(QMainWindow* mainwindow, const QString& dirname);
    QAction* addMenuFromConfigToMenu(QMainWindow *mainwindow, const QString &config, const QList<QPair<QString, QString>>& menupath);
    void addMenuFromConfigToMenu(QMainWindow *mainwindow, const QString &filename, const QString& title);
    void addMenu(const meteo::menu::MenuItem& menu);

    TAction* action( const QString& name );
    QByteArray restore(const QString& name);
    void setValue(const QString& name, const QVariant& value);
    QVariant value(const QString& name) const;
    void closeX11();

    QStatusBar* statusBar() const ;
    void showStatusMessage( const QString& message, int msecs = 0 );
    void clearStatusMessage();
    QMdiSubWindow* currentActiveSubWindow() { return currentActive_; }
    QMdiSubWindow* lastActiveSubWindow() { return lastActive_; }

  private:
    QMdiSubWindow* lastActive_ = nullptr;
    QMdiSubWindow* currentActive_ = nullptr;
    void checkMenuConfDuplicate(QMap<std::string, int> *idMenu, meteo::menu::MenuConfig menubuf);
    void checkMenuItemDuplicateRecoursive(QMap<std::string,int>* idMenu, meteo::menu::MenuItem* mainmenu);
    void sortMenuconf(meteo::menu::MenuConfig* menuconf);
    void sortMenuItemRecoursive(meteo::menu::MenuItem* mainmenu);

    WidgetHandler& operator=( const WidgetHandler& ) { return *this; }
    void parse(const meteo::menu::MenuItem& menu, QMenu* m);
    QAction* actionByName(QList<QAction*> list, const QString& name);

    QWidget* createWidget( const QString& name,
                           const QString& title,
                           const QString& options = QString() );


  private:
    QMap<QString, Callback> wgtfuncs_;

    QMap<QString, QString> wgtapps_;
    QMap<QString, QStringList> wgtargs_;

    QMap<QString,QWidget*> created_;
    QMap<QString, QWidget*> windows_;
    QMap<QString, TAction*> tactions_;
    QMap<QString, QAction*> qactions_;

    meteo::app::MainWindow* mainwindow_;
    meteo::app::AppWatcher* appwatcher_;

    //QMap<QWidget*, QProcess*> procmap_;
    //QMap<QString, QProcess*> namemap_;
    QString lastCerr_;
    QToolBar *stations_tool_bar_;
    StationWidget *stations_widget_;

  private slots:
    void slotCreatedDestroyed( QObject* o );  

  signals:
    void open(const QString& name, const QString& uuid);
};

}

typedef TSingleton<mapauto::WidgetHandler> WidgetHandler;

namespace meteo {
namespace global {
void showStatusMessage( const QString& message, int msecs = 0 );
void clearStatusMessage();
}
}

#endif
