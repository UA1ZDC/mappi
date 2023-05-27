#include <commons/geobasis/geopoint.h>
#include <commons/geobasis/geovector.h>
#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/paths.h>
#include <meteo/commons/global/global.h>
#include <meteo/commons/global/common.h>
#include <meteo/commons/rpc/rpc.h>
#include <meteo/commons/settings/settings.h>

#include <QCoreApplication>
#include <QString>
#include <QTextCodec>
#include <QElapsedTimer>

#include <iostream>
#include <string>

#include "client.h"

namespace sprinfclient
{

void testGetMilitaryDistrict(meteo::rpc::Channel* ctrl)
{
  QElapsedTimer ttt; ttt.start();
  meteo::sprinf::StatementRequest request;
  request.add_id(1);
 /* request.add_id(2);
  request.add_id(3);
  request.add_id(4);
  request.add_id(5);*/
  meteo::sprinf::Regions* response = ctrl->remoteCall(&meteo::sprinf::SprinfService::GetMilitaryDistrict, request, 10000);
  debug_log << QString::fromUtf8("Время выполнения: %1 мсек").arg(ttt.elapsed());
  for(auto h:response->region()){

    debug_log << h.name()<<h.region_coords_size();
  }
}


void testGetRegions(meteo::rpc::Channel* ctrl)
{
  QElapsedTimer ttt; ttt.start();
  meteo::sprinf::MultiStatementRequest request;

  request.set_query(QString("Пенз").toStdString());
  meteo::sprinf::Regions* response = ctrl->remoteCall(&meteo::sprinf::SprinfService::GetRegions, request, 10000);
  debug_log << QString::fromUtf8("Время выполнения: %1 мсек").arg(ttt.elapsed());
  debug_log << response->Utf8DebugString();
}

}

int main(int argc, char* argv[])
{
  TAPPLICATION_NAME("meteo");
  meteo::gSettings(meteo::global::Settings::instance());

  if ( false == meteo::global::Settings::instance()->load() ) {
    error_log << QObject::tr("Не удалось загрузить настройки.");
    return EXIT_FAILURE;
  }

  QCoreApplication app(argc, argv);

  Q_UNUSED(app);

  meteo::rpc::Channel* ctrl = meteo::global::serviceChannel( meteo::settings::proto::kSprinf);
  if ( 0 == ctrl) {
    error_log << meteo::msglog::kServiceConnectFailedSimple.arg(meteo::settings::proto::kSprinf);
    return 1;
  }
  sprinfclient::testGetMilitaryDistrict(ctrl);
 // sprinfclient::testGetRegions(ctrl);

//  debug_log << QString::fromUtf8("testMeteoParams");
//  sprinfclient::testMeteoParams(ctrl);
 // debug_log << QString::fromUtf8("testAddStation"); sprinfclient::testAddStation(ctrl);
//  debug_log << QString::fromUtf8("testStationTypes"); sprinfclient::testStationTypes(ctrl);
//  debug_log << QString::fromUtf8("testGmiTypes"); sprinfclient::testGmiTypes(ctrl);
//  debug_log << QString::fromUtf8("testCategories"); sprinfclient::testCategories(ctrl);
//  debug_log << QString::fromUtf8("testDisciplines"); sprinfclient::testDisciplines(ctrl);
//  debug_log << QString::fromUtf8("testLevelTypes"); sprinfclient::testLevelTypes(ctrl);
//  debug_log << QString::fromUtf8("testStations"); sprinfclient::testStations(ctrl);
//  debug_log << QString::fromUtf8("testStationByIndex"); sprinfclient::testStationByIndex(ctrl);
//  debug_log << QString::fromUtf8("testStationByType"); sprinfclient::testStationByType(ctrl);
//  debug_log << QString::fromUtf8("testStationByCountry"); sprinfclient::testStationByCountry(ctrl);
//  debug_log << QString::fromUtf8("testStationByPeriod"); sprinfclient::testStationByPeriod(ctrl);
//  debug_log << QString::fromUtf8("testStationByName"); sprinfclient::testStationByName(ctrl);
//  debug_log << QString::fromUtf8("testStationByPosition"); sprinfclient::testStationByPosition(ctrl);
//  debug_log << QString::fromUtf8("testStationInPolygon"); sprinfclient::testStationInPolygon(ctrl);
//  debug_log << QString::fromUtf8("testMeteoCenters"); sprinfclient::testMeteoCenters(ctrl);
//  debug_log << QString::fromUtf8("testParamBufr"); sprinfclient::testParamBufr(ctrl);
//  debug_log << QString::fromUtf8("testParamGrib"); sprinfclient::testParamGrib(ctrl);
//  debug_log << QString::fromUtf8("testParamDiscipline"); sprinfclient::testParamDiscipline(ctrl);
//  debug_log << QString::fromUtf8("testParamCategory"); sprinfclient::testParamCategory(ctrl);
//  debug_log << QString::fromUtf8("testBufrTables"); sprinfclient::testBufrTables(ctrl);
//  debug_log << QString::fromUtf8("testBufrParameter"); sprinfclient::testBufrParameter(ctrl);
//  debug_log << QString::fromUtf8("testMeteoParams"); sprinfclient::testMeteoParams(ctrl);
//  debug_log << QString::fromUtf8("testAddStation"); sprinfclient::testAddStation(ctrl);
  return EXIT_SUCCESS;
}

