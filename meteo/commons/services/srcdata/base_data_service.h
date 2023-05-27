#ifndef METEO_COMMONS_SERVICES_SRCDATA_BASE_DATA_SERVICE_H
#define METEO_COMMONS_SERVICES_SRCDATA_BASE_DATA_SERVICE_H

#include <qobject.h>

#include <meteo/commons/global/global.h>
#include <meteo/commons/proto/surface_service.pb.h>

class NS_PGBase;

namespace meteo {
namespace surf {

class TBaseDataService : public QObject {
  Q_OBJECT

  google::protobuf::Closure* done_;
  int serviceType_;

protected:

public:
  TBaseDataService( google::protobuf::Closure* done, int serviceType = settings::proto::kSrcData);
  virtual ~TBaseDataService();

  google::protobuf::Closure* GetClosure() { return done_; }

public slots:
  virtual void run() = 0;

signals:
  void finished(TBaseDataService *);

};

} // surf
} // meteo

#endif // METEO_COMMONS_SERVICES_SRCDATA_BASE_DATA_SERVICE_H
