#ifndef GENERICCHAINBUILDER_H
#define GENERICCHAINBUILDER_H


#include <qobject.h>
#include <qtimer.h>
#include <qobject.h>
#include <meteo/commons/global/global.h>
#include <meteo/commons/rpc/rpc.h>
#include <meteo/commons/proto/msgcenter.pb.h>
#include <meteo/commons/faxes/datachain.h>

namespace meteo {

class GenericChainBuilder : public QObject
{
  Q_OBJECT
public:
  GenericChainBuilder();
  ~GenericChainBuilder();

  void init();

protected:
  class ChunkStats{
  public:
    meteo::tlg::Header header_;
    qint32 totalChunks_;
    QDateTime lastReceivedChunkDt_;
    QHash<int, meteo::ChainSegment> knownChunks_;
    QList<qint64> originalTlgIds_;
    QDateTime convertedDt_;
    int imageid_;
  };

  virtual QStringList subscribeMsgTypes() = 0;
  virtual bool saveChain(const ChunkStats& chain, const QByteArray& data) = 0;
  virtual QString loaderUid() = 0;
  virtual QString buildObjectName() = 0;

signals:
  void onAllChunksReceived(int imageid);
private slots:
  void onCleanupTimeout();
  void slotSubscribeChannelDisconnected();
  void connect();
  void onSubscribeMessageReceived(meteo::tlg::MessageNew* tlg);
  void slotOnAllChunksReceived(int imageid);
  void slotOnRemoteCallChannelDisconnected();
  void slotOnUndecodedTryReceive();

private:

  bool processTlg(const meteo::tlg::MessageNew& tlg);

  void tryBuildImageByStats(const ChunkStats* stats);
  bool markDecoded(const QList<qint64> ids);
  bool tryRemoteChannelReconnect();


private:
  static const int timeoutMs = 10*1000;
  static const int chunkStatsStorageTimeout = 100*1000; //100s
  static const int undecodedRcvTimeout = 200*1000; //100s
  static QString msgCenterTitle;

  static const int undecodedMaxTime = 24*3600*1000; // Срок(мсек), по истечении которого попытки сбоки документа больше предприниматься не будут
  static const int undecodedMinTime = 2*chunkStatsStorageTimeout; //Срок(мсек), который должен пройти прежде, чем есть смысл запрашивать телеграммы из БД повторно
  QTimer reconnectTimer_;
  QTimer chunksStorageTimer_;
  QTimer undecodedRcvTimer_;
  bool running_ = false;

  meteo::rpc::Channel* subscribeChannel_ = nullptr;
  meteo::rpc::Channel* remoteCallChannel_ = nullptr;

  QHash<int, ChunkStats*> imageChunkStats_;
};

}

#endif
