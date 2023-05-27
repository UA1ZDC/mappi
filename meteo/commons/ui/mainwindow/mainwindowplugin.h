#ifndef METEO_COMMONS_UI_MAINWINDOW_MAINWINDOWPLUGIN_H
#define METEO_COMMONS_UI_MAINWINDOW_MAINWINDOWPLUGIN_H

#include <QtGui>
#include <qpair.h>

namespace meteo {

namespace app {

class MainWindow;

/*!
 * Класс предназначен для того, чтобы создавать плагины,
 * которые загружаются во время создания главного окна приложения
 * и добавляют пункты меню в главное меню программы с помощью функции
 * meteo::map::MainWindow::addSlotToMenu
 */
class MainWindowPlugin : public QObject
{
  Q_OBJECT
  public:
    MainWindowPlugin( const QString& name );
    virtual ~MainWindowPlugin();

    MainWindow* mainwindow() const { return window_; }

    const QString& name() const { return name_; }

    virtual QWidget* createWidget( MainWindow* mw ) const { Q_UNUSED(mw); return nullptr; }
    virtual QWidget* createWidget( MainWindow* mw, const QString& option ) const { Q_UNUSED(mw); Q_UNUSED(option); return nullptr; }

  protected:
    MainWindow* window_;

    /*!
     * \brief readConf - Чтение конфигурационного файла пункта меню
     * \param fname - Путь к файлу конфигурации
     * \return pair<Строка с конфигурацией, успех чтения>
     */
    static const QPair<QString, bool> readConf(const QString& fname);

private:
    const QString name_;
};

} // app
} // meteo

#endif
