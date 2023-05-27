#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <qmainwindow.h>

#include <meteo/commons/proto/map_document.pb.h>
#include <meteo/commons/proto/meteomenu.pb.h>
#include <commons/geobasis/geopoint.h>

namespace meteo {

namespace app {
class MainWindow;
}

namespace map {

class MapView;
class MapScene;
class Document;

class MapWindow: public QMainWindow
{
    Q_OBJECT
  public:
    MapWindow( app::MainWindow* window, const proto::Document& blank, const QString& menufile = QString() );
    MapWindow( app::MainWindow* window, const proto::Document& blank, const QString& loader, const QString& menufile = QString() );
    MapWindow( app::MainWindow* window, Document* doc, const QString& menufile = QString() );
    ~MapWindow();

    void createMap( const GeoPoint& gp, ProjectionType proj );
    void setCurTitle( );

    MapView* mapview() const { return mapview_; }
    MapScene* mapscene() const ;
    Document* document() const ;
    app::MainWindow*  mainwindow() const {return window_;}

    void loadMenuFromConfig(const QString& dirname );

    void mergeMenu(meteo::menu::MenuConfig* target, meteo::menu::MenuConfig* source);
    void mergeMenuRecoursive(meteo::menu::MenuItem* target, meteo::menu::MenuItem* source);

    QAction* findActionInMenu(const QString& id);

    /*!
     * \brief Функция для добавления QAction в окно с картой
     * \param title <id, title> Надпись для добавляемого пункта меню
     * \param menupath <id, title> Вложенность пункта меню.
     * Если menupath не задан, то будет добавлен пункт меню верхнего уровня
     * \return Указатель QAction, связанный с добавленным пунктом меню
     */

    QAction* addActionToMenu( const QPair<QString, QString>& title, const QList<QPair<QString, QString>>& menupath = QList<QPair<QString, QString>>() );

    /*!
     * \brief Функция для добавления QAction в окно с картой
     * \param title Надпись для добавляемого пункта меню
     * \param menupath Вложенность пункта меню. Если menupath не задан, то будет добавлен пункт меню верхнего уровня
     * \return Указатель QAction, связанный с добавленным пунктом меню
     */
    QAction* addActionToMenu( const QString& title, const QStringList& menupath = QStringList() ) __attribute_deprecated__ = delete;

    /*!
     * \brief
     * \param title
     * \param menutitle
     * \return
     */
    QAction* addActionToMenu( const QString& title, const QString& menutitle ) __attribute_deprecated__ = delete;

    /*!
     * \brief
     * \param title
     * \param menupath
     * \param beforepath
     * \param beforeitem
     * \return
     */
    QAction* addActionToMenu( const QString& title, const QStringList& menupath, const QString& beforepath, const QString& beforeitem = QString() ) __attribute_deprecated__ = delete;

    /*!
     * \brief
     * \param title
     * \param menutitle
     * \param beforepath
     * \param beforeitem
     * \return
     */
    QAction* addActionToMenu( const QString& title, const QString& menutitle, const QString& beforepath, const QString& beforeitem = QString() ) __attribute_deprecated__ = delete;

    /*!
     * \brief Функция для добавления QAction в окно с картой из конфигурационного файла
     * \param menufile Имя файла со структурой меню
     * \param menuid Идентификатор пункт меню в файле
     * \return Указатель QAction, связанный с добавленным пунктом меню
     */
    QAction* addActionToMenuFromFile(const QString& menuid ) __attribute_deprecated__ = delete;


    /*!
     * \brief Получить указатель на QAction в главном меню с наименованием name
     * \param id id пункта меню, который треюбуется найти
     * \return указатель на найденный QAction. 0 - если не найден
     */
    QAction* menubarActionByName( const QString& id ) const ;

    /*!
     * \brief  Получить указатель на QAction с id в подменю главного меню с id baritemid
     * \param id id пункта меню, который треюбуется найти
     * \param baritemid id пункта в главном меню, в котором будет осуществляться поиск пункта с именем name
     * \return указатель на найденный QAction. 0 - если не найден
     */
    QAction* menuActionByName( const QString& id, const QString& baritemid = QString() ) const ;

    /*!
     * \brief Получить указатель на QAction с наименованием name в меню menu
     * \param menu Меню, в котором осуществляется поиск
     * \param id id искомого пункта
     * \return указатель на найденный QAction. 0 - если не найден
     */
    QAction* menuActionByName( QMenu* menu, const QString& id ) const ;

    void addPluginGroup( const QString& gr );

    void loadBaseLayersVisibilitySettings();

  protected:
    void closeEvent( QCloseEvent* e );
    bool eventFilter( QObject* o, QEvent* e );

  private:
    app::MainWindow* window_;
    MapView* mapview_;
    QString menufile_;

    QAction* addSlotToMenuRecoursive( QMenu* target, const QString& title, const QStringList& path, const QString& beforeitem = QString() ) __attribute_deprecated__ = delete;
    QAction* addSlotToMenuRecoursive(QMenu* target, const QPair<QString, QString>& title, const QList<QPair<QString, QString>>& address);
    QAction* findInMenuActionRecoursive (const QList<QAction*> menulist, const QPair<QString, QString> &title);
    QAction* addActionToMenuFromProtoMenu( const meteo::menu::MenuItem& menu, const QString& menuid, const QStringList& path, const QString& before = QString() ) __attribute_deprecated__ = delete;

    void parse(const meteo::menu::MenuItem& menu, QMenu* m);
    void applyConfSettingsRecoursive(QList<QAction*> menulist);
    void cleanMenu(QList<QAction*> menulist);
    void sortMenuconf(meteo::menu::MenuConfig* menuconf);
    void sortMenuItemRecoursive(meteo::menu::MenuItem* mainmenu);
    void checkMenuConfDuplicate(QMap<std::string, int> *idMenu, meteo::menu::MenuConfig menubuf);
    void checkMenuItemDuplicateRecoursive(QMap<std::string,int>* idMenu, meteo::menu::MenuItem* mainmenu);
};


}
}

#endif
