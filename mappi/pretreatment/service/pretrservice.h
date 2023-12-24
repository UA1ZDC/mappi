#ifndef MAPPI_PRETREATMENT_SERVICE_PRETRSERVICE_H
#define MAPPI_PRETREATMENT_SERVICE_PRETRSERVICE_H

#include <qobject.h>

#include <meteo/commons/rpc/rpc.h>
#include <mappi/proto/satellite.pb.h>
#include <mappi/pretreatment/handler/handler.h>
#include <mappi/pretreatment/savenotify/savenotify.h>

namespace mappi {
  namespace conf {
    class DataReply;
  }
}

namespace mappi {
    namespace po {


    //! Сервис, по подписке получающий имя полученного файла и отравляющий на обработку
    class PretrService : public QObject, public mappi::conf::PretreatmentService {
      Q_OBJECT
      public:
      PretrService(QObject* parent = nullptr);
      ~PretrService();

      void init(Handler::HeaderType htype, CreateNotify::Type ntype);
      
      virtual void SessionCompleted(::google::protobuf::RpcController* ctrl, const mappi::conf::SessionBundle* req,
        mappi::conf::PretreatmentResponse* resp, ::google::protobuf::Closure* done);

    signals:
      void processSession(const QString& satName, const QString& fileName, ulong sessionId);
							    
    public slots:
      void process(const QString& satName, const QString& fileName, ulong sessionId);
      
    private:
      Handler* _handler = nullptr;
      QString _handlerPath;
      mappi::po::Handler::HeaderType _headerType = mappi::po::Handler::kSeparateHeader;
      ServiceSaveNotify* _notify = nullptr;
    };

  }
}

#endif
