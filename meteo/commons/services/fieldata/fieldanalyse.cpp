#include "fieldanalyse.h"
#include "fieldanalysewrap.h"

#include <qprocess.h>
#include <qstring.h>

#include <cross-commons/debug/tlog.h>
#include <commons/textproto/pbtools.h>
#include <meteo/commons/global/global.h>

//constexpr int  kMaxForecastTime = 12 * 3600; //!< максимальное время действия прогноза

namespace meteo {
namespace field {

TFieldAnalyse::TFieldAnalyse(AppStatusThread* status)
  :TFieldDataService(status)
{
  wmethod_ = new TFieldAnalyseWrap(this);

}
const std::string ERRSTR = QObject::tr("Не удалось получить обработчик из контроллера").toStdString();

TFieldAnalyse::~TFieldAnalyse()
{
}

template <typename Thread, typename Request, typename Response >
bool TFieldAnalyse::runMethod( google::protobuf::RpcController* ctrl,
                               void (Thread::*method)( Request, Response ), Request req, Response resp){
  emit signalRequestReceive();
  rpc::ClientHandler* handler = rpc::Channel::handlerFromController( ctrl );
  if ( nullptr == handler) {
    QString error = QObject::tr("Не удалось получить обработчик из контроллера");
    error_log << error;
    return false;
  }
  handler->runMethod( ctrl, wmethod_, method, req, resp);

  return true;
}

void TFieldAnalyse::getSynSit(RpcController* c, const DataRequest* req, ValueDataReply* resp, Closure* d)
{
  emit signalRequestReceive();
  if(!runMethod( c,  &TFieldAnalyseWrap::getSynSit, req, resp))
    {
      resp->set_error(ERRSTR);
      resp->set_result(false);
      d->Run();
    }
}

/*void TFieldAnalyse::GetAdvectPoints(RpcController* c, const AdvectDataRequest* req, AdvectDataReply* resp, Closure* d)
{
  if(!runMethod( c, &TFieldAnalyseWrap::GetAdvectPoints, req, resp)){
      resp->set_error(ERRSTR);
      d->Run();
    }
}
*/
void TFieldAnalyse::GetAdvectObject(RpcController *c, const AdvectObjectRequest *req, AdvectObjectReply *resp, Closure *d)
{
  emit signalRequestReceive();
  if(!runMethod( c, &TFieldAnalyseWrap::GetAdvectObject, req, resp)){
      resp->set_error(ERRSTR);
      d->Run();
    }
}

void TFieldAnalyse::CalcForecastOpr(RpcController* c, const DataAnalyseRequest* req, SimpleDataReply* resp, Closure* d)
{
  emit signalRequestReceive();
  if(!runMethod( c, &TFieldAnalyseWrap::CalcForecastOpr, req, resp)){
      resp->set_error(ERRSTR);
      d->Run();
    }
}

void TFieldAnalyse::GetExtremumsTraj(RpcController* c, const SimpleDataRequest* req, ExtremumTrajReply* resp, Closure* d){
  emit signalRequestReceive();
  if(!runMethod( c, &TFieldAnalyseWrap::GetExtremumsTraj, req, resp)){
      resp->set_error(ERRSTR);
      d->Run();
    }
}


} // field
} // meteo

