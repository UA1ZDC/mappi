#ifndef METEO_COMMONS_MSGSTREAM_PLUGINS_PTKPP_PTKPPSTREAMRECV_H
#define METEO_COMMONS_MSGSTREAM_PLUGINS_PTKPP_PTKPPSTREAMRECV_H

#include <qmap.h>

#include <meteo/commons/proto/msgparser.pb.h>
#include <meteo/commons/proto/ptkppstream.pb.h>
#include <meteo/commons/proto/msgstream.pb.h>
#include <meteo/commons/msgstream/streammodule.h>

namespace meteo {
namespace rpc {

class Server;
class Channel;

} // rpc
} // meteo

namespace meteo {

class PtkppStreamService;

struct ConnectionItem
{
  msgstream::SubscribeData subscribe;
  rpc::Channel* channel = nullptr;
  tlg::MessageNew* message = nullptr;
  QDateTime dt;
};

class PtkppStreamRecv : public StreamModule
{
  Q_OBJECT

  enum StatusParam {
    kRecvCount    = 1024,
  };

public:
  static QString moduleId() { return QString("ptkpp-recv"); }

public:
  PtkppStreamRecv(QObject* p = 0);
  virtual ~PtkppStreamRecv();

  virtual void setOptions(const msgstream::Options &options);

  QList<msgstream::SubscribeData> subscribeList() const;
  msgstream::SubscribeData startSubscribe(const msgstream::SubscribeData& subscribe);
  msgstream::SubscribeData stopSubscribe(const msgstream::SubscribeData& subscribe);
  msgstream::SubscribeData removeSubscribe(const msgstream::SubscribeData& subscribe);

public slots:
  virtual bool slotInit();

  void slotConnect();
  void slotDisconnected();

private:
  void callbackMessage(tlg::MessageNew* message);

  bool parsePeerInfo(const QString& text, QString* name, QString* address) const;

private:
  // параметры
  msgstream::Options opt_;

  // данные
  int recvCount_ = 0;

  // служебные
  bool runConnect_ = false;
  PtkppStreamService* service_ = nullptr;
  rpc::Server* server_ = nullptr;
  QMap<tlg::MessageNew*,ConnectionItem> connections_;
};


} // meteo


#endif // METEO_COMMONS_MSGSTREAM_PLUGINS_PTKPP_PTKPPSTREAMRECV_H
