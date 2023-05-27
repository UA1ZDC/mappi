#ifndef METEO_COMMONS_MSGDATA_TYPES_H
#define METEO_COMMONS_MSGDATA_TYPES_H

#include <meteo/commons/global/requestcard.h>
#include <meteo/commons/proto/msgcenter.pb.h>

namespace meteo {

//! Данный тип содержит параметры запроса DistributeMsg.
typedef RequestCard<const msgcenter::DistributeMsgRequest, tlg::MessageNew> DistributeMsgCard;
//! Данный тип содержит параметры запроса GetTelegram.
typedef RequestCard<const msgcenter::GetTelegramRequest, msgcenter::GetTelegramResponse> GetTelegramCard;

//! Параметры отбора сообщений для отправки подписчикам
class SubscribeFilter
{
public:
  bool checkExternal = false;
  bool external;
  QList<QRegExp> ahd;
  QList<std::string> type;
  QList<tlg::MsgType> msgType;
  QList<tlg::Priority> priority;
  QList<tlg::AddressMode> mode;
  QList<tlg::Format> format;
};

enum AppStatusParamId {
  kNotifier  =    0,
  kUpdater   = 1000,
  kGetter    = 2000,
  kCreator   = 3000,
  // kMsgReceiver,
  kRecvDbStatus = 10000,
  kRecvFsStatus,
  kRecvDuplicate,
  kWriteToDb,
  kRecvCounter  = 10100,
  kCenterSend   = 11000,
  kDbSkippedSaveCount = 12000
};

struct TimeCountMetric
{
  TimeCountMetric(){}
  TimeCountMetric(int t, int c) : time(t), count(c) {}

  int time  = 0;
  int count = 0;
};

} // meteo

#endif // METEO_COMMONS_MSGDATA_TYPES_H
