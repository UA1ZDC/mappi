#ifndef METEO_COMMONS_SERVICES_MAP_METHODBAG_H
#define METEO_COMMONS_SERVICES_MAP_METHODBAG_H

#include <qmutex.h>
#include <qprocess.h>

#include <meteo/commons/proto/document_service.pb.h>
#include <meteo/commons/rpc/clienthandler.h>

namespace meteo {
namespace map {

class StatusSender;
class Service;
class Document;

class MethodBag : public QObject
{
  Q_OBJECT
  public:
    MethodBag( Service* s );
    ~MethodBag();

    void CreateDocument( QPair< rpc::ClientHandler*, const proto::Document* > param, proto::Response* resp );
    void GetDocument( QPair< rpc::ClientHandler*, const proto::ExportRequest* > param, proto::ExportResponse* resp );
    void RemoveDocument( QPair< rpc::ClientHandler*, const proto::Document* > param, proto::Response* resp );
    void CreateMap( QPair< rpc::ClientHandler*, const proto::Map* > param, proto::Response* resp );
    void ProcessJob( const proto::Job* req, proto::ExportResponse* resp );
    void SaveDocument( QPair< rpc::ClientHandler*, const proto::ExportRequest* > param, proto::Response* resp );
    void ImportDocument( QPair< rpc::ClientHandler*, const proto::ImportRequest* > param, proto::Response* resp );
    void GetAvailableDocuments( const proto::MapRequest* req, proto::MapList* resp );
    void GetAvailableClimat( const proto::ClimatRequest* req, proto::ClimatResponse* resp );
    void GetLastJobs( const proto::Dummy* req, proto::JobList* resp );

    void GetFaxes( QPair< rpc::ClientHandler*, const proto::FaxRequest*> param, proto::FaxReply* resp );
    void GetWeatherJobs( const proto::JobList* req, proto::JobList* resp );
    void SetWeatherJobs( const proto::JobList* req, proto::Response* resp );

    void GetSateliteImageList( const proto::GetSateliteImageListRequest* req,
                               proto::GetSateliteImageListReply* resp );

    void GetSateliteImageFile( const proto::GetSateliteImageRequest* req,
                               proto::GetSateliteImageReply* resp );

    void GetTile( const proto::TileRequest* req, proto::TileResponse* resp );
    void GetPunches( const proto::PunchRequest* req, proto::PunchResponse* resp );
    void GetPunchMaps( const proto::Map* req, proto::MapList* resp );
    void GetOrnament( const proto::Dummy* req, proto::Ornaments* resp );

  private:
    bool registerClient( rpc::Channel* client );
    bool addDocument( rpc::Channel* client, Document* doc );
    Document* document( rpc::Channel* client );
    Document* removeDocument( rpc::Channel* client );

    bool getDocumentFromGridfs( const proto::ExportRequest* req, proto::ExportResponse* resp );

    QByteArray getNetImage();
    void startNextJob();

  public slots:
    void slotClientConnected( meteo::rpc::Channel* ch );
    void slotClientDisconnected( meteo::rpc::Channel* ch );
    void slotMethodsFinished( meteo::rpc::ClientHandler* hndlr );
    void slotProcFinished( int exitcode, QProcess::ExitStatus status );

  private:
    Service* service_ = nullptr;
    StatusSender* sender_ = nullptr;
    QMap< rpc::Channel*, Document* > connections_;
    QMutex mutex_;
    proto::DocServiceParam param_;
    int procnum_ = 0;
    QMap< QProcess*, proto::Job > procpool_;
    QMap< QProcess*, proto::Job > runpool_;

  friend class StatusSender;
};

}
}

#endif
