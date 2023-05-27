#ifndef STARTAPP_H
#define STARTAPP_H

#include <qprocess.h>
#include <qdialog.h>

namespace meteo {


class StartApp : public QObject
{
    Q_OBJECT
public:
  StartApp();
  ~StartApp();
  void startUserSettings();
private slots:
  void slotUserSettingsFinished(int status);
private:
  QProcess* usersettings_;
};

} // meteo

#endif // STARTAPP_H
