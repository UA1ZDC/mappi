#ifndef METEO_COMMONS_UI_CUSTOM_FORECASTWIDGETINPUT_H
#define METEO_COMMONS_UI_CUSTOM_FORECASTWIDGETINPUT_H

#include <cross-commons/debug/tlog.h>
#include <QString>
#include <QLineEdit>
#include <QLabel>
#include <QHBoxLayout>
#include <QWidget>

class ForecastWidgetInput: public QWidget
{
  Q_OBJECT
  public:
    ForecastWidgetInput( QWidget* parent );
    ~ForecastWidgetInput();

    QString getName() const {return name_;};
    QString getText() const {return this->getName()+" "+text_;};
    
    void setName(QString name);

    
    void setGradation(int grad);

  private:
    // наименование поля
    QLabel *label_;
    // поле ввода
    QLineEdit *edit_;
    // // лайаут
    QHBoxLayout *layout_;

    // название поля
    QString name_;
    // градации
    int gradation_;
    
    QString text_;

  signals:
    //! Данный сигнал испускается при вводе данных
    void inputChange();

  private slots:
    // слот который ловит сигнал нажатия на кнопку и эмитит свой сигнал с параметрами
    void inputChanged(QString);

};

#endif
