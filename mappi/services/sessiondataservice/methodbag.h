#ifndef MAPPI_SERVICES_SESSIONDATASERVICE_METHODBAG_H
#define MAPPI_SERVICES_SESSIONDATASERVICE_METHODBAG_H


#include <meteo/commons/rpc/rpc.h>
// #include <meteo/commons/proto/forecast.pb.h>
// #include <meteo/commons/proto/field.pb.h>

#include <mappi/proto/sessiondataservice.pb.h>

// #include <sql/nosql/nosqlc.h>
// #include <sql/nosql/nosqlquery.h>
#include <sql/dbi/dbi.h>
#include <sql/dbi/dbientry.h>

namespace mappi {

  class SessionDataService;
  
  class MethodBag : public QObject
  {
    Q_OBJECT
  public:
    MethodBag(SessionDataService *s);
    ~MethodBag();

    void SaveSession( QPair< meteo::rpc::ClientHandler*, const mappi::proto::SessionData*> request, mappi::proto::Response* response );
    void SavePretreatment( QPair< meteo::rpc::ClientHandler*, const mappi::proto::Pretreatment*> request, mappi::proto::Response* response );

    void GetAvailableSessions( QPair< meteo::rpc::ClientHandler*, const mappi::proto::SessionData*> request, mappi::proto::SessionList* response );
    void GetSession( QPair< meteo::rpc::ClientHandler*, const mappi::proto::SessionData*> request, mappi::proto::SessionList* response);
    
    void MarkSessionThematicDone( QPair< meteo::rpc::ClientHandler*, const mappi::proto::SessionData*> request, mappi::proto::Response* response );
      
    void GetAvailablePretreatment( QPair< meteo::rpc::ClientHandler*, const mappi::proto::Pretreatment*> request, mappi::proto::PretreatmentList* response );
    
      
    void GetAvailableThematic( QPair< meteo::rpc::ClientHandler*, const mappi::proto::ThematicData*> request, mappi::proto::ThematicList* response );
    void GetGeotiffInfo( QPair< meteo::rpc::ClientHandler*, const mappi::proto::ThematicData*> request, mappi::proto::GeotifHeader* response );
    void SaveThematic( QPair< meteo::rpc::ClientHandler*, const mappi::proto::ThematicData*> request, mappi::proto::Response* response );
            
    void GetAvailableThematicTypes( QPair< meteo::rpc::ClientHandler*, const Dummy*> request, mappi::proto::ThematicList* response );
    void GetGrayscaleChannels(QPair< meteo::rpc::ClientHandler*,  const Dummy*> request, mappi::proto::ThematicList* response );


    //статистика
    // const QPair< QDateTime, QString >& runnedstat() const { return runnedstat_; }
    // const QList< QPair< QDateTime, QString > >& deferredstat() const { return deferredstat_; }

  private:
    bool sessionFromQuery(const meteo::DbiEntry &result, proto::SessionData* session);
    bool themFromQuery(const meteo::DbiEntry &doc, proto::ThematicData* them);    


  private:
    // QPair< QDateTime, QString > runnedstat_;
    // QList< QPair< QDateTime, QString > > deferredstat_;

    SessionDataService* service_ = nullptr;
    
  };
   
}



#endif
