#ifndef CONTROLPARAMSWATCHER_H
#define CONTROLPARAMSWATCHER_H

#include <qhash.h>
#include <qdatetime.h>
#include <qstring.h>
#include <memory>
#include <qtimer.h>
#include <qmutex.h>
#include <cross-commons/singleton/tsingleton.h>
#include <meteo/commons/proto/appconf.pb.h>

namespace meteo {
namespace internal {

class ParamsHandler: public QObject
{
  Q_OBJECT
public:
  ParamsHandler();

  //Параметры со счетчиком
  enum IntegerParam {
    kTlgReceived = 0, //Количество телеграмм, принятых по каналу подписки
    kTlgReloaded = 1, //Количество телеграмм, загруженных с использованием RPC
    kTlgDecoded = 2, //Количество телеграмм, отмеченных как раскодированные
    kDocumentsBuilded = 3, //Количество собранных документов
    kTlgDroppedByTimeout = 4, //Количество телеграмм, удаленных по истечении времени загрузки
    kTlgPraseError = 5, //Количество телеграмм, при разборе которых произошли ошибки
    kSegmentDupCount = 6 //Количество дубликатов сегментов
  };

  enum BooleanParams {
    kRpcChannelAlive,
    kSubcribeChannelAlive
  };

  void incParam(const QObject* source, IntegerParam param, int value = 1);
  void setBooleanParam(const QObject* source, BooleanParams param, bool value);

private:
  class StatsHandler {
  public:
    QHash<IntegerParam, int> intergerParams_;
    QHash<BooleanParams, bool> booleanParams_;
  };

private slots:
  void sendStatsToAppmanager();

private:
  static const int timeout = 5000; //5s
  StatsHandler* tryGetHandler(const QObject* source);
  void fillStateProto(meteo::app::OperationStatus* state);

  QHash<QString, StatsHandler*> handlers_;

  QMutex mutex_;
  bool updated_ = false;
  QTimer notifyTimer_;
};

}

typedef  TSingleton<internal::ParamsHandler> ParamsHandler;


}
#endif
