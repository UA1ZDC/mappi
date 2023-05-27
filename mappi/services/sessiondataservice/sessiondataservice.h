#ifndef MAPPI_SERVICES_SESSIONDATASERVICE_SESSIONDATASERVICE_H
#define MAPPI_SERVICES_SESSIONDATASERVICE_SESSIONDATASERVICE_H

#include <meteo/commons/global/global.h>
#include <mappi/global/global.h>

#include <mappi/proto/sessiondataservice.pb.h>

#include <mappi/settings/mappisettings.h>

namespace mappi {

class MethodBag;

  class SessionDataService : public proto::SessionDataService
  {
  public:
    SessionDataService();
    ~SessionDataService();

    void SaveSession(::google::protobuf::RpcController* ctrl,
		     const proto::SessionData* req,
		     proto::Response* resp,
		     ::google::protobuf::Closure* done);
    

    void SavePretreatment(::google::protobuf::RpcController* ctrl,
			  const proto::Pretreatment* req,
			  proto::Response* resp,
			  ::google::protobuf::Closure* done);

    void GetAvailableThematic(::google::protobuf::RpcController* ctrl,
			      const proto::ThematicData* req,
			      proto::ThematicList* resp,
			      ::google::protobuf::Closure* done);
    
    void GetAvailableSessions(::google::protobuf::RpcController* ctrl,
			      const proto::SessionData* req,
			      proto::SessionList* resp,
			      ::google::protobuf::Closure* done);
    
    void GetAvailablePretreatment(::google::protobuf::RpcController* ctrl,
				  const proto::Pretreatment* req,
				  proto::PretreatmentList* resp,
				  ::google::protobuf::Closure* done);
    
    void SaveThematic(::google::protobuf::RpcController* ctrl,
		      const proto::ThematicData* req,
		      proto::Response* resp,
		      ::google::protobuf::Closure* done);
    
    void MarkSessionThematicDone(::google::protobuf::RpcController* ctrl,
				 const proto::SessionData* req,
				 proto::Response* resp,
				 ::google::protobuf::Closure* done);
    
    void GetSession(::google::protobuf::RpcController* ctrl,
		    const proto::SessionData* req,
		    proto::SessionList* resp,
		    ::google::protobuf::Closure* done);
    
    void GetAvailableThematicTypes(::google::protobuf::RpcController* ctrl,
				   const Dummy* req,
				   proto::ThematicList* resp,
				   ::google::protobuf::Closure* done);
    
    void GetGrayscaleChannels(::google::protobuf::RpcController* ctrl,
			      const Dummy* req,
			      proto::ThematicList* resp,
			      ::google::protobuf::Closure* done);

    void GetGeotiffInfo(google::protobuf::RpcController *ctrl, const mappi::proto::ThematicData *req,
			mappi::proto::GeotifHeader *resp, google::protobuf::Closure *done);
    
  private:
    
    MethodBag* methodbag_ = nullptr;  
    
  };
  
} //mappi

#endif // SESSIONDATASERVICE_H
