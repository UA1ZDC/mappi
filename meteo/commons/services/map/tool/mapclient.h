#ifndef METEO_COMMONS_SERVICES_MAP_TOOL_MAPCLIENT_H
#define METEO_COMMONS_SERVICES_MAP_TOOL_MAPCLIENT_H

#include <qbytearray.h>
#include <qstring.h>
#include <qmap.h>
#include <qlist.h>
#include <qdatetime.h>

#include <cross-commons/app/options.h>
#include <meteo/commons/planner/timesheet.h>
#include <meteo/commons/proto/document_service.pb.h>
#include <meteo/commons/proto/weather.pb.h>
#include <meteo/commons/rpc/rpc.h>
#include <qhash.h>


namespace rpc {
  class TController;
  class Channel;
}

const commons::Arg kHelp =  commons::Arg::make( "-h",   "--help", false );
const commons::Arg kJob  =  commons::Arg::make( "-j",   "--job", true );
const commons::Arg kList =  commons::Arg::make( "-l",   "--job-list", false );
const commons::Arg kAvai =  commons::Arg::make( "-a",   "--available", false );
const commons::Arg kDate =  commons::Arg::make( "-d",   "--date", true );
const commons::Arg kBegin=  commons::Arg::make( "-b",   "--begin", true );
const commons::Arg kEnd  =  commons::Arg::make( "-e",   "--end", true );
const commons::Arg kMap  =  commons::Arg::make( "-m",   "--map", true );
const commons::Arg kFormat= commons::Arg::make( "-f",   "--format", true );
const commons::Arg kHour =  commons::Arg::make( "-H",   "--hour", true );
const commons::Arg kCenterParam= commons::Arg::make( "-C",   "--center", true );
const commons::Arg kModel=  commons::Arg::make( "-M",   "--model", true );
const commons::Arg kPath =  commons::Arg::make( "-p",   "--path", true );
const commons::Arg kMapCnt= commons::Arg::make( "-c",   "--mapcenter", true );
const commons::Arg kScale=  commons::Arg::make( "-s",   "--scale", true );
const commons::Arg kProj =  commons::Arg::make( "-P",   "--projection", true );
const commons::Arg kSize =  commons::Arg::make( "-S",   "--size", true );

namespace meteo {
namespace map {

class Client
{
  public:
    Client();
    ~Client();

    QHash< proto::Job, QList<proto::Map> > getMapsFromCommandLine() const ;

    bool processJob( proto::Job& job );
    bool processJob( const proto::Job& job, const proto::Map& map, meteo::rpc::Channel* ctrl );
//
//    bool processJob( const proto::Job& job );
//    bool processJob( const proto::Job& job, const QDateTime& dt, rpc::TController* ctrl );


    const QString& documentUuid() const { return doc_uuid_; }

    bool saveFile( const proto::Map& info, const QByteArray& data, const QString& path );
    bool saveServerFile( const proto::Job& job, meteo::rpc::Channel* ctrl );

    void getAvailableDocuments( meteo::rpc::Channel* ch );

    void loadDocJobs(bool* ok = nullptr);
    void load(bool* ok = nullptr);

    QMap< QString, meteo::map::proto::Job > docjobs_;
    QList<QDateTime> termsForJob( const proto::Job& job ) const ;
    QList<proto::Map> mapsForJob( const proto::Job& job ) const ;

  private:
    QString doc_uuid_;
    QStringList doctemplates_;

    proto::Job jobDetali( const proto::Job& job ) const ;

    QList<QDateTime> jobDates( const proto::Job& job ) const ;
    QList<QDateTime> dtListFromBegintoEnd(const proto::Job &job) const;
};

}
}

#endif
