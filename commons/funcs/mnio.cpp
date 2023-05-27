#include "mnio.h"

#include <unistd.h>
//#include <commons/defines/mn_defines.h>
#include <cross-commons/debug/tlog.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <unistd.h>


#ifdef T_OS_ASTRA_LINUX
#include <parsec/parsec_mac.h>
#include <parsec/mac.h>
#endif

qint64 MnCommon::readFromSocket(QTcpSocket *socket, QByteArray* incomingBuffer, qint64 bytes)
{
  QByteArray arr( bytes, 0 );
  int64_t shift = 0;
  int64_t sz = 0;
  do {
    sz = socket->read( arr.data() + shift, bytes - shift );
    shift += sz;
  } while ( 0 < bytes && 0 < sz );
  if ( 0 < shift ) {
    incomingBuffer->append( QByteArray::fromRawData( arr.data(), shift ) );
  }
  return shift;

}

qint64 MnCommon::writeToSocket(QTcpSocket *socket, const QByteArray& outgoingBuffer)
{
  int64_t sz = 0;
  int64_t shift = 0;
  int64_t allsz = outgoingBuffer.size();
  do {
    sz = socket->write( outgoingBuffer.constData() + shift, allsz - shift );
    shift += sz;
  } while ( 0 < sz && shift < allsz );
  return shift;
}



void MnCommon::setSocketOptions(int s, int32_t fl,int32_t maxidle,int32_t cnt,int32_t intrvl )
{
//  int32_t fl = 1;
  if ( 0 != ::setsockopt( s, SOL_SOCKET, SO_KEEPALIVE, &fl, sizeof(fl) ) ) {
    error_log << QObject::tr("Не удалось установить параметр сокета SO_KEEPALIVE = 1");
    return;
  }
  //int32_t maxidle = 10; //10 секунд
  if ( 0 != ::setsockopt( s, IPPROTO_TCP, TCP_KEEPIDLE, &maxidle, sizeof(maxidle) ) ) {
    error_log << QObject::tr("Не удалось установить параметр сокета TCP_KEEPIDLE= %1")
      .arg(maxidle);
    return;
  }
  //int32_t cnt = 3; //посылка трех пакетов
  if ( 0 != ::setsockopt( s, SOL_TCP, TCP_KEEPCNT, &cnt, sizeof(cnt) ) ) {
    error_log << QObject::tr("Не удалось установить параметр сокета SO_KEEPCNT= %1")
      .arg(cnt);
    return;
  }
  //int32_t intrvl = 2; //интервал между проверочными пакетами = 2 секунды после 5 секунд простоя
  if ( 0 != ::setsockopt( s, SOL_TCP, TCP_KEEPINTVL, &intrvl, sizeof(intrvl) ) ) {
    error_log << QObject::tr("Не удалось установить параметр сокета SO_KEEPINTVL= %1")
      .arg(intrvl);
    return;
  }
}

uint8_t MnCommon::socket_mac_level(int sock)
{
#ifdef T_OS_ASTRA_LINUX
  parsec_mac_label_t mac;
  parsec_fstatmac(sock, &mac);
  return mac.mac.lev;
#else
  Q_UNUSED(sock);
  return 0;
#endif
}


uint8_t process_self_mac_level()
{
#ifdef T_OS_ASTRA_LINUX
    pid_t self = getpid();
    parsec_mac_t mac;
    parsec_getmac(self,&mac);
    debug_log << mac.cat << mac.lev;
    return mac.lev;
#else
    return 0;
#endif
}
