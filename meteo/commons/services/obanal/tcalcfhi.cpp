#include "tcalcfhi.h"

#include <memory>

#include <qmap.h>

#include <cross-commons/debug/tlog.h>

#include <sql/nosql/nosqlquery.h>

#include <commons/meteo_data/meteo_data.h>
#include <commons/meteo_data/tmeteodescr.h>

#include <meteo/commons/global/global.h>
#include <meteo/commons/proto/surface_service.pb.h>
#include <meteo/commons/primarydb/ancdb.h>


static constexpr int kTimeout = 30000;

namespace meteo {
namespace obanal {

double calcKv(const double ff)
{
  double k = 1.0;
  if(0 > ff) { return k; }

  double delta = 0.03 * ff;
  k += (0.5 < delta ? 0.5 : delta);
  return k;
}

bool calcAndSaveFHI(const QDate& date)
{
  QStringList needparam = QStringList()
    << "JJJ"
    << "MM"
    << "YY"
    << "GG"
    << "Lo"
    << "gg"
//    << "FHI"
    << "sec"
    << "level_type"
    << "CCCC"
    << "station_type"
    << "station_index"
    << "category"
    << "La";
  auto ancmongo = ancdb();
  if ( nullptr == ancmongo ) {
    error_log << QObject::tr("Не удалось получить доступ к справочнику станций.");
    return false;
  }
  meteo::rpc::Channel* ch = global::serviceChannel(settings::proto::kSrcData);
  if ( nullptr == ch ) {
    error_log << QObject::tr("Нет связи с сервисом данных. Расчет индекса пожарной опасности не возможен.");
    return false;
  }

  meteodescr::TMeteoDescriptor* mdescr = TMeteoDescriptor::instance();
  auto category_descr = mdescr->descriptor("category");

  QMap< QPair< QString, int >, double > R24map;
  surf::DataRequest request;
//  request.add_type(surf::kSynopFix);
  QDateTime sdt = QDateTime( date, QTime(0,0,0) );
  QDateTime edt = QDateTime( date, QTime(23,59,59) );
  request.set_date_start( sdt.toString( Qt::ISODate ).toStdString() );
  request.set_date_end( edt.toString( Qt::ISODate ).toStdString() );
  request.set_level_p(0);
  request.set_type_level(1);
  request.set_query_type(surf::kLastValue);
  request.add_meteo_descrname(QString("R24").toStdString());

  meteo::surf::DataReply* resp = ch->remoteCall(&meteo::surf::SurfaceService::GetMeteoDataByDay, request, kTimeout );
//  delete ch; ch = nullptr;
  if ( nullptr == resp) {
    error_log << QObject::tr("Не получена информация о количестве осадков. Расчет индекса пожарной опасности не возможен.");
    return false;
  }

  for ( const std::string& data: resp->meteodata() ) {
    QByteArray barr(data.data(), data.size());
    TMeteoData md;
    md << barr;
    QString stindent = mdescr->stationIdentificator(md);
    int category = md.getValue( category_descr, -1, false );
    if ( true == md.hasParam("R24") ) {
      TMeteoParam R24 = md.meteoParam("R24");
      if ( control::SPECIAL_VALUE > R24.quality() ) {
        R24map.insert( qMakePair( stindent, category ), R24.value() );
      }
    }
  }
  delete resp;
  request.Clear();

  QMap< QPair< QString, int >, double > FHImap;
//  request.add_type(surf::kSynopFix);
  sdt = QDateTime( date, QTime(0,0,0) );
  edt = QDateTime( date, QTime(23,59,59) );
  request.set_date_start( sdt.toString( Qt::ISODate ).toStdString() );
  request.set_date_end( edt.toString( Qt::ISODate ).toStdString() );
  request.set_level_p(0);
  request.set_type_level(1);
  request.set_query_type(surf::kLastValue);
  request.add_meteo_descrname(QString("FHI").toStdString());

  resp = ch->remoteCall(&meteo::surf::SurfaceService::GetMeteoDataByDay, request, kTimeout );
//  delete ch; ch = nullptr;
  if ( nullptr == resp) {
    error_log << QObject::tr("Не получена информация о количестве осадков. Расчет индекса пожарной опасности не возможен.");
    return false;
  }

  for ( const std::string& data: resp->meteodata() ) {
    QByteArray barr(data.data(), data.size());
    TMeteoData md;
    md << barr;
    QString stindent = mdescr->stationIdentificator(md);
    int category = md.getValue( category_descr, -1, false );
    if ( true == md.hasParam("FHI") ) {
      TMeteoParam FHI = md.meteoParam("FHI");
//      debug_log << "NORMAS =" << FHI.value();;
      if(control::SPECIAL_VALUE > FHI.quality()) {
        FHImap.insert( qMakePair( stindent, category ), FHI.value());
      }
    }
  }
  delete resp;
  request.Clear();

  double beg_lon = -180.0;
  for (int i = -12; i < 13; ++i ) {
    request.Clear();
    double end_lon = beg_lon + 15.00;
    if ( 12 == std::abs(i) ) {
      end_lon = beg_lon + 7.50;
    }

    QDateTime locdt = QDateTime(date, QTime(13, 0, 0)).addSecs(-i * 3600);
    if ( locdt.date() != date ) {
      locdt.setDate(date);
    }

//    request.add_type(surf::kSynopFix);
    request.set_date_start(date.toString(Qt::ISODate).toStdString());
    request.set_level_p(0);
    request.set_type_level(1);
    request.set_query_type(surf::kNearestTermValue);
//    request.set_min_lo(beg_lon);
//    request.set_max_lo(end_lon);
    request.set_term(locdt.toString(Qt::ISODate).toStdString());
    request.set_max_secs(1.5 * 3600);
    request.add_meteo_descrname(QString("T").toStdString());
    request.add_meteo_descrname(QString("Td").toStdString());
    request.add_meteo_descrname(QString("ff").toStdString());
//    request.add_meteo_descrname(QString("FHI").toStdString());

    auto pnt = request.mutable_region()->add_point();
    pnt->set_fi(-90);
    pnt->set_la(beg_lon);
    pnt = request.mutable_region()->add_point();
    pnt->set_fi(90);
    pnt->set_la(beg_lon);
    pnt = request.mutable_region()->add_point();
    pnt->set_fi(90);
    pnt->set_la(end_lon);
    pnt = request.mutable_region()->add_point();
    pnt->set_fi(-90);
    pnt->set_la(end_lon);
    pnt = request.mutable_region()->add_point();
    pnt->set_fi(-90);
    pnt->set_la(beg_lon);

    meteo::surf::DataReply* resp = ch->remoteCall(&meteo::surf::SurfaceService::GetMeteoDataByDay, request, kTimeout);
    if ( nullptr == resp ) {
      error_log << QObject::tr("Не получена информация о требуемых параметрах для часового пояса %1-%2. Расчет индекса пожарной опасности не возможен.")
        .arg(beg_lon)
        .arg(end_lon);
      continue;
    }

    for(const std::string& data: resp->meteodata()) {
      QByteArray barr(data.data(), data.size());
      TMeteoData md;
      md << barr;

      QString stindent = mdescr->stationIdentificator(md);
      int category = md.getValue( category_descr, -1, false );
      StationInfo info;
      if ( false == ancmongo->fillStationInfo( &md, category, &info ) ) {
        error_log << QObject::tr("Не удалось получить информацию о станции = %1. Тип данных = %2").arg(stindent).arg(category);
        continue;
      }

      GeoPoint coord;
      if(false == mdescr->getCoord(md, &coord)) {
        warning_log << QObject::tr("Станция без координат. Пропуск.");
        continue;
      }
//      if ( -68.0 > coord.latDeg() || 68.0 < coord.latDeg() ) {
//        continue;
//      }

      double FHI = 0.0;
      if ( true == FHImap.contains( qMakePair( stindent, category ) ) ) {
        FHI = FHImap.value( qMakePair( stindent, category ) );
//        debug_log << "NORMAS2 = " << FHI;
      }

      if ( true == md.hasParam("T") && true == md.hasParam("Td") && true == md.hasParam("ff") ) {
        TMeteoParam T  = md.meteoParam("T");
        TMeteoParam Td = md.meteoParam("Td");
        TMeteoParam ff = md.meteoParam("ff");
        if ( control::SPECIAL_VALUE > T.quality() &&
             control::SPECIAL_VALUE > Td.quality() &&
             control::SPECIAL_VALUE > ff.quality() )
        {
          FHI = calcFHI( FHI, ff.value(), T.value(), Td.value() );
        }
      }
      if ( 0 > FHI) {
        FHI = 0.0;
      }
//      debug_log << "NOVY FHI = " << FHI;
      FHI = FHItestR24( FHI, R24map.value( qMakePair( stindent, category ), 0) );
//      debug_log << "POSLE OSADKOV R24 == " <<R24map.value( qMakePair( stindent, category ), 0)<< "FHI ="<< FHI ;

//      md.remove( "T", true );
//      md.remove( "Td", true );
//      md.remove( "ff", true );
      QStringList allnames = md.allNames();
      for ( auto s : allnames ) {
        if ( false == needparam.contains(s) ) {
          md.remove(s);
        }
      }
      md.set( "FHI", "", FHI, control::RIGHT );
      ancmongo->saveReport( md, category, QDateTime( date.addDays(1), QTime(0,0,0)), info, "meteoreport" );
//      addFHIMeteoData( info, md, date.addDays(1) );
    }
    beg_lon = end_lon;
    delete resp;
  }
  delete ch;

  return true;
}

double FHItestR24(const double FHI, const double R24)
{
  if ( true == MnMath::isEqual( 0.0, R24 ) ) {
    return FHI;
  }
  if ( FHI <= 300.0 && 2.0 <= R24) {
    return 0.0;
  }
  else if ( FHI > 12000.0 && 15.0 <= R24 ) {
    return 0.0;
  }
  else if ( R24 * 1000 - 2000 >= FHI ) {
    return 0.0;
  }
  return FHI;
}

double calcFHI(double kp, const double ff, const double T, const double Td)
{
  kp += calcKv(ff) * T * (T - Td);
  return kp;
}

///void addFHIMeteoData( const StationInfo& info, const TMeteoData& md, const QDate& dt )
///{
///  auto ancmongo = anc::dbmongo();
///  ancmongo->saveReport(  );
///  Q_UNUSED(info);
///  Q_UNUSED(md);
///  Q_UNUSED(dt);
///  return;
///}

}
}
