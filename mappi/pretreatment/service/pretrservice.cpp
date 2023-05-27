#include "pretrservice.h"

#include <commons/textproto/tprototext.h>
#include <commons/textproto/pbtools.h>

#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/paths.h>

#include <meteo/commons/global/global.h>
#include <mappi/settings/mappisettings.h>

#include <mappi/proto/reception.pb.h>
#include <mappi/proto/sessiondataservice.pb.h>
#include <mappi/pretreatment/formats/satformat.h>
#include <mappi/pretreatment/handler/handler.h>
#include <mappi/schedule/schedule.h>
#include <mappi/pretreatment/savenotify/savenotify.h>

#include <qfile.h>

using namespace mappi;
using namespace po;


PretrService::PretrService(QObject* parent /*=nullptr*/):
  QObject(parent)
{
}


PretrService::~PretrService()
{
  delete _handler;
  _handler = nullptr;
  delete _notify;
  _notify = nullptr;
}

void PretrService::init(Handler::HeaderType htype, CreateNotify::Type ntype)
{
  mappi::conf::Reception conf = mappi::inter::Settings::instance()->reception();
  
  _handler = new mappi::po::Handler;
  _handler->setHeaderType(htype);
  _handler->setPath(pbtools::toQString(conf.file_storage().root()));

  _notify = CreateNotify::createServiceNotify(ntype);

  
  connect(this, &PretrService::processSession, this, &PretrService::process);
  
}

//NOTE если добавить в передаваемые параметры тип заголовка, то можно обрабатывать и виндовый, и наш формат одновременно
void PretrService::SessionCompleted(::google::protobuf::RpcController* ctrl, const mappi::conf::SessionBundle* req,
  mappi::conf::PretreatmentResponse* resp, ::google::protobuf::Closure* done)
{
  Q_UNUSED(ctrl);

   
  schedule::Session session;
  schedule::Session::fromProto(req->session(), &session);
  debug_log << session.data().toString();
  debug_log << session.data().fileName();

  mappi::conf::Reception conf = mappi::inter::Settings::instance()->reception();
  
  QString path = QString("%1/%2/%3")
    .arg(QString::fromStdString(conf.file_storage().root()))
    .arg(QString::fromStdString(conf.file_storage().session()))
    .arg(session.data().fileName());
  debug_log << path;

  emit processSession(path, -1);

  resp->set_result(true);
  done->Run();
}


void PretrService::process(const QString& filename, ulong sessionId)
{
  if (nullptr == _handler) return;

  _handler->clear();
  _handler->setFile(filename); 

  if (nullptr != _notify) {
    _notify->setSource(sessionId);
  }
  
  if (!_handler->process(_notify)) {
    error_log << "Ошибка обрабоки данных";
    return;
  } 
}
