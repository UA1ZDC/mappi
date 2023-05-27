#include "selectcenters.h"

#include <meteo/commons/global/global.h>
#include <meteo/commons/proto/sprinf.pb.h>
//#include <meteo/commons/rpc/selfchecked/selfcheckedcontroller.h>
#include <meteo/commons/proto/obanal.pb.h>
#include <meteo/commons/settings/settings.h>

#include <QStringList>

namespace {

int requestTimeout() { return 30000; }

}

namespace meteo {

SelectCentersWidget::SelectCentersWidget(QWidget* parent) :
  SelectValuesWidget(parent)
{
  setWindowTitle(QString::fromUtf8("%1: Центры").arg(windowTitle()));
}

QStringList SelectCentersWidget::loadAllValuesList()
{
  //settings::TMeteoSettings::instance()->load();
  rpc::Channel* ctrl = meteo::global::serviceChannel( meteo::settings::proto::kSprinf );
  sprinf::MeteoCenterRequest req;
  sprinf::MeteoCenters* res = ctrl->remoteCall(&sprinf::SprinfService::GetMeteoCenters, req, ::requestTimeout());
  if (res != 0) {
    for (int i = 0, sz = res->center_size(); i < sz; ++i) {
      int index = res->center(i).id();
      QString name = QString::fromStdString(res->center(i).name());
      allCenters_.insert(index, name);
    }
  }
  delete res;

  return allCenters_.values();
}

QStringList SelectCentersWidget::loadValuesList(const QString& str)
{
  if (allCenters_.isEmpty() == true) {
    loadAllValuesList();
  }

  QList<int> included;
  foreach (QString each, str.split(",")) {
    if (each.startsWith('[') == true ||
        each.endsWith(']') == true) {
      each.remove('[').remove(']');
    }
    included.append(each.trimmed().toInt());
  }

  QStringList result;
  for (int i = 0, sz = included.size(); i < sz; ++i) {
    if (allCenters_.contains(included[i]) == true) {
      result.append(allCenters_[included[i]]);
    }
  }
  return result;
}

QString SelectCentersWidget::getValues() const
{
  if (availableValues().isEmpty() == true ||
      selectedValues().isEmpty() == true) {
    return QString();
  }

  typedef QStringList::const_iterator Iter;

  QStringList centers;
  for (Iter it = selectedValues().constBegin(), end = selectedValues().constEnd(); it != end; ++it) {
    foreach (int index, allCenters_.keys(*it)) {
      centers.append(QString::number(index));
    }
  }
  centers.removeDuplicates();
  return QString("[%1]").arg(centers.join(", "));
}

SelectStationsWidget::SelectStationsWidget(QWidget* parent) :
  SelectCentersWidget(parent),
  type_(-1)
{
  setWindowTitle(windowTitle().replace(QString::fromUtf8("Центры"),
                                       QString::fromUtf8("Станции")));
}

void SelectStationsWidget::setType(int type)
{
  type_ = type;
}

QStringList SelectStationsWidget::loadAllValuesList()
{
  //settings::TMeteoSettings::instance()->load();
  rpc::Channel* ctrl = meteo::global::serviceChannel( meteo::settings::proto::kSprinf ) ;
  sprinf::MultiStatementRequest req;
  if (type_ != -1) {
    foreach (int each, getStationTypes()) {
      req.add_type(each);
    }
  }
  sprinf::Stations* res = ctrl->remoteCall(&sprinf::SprinfService::GetStations, req, ::requestTimeout());
  if (res != 0) {
    for (int i = 0, sz = res->station_size(); i < sz; ++i) {
      int index = res->station(i).index();
      QString name = QString::fromStdString(res->station(i).name().rus());
      if (name.isEmpty() == true) {
        name = QString::fromStdString(res->station(i).name().international());
      }
      if (name.isEmpty() == true) {
        name = QString::fromStdString(res->station(i).name().short_());
      }
      if (name.isEmpty() == false) {
        allCenters_.insert(index, name);
      }
    }
  }
  delete res;

  return allCenters_.values();
}

QList<int> SelectStationsWidget::getStationTypes() const
{
  // returned values: sprinf.stations_types.id

  QList<int> result;
  switch (type_) {
    case meteo::obanalsettings::kSurface:
        result << 0 << 1;
      break;
    case meteo::obanalsettings::kAero:
        result << 3 << 4;
      break;
    case meteo::obanalsettings::kRadar:
        result << 8;
      break;
    case meteo::obanalsettings::kOcean:
        result << 6;
      break;
    default:
      break;
  }
  return result;
}

} // meteo
