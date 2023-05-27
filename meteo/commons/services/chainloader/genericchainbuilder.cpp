#include "genericchainbuilder.h"

#include "sateliteloaderparamswatcher.h"

#include <commons/textproto/pbtools.h>
#include <sql/dbi/gridfs.h>
#include <meteo/commons/global/dbnames.h>
#include <meteo/commons/msgparser/tlgparser.h>
#include <sql/nosql/nosqlquery.h>

namespace meteo {
QString GenericChainBuilder::msgCenterTitle;


GenericChainBuilder::GenericChainBuilder()
{
}

void GenericChainBuilder::init()
{
  if ( true == msgCenterTitle.isEmpty() ){
    msgCenterTitle = meteo::global::serviceTitle(meteo::settings::proto::kMsgCenter);
  }

  reconnectTimer_.setInterval(timeoutMs);

  QObject::connect(&reconnectTimer_, &QTimer::timeout,
                   this, &GenericChainBuilder::connect);
  QObject::connect(this, &GenericChainBuilder::onAllChunksReceived,
                   this, &GenericChainBuilder::slotOnAllChunksReceived);
  reconnectTimer_.start();

  chunksStorageTimer_.setInterval(chunkStatsStorageTimeout );
  QObject::connect(&chunksStorageTimer_, &QTimer::timeout,
                   this, &GenericChainBuilder::onCleanupTimeout);
  chunksStorageTimer_.start();

  undecodedRcvTimer_.setInterval(undecodedRcvTimeout);
  //undecodedRcvTimer_.setInterval(1000);
  QObject::connect(&undecodedRcvTimer_, &QTimer::timeout, this, &GenericChainBuilder::slotOnUndecodedTryReceive);
  undecodedRcvTimer_.start();
}

GenericChainBuilder::~GenericChainBuilder()
{
  if ( nullptr != subscribeChannel_ ){
    delete subscribeChannel_;
    subscribeChannel_ = nullptr;
  }

  if ( nullptr != remoteCallChannel_ ){
    delete remoteCallChannel_;
    remoteCallChannel_ = nullptr;
  }

  while ( false == imageChunkStats_.isEmpty() ){
    int id = imageChunkStats_.keys().first();
    ChunkStats* chunk = imageChunkStats_.value(id, nullptr);
    imageChunkStats_.remove(id);
    if ( nullptr == chunk ){
      continue;
    }
    delete chunk;
  }
}

void GenericChainBuilder::connect()
{
  if ( nullptr != subscribeChannel_ ) {
    error_log << QObject::tr("Ошибка: подключение уже произведено, но почему-то connect вызван повторно");
    return;
  }

  meteo::msgcenter::DistributeMsgRequest req;
  for ( const QString& msgtype: subscribeMsgTypes() ) {
    req.set_uid(loaderUid().toStdString());
    req.set_received_only(true);
    auto rule = req.add_rule();
    rule->set_type(meteo::msgcenter::RuleType::kAllow);
    rule->set_to("uid");
    rule->mutable_tlgfilter()->set_msgtype(msgtype.toStdString());
    rule->set_priority(0);
  }

  {
    auto rule = req.add_rule();
    rule->set_type(meteo::msgcenter::RuleType::kDeny);
    rule->set_to("uid");
    rule->set_from("all");
    rule->set_priority(1);
  }

  this->subscribeChannel_ = meteo::global::serviceChannel(meteo::settings::proto::kMsgCenter);
  if ( nullptr == this->subscribeChannel_ ){
    error_log << meteo::msglog::kNoConnect.arg(msgCenterTitle);
    return;
  }

  QObject::connect( this->subscribeChannel_, &meteo::rpc::Channel::disconnected, this, &GenericChainBuilder::slotSubscribeChannelDisconnected);
  bool result = this->subscribeChannel_->subscribe(&meteo::msgcenter::MsgCenterService::DistributeMsg,
                                                   req,
                                                   this,
                                                   &GenericChainBuilder::onSubscribeMessageReceived);

  if ( true == result ) {
    info_log << QObject::tr("Подписка к %1 осуществлена успешно")
                .arg(meteo::global::serviceTitle(meteo::settings::proto::kMsgCenter));
  }
  else {
    error_log << QObject::tr("Не удалось произвести подписку к сервису %1")
                 .arg(meteo::global::serviceTitle(meteo::settings::proto::kMsgCenter));;
    delete subscribeChannel_;
    subscribeChannel_ = nullptr;
  }

  ParamsHandler::instance()->setBooleanParam(this,
                                             internal::ParamsHandler::BooleanParams::kSubcribeChannelAlive,
                                             true );

  this->reconnectTimer_.stop();
  QTimer::singleShot(0, this, &GenericChainBuilder::slotOnUndecodedTryReceive );
}

void GenericChainBuilder::slotSubscribeChannelDisconnected()
{
  delete subscribeChannel_;
  subscribeChannel_ = nullptr;
  ParamsHandler::instance()->setBooleanParam(this,
                                             internal::ParamsHandler::BooleanParams::kSubcribeChannelAlive,
                                             false );
  reconnectTimer_.start();
}

void GenericChainBuilder::onSubscribeMessageReceived(meteo::tlg::MessageNew* tlg)
{
  ParamsHandler::instance()->incParam(this,
                                      internal::ParamsHandler::IntegerParam::kTlgReceived);
  this->processTlg(*tlg);
}

void GenericChainBuilder::slotOnAllChunksReceived(int imageid)
{
  ChunkStats* stats = imageChunkStats_.value(imageid);
  if ( nullptr == stats ){
    error_log << QObject::tr("Ошибка: пришел сигнал, что все фрагменты изображения получены, но статистика по фрагментам отсутствует");
    return;
  }
  imageChunkStats_.remove(imageid);
  tryBuildImageByStats(stats);
  delete stats;
}

void GenericChainBuilder::tryBuildImageByStats(const ChunkStats* stats)
{

  meteo::Chain dataChain(stats->knownChunks_.values());

  QByteArray resultImage = dataChain.tryExtract();
  if ( true == resultImage.isEmpty() ){
    error_log << QObject::tr("Ошибка при извлечении данных о изображении %1. Изображение не собрано")
                 .arg(stats->imageid_);
    return;
  }

  if ( false == saveChain(*stats, resultImage) ){
    error_log << QObject::tr("Ошибка при сохранении извлеченных данных.");
    return;
  }

  ParamsHandler::instance()->incParam(this,
                                      internal::ParamsHandler::IntegerParam::kDocumentsBuilded);

  if ( false == markDecoded(stats->originalTlgIds_) ) {
    warning_log << QObject::tr("Телеграммы были успешно собраны в целевой файл, но не были отмечены как декодированные. Возможно они будут собраны повторно.");
  }

  ParamsHandler::instance()->incParam(this,
                                      internal::ParamsHandler::IntegerParam::kTlgDecoded,
                                      stats->originalTlgIds_.size());
}

void GenericChainBuilder::onCleanupTimeout()
{  
  QList<int> imagesChunksToDelete;
  for ( int imageId : imageChunkStats_.keys() )  {
    auto stats = imageChunkStats_.value(imageId, nullptr);
    if ( nullptr == stats ){
      imageChunkStats_.remove(imageId);
    }
    auto dtToDelete = QDateTime::currentDateTimeUtc().addMSecs(-chunkStatsStorageTimeout);
    if ( stats->lastReceivedChunkDt_ > dtToDelete  ){
      continue;
    }
    warning_log << QObject::tr("%1 %2 слишком давно получило последний фрагмент, но все еще не собрано. Удаление из очереди сборки.")
                   .arg(buildObjectName())
                   .arg(imageId);
    imagesChunksToDelete << imageId;
  }


  for ( auto key: imagesChunksToDelete ){
    ChunkStats* stats = imageChunkStats_[key];
    debug_log << QObject::tr("Статистика по полученным фрагментам %1").arg(stats->imageid_);
    for (auto segment: stats->knownChunks_) {
      debug_log << QObject::tr("Фрагмент %2. Тип: %3")
                   .arg(segment.formatString().nnnn())
                   .arg(segment.formatString().code());
    }
    debug_log << QObject::tr("Получено фрагментов: %1. Ожидалось получить: %2.")
                 .arg(stats->knownChunks_.size())
                 .arg(stats->totalChunks_);
    debug_log << stats->header_.Utf8DebugString();

    imageChunkStats_.remove(key);
    if ( nullptr == stats ){
      continue;
    }
    delete stats;
  }

  ParamsHandler::instance()->incParam(this,
                                      internal::ParamsHandler::IntegerParam::kTlgDroppedByTimeout,
                                      imagesChunksToDelete.size());
}

bool GenericChainBuilder::processTlg(const meteo::tlg::MessageNew& tlg)
{
  if ( false == tlg.metainfo().has_id() ){
    error_log << QObject::tr("Телеграмма не имеет уникального номера в БД. Обработка телеграммы прервана.");
    return false;
  }

  const QDateTime& now = QDateTime::currentDateTimeUtc();
  const QByteArray& data = pbtools::fromBytes(tlg.msg());
  const meteo::ChainSegment segment(data);
  //const meteo::faxes::FaxSegment& segment = meteo::faxes::Extractor::extractSegment(data);
  if ( false == segment.isValid() ) {
    ParamsHandler::instance()->incParam(this,
                                        internal::ParamsHandler::IntegerParam::kTlgPraseError);
    return false;
  }

  ChunkStats* stats = imageChunkStats_.value(segment.formatString().magic(), nullptr);
  if ( nullptr == stats ) {
    stats = new ChunkStats();
    imageChunkStats_.insert(segment.formatString().magic(), stats);
    stats->imageid_ = segment.formatString().nnnn();
    stats->header_.CopyFrom(tlg.header());
    stats->totalChunks_ = -1;
    if ( true == tlg.metainfo().has_converted_dt() ){
      stats->convertedDt_ = NosqlQuery::datetimeFromString(tlg.metainfo().converted_dt());
    }
    else {
      stats->convertedDt_ = now;
    }
  }

  stats->lastReceivedChunkDt_ = now;
  if ( true == stats->knownChunks_.contains(segment.formatString().nnnn()) ){
    debug_log << QObject::tr("Факс %1, сегмент %2 - принят повторно")
                   .arg(segment.formatString().magic())
                   .arg(segment.formatString().nnnn());
    markDecoded( QList<qint64>()<<tlg.metainfo().id() );
    ParamsHandler::instance()->incParam(this,
                                        internal::ParamsHandler::IntegerParam::kSegmentDupCount);
  }

  stats->knownChunks_.insert(segment.formatString().nnnn(), segment);

  if ( meteo::FormatString::kEnd == segment.formatString().segType() ) {
    stats->totalChunks_ = segment.formatString().nnnn();
  }

  stats->originalTlgIds_ << tlg.metainfo().id();

  if ( stats->knownChunks_.size() == stats->totalChunks_ ) {
    emit onAllChunksReceived(segment.formatString().magic());
  }

  return true;
}

void GenericChainBuilder::slotOnRemoteCallChannelDisconnected()
{
  delete remoteCallChannel_;
  remoteCallChannel_ = nullptr;

  ParamsHandler::instance()->setBooleanParam(this,
                                             internal::ParamsHandler::kRpcChannelAlive,
                                             false);
}


bool GenericChainBuilder::markDecoded(const QList<qint64> ids){

  if ( false == tryRemoteChannelReconnect() ){
    return false;
  }

  meteo::msgcenter::DecodeRequest request;
  for ( qint64 tlgid : ids ) {
    request.add_id(tlgid);
  }
  meteo::msgcenter::Dummy* responce = remoteCallChannel_->remoteCall(&meteo::msgcenter::MsgCenterService::SetDecode, request, 10000);
  if ( nullptr == responce ){
    error_log << meteo::msglog::kServiceAnsverFailed.arg(msgCenterTitle);
    return false;
  }
  delete responce;
  return true;
}

bool GenericChainBuilder::tryRemoteChannelReconnect()
{
  if ( nullptr == remoteCallChannel_ ){
    remoteCallChannel_ = meteo::global::serviceChannel(meteo::settings::proto::kMsgCenter);
    if ( nullptr == this->subscribeChannel_ ){
      error_log << meteo::msglog::kNoConnect.arg(msgCenterTitle);
      return false;
    }
    QObject::connect(remoteCallChannel_, &meteo::rpc::Channel::disconnected, this, &GenericChainBuilder::slotOnRemoteCallChannelDisconnected);
  }
  if ( nullptr == remoteCallChannel_ ){
    return false;
  }
  ParamsHandler::instance()->setBooleanParam(this,
                                             internal::ParamsHandler::kRpcChannelAlive,
                                             true);
  return true;
}

void GenericChainBuilder::slotOnUndecodedTryReceive()
{
  if ( false == tryRemoteChannelReconnect() ){
    return;
  }

  QDateTime currentDt = QDateTime::currentDateTimeUtc();
  QDateTime minDt = currentDt.addMSecs(-undecodedMaxTime);
  QDateTime maxDt = currentDt.addMSecs(-undecodedMinTime);

  meteo::msgcenter::GetUndecodedFaxTelegramRequest request;

  for ( const QString& msgtype: subscribeMsgTypes() ) {
    request.add_msgtype(msgtype.toStdString());
  }
  //QElapsedTimer timer; timer.start();

 // request.set_beg_dt(minDt.toString(Qt::ISODate).toStdString());
 // request.set_end_dt(maxDt.toString(Qt::ISODate).toStdString());
  request.set_limit(10);
  auto responce = std::unique_ptr<meteo::msgcenter::GetTelegramResponse>( remoteCallChannel_->remoteCall(&meteo::msgcenter::MsgCenterService::GetUndecodedFaxTelegram, request, 10000) );
  if ( nullptr == responce ){
    error_log << meteo::msglog::kServiceAnsverFailed.arg(msgCenterTitle);
    return;
  }
 // debug_log << tr("GetUndecodedFaxTelegram за %1 мсек.").arg(timer.restart());

  QByteArray arr;
  meteo::TlgParser parser(arr);
  for (const  ::meteo::tlg::MessageNew &msg : responce->msg() ){
    tlg::MessageNew mes;
    QByteArray raw;
    //debug_log<< msg.Utf8DebugString();
    raw = QByteArray::fromStdString(msg.msg());
    parser.setData(raw);
    if ( false == parser.parseNextMessage(&mes) ) {
      error_log << QObject::tr("Не удалось обработать телеграмму с идентификатором %1.");
    }
    //debug_log << tr("parseNextMessage за %1 мсек.").arg(timer.restart());

    mes.mutable_metainfo()->CopyFrom(msg.metainfo());
    if ( false == processTlg(mes) ){
      error_log << QObject::tr("Ошибка при разборе телеграммы:")
                << msg.header().Utf8DebugString()
                << msg.metainfo().Utf8DebugString();
      markDecoded( QList<qint64>()<<msg.metainfo().id() ); //TODO надо бы помечать как ошибка. Но нет поля в таблице...
    }

   // debug_log << tr("processTlg за %1 мсек.").arg(timer.restart());

  }

  ParamsHandler::instance()->incParam(this,
                                      internal::ParamsHandler::IntegerParam::kTlgReloaded,
                                      responce->msg_size());
}


}
