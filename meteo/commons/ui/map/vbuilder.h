#ifndef METEO_COMMONS_UI_MAP_VBUILDER_H
#define METEO_COMMONS_UI_MAP_VBUILDER_H

#include <qlist.h>

#include <meteo/commons/proto/meteo.pb.h>
#include <meteo/commons/proto/weather.pb.h>
#include <meteo/commons/proto/field.pb.h>

class QDateTime;

namespace zond {
class PlaceData;
}

namespace meteo {
namespace sprinf {
class StationFullInfo;
}
namespace map {

class VProfileDoc;

class VBuilder
{
public:
  VBuilder(VProfileDoc* doc);
  bool createVProfile( const proto::Map& map );

private:
  bool createSurfaceVProfile( const proto::Map& map );
  bool createFieldVProfile( const proto::Map& map );
  bool createFieldSpace( const proto::Map& map );
  bool createFieldTime( const proto::Map& map );
  bool createFieldTimeList( const proto::Map& map );
  bool createFieldBulletinSpace( const proto::Map& map );
  bool createFieldBulletinTime( const proto::Map& map );

  bool createDoc( QList<zond::PlaceData>* list, const proto::Map& map );
  void fillZond(const surf::TZondValueReply* zondRep, QList<zond::PlaceData> *list, const int echelon, const field::DataRequest* req = nullptr);
  //void fillRequest( field::DataRequest& request );
  void findDatesForBulletinTime( const meteo::field::DateReply* dateReply, QList<std::string>* dates , const QDateTime &dts, const QDateTime &dte );
//  sprinf::StationFullInfo* getStationInfo(int index);
  sprinf::Stations* getStationInfo(const sprinf::MultiStatementRequest& request);
  sprinf::Stations* getStationInfo(const QString& index);
  static bool foSortPoTime( const zond::PlaceData& p1, const zond::PlaceData& p2 );

private:
  VProfileDoc* doc_ = nullptr;

};

}
}


#endif // METEO_COMMONS_UI_MAP_VBUILDER_H
