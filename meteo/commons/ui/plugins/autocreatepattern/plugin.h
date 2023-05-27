/**
  * @author Лебеденко М.С. ms1993@mail.ru
  * @brief Плагин предназначен для вызова из контекстного меню пункта "Создание шаблона",
  * который приводит к вызову виджета с наименованием шаблона, а потом самостоятельно
  * получает данные с карты а параметрах шаблона. В завершении плагин открывает виджет
  * для создания файла работ и сохраняет его.
  **/
#ifndef METEO_COMMONS_UI_PLUGINS_AUTOCREATEPATTERN_H
#define METEO_COMMONS_UI_PLUGINS_AUTOCREATEPATTERN_H

#include <meteo/commons/ui/map/view/actions/action.h>
#include <meteo/commons/ui/jobwidget/jobwidget.h>
#include <meteo/commons/rpc/rpc.h>

namespace meteo {
namespace map {

class Plugin : public ActionPlugin
{
  Q_OBJECT
  Q_PLUGIN_METADATA( IID "autocreatepattern" FILE "autocreatepattern.json" )
  public:
    Plugin();
    ~Plugin();

    Action* create( MapScene* scene ) const ;
};

class AutoCreatePattern : public Action
{
  Q_OBJECT
  public:
    AutoCreatePattern( MapScene* scene );
    ~AutoCreatePattern();

    virtual void addActionsToMenu( Menu* menu ) const ;
    void saveJobOnServer();
  private:
    QAction* insertaction_;
    proto::Map map;
    proto::Map map_info;
    JobWidget* jobWidget_=0;

private slots:
  void slotAutoCreatePattern();

};

}
}

#endif
