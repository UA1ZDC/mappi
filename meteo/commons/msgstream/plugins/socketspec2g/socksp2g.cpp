#include "socksp2g.h"

#include <arpa/inet.h>
#include <cstring>

#include <qdir.h>
#include <qfile.h>
#include <QtNetwork>

#include <meteo/commons/msgparser/common.h>
#include <meteo/commons/msgparser/tlgparser.h>
#include <meteo/commons/msgparser/msgmetainfo.h>
#include <meteo/commons/msgstream/streammodule.h>
#include <meteo/commons/global/global.h>
#include <commons/compresslib/mcompress.h>

namespace meteo {
namespace socksp2g {

bool ServicePacket::parse( const QByteArray& raw)
{
  if ( raw.size() < SERV_PACKET_SIZE ) { return false; }

  const char *c = raw.data();

  std::memcpy(data_.magic, c, 5 );
  std::memcpy(&(data_.type), c+5, 1 );
  std::memcpy(&(data_.num), c+6, 4 );
  std::memcpy(&(data_.len), c+10, 4 );
  std::memcpy(&(data_.info_len), c+14, 2 );

  if(data_.magic[0] != 'U'){
    return false;
  }
  data_.num  = qFromBigEndian<quint32>((const uchar*)&data_.num);
  data_.len  = qFromBigEndian<quint32>((const uchar*)&data_.len);
  data_.info_len  = qFromBigEndian<quint16>((const uchar*)&data_.info_len);

  return true;
}

ServicePacket ServicePacket::createRR(int8_t* magic)
{
  ServicePacket pack;
  pack.setMagic(magic);
  pack.data_.type = kTcpIpRR;

  return pack;
}

ServicePacket::ServicePacket()
{
  clear();
}

ServicePacket::ServicePacket(PackType t, int8_t* magic)
{
  clear();

  data_.type = t;

  setMagic(magic);
}

RecvState ServicePacket::setPacket(const QByteArray *raw)
{

  if ( !socksp2g::ServicePacket::parse(*raw) ) {
    // return socksp2g::kStart;
    if ( socksp2g::kTcpIpRR == data().type ) {
      return socksp2g::kRR;
    }
    return kError;
  }

  //raw->remove(0, sizeof(socksp2g::ServicePackData));

  if ( socksp2g::kTcpIpRR == data().type ) {
    return socksp2g::kRR;
  }
  if ( socksp2g::kTcpIpData== data().type )
  {
    return kInfoMsg;
  }

  if ( socksp2g::kTcpIpFile==data().type)
  {
    return kInfoFile;
  }

  if ( socksp2g::kTcpIpAck == data().type ) {
    return  kAck;
  }

  if ( socksp2g::kTcpIpEnd == data().type ) {
    return  kEnd;
  }
  debug_log << data().type<<QObject::tr("Ожидался TCP_IP_DATA, TCP_IP_DATA_Z, TCP_IP_ACK,TCP_IP_END или TCP_IP_RR.");
  //slotStop();
  // slotRestart();
  return socksp2g::kStart;
}


void ServicePacket::setMagic(int8_t* magic)
{
  if ( 0 == magic ) { return; }

  std::memcpy(data_.magic, magic, sizeof (data_.magic));
}

void ServicePacket::clear()
{
  std::memset(data_.magic, 0, sizeof (data_.magic));
  data_.type=0;     //!< Тип пакета, определяет назначение пакета (PackType).
  data_.num=0;      //!< Последовательный номер сообщения при передаче (в бинарном виде).
  data_.len=0;      //!< Размер текста сообщения в байтах, при передаче в упакованном виде определяет размер сжатого текста.
  data_.info_len=0;
}

QByteArray ServicePacket::toNet() const
{
  QByteArray ba(SERV_PACKET_SIZE, '\0' );

  ServicePackData p;
  p.type = data_.type;
  std::memcpy(p.magic, data_.magic, sizeof (data_.magic));

  p.num  = qToBigEndian<quint32>(data_.num);
  p.len  = qFromBigEndian<quint32>(data_.len);
  p.info_len  = qFromBigEndian<quint16>(data_.info_len);

  std::memcpy(ba.data(), p.magic, sizeof (data_.magic));
  std::memcpy(ba.data()+5, &p.type, sizeof (p.type));
  std::memcpy(ba.data()+6, &p.num, sizeof (p.num));
  std::memcpy(ba.data()+10, &p.len, sizeof (p.len));
  std::memcpy(ba.data()+14, &p.info_len,sizeof (p.info_len));

  return ba;
}

bool ServicePacket::match(const ServicePacket& pack) const
{
  // сравниваем без учёта типа
  bool res = (pack.data_.info_len == data_.info_len)&&
             (pack.data_.len == data_.len)&&
             (pack.data_.num == data_.num)&&
             (pack.data_.magic[0] == data_.magic[0])&&
      (pack.data_.magic[1] == data_.magic[1])&&
      (pack.data_.magic[2] == data_.magic[2])&&
      (pack.data_.magic[3] == data_.magic[3])&&
      (pack.data_.magic[4] == data_.magic[4]);
  return res;
}



int InfoPacket::setPacket(ServicePacket *start, const QByteArray *raw, bool compressed)
{

  if ( int(start->data().len + start->data().info_len) > raw->size() ) { return -1; }

  QByteArray head = raw->left(start->data().info_len);
  QByteArray msg = raw->mid(start->data().info_len, start->data().len);
  //  if ( socksp2g::kTcpIpDataZC == start->data().type ) {
  if ( true == compressed ) {
    compress::Compress compressor;
    bool ok=true;
    QByteArray ba = compressor.decompress(msg,&ok);
    if (!ok || ba.isNull() ) {
      debug_log << QObject::tr("Ошибка при распаковке метеосообщения.");
      return -1;
    }
    msg = ba;
  }

  switch (start->data().type) {
    case kTcpIpFile:
      if ( !parseFile(head, msg) ) {
        debug_log << QObject::tr("Ошибка распознавания телеграммы:") << QByteArray::fromRawData((char*)this->head().ahd, 10);
        return -2;
      }
    break;

    case kTcpIpData:
    default:
      if ( !parseMsg(head, msg) ) {
        debug_log << QObject::tr("Ошибка распознавания телеграммы:") << QByteArray::fromRawData((char*)this->head().ahd, 10);

        return -2;
      }
    break;
  }
  return 0;
}

bool InfoPacket::parseMsg( const QByteArray& head, const QByteArray& tlg)
{

  msg_.Clear();
  if ( head.size() < (int)INFO_MSG_PACKET_SIZE ) {
    debug_log << tlg;
    return false;
  }
  std::memcpy(&msghead_, head.constData(), INFO_MSG_PACKET_SIZE);


  msghead_.text_offset = ntohs(msghead_.text_offset);
  switch (msghead_.format) {
    case kWMOFormat:
      msg_.set_format(::meteo::tlg::kWMO);
    break;
    case kGMSFormat:
      msg_.set_format(::meteo::tlg::kGMS);
    break;
    default:
      msg_.set_format(::meteo::tlg::kOther);
  }
  ::meteo::TlgParser parser(tlg);
  if ( !parser.parseNextMessage(&msg_) ) {
    return false;
  }

  meteo::MsgMetaInfo meta(msg_);
  QDateTime dt = meta.calcConvertedDt();
  msg_.mutable_metainfo()->set_converted_dt(dt.toString(Qt::ISODate).toUtf8().constData());
  return true;
}

bool InfoPacket::parseFile(const QByteArray& head, const QByteArray& tlg)
{
  msg_.Clear();
  std::memcpy(&filehead_, head.constData(), INFO_FILE_NAME_PACKET_SIZE);
  filehead_.size = ntohs(filehead_.size);
  if((uint)tlg.size() <  filehead_.size){
    return false;
  }
  QString filename = head.mid(INFO_FILE_NAME_PACKET_SIZE);
  QDir d; d.mkpath(outdir_);

  QFile file(outdir_+"\\"+filename);
  if (!file.open(QIODevice::WriteOnly))
    return false;
  file.write(tlg);
  file.close();
  return true;
}


InfoPacket InfoPacket::fromMessage(const meteo::tlg::MessageNew& msg)
{
  InfoPacket pack;

  pack.msg_ = msg;

  pack.msghead_.format = 0; // FIXME: как правильно заполнить?
  pack.msghead_.text_offset = 0; // FIXME: вычислить размер заголовка и начальной строки

  QByteArray ahd(10, ' ');
  if ( msg.header().has_t1() && msg.header().t1().size()>0) { ahd[0] = msg.header().t1().at(0); }
  if ( msg.header().has_t2() && msg.header().t2().size()>0) { ahd[1] = msg.header().t2().at(0); }
  if ( msg.header().has_a1() && msg.header().a1().size()>0) { ahd[2] = msg.header().a1().at(0); }
  if ( msg.header().has_a2() && msg.header().a2().size()>0) { ahd[3] = msg.header().a2().at(0); }
  if ( msg.header().has_ii() ) { ahd.replace(4, 2, QByteArray::number(msg.header().ii()).rightJustified(2, '0')); }

  if ( msg.header().cccc().size() == 4 ) {
    ahd.replace(6, 4, QByteArray::fromRawData(msg.header().cccc().data(), msg.header().cccc().size()));
  }
  std::memcpy(&pack.msghead_.ahd, ahd.constData(), sizeof(pack.msghead_.ahd));

  if ( msg.header().yygggg().size() == sizeof(pack.msghead_.yygg) )
  {
    std::memcpy(&pack.msghead_.yygg, msg.header().yygggg().data(), sizeof(pack.msghead_.yygg));
  }
  if ( msg.header().bbb().size() == sizeof(pack.msghead_.bbb) )
  { std::memcpy(&pack.msghead_.bbb, msg.header().bbb().data(), sizeof(pack.msghead_.bbb)); }

  return pack;
}

QByteArray InfoPacket::toNet() const
{
  QByteArray ba(INFO_MSG_PACKET_SIZE, '\0' );

  InfoMsgPackHead p;
  p = msghead_;
  p.text_offset  = qFromBigEndian<quint16>(p.text_offset);
  std::memcpy(ba.data(), &p, ba.size());
  return ba;
}

QByteArray InfoPacket::rawTlg() const
{
  return ::meteo::tlg::raw2tlg(msg_);
}

void InfoPacket::clear()
{
  msg_.Clear();

  std::memset(&msghead_, 0, INFO_MSG_PACKET_SIZE);
}

} // socksp2g
} // meteo

TLog& operator <<(TLog& log, const meteo::socksp2g::ServicePacket& pack)
{
  return log << "ServicePacket:\n"
         << "magic:" << QString(QByteArray((const char*)pack.data().magic, 5)) << "\n"
         << "type:" << pack.data().type << "\n"
         << "num:" << pack.data().num << "\n"
         << "len:" << pack.data().len << "\n"
         << "info_len:" << pack.data().info_len;
}

TLog& operator <<(TLog& log, const meteo::socksp2g::InfoPacket& pack)
{
  return log << "InfoPacket:\n"
         << "head --\n"
         << "  format:" << pack.head().format << "\n"
         << "  text_offset:" << pack.head().text_offset << "\n"
         << "  ahd:" << QString(QByteArray((const char*)pack.head().ahd, 10)) << "\n"
         << "  yygg:" << QString(QByteArray((const char*)pack.head().yygg, 6)) << "\n"
         << "  bbb:" << QString(QByteArray((const char*)pack.head().bbb, 3)) << "\n"
         << "-------\n"
         << "tlg --\n"
         << pack.rawTlg()
         << "------\n";
}
