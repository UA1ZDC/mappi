#ifndef METEO_COMMONS_UI_CUSTOM_FORECASTWIDGETRESULTBOX_H
#define METEO_COMMONS_UI_CUSTOM_FORECASTWIDGETRESULTBOX_H

#include <qwidget.h>
#include <QList>
#include <QGroupBox>
#include <QString>
#include <QVBoxLayout>
#include <QSpacerItem>
#include <QScrollArea>

#include <cross-commons/debug/tlog.h>
#include "forecastwidgetitem.h"

class ForecastWidgetResultBox: public QScrollArea
{
  Q_OBJECT
  public:
    ForecastWidgetResultBox( QWidget* parent );
    ~ForecastWidgetResultBox();

    // добавляем
    void add( ForecastWidgetItem* item );
    // добавляем c параметрами
    void addWithParam( QString name, QString type, QString cat, bool  bcomma, bool comma, bool catcomma, bool notfirst );
    // вставляем
    void insert( ForecastWidgetItem* item, int index );
    // удаляем
    void remove(int index);
    // подсчитываем количество кнопок
    int count();
    // удаляем
    void setName(QString name);
    // получаем по индексу
    ForecastWidgetItem* get(int index);

    // компилируем текст исходя из
    // набранных айтемов
    void compileText();

    // получаем текст уже скомпилированный
    QString getText();


  private:
    // функция инициализации
    void Init();

    // лайаут
    QVBoxLayout *layout_;
    // список указателей на виджеты
    QList <ForecastWidgetItem *> *items_;

    // выбранный айтем
    int selectedItem_;

    int id_;
    // название таба и есть первый текст
    QString name_;

    // текст, который результирующий
    // рассчитаный с запятыми и прочим
    QString *resulttext_;

  signals:
    // список изменен!
    // испускаем, когда поменялся список (добавили/удалили айтем)
    void itemsChanged();
    // емитим, когда меняется текст 
    void boxChanged();

  private slots:
    // ловим изменения в айтемах
    void itemChange();
    void itemDelete(ForecastWidgetItem* item);
};

#endif
