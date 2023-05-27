#ifndef METEO_COMMONS_UI_CUSTOM_FORECASTWIDGETITEM_H
#define METEO_COMMONS_UI_CUSTOM_FORECASTWIDGETITEM_H

#include <qtoolbutton.h>
#include <QString>
#include <QLabel>
#include <QHBoxLayout>
#include <QCursor>


#include <QGraphicsColorizeEffect>
#include <QColor>
#include <QIcon>
#include <QPropertyAnimation>

#include <cross-commons/debug/tlog.h>

class ForecastWidgetItem: public QWidget
{
  Q_OBJECT
  public:
    ForecastWidgetItem( QWidget* parent );
    ~ForecastWidgetItem();

    QString getName() const {return name_;};
    QString getType() const {return type_;};
    QString getCategory() const {return category_;};
    bool getBcomma() const {return bcomma_;};
    bool getComma() const {return comma_;};
    bool getCatcomma() const {return catcomma_;};
    bool getNotfirst() const {return notfirst_;};    
    
    void setName(QString name);
    void setType(QString type);
    void setCategory(QString category);
    void setBcomma(bool bcomma);
    void setComma(bool comma);
    void setCatcomma(bool catcomma);
    void setNotfirst(bool notfirst);
    

  private:

    // кнопка закрытия
    QToolButton *close_;
    // // лайаут
    QHBoxLayout *layout_;
    // // лайаут
    QLabel *label_;
    
    QString name_;
    QString type_;
    QString category_;
    bool bcomma_;
    bool comma_;
    bool catcomma_;
    bool notfirst_;

    // // длительность мерцания
    int duration_;

  signals:
    //! Данный сигнал испускается при нажатии на кнопку добавления погоды
    void updated();
    void deleteItem(ForecastWidgetItem* item);

  private slots:
    // слот который ловит сигнал нажатия на кнопку и эмитит свой сигнал с параметрами
    void buttonClicked();
    void destroySlot();

};

#endif
