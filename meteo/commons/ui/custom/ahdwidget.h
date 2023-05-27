#ifndef METEO_COMMONS_UI_CUSTOM_AHDWIDGET_H
#define METEO_COMMONS_UI_CUSTOM_AHDWIDGET_H

#include <qwidget.h>

namespace Ui {
class AhdWidget;
}

class AhdWidget : public QWidget
{
  Q_OBJECT

public:
  explicit AhdWidget(QWidget *parent = 0);
  virtual ~AhdWidget();

  //! Возвращает все поля в виде отформатированной строки.
  QString toString() const;
  //! Возвращает поля заголовка в виде отформатированной строки. Параметр format определяет формат возвращаемой строки.
  //! Могут быть использованы следующие выражения:
  //! TT
  //! AA
  //! ii
  //! CCCC
  //! YYGGgg
  //! BBB
  QString toString(const QString& format) const;

signals:
  //! Данный сигнал испускается при изменении любого поля заголовка.
  void changed();

private:
  Ui::AhdWidget* ui_;
};

#endif // METEO_COMMONS_UI_CUSTOM_AHDWIDGET_H
