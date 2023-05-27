#ifndef METEO_UI_SETTINGS_APPSTART_ARGSVIEW_H
#define METEO_UI_SETTINGS_APPSTART_ARGSVIEW_H

#include <QDialog>

class QString;

namespace Ui {
  class ArgsView;
}

namespace meteo {

class AppStartArgsViewWidget : public QDialog
{
  Q_OBJECT
public:
  explicit AppStartArgsViewWidget(const QString& txt, QWidget* parent = 0);
  ~AppStartArgsViewWidget();

private:
  Ui::ArgsView* ui_;

};

} // meteo

#endif // METEO_UI_SETTINGS_APPSTART_ARGSVIEW_H
