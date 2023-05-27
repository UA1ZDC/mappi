#ifndef METEO_COMMONS_UI_MAINWINDOW_MAINWINDOW_H
#define METEO_COMMONS_UI_MAINWINDOW_MAINWINDOW_H

#include <qstring.h>
#include <qstringlist.h>
#include <qevent.h>
#include <qmainwindow.h>
#include <qmdiarea.h>
#include <qstackedwidget.h>
#include <qmap.h>

#include <meteo/commons/proto/meteomenu.pb.h>
#include <meteo/commons/ui/mainwindow/mainwindowplugin.h>

class QMenuBar;

namespace meteo {
namespace app {

class MapView;
class MdiSubWindow;

//! Главное окно приложения meteo.map
class MainWindow : public QMainWindow
{
  Q_OBJECT
  public:
    MainWindow();
    ~MainWindow();

    void setAppName(const QString &);
    /*!
     * \brief askUser Функция для запроса "Да"/"Нет" пользователю
     */
    bool askUser(const QString& request);


    /*!
     * \brief loadPlugins Функция для загрузки плагинов, которые взаимодествуют с главным окном программы
     */
    void loadPlugins();
    /*!
     * \brief addPluginGroup функция добавления группы плагинов для загрузки
     * \param gr название группы
     */
    void addPluginGroup( const QString& gr );

    bool pluginLoaded(const QString& name) { return plugins_.contains(name); }
    MainWindowPlugin* plugin(const QString& name) { return plugins_.value(name, nullptr); }

    /*!
     * \brief Функция для добавления QAction в главное меню окна
     * \param title <id, title> Надпись для добавляемого пункта меню
     * \param menupath <id, title> Вложенность пункта меню.
     * Если menupath не задан, то будет добавлен пункт меню верхнего уровня
     * \return Указатель QAction, связанный с добавленным пунктом меню
     */
    QAction* addActionToMenu(const QPair<QString, QString>& title, const QList<QPair<QString, QString>>& menupath = QList<QPair<QString, QString>>());

    /*!
     * \brief addMenuFromConfigToMenu Функция добавления пункта меню окна
     * \param config описание пункта меню meteo::menu::MenuItem(proto)
     * \param path вложенность пункта меню <id, title>
     * \return Указатель QAction, связанный с добавленным пунктом меню
     */
    QAction* addMenuFromConfigToMenu(const QString &config, const QList<QPair<QString, QString>>& path);

    //! Рекурсивная функция для поиска пути к элементу меню с идентификатором itemId.
    QStringList itemPath(const meteo::menu::MenuItem& menu, const QString& itemId) const;
    //! Рекурсивная функция для поиска пути к элементу меню с названием itemTitle.
    QStringList itemPathByTitle(const meteo::menu::MenuItem& menu, const QString& itemTitle) const;

    MdiSubWindow* toMdi(QWidget* wgt);
    QMdiArea* mdi() const { return mdi_; }
    void setActiveWindow( QWidget* wgt );

    void setupToolbar();
    void saveToolbar();




    /*!
     * \brief MainWindow::findAction рекурсивный поиск действия в лесе действий actions
     * \param actions - лес действий
     * \param id      - идентификатор искомого действия
     * \return nullptr - если действие не найдено
     */
    static QAction* findAction(const QList<QAction*> actions, const QString& id);
    /*!
     * \brief MainWindow::globalFindAction рекурсивный поиск действия во всем дереве действий menuBar
     * \param id - идентификатор искомого действия
     * \return nullptr - если действие не найдено
     */
    QAction* findAction(const QString& id) const;


  protected:
    void showEvent( QShowEvent* event );
    void closeEvent(QCloseEvent * event);
    void resizeEvent(QResizeEvent* e);
    void moveEvent(QMoveEvent* e);

    QAction* addSlotToMenuRecoursive(QMenu* target, const QPair<QString, QString>& title, const QList<QPair<QString, QString>>& address);

  private slots:
    void slotWindowMenuAboutShow();
    void slotWindowMenuAboutHide();
    void slotExcept();
    void slotRemoveDocument( MdiSubWindow* window );
    void slotActivateWindow();
    void restoreSubWindow();
    void slotSubWindowActivated(QMdiSubWindow* window );

    void slotTabModeToggled( bool fl );
    void slotToolbar();

    void slotCloseSubwindowsAndExit();
    void slotSetTitle(const QString & t);

  private:
    QMdiArea* mdi_;
    QMenu* menuWindow_;
    QMenu* menuToolbar_;

    QList<QAction*> menuactions_;
    QMap< QString, MainWindowPlugin* > plugins_;
    QStringList plugroups_;
    void cleanMenu(QList<QAction*> menulist);
    void cleanMenu();
    QString app_name_;
};
} // app
} // meteo


#endif
