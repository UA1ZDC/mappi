#include "rawdatadriver.h"

#include <qdatetime.h>
#include <qbytearray.h>

#include <cross-commons/debug/tlog.h>

#include <meteo/commons/proto/msgparser.pb.h>

#include <meteo/commons/msgparser/common.h>
#include <meteo/commons/msgparser/tlgparser.h>

namespace meteo {

bool RawDataDriver::mkMsg(tlg::MessageNew* msg, const QDateTime& dt, const QByteArray& data) const
{
  QByteArray ba;
  ba += QString(dt.toString(Qt::ISODate) + " " + prefix_).toUtf8();
  ba += data;

  tlg::MessageNew m;
  m.set_format(tlg::kWMO);
  m.set_msg(ba.constData());

  ba = tlg::proto2tlg(m);

  TlgParser parser(ba);
  if ( !parser.parseNextMessage(msg) ) {
    debug_log << QObject::tr("Не удалось распарсить сформированное сообщение.");
    return false;
  }

  return true;
}

} // meteo
