#ifndef METEO_COMMONS_UI_MAP_WEATHERWRAP_WEATHERWRAP_H
#define METEO_COMMONS_UI_MAP_WEATHERWRAP_WEATHERWRAP_H


#include <boost/python.hpp>
#include <qbytearray.h>
#include <meteo/commons/proto/weather.pb.h>
#include <meteo/commons/proto/document_service.pb.h>

#include <qobject.h>

namespace meteo {
namespace map {
class VProfileDoc;

class CreateMap : public QObject
{
  Q_OBJECT
public:
  CreateMap( proto::Map& map, meteo::map::VProfileDoc* doc );

public slots:
  void slotCreateMap();

signals:
  void finished();
private:
  proto::Map map_;
  meteo::map::VProfileDoc* doc_;
};

class VProfiler
{
public:
  VProfiler();
  boost::python::object createVProfile();
  void addStation(const std::string station);
  void setDatetime(const std::string dateTime);
  void setEchelon(int echelon);
  void setSerialProto(const std::string serialProto);
  void initFromProto();
  void initAv12d();

private:
  proto::Map map_;
  meteo::map::VProfileDoc* doc_;
  meteo::map::proto::av12script av12proto_;


};

}
}
#endif // METEO_COMMONS_UI_MAP_WEATHERWRAP_WEATHERWRAP_H
