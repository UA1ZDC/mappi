#ifndef METEO_COMMONS_UI_CUSTOM_FORECASTWIDGETBUTTON_H
#define METEO_COMMONS_UI_CUSTOM_FORECASTWIDGETBUTTON_H

#include <qpushbutton.h>
#include <cross-commons/debug/tlog.h>
#include <QString>
#include <QCursor>


#include <QGraphicsColorizeEffect>
#include <QColor>
#include <QPropertyAnimation>
#include <QPalette>

class ForecastWidgetButton: public QPushButton
{
  Q_OBJECT
  public:
    ForecastWidgetButton( QWidget* parent );
    ~ForecastWidgetButton();

    QString getName() const {return name_;};
    QString getType() const {return type_;};
    QString getCategory() const {return category_;};
    bool getBcomma() const {return bcomma_;};
    bool getComma() const {return comma_;};
    bool getCatcomma() const {return catcomma_;};
    bool getNotfirst() const {return notfirst_;};    
    void setColor(int r, int g, int b);
    
    void setName(QString name);
    void setType(QString type);
    void setCategory(QString category);
    void setBcomma(bool bcomma);
    void setComma(bool comma);
    void setCatcomma(bool catcomma);
    void setNotfirst(bool notfirst);

  private:
   
    // // длительность
    int duration_;

    // цвет анимации
    int r_;
    int g_;
    int b_;

    QString name_;
    QString type_;
    QString category_;
    bool bcomma_;
    bool comma_;
    bool catcomma_;
    bool notfirst_;
    

  signals:
    //! Данный сигнал испускается при нажатии на кнопку добавления погоды
    void clickedWithParam( QString, QString, QString, bool, bool, bool, bool);

  private slots:
    // слот который ловит сигнал нажатия на кнопку и эмитит свой сигнал с параметрами
    void buttonClicked();

};

#endif
