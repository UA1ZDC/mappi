#include "sriv512.h"

#include <arpa/inet.h>
#include <cstring>

#include <meteo/commons/msgparser/common.h>
#include <meteo/commons/msgparser/tlgparser.h>
#include <meteo/commons/msgparser/msgmetainfo.h>
#include <meteo/commons/msgstream/streammodule.h>
#include <meteo/commons/global/global.h>
#include <commons/compresslib/mcompress.h>

namespace meteo {
namespace sriv512 {

bool ServicePacket::parse(ServicePacket* pack, const QByteArray& raw)
{
  if ( 0 == pack ) { return false; }

  if ( raw.size() < (int)sizeof(ServicePackData) ) { return false; }

  std::memcpy(&pack->data_, raw.constData(), sizeof(ServicePackData));

  pack->data_.num = htons(pack->data_.num);
  pack->data_.len = htons(pack->data_.len);
  pack->data_.type = htons(pack->data_.type);
  pack->data_.info_len = htons(pack->data_.info_len);
  return true;
}

ServicePacket ServicePacket::createRR()
{
  ServicePacket pack;
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

RecvState ServicePacket::setPacket(const QByteArray *raw){

  if ( !sriv512::ServicePacket::parse(this, *raw) ) { return kError; }

  //raw->remove(0, sizeof(sriv512::ServicePackData));

  if ( sriv512::kTcpIpRR == data().type ) {
      return sriv512::kRR;
    }
  if ( (QList<int>() << sriv512::kTcpIpData
        << sriv512::kTcpIpDataZ
        << sriv512::kTcpIpDataZC).contains(data().type) )
    {

      return kInfo;
    }
  if ( sriv512::kTcpIpAck == data().type ) {
      return  kAck;
    }

  if ( sriv512::kTcpIpEnd == data().type ) {
      return  kEnd;
    }
  debug_log << data().type<<QObject::tr("Ожидался TCP_IP_DATA, TCP_IP_DATA_Z, TCP_IP_ACK,TCP_IP_END или TCP_IP_RR.");
  //slotStop();
  // slotRestart();
  return sriv512::kStart;
}


void ServicePacket::setMagic(int8_t* magic)
{
  if ( 0 == magic ) { return; }

  std::memcpy(data_.magic, magic, 5);
}

void ServicePacket::clear()
{
  std::memset(&data_, '\0', 16);
}

QByteArray ServicePacket::toNet() const
{
  QByteArray ba(sizeof(ServicePackData), '\0' );

  ServicePackData p;
  std::memcpy(&p, &data_, sizeof(ServicePackData));

  p.len = htonl(p.len);
  p.num = htonl(p.num);

  std::memcpy(ba.data(), &p, sizeof(ServicePackData));

  return ba;
}

bool ServicePacket::match(const ServicePacket& pack) const
{
  ServicePackData p1;
  std::memcpy(&p1, &data_, sizeof(ServicePackData));

  ServicePackData p2;
  std::memcpy(&p2, &pack.data_, sizeof(ServicePackData));

  // сравниваем без учёта типа
  p1.type = 0;
  p2.type = 0;

  QByteArray ba1(sizeof(ServicePackData), '\0');
  QByteArray ba2(sizeof(ServicePackData), '\0');

  std::memcpy(ba1.data(), &p1, ba1.size());
  std::memcpy(ba2.data(), &p2, ba2.size());

  return ba1 == ba2;
}


bool InfoPacket::setPacket(ServicePacket *start, const QByteArray *raw){

  if ( int(start->data().len + start->data().info_len) > raw->size() ) { return false; }

  QByteArray head = raw->left(start->data().info_len);
  QByteArray msg = raw->mid(start->data().info_len, start->data().len);
  if ( sriv512::kTcpIpDataZC == start->data().type ) {
    compress::Compress compressor;
    bool ok=true;
    QByteArray ba = compressor.decompress(msg,&ok);
    if (!ok || ba.isNull() ) {
      debug_log << QObject::tr("Ошибка при распаковке метеосообщения.");
      return false;
    }
    msg = ba;
  }

  if ( !sriv512::InfoPacket::parse(this, head, msg) ) {
    debug_log << QObject::tr("Ошибка распознавания телеграммы:") << QByteArray::fromRawData((char*)this->head().ahd, 10);
    //??? return false;
  }
  return true;
}


bool InfoPacket::parse(InfoPacket* pack, const QByteArray& head, const QByteArray& tlg)
{
  if ( 0 == pack ) {
    return false;
  }

  pack->msg_.Clear();

  ::meteo::TlgParser parser(tlg);
  if ( !parser.parseNextMessage(&pack->msg_) ) {
    return false;
  }

  meteo::MsgMetaInfo meta(pack->msg_);
  QDateTime dt = meta.calcConvertedDt();
  pack->msg_.mutable_metainfo()->set_converted_dt(dt.toString(Qt::ISODate).toUtf8().constData());

  if ( head.size() < (int)sizeof(InfoPackHead) ) {
    return false;
  }

  std::memcpy(&pack->head_, head.constData(), sizeof(InfoPackHead));

  pack->head_.text_offset = ntohs(pack->head_.text_offset);

  return true;
}

InfoPacket InfoPacket::fromMessage(const meteo::tlg::MessageNew& msg)
{
  InfoPacket pack;

  pack.msg_ = msg;

  pack.head_.format = 0; // FIXME: как правильно заполнить?
  pack.head_.text_offset = 0; // FIXME: вычислить размер заголовка и начальной строки

  QByteArray ahd(10, ' ');
  if ( msg.header().has_t1() && msg.header().t1().size()>0) { ahd[0] = msg.header().t1().at(0); }
  if ( msg.header().has_t2() && msg.header().t2().size()>0) { ahd[1] = msg.header().t2().at(0); }
  if ( msg.header().has_a1() && msg.header().a1().size()>0) { ahd[2] = msg.header().a1().at(0); }
  if ( msg.header().has_a2() && msg.header().a2().size()>0) { ahd[3] = msg.header().a2().at(0); }
  if ( msg.header().has_ii() ) { ahd.replace(4, 2, QByteArray::number(msg.header().ii()).rightJustified(2, '0')); }

  if ( msg.header().cccc().size() == 4 ) {
      ahd.replace(6, 4, QByteArray::fromRawData(msg.header().cccc().data(), msg.header().cccc().size()));
    }
  std::memcpy(&pack.head_.ahd, ahd.constData(), 10);

  if ( msg.header().yygggg().size() == 6 ) { std::memcpy(&pack.head_.yygg, msg.header().yygggg().data(), 6); }
  if ( msg.header().bbb().size() == 3 )    { std::memcpy(&pack.head_.bbb, msg.header().bbb().data(), 3); }

  return pack;
}

QByteArray InfoPacket::toNet() const
{
  QByteArray ba(sizeof(InfoPackHead), '\0' );

  InfoPackHead p;
  std::memcpy(&p, &head_, ba.size());

  p.text_offset = htons(p.text_offset);

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

  std::memset(&head_, '\0', sizeof(InfoPackHead));
}

} // sriv512
} // meteo

TLog& operator <<(TLog& log, const meteo::sriv512::ServicePacket& pack)
{
return log << "ServicePacket:\n"
         << "magic:" << QString(QByteArray((const char*)pack.data().magic, 5)) << "\n"
         << "type:" << pack.data().type << "\n"
         << "num:" << pack.data().num << "\n"
         << "len:" << pack.data().len << "\n"
         << "info_len:" << pack.data().info_len;
}

TLog& operator <<(TLog& log, const meteo::sriv512::InfoPacket& pack)
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
