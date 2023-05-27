#include "forecastaccuracy.h"
#include "forecastaccuracydatamodel.h"

#include <cross-commons/debug/tmap.h>

#include <commons/meteo_data/tmeteodescr.h>
#include <commons/textproto/pbtools.h>

#include <meteo/commons/global/global.h>
#include <meteo/commons/proto/sprinf.pb.h>
#include <meteo/commons/rpc/rpc.h>

namespace meteo {

static auto kMethod = QObject::tr("method");

ForecastAccuracyViewer::ForecastAccuracyViewer(QWidget* parent) :
  CustomViewer(parent)
{
  setObjectName("forecast_accuracyviewer");
  this->setDataModel(new meteo::ui::ForecastAccuracyDataModel(100));
}


bool ForecastAccuracyViewer::init()
{
  bool res = CustomViewer::init();  
  this->tablewidget()->horizontalHeader()->setStretchLastSection(true);
  return res;
}

bool ForecastAccuracyViewer::load()
{
  this->slotUpdate();
  return true;
}

void ForecastAccuracyViewer::setDefaultSectionSize()
{
  if( nullptr == header() ){
      return;
  }
  header()->blockSignals(true); 
  header()->resizeSection(logicalIndexByName("punkt.punkt_id"), 170);
  header()->resizeSection(logicalIndexByName("method"), 120);
  header()->resizeSection(logicalIndexByName("opr"), 170);
  header()->resizeSection(logicalIndexByName("not_opr"), 170);
  header()->resizeSection(logicalIndexByName("total"), 170);


  header()->blockSignals(false);
}

void ForecastAccuracyViewer::setDefaultSectionOrder()
{
}


void ForecastAccuracyViewer::setDefaultSectionVisible()
{
  if( nullptr == tablewidget() ){
    return;
  }
 }

void ForecastAccuracyViewer::setMenuColumn()
{

}

}
// }
