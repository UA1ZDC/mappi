#include "unimas.h"

#include <cross-commons/app/paths.h>
#include <cross-commons/debug/tlog.h>
#include <meteo/commons/msgparser/msgmetainfo.h>

#include <QProcess>

#include <arpa/inet.h>
#include <cstring>


namespace meteo {


TServicePacket::TServicePacket(TServicePacket::Mode m, const char* data) :
          type( TCP == m ? tcp.type : utcp.type ),
   utcp_format( utcp.format ),
      utcp_ijp( utcp.ijp ),
utcp_text_offs( utcp.text_offs ),
       tcp_ijp( tcp.ijp ),
           len( TCP == m ? tcp.len : utcp.len ),
           num( TCP == m ? tcp.num : utcp.num ),
           ahd( TCP == m ? tcp.ahd : utcp.ahd ),
           pri( TCP == m ? tcp.pri : utcp.pri ),
         mode_( m )
{
  packetSize_ = isTcpMode() ? sizeof(TcpServicePacket) : sizeof(UtcpServicePacket);

  clear();
  copyFrom( data );
}

TServicePacket::TServicePacket(const TServicePacket& p) :
           type( TCP == p.mode_ ? tcp.type : utcp.type ),
    utcp_format( utcp.format ),
       utcp_ijp( utcp.ijp ),
 utcp_text_offs( utcp.text_offs ),
        tcp_ijp( tcp.ijp ),
            len( TCP == p.mode_ ? tcp.len : utcp.len ),
            num( TCP == p.mode_ ? tcp.num : utcp.num ),
            ahd( TCP == p.mode_ ? tcp.ahd : utcp.ahd ),
            pri( TCP == p.mode_ ? tcp.pri : utcp.pri )

{
  if ( p.isTcpMode() ) {
    std::memcpy( &tcp, &p.tcp, p.packetSize_ );
  }
  else {
    std::memcpy( &utcp, &p.utcp, p.packetSize_ );
  }

  packetSize_ = p.packetSize_;
  mode_ = p.mode_;
}

TServicePacket& TServicePacket::operator=(const TServicePacket& p)
{
  if ( p.isTcpMode() ) {
    std::memcpy( &tcp, &p.tcp, p.packetSize_ );
  }
  else {
    std::memcpy( &utcp, &p.utcp, p.packetSize_ );
  }

  packetSize_ = p.packetSize_;
  mode_ = p.mode_;

  return *this;
}

QByteArray TServicePacket::toNet() const
{
  QByteArray ba( packetSize_, '\0' );

  if ( isTcpMode() ) {
    TcpServicePacket p;
    std::memcpy( &p, &tcp, sizeof(TcpServicePacket) );

    p.len = htons( p.len );
    p.num = htons( p.num );
    p.ijp = htonl( p.ijp );

    std::memcpy( ba.data(), &p, sizeof(TcpServicePacket) );
  }
  else {
    UtcpServicePacket p;
    std::memcpy( &p, &utcp, sizeof(UtcpServicePacket) );

    p.len = htons( p.len );
    p.num = htons( p.num );
    p.ijp.jno = htons( p.ijp.jno );
    p.ijp.ijp = htonl( p.ijp.ijp );
    p.text_offs = htons( p.text_offs );

    std::memcpy( ba.data(), &p, sizeof(UtcpServicePacket) );
  }

  return ba;
}

void TServicePacket::clear()
{
  std::memset(&tcp,'\0',sizeof(TcpServicePacket));
  std::memset(&utcp,'\0',sizeof(UtcpServicePacket));
}

TServicePacket TServicePacket::toAck() const
{
  TServicePacket ack( mode_ );
  ack = *this;
  ack.type = kTcpIpAck;
  return ack;
}

int TServicePacket::parse(const QByteArray& ba)
{
  if ( ba.size() < packetSize_ ) {
    return 0;
  }

  copyFrom( ba.constData() );

  return packetSize_;
}

bool TServicePacket::operator!=(const TServicePacket& p) const
{
  bool r = false;

  r = (r || (len != p.len));
  r = (r || (num != p.num));
  r = (r || (pri != p.pri));
  r = (r || (0 != memcmp(ahd,p.ahd,12)));

  if ( isTcpMode() ) {
    r = (r || (tcp.ijp != p.tcp_ijp));
  }
  else {
    r = (r || (utcp_format != p.utcp_format));
    r = (r || (utcp_ijp.ijp != p.utcp_ijp.ijp));
    r = (r || (utcp_ijp.jno != p.utcp_ijp.jno));
    r = (r || (utcp_ijp.textno != p.utcp_ijp.textno));
    r = (r || (utcp_text_offs != p.utcp_text_offs));
  }

  return r;
}

bool TServicePacket::operator==(const TServicePacket& p)const
{
  return !(*this != p);
}

void TServicePacket::printDebug() const
{
  debug_log << "TServicePacket {";
  var( this->type );
  if ( this->isTcpMode() ) {
    var( this->tcp_ijp );
  }
  else {
    var( this->utcp_ijp.ijp );
    var( this->utcp_ijp.jno );
    var( this->utcp_ijp.textno );
    var( this->utcp_format );
  }
  var( this->len );
  var( this->num );
  var( this->pri );
  debug_log << "this->ahd" << QString::fromUtf8((const char*)this->ahd,12).toUtf8();
  debug_log << "}";
}

void TServicePacket::copyFrom(const char* data)
{
  if ( nullptr == data ) {
    return;
  }

  if ( isTcpMode() ) {
    std::memcpy( &tcp, data, packetSize_ );

    tcp.len = ntohs( tcp.len );
    tcp.num = ntohs( tcp.num );
    tcp.ijp = ntohl( tcp.ijp );
  }
  else {
    std::memcpy( &utcp, data, packetSize_ );

    utcp.len = ntohs( utcp.len );
    utcp.num = ntohs( utcp.num );
    utcp.ijp.jno = ntohs( utcp.ijp.jno );
    utcp.ijp.ijp = ntohl( utcp.ijp.ijp );
    utcp.text_offs = ntohs( utcp.text_offs );
  }
}


} // meteo
