#ifndef SELFADDRESS_H
#define SELFADDRESS_H

#include <qstring.h>
#include <qstringlist.h>

class QHostAddress;

namespace MnCommon {
  QStringList selfIpAddresses();
  QString selfIpAddress();

  bool addressInMySubnet( const QHostAddress& addr );
  bool addressInMySubnet( const QString& addr );
}

#endif // SELFADDRESS_H
