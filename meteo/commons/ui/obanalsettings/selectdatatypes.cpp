#include "selectdatatypes.h"

#include <meteo/commons/global/global.h>
#include <meteo/commons/proto/sprinf.pb.h>
//#include <meteo/commons/rpc/selfchecked/selfcheckedcontroller.h>
#include <meteo/commons/settings/settings.h>

#include <QStringList>

namespace {

int requestTimeout() { return 1000; }

}

namespace meteo {

SelectDataTypesWidget::SelectDataTypesWidget(QWidget* parent) :
  SelectValuesWidget(parent)
{
  setWindowTitle(QString::fromUtf8("%1: Типы данных").arg(windowTitle()));
}

QStringList SelectDataTypesWidget::loadAllValuesList()
{
  //settings::TMeteoSettings::instance()->load();
  rpc::Channel* ctrl =  meteo::global::serviceChannel( meteo::settings::proto::kSprinf );
  sprinf::MeteoParameterRequest req;
  sprinf::MeteoParameters* res = ctrl->remoteCall(&sprinf::SprinfService::GetMeteoParametersByBufr, req, ::requestTimeout());
  if (res != 0) {
    for (int i = 0, sz = res->parameter_size(); i < sz; ++i) {
      int descr = res->parameter(i).bufr();
      QString name = QString::fromStdString(res->parameter(i).name());
      allDataTypes_.insert(descr, name);
    }
  }
  delete res;

  return allDataTypes_.values();
}

QStringList SelectDataTypesWidget::loadValuesList(const QString& str)
{
  loadAllValuesList();

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
    if (allDataTypes_.contains(included[i]) == true) {
      result.append(allDataTypes_[included[i]]);
    }
  }
  return result;
}

QString SelectDataTypesWidget::getValues() const
{
  if (availableValues().isEmpty() == true ||
      selectedValues().isEmpty() == true) {
    return QString();
  }

  typedef QStringList::const_iterator Iter;

  QStringList datatypes;
  for (Iter it = selectedValues().constBegin(), end = selectedValues().constEnd(); it != end; ++it) {
    foreach (int index, allDataTypes_.keys(*it)) {
      datatypes.append(QString::number(index));
    }
  }
  datatypes.removeDuplicates();
  return QString("[%1]").arg(datatypes.join(", "));
}

} // meteo
