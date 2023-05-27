#include "selfaddress.h"

#include <qnetworkinterface.h>
#include <qhostaddress.h>

namespace MnCommon {

QStringList selfIpAddresses()
{
  QStringList iplist;
  QList<QHostAddress> all = QNetworkInterface::allAddresses();
  for ( int i = 0, sz = all.size(); i < sz; ++i ) {
    QHostAddress addr = all[i];
    if (
        QHostAddress::Null == addr ||
        QHostAddress::LocalHost == addr ||
        QHostAddress::LocalHostIPv6 == addr ||
        QHostAddress::Broadcast == addr ||
        QHostAddress::Any == addr ||
        QHostAddress::AnyIPv6 == addr ||
        QAbstractSocket::IPv4Protocol != addr.protocol()
       ) {
      continue;
    }
    iplist.append( addr.toString() );
  }
  return iplist;
}

QString selfIpAddress()
{
  QStringList iplist = selfIpAddresses();
  if ( 0 == iplist.size() ) {
    return "127.0.0.1";
  }
  return iplist[0];
}

bool addressInMySubnet( const QHostAddress& addr )
{
  QStringList list = selfIpAddresses();
  for ( int i = 0, sz = list.size(); i < sz; ++i ) {
    const QString& a = list[i];
    if ( true == addr.isInSubnet( QHostAddress(a), 24 ) ) {
      return true;
    }
  }
  return false;
}

bool addressInMySubnet( const QString& addr )
{
  return addressInMySubnet( QHostAddress(addr) );
}

}
