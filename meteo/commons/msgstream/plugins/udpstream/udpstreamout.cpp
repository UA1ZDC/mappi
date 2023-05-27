#include "udpstreamout.h"
#include "udp_packet.h"

#include <cross-commons/debug/tlog.h>

#include <commons/textproto/pbtools.h>
#include <commons/textproto/tprototext.h>

#include <meteo/commons/global/log.h>
#include <meteo/commons/grib/decoder/tgribdecode.h>
#include <meteo/commons/msgparser/msgmetainfo.h>
#include <meteo/commons/msgparser/common.h>
#include <meteo/commons/proto/tgribformat.pb.h>
#include <commons/compresslib/mcompress.h>
#include <boost/crc.hpp>
#include <meteo/commons/msgparser/tlgparser.h>
#include <QDataStream>

namespace meteo {

UdpStreamOut::UdpStreamOut(QObject* p)
  : StreamModule(p)
{
  maxTime_ = 0;
  contentOnly_ = false;
  fileNumb_ = 0;
  status_ = 0;
  udpSocket_ = new QUdpSocket(this);
}

UdpStreamOut::~UdpStreamOut()
{
  delete status_;
  status_ = 0;
  udpSocket_->close();
  delete udpSocket_;
  udpSocket_ = nullptr;
}

void UdpStreamOut::setOptions(const msgstream::Options& options)
{
  if(options.udp().has_msg_limit()){
      msg_limit_ = options.udp().msg_limit();
    }
  if(options.udp().has_size_limit()){
      size_limit_ = options.udp().size_limit();
    }
  if(options.udp().has_time_limit()){
      maxTime_ = options.udp().time_limit();
    }
  if(options.udp().has_content_only()){
      contentOnly_ = options.udp().content_only();
    }
  if(options.has_id()){
      id_ = pbtools::toQString(options.id());
    }
  if(options.udp().has_meta_template()){
      meta_template_ = pbtools::toQString(options.udp().meta_template());
    }
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
  if(options.udp().has_chunk_size()){
      chunk_size_ = options.udp().chunk_size();
    }
  if ( options.udp().has_compress_mode() ) {
      compressionMode_ = options.udp().compress_mode();
    }
  read_interval_ = options.udp().read_interval();
  sendTryCount_ = options.udp().send_try();
  need_fec_ = options.udp().need_fec();
  if(need_fec_){
    chunk_size_ = RS::PAYLOAD;
   // size_limit_ = 65535;
  }
  /*
  if ( options.udp().has_osh_host() ) {
    osh_host_ = pbtools::toQString(options.udp().osh_host());
  }else {
    error_log << tr("В настройках не указан адрес шлюза. Будет использован адрес ")<< osh_host_;
  }
*/

  packets_.setMagic(0);
  packets_.setSender(host_+":"+QString::number(port_));
  packets_.setChunkSize(chunk_size_);
  parser_ = new TlgParser(QByteArray());

}

bool UdpStreamOut::slotInit()
{
  status_->setTitle(kSendCount, tr("Передано сообщений"));
  status_->setTitle(kSendSize, tr("Передано данных"));
  status_->setTitle(kSendCountToday, tr("Передано сообщений (сегодня)"));
  status_->setTitle(kSendSizeToday, tr("Передано данных (сегодня)"));
  status_->setTitle(kSendChunkCount, tr("Передано датаграмм"));
  status_->setTitle(kSendFileCount, tr("Передано файлов"));

  status_->setParam(kSendCount, 0);
  status_->setParam(kSendCountToday, 0);
  status_->setParam(kSendSize, 0);
  status_->setParam(kSendSizeToday, 0);
  status_->setParam(kSendChunkCount, 0);
  status_->setParam(kSendFileCount, 0);
  lastCheck_ = QDateTime::currentDateTime();
  startTimer(kTimeToLastActivity);
  if ( 0 == outgoing_ ) { return false; }
  return true;
}

void UdpStreamOut::slotNewIncoming() {
  slotProcess();
}


qint64 UdpStreamOut::write(const QByteArray &datagram){
  if(nullptr ==  udpSocket_) {
      debug_log << tr("не создан сокет");
      udpSocket_ = new QUdpSocket(this);
    }
  qint64 res = -1;
  switch (compressionMode_) {
    case msgstream::COMPRESS_OPTIM:
    case msgstream::COMPRESS_ALWAYS:{
        compress::Compress compressor;
        res= udpSocket_->writeDatagram(compressor.compress(datagram ), QHostAddress(host_), port_);
      }
      break;
    default:
      res = udpSocket_->writeDatagram(datagram, QHostAddress(host_), port_);
      break;
    }
  if(res >0)
    {
      sendChunkCount_++;
      sendSize_ += res;
      sendSizeToday_ += datagram.size();
      status_->setParam(kSendSize, AppStatusThread::sizeStr(sendSize_));
      status_->setParam(kSendSizeToday, AppStatusThread::sizeStr(sendSizeToday_));
      status_->setParam(kSendChunkCount, sendChunkCount_);
    } else {
    debug_log << udpSocket_->errorString();
  }
  return res;
}

qint64 UdpStreamOut::writePackets(){
  qint64 ws = 0;
  uint32_t new_size = packets_.getDataSize();
  static boost::crc_32_type crc32_calculator;
  QByteArray dc = packets_.getPayload();
  crc32_calculator.reset();
  crc32_calculator.process_bytes(dc.data() , dc.size() );
  uint32_t crc32 = crc32_calculator.checksum();
  UdpPacketMap::iterator it = packets_.begin();
  UdpPacketMap::iterator eit = packets_.end();
 // QFile file("/tmp/out/"+packets_.getDataName()+ ".code");
 // file.open(QIODevice::WriteOnly);
  QByteArray ba;
  for(;it!=eit;++it){
    it.value().setDataSize(new_size);
    it.value().setMagic(crc32);
    it.value().getDataToSend(&ba);
    if(ba.size() < 1) break;
    ws+=write(ba);
  /*     if (file.isOpen()){
         QByteArray bdata;
         it.value().getData(&bdata);
         file.write(bdata);
       }*/
  }
    //file.close();
  return ws;
}

void UdpStreamOut::sendPackets()
{
  test(outba_);
  if(true == need_fec_){
    packets_.codePayloadAndCreate(outba_);
  } else {
    packets_.create(outba_,outba_.size());
  }
  outba_.clear();
  int32_t her =0;
  for(uint i=0;i < sendTryCount_;++i){
    int32_t wr =  writePackets();
    if(-1 == wr ){ error_log << tr("Ошибка записи в сокет"); }
    her+=wr;
  }
  status_->setParam(kSendFileCount, ++sendFileCount_);
  status_->setParam(kSendCount, sendCount_);
  status_->setParam(kSendCountToday, sendCountToday_);
  packets_.clear();
}

void UdpStreamOut::test(const QByteArray &ba){
  parser_->appendData(ba);
  tlg::MessageNew msg;
  while ( parser_->parseNextMessage(&msg) ) {
    ++sendCount_;
    ++sendCountToday_;
  }

}


void UdpStreamOut::slotProcess()
{
  static int writeCounter = 0;
  static QDateTime beginDt = QDateTime::currentDateTime();
  if ( 0 == outgoing_ ) { return; }
  if( outgoing_->size() == 0 ){
    if( 0 < outba_.size() && isLimitsReached(writeCounter, outba_.size(), beginDt) ) {
      sendPackets();
    }
    QTimer::singleShot( read_interval_, this, SLOT(slotProcess()) );
    return;
  }
  while ( outgoing_->size() > 0 ) {
    tlg::MessageNew msg = outgoing_->first();
    if ( packets_.size()==0 || packets_.getDataName() !=generateDataName(msg) ) {
        packets_.setDataName(generateDataName(msg));
        beginDt = QDateTime::currentDateTime();
      }
    QByteArray ba;
    if ( contentOnly_ ) {
      ba = tlg::tlg2image(pbtools::fromBytes(msg.msg()));
    }
    else {
      ba = tlg::raw2tlg(msg);
    }
    if(1 > ba.size()){
      outgoing_->removeFirst();
      continue;
    }
    if( isLimitsReached(writeCounter, outba_.size()+ba.size(), beginDt) ) {
      if(0 == outba_.size()){ //на случай, если нужно передать сообщение > size_limit_...
        outgoing_->removeFirst();
        outba_+=ba;
      }
      sendPackets();
      writeCounter = 0;
      packets_.setDataName(generateDataName(msg));
      beginDt = QDateTime::currentDateTime();
    } else {
      outgoing_->removeFirst();
      outba_+=ba;
    }
    ++writeCounter;
  }
  QTimer::singleShot( 30*read_interval_, this, SLOT(slotProcess()) );
}


QString UdpStreamOut::generateDataName(const tlg::MessageNew& msg) const
{
  QDateTime dt = QDateTime::currentDateTimeUtc();
  QMap<QString,QString> values;
  values.insert( "{YYYY}", dt.toString("yyyy") );
  values.insert( "{YY}", dt.toString("yy") );
  values.insert( "{MM}", dt.toString("MM") );
  values.insert( "{DD}", dt.toString("dd") );
  values.insert( "{hh}", dt.toString("hh") );
  values.insert( "{mm}", dt.toString("mm") );
  values.insert( "{ss}", dt.toString("ss") );
  values.insert( "{YYGGgg}", pbtools::toQString(msg.header().yygggg()) );
  values.insert( "{ID}", QString("%1").arg(++fileNumb_, 8, 10, QChar('0')) );
  values.insert( "{CCCC}", pbtools::toQString(msg.header().cccc()) );

  QString fileName = meta_template_;
  foreach( QString t, values.keys() ){
      fileName.replace( t, values[t] );
    }
  return fileName;
}

bool UdpStreamOut::isLimitsReached(int writeCounter,
                                   qint64 fileSize,
                                   const QDateTime& beginDt)
{
  bool messageLimit = (msg_limit_ > 0) && (writeCounter == msg_limit_);
  bool sizeLimit = (size_limit_ > 0) && (fileSize >= size_limit_);

  QDateTime endDt = beginDt.addSecs(maxTime_);
  bool timeLimit = (maxTime_ > 0) && (endDt <= QDateTime::currentDateTime());

  return messageLimit || sizeLimit || timeLimit;
}

void UdpStreamOut::timerEvent(QTimerEvent* event)
{
  Q_UNUSED( event );

  QDateTime dt = QDateTime::currentDateTime();
  if ( lastCheck_.date() != dt.date() ) {
      sendSizeToday_ = 0;
      sendCountToday_ = 0;
      //sendChunkCount_ = 0;
    }
  lastCheck_ = dt;
}

}
