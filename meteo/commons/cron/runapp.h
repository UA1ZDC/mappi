#ifndef METEOPRODUCTION_CRON_RUNAPP_H
#define METEOPRODUCTION_CRON_RUNAPP_H

#include <qstringlist.h>

#include <meteo/commons/planner/planner.h>

namespace meteo {

class RunApp : public ::meteo::PlannerTask
{
  Q_OBJECT
public:
  explicit RunApp( QObject* parent = 0 );
  ~RunApp();

  int addApp(const QString& path) { ++id_; apps_[id_] = path; return id_; }
  void addArg(int appId, const QString& arg ) { args_[appId] += arg; }
  void addArg(int appId, const QString& arg, const QString& value ) { args_[appId] << arg << value; }

  virtual void run();

private:
  void runApp(const QString& appPath, const QStringList& args) const;

private:
  int id_;
  QMap<int,QString> apps_;
  QMap<int,QStringList> args_;
};

}

#endif // METEOPRODUCTION_CRON_RUNAPP_H
