#ifndef METEO_MAPPI_CLEANER_APPSTATUS_H
#define METEO_MAPPI_CLEANER_APPSTATUS_H

#include <qmap.h>
#include <qdatetime.h>

#include <meteo/commons/global/statussender.h>

namespace meteo {
class Planner;
} // meteo

namespace mappi {
namespace cleaner {

class AppStatus : public meteo::StatusSender
{
  Q_OBJECT

  static const std::string kRunnig;
  static const std::string kStopped;

public:
  explicit AppStatus(meteo::Planner* planner, QObject* parent = 0);

  virtual meteo::app::OperationStatus operationStatus() const;

private:
  // данные
  mutable QMap<QString,meteo::app::OperationParam> data_;

  // служебные
  meteo::Planner* planner_;
};

} // cleaner
} // mappi

#endif // METEO_MAPPI_CLEANER_APPSTATUS_H
