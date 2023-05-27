#ifndef MAPPI_CRON_RUNAPPLICATION_H
#define MAPPI_CRON_RUNAPPLICATION_H

#include <meteo/commons/planner/planner.h>

namespace meteo {
namespace mappi {

class RunApp : public ::meteo::PlannerTask
{
  Q_OBJECT
  public:
    explicit RunApp( QObject* parent = 0 );
    ~RunApp();

    void setPath( const QString& path ) { path_ = path; }
    void addArg(const QString& key ) { args_.insert(key, ""); }
    void addArg(const QString& key, const QString& value ) { args_.insert(key, value); }

    void run();

  private:
    QString path_;
    QMap< QString, QString > args_;
};

}
}

#endif
