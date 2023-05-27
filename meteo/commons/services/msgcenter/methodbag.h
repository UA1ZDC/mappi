#ifndef METEO_COMMONS_SERVICES_METHODBAG_H
#define METEO_COMMONS_SERVICES_METHODBAG_H

#include <qobject.h>

#include <meteo/commons/proto/msgcenter.pb.h>

namespace meteo {

namespace tlg {
  class MessageNew;
}

class MsgCenter;
class DbiQuery;
class DbiEntry;

class MethodBag : public QObject
{
  Q_OBJECT
  public:
    MethodBag( MsgCenter* s );
    ~MethodBag();

    void GetUndecodedFaxTelegram( const msgcenter::GetUndecodedFaxTelegramRequest* req, msgcenter::GetTelegramResponse* resp );
    void GetTelegram( const msgcenter::GetTelegramRequest* req, msgcenter::GetTelegramResponse* resp );
    void SetDecode( const msgcenter::DecodeRequest* req, msgcenter::Dummy* resp );
    void GetReport( const msgcenter::GetReportRequest* req, msgcenter::GetReportResponse* resp );

  private:
    void makeQuery( const msgcenter::GetTelegramRequest& r, DbiQuery* query ) const ;
    void fillMessage( const DbiEntry& doc, tlg::MessageNew* mes ) const ;

  private:
    MsgCenter* service_;
};

}

#endif
