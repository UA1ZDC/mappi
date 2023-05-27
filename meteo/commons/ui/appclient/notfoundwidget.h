#ifndef METEO_COMMONS_UI_APPCLIENT_NOTFOUNDWIDGET_H
#define METEO_COMMONS_UI_APPCLIENT_NOTFOUNDWIDGET_H

#include <QWidget>

namespace Ui
{
class NotFoundForm;
}


class NotFoundWidget : public QWidget
{
  Q_OBJECT
public:
  explicit NotFoundWidget(QWidget *parent = 0);
  void setText(const QString& text);
private:
  Ui::NotFoundForm* ui_;

signals:

public slots:

};

#endif // METEO_COMMONS_UI_APPCLIENT_NOTFOUNDWIDGET_H
