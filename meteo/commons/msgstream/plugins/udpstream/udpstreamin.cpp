#include "udpstreamin.h"

#include <cmath>
#include <climits>

#include <qelapsedtimer.h>
#include <qfilesystemwatcher.h>

#include <cross-commons/debug/tlog.h>

#include <cross-commons/debug/tmap.h>
#include <commons/textproto/pbtools.h>
#include <commons/textproto/tprototext.h>

#include <meteo/commons/msgparser/common.h>
#include <meteo/commons/msgparser/msgmetainfo.h>
#include <meteo/commons/msgparser/tlgparser.h>
#include <commons/compresslib/mcompress.h>
#include <boost/crc.hpp>
#include <QDataStream>
namespace meteo {

UdpStreamIn::UdpStreamIn(QObject* p)
  : StreamModule(p)
{
  parser_ = nullptr;
  status_ = nullptr;
}

UdpStreamIn::~UdpStreamIn()
{
  delete parser_;
  parser_ = nullptr;
  delete status_;
  status_ = nullptr;
  delete reader_;
  reader_ = nullptr;
}

void UdpStreamIn::setOptions(const msgstream::Options& options)
{
  opt_ = options;
  id_ = pbtools::toQString(opt_.id());

  if(options.udp().has_host()){
    host_ = pbtools::toQString(options.udp().host());
  }else {
    error_log << tr("В настройках не указан адрес приема данных. Будет использован адрес ")<< host_;
  }
  if(options.udp().has_port()){
    port_ = options.udp().port();
  }else {
    error_log << tr("В настройках не указан порт приема данных. Будет использован порт ")<< port_;
  }
  if ( options.udp().has_meta_template() ) {
    meta_template_ = pbtools::toQString(options.udp().meta_template());
  }
  if ( options.udp().has_compress_mode() ) {
    compressionMode_ = options.udp().compress_mode();
  }
  if ( options.udp().has_chunk_size() ) {
    chunk_size_ = options.udp().chunk_size();
  }
  need_fec_ = options.udp().need_fec();
  if(need_fec_){
    chunk_size_ = RS::PAYLOAD;
  }
  expire_time_= options.udp().expire_time();
  state_interval_= options.udp().state_interval();
  read_interval_ = options.udp().read_interval();
}

bool UdpStreamIn::slotInit()
{
  if(nullptr != status_){
    status_->setTitle(kRecvCount, tr("Принято сообщений"));
    status_->setTitle(kRecvSize, tr("Принято данных"));
    status_->setTitle(kRecvCountToday, tr("Принято сообщений (сегодня)"));
    status_->setTitle(kRecvSizeToday, tr("Принято данных (сегодня)"));
    status_->setTitle(kLinkStatus, tr("Состояние подключения"));
    status_->setTitle(kRecvChunkStreamCount, tr("Датаграмм в очереди обработки"));
    status_->setTitle(kRecvChunkCount, tr("Принято датаграмм"));
    status_->setTitle(kRecvChunkUnkCount, tr("Необработано датаграмм"));
    status_->setTitle(kRecvFileCount, tr("Принято файлов"));
    status_->setTitle(kCorrectedCount, tr("Восстановлено файлов"));
    status_->setTitle(kNotCorrectedCount, tr("Не удалось восстановить файлов"));

    status_->setParam(kLinkStatus, tr("Отсутствует. Необходимо перезапустить поток."),app::OperationState_ERROR);
    status_->setParam(kRecvCount, 0);
    status_->setParam(kRecvCountToday, 0);
    status_->setParam(kRecvSize, 0);
    status_->setParam(kRecvSizeToday, 0);
    status_->setParam(kRecvChunkStreamCount, 0);
    status_->setParam(kRecvChunkCount, 0);
    status_->setParam(kRecvChunkUnkCount, 0);
    status_->setParam(kRecvFileCount, 0);

  }
  lastCheck_ = QDateTime::currentDateTime();
  QObject::startTimer(opt_.time_to_last_activity());
  if ( 0 == incoming_ ) { return false; }
  parser_ = new TlgParser(QByteArray());
  center_window_ = opt_.center_window();

  reader_ = new UdpReader();
  reader_->setParams(host_,port_, chunk_size_);
  QThread* thread = new QThread;
  reader_->moveToThread(thread);
  QObject::connect( thread, &QThread::started, reader_, &meteo::UdpReader::slotInit );
  QObject::connect( reader_, &meteo::UdpReader::newData, this, &meteo::UdpStreamIn::slotNewData );
  QObject::connect( reader_, &meteo::UdpReader::errorSignal, this, &meteo::UdpStreamIn::slotError );
  QObject::connect( reader_, &meteo::UdpReader::stateSignal, this, &meteo::UdpStreamIn::slotStateChange );

  thread->start();
  QTimer::singleShot(state_interval_*1000,this,SLOT(slotCheckTimeout()));
  QTimer::singleShot(read_interval_,this,SLOT(slotReceiveTimeout()));
  return false;
}

void UdpStreamIn::slotCheckTimeout()
{

  if(nullptr != reader_){

    QString state;
    if(QAbstractSocket::BoundState != reader_->socketState()){
      reader_->reconnect();
    }
  }
  QTimer::singleShot(state_interval_*1000,this,SLOT(slotCheckTimeout()));
}

void UdpStreamIn::slotReceiveTimeout()
{
  processPendingDatagrams();
  QTimer::singleShot(read_interval_, this,SLOT(slotReceiveTimeout()));
}

void UdpStreamIn::slotError(QString err)
{
  status_->setParam(kLinkStatus, err+tr(" Необходимо перезапустить поток."),app::OperationState_ERROR);
  error_log << "Ошибка подключения к сокету" << err;
}

void UdpStreamIn::slotStateChange(QString state)
{
  //info_log << "Состояние сокета:" << state;
  if(reader_->isBind()){
    if(nullptr != status_) status_->setParam(kLinkStatus, tr("Установлено"),app::OperationState_NORM);
  }else{
    status_->setParam(kLinkStatus, state+tr(" Необходимо перезапустить поток."),app::OperationState_ERROR);
    error_log<< tr("Ошибка подключения к сокету. Возможно прием уже запущен.");
    exit(0);
  }
}

void UdpStreamIn::slotNewData()
{
  processPendingDatagrams();
  //QTimer::singleShot(read_interval_,this,SLOT(slotNewData()));
}

void UdpStreamIn::processPendingDatagrams()
{
  if(nullptr == reader_) return;
  QByteArray ba;
  while(reader_->takeFirst(&ba)){
    int rs = ba.size();
    if(rs<1) continue;
    recvSize_ += rs;
    recvSizeToday_ += rs;
    status_->setParam(kRecvChunkCount, ++recvProcessedChunkCount_);
    status_->setParam(kRecvChunkStreamCount, reader_->packetsCount());
    status_->setParam(kRecvSize, AppStatusThread::sizeStr(recvSize_));
    status_->setParam(kRecvSizeToday, AppStatusThread::sizeStr(recvSizeToday_));
    process(ba);
  }
}

void UdpStreamIn::process(const QByteArray& datagram){

  UdpPacket packet;
  switch (compressionMode_) {
    case msgstream::COMPRESS_OPTIM:
    case msgstream::COMPRESS_ALWAYS:{
      compress::Compress compressor;
      if(!packet.parse(compressor.decompress(datagram ))){
        error_log << tr("Ошибка разбора пакета");
        return;
      }
    }
    break;
    default:
      if(!packet.parse(datagram)){
        debug_log << tr("Ошибка разбора пакета");
        return;
      }
    break;
  }
  QString dataname = packet.getDataName();
  QString sendername = packet.getSenderName();
  uint32_t magic = packet.getMagic();

  QMap <uint32_t, UdpPacketMap > &cur_data_pakets = packets_[sendername];
  UdpPacketMap &cur_pakets = cur_data_pakets[magic];
  cur_pakets.insertOne(packet.getChunkShift(),packet);
  if(!cur_pakets.isFull()){
    status_->setParam(kRecvChunkUnkCount, countPackets());
    return;
  }
  if(0 == received_files_.count(magic)){
    received_files_.insert(magic);
    processPackets(&cur_pakets, dataname, false);
  } else {
    debug_log << "файл уже принят ранее!"<<dataname;
  }
  status_->setParam(kRecvFileCount,++fileRecvCount_);
  packets_[sendername].remove(magic);
  if(0 == packets_[sendername].size()){
    packets_.remove(sendername);
  }
  status_->setParam(kRecvChunkUnkCount, countPackets());
}

bool UdpStreamIn::processPackets(UdpPacketMap* packets, const QString& dataname, bool has_errors)
{
  MsgMetaInfo meta;
  if( !meta_template_.isEmpty() ){
    meta = MsgMetaInfo(meta_template_, dataname);
  }

  if(true == need_fec_){
      int fail =0,corrected=0;
      parser_->appendData( packets->getDecodedPayload(&corrected,&fail,has_errors));
      if(fail != 0 || corrected !=0){
        recvCorrected_   +=corrected;
        recvNotCorrected_ += fail;
        status_->setParam(kCorrectedCount, recvCorrected_);
        status_->setParam(kNotCorrectedCount, recvNotCorrected_);
      }
    }
  else {
    parser_->appendData( packets->getPayload());
  }

  tlg::MessageNew msg;
  while ( parser_->parseNextMessage(&msg) ) {
    if ( !msg.header().has_t1() && !meta.t1.isEmpty() ) { msg.mutable_header()->set_t1(meta.t1.toUtf8().constData()); }
    if ( !msg.header().has_t2() && !meta.t2.isEmpty() ) { msg.mutable_header()->set_t2(meta.t2.toUtf8().constData()); }
    if ( !msg.header().has_a1() && !meta.a1.isEmpty() ) { msg.mutable_header()->set_a1(meta.a1.toUtf8().constData()); }
    if ( !msg.header().has_a2() && !meta.a2.isEmpty() ) { msg.mutable_header()->set_a2(meta.a2.toUtf8().constData()); }
    if ( !msg.header().has_ii() && !meta.ii.isEmpty() ) { msg.mutable_header()->set_ii(meta.ii.toInt()); }
    if ( !msg.header().has_cccc() && !meta.cccc.isEmpty() ) { msg.mutable_header()->set_cccc(meta.cccc.toUtf8().constData()); }
    if ( !msg.header().has_yygggg() && !meta.yygggg.isEmpty() ) { msg.mutable_header()->set_yygggg(meta.yygggg.toUtf8().constData()); }
    msg.mutable_metainfo()->set_from(id_.toUtf8().constData());
    meta.update(msg);
    QDateTime dt = meta.calcConvertedDt();
    if ( dt.isValid() ) {
      msg.mutable_metainfo()->set_converted_dt(pbtools::toString(dt.toString(Qt::ISODate)));
    }
    incoming_->append(msg);
    status_->setParam(kRecvCount, ++recvCount_);
    status_->setParam(kRecvCountToday, ++recvCountToday_);
  }
  status_->setParam(kRecvChunkUnkCount, countPackets());

  // status_->setParam(kOperation, tr("Обработка завершена"));
  return true;
}

void UdpStreamIn::clearExpired()
{
  //debug_log << tr("clearExpired неизвестных пакетов ");
  QMap< QString, QMap <uint32_t, UdpPacketMap > >::iterator it1 = packets_.begin();
  QMap< QString, QMap <uint32_t, UdpPacketMap > >::iterator eit1 = packets_.end();
  for(;it1 != eit1; ++it1){
    QMap <uint32_t, UdpPacketMap >::iterator it2 = it1->begin();
    QMap <uint32_t, UdpPacketMap >::iterator eit2 = it1->end();
    while(it2!=eit2){
      //debug_log<<expire_time_ << it2->dt().secsTo(QDateTime::currentDateTime());
      if(expire_time_ < it2->dt().secsTo(QDateTime::currentDateTime())){
        processPackets(&(*it2),it2->getDataName(),true);
        it2 = it1.value().erase(it2);
      } else {
        ++it2;
      }
    }
  }
}

int UdpStreamIn::countPackets()
{
  QMap< QString, QMap <uint32_t, UdpPacketMap > >::iterator it1 = packets_.begin();
  QMap< QString, QMap <uint32_t, UdpPacketMap > >::iterator eit1 = packets_.end();
  int kol_p = 0;
  for(;it1 != eit1; ++it1){
    QMap <uint32_t, UdpPacketMap >::iterator it2 = it1->begin();
    QMap <uint32_t, UdpPacketMap >::iterator eit2 = it1->end();
    while(it2!=eit2){
      kol_p+=it2->size();
      it2++;
    }
  }
  return kol_p;
  //debug_log << tr("удалено неизвестных пакетов ")<<kol_removed;
}


void UdpStreamIn::timerEvent(QTimerEvent* event)
{
  Q_UNUSED( event );
  QDateTime dt = QDateTime::currentDateTime();
  if ( lastCheck_.date() != dt.date() ) {
    recvSizeToday_ = 0;
    recvCountToday_ = 0;
    received_files_.clear();
  }
  lastCheck_ = dt;
  clearExpired();
}

} // meteo
