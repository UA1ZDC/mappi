#ifndef METEO_COMMONS_CRON_CRONAPP_H
#define METEO_COMMONS_CRON_CRONAPP_H

#include <qhash.h>

#include <meteo/commons/planner/configloader.h>

namespace meteo {

class Planner;
class RunApp;

} // meteo

namespace meteo {
namespace cron {

class RunApp;
class RunAppList;
class Settings;

} // cron
} // meteo

namespace meteo {

class CronApp : public ConfigLoader
{
  Q_OBJECT
public:
  CronApp(const QString& path, Planner* planner, QObject* parent = 0);

public slots:
  virtual bool slotLoad(const QString& path = QString());

private:
  void loadRunApps(const cron::Settings& opt);

private:
  RunApp* createTask(const cron::RunApp& opt) const;
  RunApp* createTask(const cron::RunAppList& opt) const;

private:
  // параметры
  QString path_;

  // данные
  QHash<int64_t,QString> tasks_; // taskid : md5

  // служебные
  Planner* planner_;
};

} // meteo

#endif // METEO_COMMONS_CRON_CRONAPP_H
