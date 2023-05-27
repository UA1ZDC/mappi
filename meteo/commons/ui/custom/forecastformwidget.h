#ifndef METEO_COMMONS_UI_CUSTOM_FORECASTFORMWIDGET_H
#define METEO_COMMONS_UI_CUSTOM_FORECASTFORMWIDGET_H

#include <qwidget.h>
#include <qstring.h>

#include <cross-commons/debug/tlog.h>
#include <meteo/commons/global/global.h>
#include <commons/textproto/tprototext.h>
#include <meteo/commons/proto/forecastwidget.pb.h>
#include "forecastwidgetbutton.h"
#include "forecastwidgetinput.h" 
#include "forecastwidgetresultbox.h"

#include <QTabWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QGroupBox>
#include <QListView>
#include <QLabel>
#include <QSpacerItem>
#include <QList>
#include <QClipboard>


namespace Ui {
  class ffWidget;
}

class ForecastFormWidget : public QWidget
{
  Q_OBJECT

public:
  explicit ForecastFormWidget(QWidget *parent = 0, QString path=meteo::global::kForecastWidgetFileName);
  virtual ~ForecastFormWidget();


  // инициализация формы
  // загрузка кнопок из прото файла
  void InitForm();
  // Меняем текст кнопки при нажатии
  void changeLastSectionButtonText();

  // получаем текст уже скомпилированный
  QString getText();

  // компилируем текст исходя из
  // набранных айтемов
  void compileText();

signals:
  // испускаем сигнал, когда нажали на кнопку завершения 
  void submitFormText(QString);
  

private slots:
  void buttonForecastClicked(QString, QString, QString, bool, bool, bool, bool);
  void buttonLastSectionClicked();

  // слот  для принятия изменений от чайлдов
  void childChanged();

  // копирование в буфер обмена
  void clipboardCopy();

  // нажатие на кнопку сабмита окна
  void submitButtonClicked();

private:
  Ui::ffWidget* ui_;

  // путь к файлу с параметрами
  QString paramfilepath_;

  // текст, который результирующий
  // рассчитаный с запятыми и прочим
  QString *resulttext_;

  // список боксов
  // в которых набираются прогнозы
  QList <ForecastWidgetResultBox *> resBoxes_;

  // список полей ввода температуры
  QList <ForecastWidgetInput *> temperatureInputs_;

  // тексты для кнопок
  QString lastButtonText_ = QObject::tr("Далее >>");
  QString lastButtonTextFinish_ = QObject::tr("Сохранить");
};

#endif // METEO_COMMONS_UI_CUSTOM_FORECASTFORMWIDGET_H
