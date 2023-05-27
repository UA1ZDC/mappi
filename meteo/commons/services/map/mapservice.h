#ifndef METEO_COMMONS_SERVICES_MAP_MAPSERVICE_H
#define METEO_COMMONS_SERVICES_MAP_MAPSERVICE_H

#include <qstring.h>
#include <qmap.h>

#include <commons/obanal/tfield.h>
#include <cross-commons/app/tsigslot.h>
#include <meteo/commons/proto/document_service.pb.h>
#include <meteo/commons/global/global.h>
#include <meteo/commons/rpc/clienthandler.h>

#include <meteo/commons/services/map/methodbag.h>

namespace meteo {
namespace map {

class Document;
class LayerIso;

class Service : public QObject, public proto::DocumentService
{
  public:
    Service();
    ~Service();

    MethodBag* methodbag() const { return methodbag_; }

    void CreateDocument( google::protobuf::RpcController* c,
                  const proto::Document* req,
                  proto::Response* resp,
                  google::protobuf::Closure* d );

    void GetDocument( google::protobuf::RpcController* c,
                  const proto::ExportRequest* req,
                  proto::ExportResponse* resp,
                  google::protobuf::Closure* d );
    void RemoveDocument( google::protobuf::RpcController* c,
                  const proto::Document* req,
                  proto::Response* resp,
                  google::protobuf::Closure* d );


    void SaveDocument( google::protobuf::RpcController* c,
                  const proto::ExportRequest* req,
                  proto::Response* resp,
                  google::protobuf::Closure* d );

    void ImportDocument( google::protobuf::RpcController* c,
                         const proto::ImportRequest* req,
                         proto::Response* resp,
                         google::protobuf::Closure* d);

    void CreateMap( google::protobuf::RpcController* c,
                  const proto::Map* req,
                  proto::Response* resp,
                  google::protobuf::Closure* d );

    
    /*! 
     * \brief Выполнение задания на построение документа в отдельном процессе.
     * Метод завершает свое выполнение не дожидаясь окончания выполнения задания.
     * \param c - контроллер выполнения rpc-вызова
     * \param req - параметры задания
     * \param resp - результат выполнения задания
     * \param d - колбэк для отправки ответа
     */
    void ProcessJob(::google::protobuf::RpcController* c,
                       const ::meteo::map::proto::Job* req,
                       ::meteo::map::proto::ExportResponse* resp,
                       ::google::protobuf::Closure* d);

    void GetAvailableDocuments( google::protobuf::RpcController* c,
                  const proto::MapRequest* req,
                  proto::MapList* resp,
                  google::protobuf::Closure* d );

    void GetLastJobs( google::protobuf::RpcController* c,
                  const proto::Dummy* req,
                  proto::JobList* resp,
                  google::protobuf::Closure* d );

    void GetAvailableClimat(google::protobuf::RpcController* c,
                            const proto::ClimatRequest* req,
                            proto::ClimatResponse* resp,
                            google::protobuf::Closure* d );



    void GetWeatherJobs( google::protobuf::RpcController* c,
                  const proto::JobList* req,
                  proto::JobList* resp,
                  google::protobuf::Closure* d );

    void SetWeatherJobs( google::protobuf::RpcController* c,
                  const proto::JobList* req,
                  proto::Response* resp,
                  google::protobuf::Closure* d );
    
    void GetFaxes( google::protobuf::RpcController* c,
                 const proto::FaxRequest* req,
                 proto::FaxReply* resp,
                 google::protobuf::Closure* d );

    void GetSateliteImageList( google::protobuf::RpcController* c,
                             const proto::GetSateliteImageListRequest* req,
                             proto::GetSateliteImageListReply* resp,
                             google::protobuf::Closure* d );


    void GetSateliteImageFile( google::protobuf::RpcController* c,
                             const proto::GetSateliteImageRequest* req,
                             proto::GetSateliteImageReply* resp,
                             google::protobuf::Closure* d );

    void GetTile(::google::protobuf::RpcController* controller,
               const ::meteo::map::proto::TileRequest* request,
               ::meteo::map::proto::TileResponse* response,
               ::google::protobuf::Closure* done);

    void GetPunches(::google::protobuf::RpcController* controller,
                  const ::meteo::map::proto::PunchRequest* request,
                  ::meteo::map::proto::PunchResponse* response,
                  ::google::protobuf::Closure* done);
  
    void GetPunchMaps(::google::protobuf::RpcController* controller,
                    const ::meteo::map::proto::Map* request,
                    ::meteo::map::proto::MapList* response,
                    ::google::protobuf::Closure* done);

    void GetOrnament(::google::protobuf::RpcController* controller,
                       const ::meteo::map::proto::Dummy* request,
                       ::meteo::map::proto::Ornaments* response,
                       ::google::protobuf::Closure* done);

  private:
    MethodBag* methodbag_ = nullptr;

};

}
}

#endif
