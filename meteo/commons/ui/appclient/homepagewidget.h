#ifndef METEO_COMMONS_UI_APPCLIENT_HOMEPAGEWIDGET_H
#define METEO_COMMONS_UI_APPCLIENT_HOMEPAGEWIDGET_H

#include <qwidget.h>

namespace Ui { class HomePageForm; }

namespace meteo {

class HomePageWidget : public QWidget
{
  Q_OBJECT
public:
  explicit HomePageWidget(QWidget *parent = 0);

signals:
  void updateServices();
  void removeConnection();

private slots:
  void slotHelp();

private:
  Ui::HomePageForm* ui_;
};

} // meteo

#endif // METEO_COMMONS_UI_APPCLIENT_HOMEPAGEWIDGET_H
