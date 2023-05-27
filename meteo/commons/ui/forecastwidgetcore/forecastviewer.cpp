#include "forecastviewer.h"
#include "forecastdatamodel.h"

#include <cross-commons/debug/tmap.h>

#include <commons/textproto/pbtools.h>

#include <meteo/commons/global/global.h>
#include <meteo/commons/proto/sprinf.pb.h>
#include <meteo/commons/rpc/rpc.h>

#include <meteo/commons/ui/customviewer/customviewerdatamodel.h>

namespace meteo {

ForecastViewer::ForecastViewer(QWidget* parent) :
  CustomViewer(parent)
{
  setObjectName("forecast_viewer");
  this->setDataModel(new meteo::ui::ForecastDataModel(100));
}


bool ForecastViewer::init()
{
  bool res = CustomViewer::init();
  auto model = getDataModel();
  QDateTime today = QDateTime( QDate::currentDate(), QTime(0,0));
  Condition condition = ConditionDateTimeInterval("dt",
                                                  model->columnDisplayName("dt"),
                                                  today,
                                                  today.addDays(1));

  getDataModel()->addCondition("dt", condition, true);

  header()->setSecondValueDisplayed(getDataModel()->columnIndexByName("dt"), false);
  header()->setSecondValueDisplayed(getDataModel()->columnIndexByName("dt_beg"), false);
  header()->setSecondValueDisplayed(getDataModel()->columnIndexByName("dt_end"), false);
  header()->setSecondValueDisplayed(getDataModel()->columnIndexByName("param.accuracy_field_date"), false);
  header()->setSecondValueDisplayed(getDataModel()->columnIndexByName("param.accuracy_data_date"), false);

  return res;
}

void ForecastViewer::setDefaultSectionSize()
{
  if( nullptr == header() ){
      return;
  }
  header()->blockSignals(true);
  header()->resizeSection(logicalIndexByName("_id"), 90);
  header()->resizeSection(logicalIndexByName("site.name"), 170);
  header()->resizeSection(logicalIndexByName("param.descr"), 420);
  header()->resizeSection(logicalIndexByName("level"), 220);
  header()->resizeSection(logicalIndexByName("level_type"), 220);
  header()->resizeSection(logicalIndexByName("hour"), 90);
  header()->resizeSection(logicalIndexByName("center"), 270);
  header()->resizeSection(logicalIndexByName("param.real_value"), 220);
  header()->resizeSection(logicalIndexByName("param.real_field_value"), 220);
  header()->resizeSection(logicalIndexByName("param.method"), 170);
  header()->resizeSection(logicalIndexByName("param.code"), 140);
  header()->resizeSection(logicalIndexByName("param.quality"), 110);
  header()->resizeSection(logicalIndexByName("dt"), 200);
  header()->resizeSection(logicalIndexByName("dt_beg"), 170);
  header()->resizeSection(logicalIndexByName("dt_end"), 170);
  header()->resizeSection(logicalIndexByName("param.accuracy_field_date"), 170);
  header()->resizeSection(logicalIndexByName("param.accuracy_data_date"), 250);
  header()->blockSignals(false);
}

void ForecastViewer::setDefaultSectionOrder()
{
}

void ForecastViewer::setDefaultSectionVisible()
{
  if( nullptr == tablewidget() ){
      return;
  }

  tablewidget()->setColumnHidden(logicalIndexByName("_id"), true);
  tablewidget()->setColumnHidden(logicalIndexByName("param.real_value"), true);
  tablewidget()->setColumnHidden(logicalIndexByName("param.real_field_value"), true);
  tablewidget()->setColumnHidden(logicalIndexByName("param.quality"), true);
  tablewidget()->setColumnHidden(logicalIndexByName("param.accuracy_field_date"), true);
  tablewidget()->setColumnHidden(logicalIndexByName("param.accuracy_data_date"), true);
}

void ForecastViewer::setMenuColumn()
{  
}

}
