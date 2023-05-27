#include "lingdriver.h"

#include <commons/textproto/pbtools.h>
#include <commons/textproto/tprototext.h>

#include <meteo/commons/proto/msgparser.pb.h>

#include <meteo/commons/msgparser/common.h>
#include <meteo/commons/msgparser/tlgparser.h>
#include <meteo/commons/msgparser/rawparser.pb.h>

#include "tcpstream.pb.h"

namespace meteo {

LingDriver::LingDriver(AppStatusThread* status)
  : RawDataDriver(status)
{
  QString file = ":/meteo/msgstream/formats/ling.conf";

  msgparser::RawBinFormat format;
  if ( !TProtoText::fromFile(file, &format) ) {
    error_log << QObject::tr("Не удалось загрузить описание формата из файла '%1'").arg(file);
  }

  rawParser_.setFormat(format);

  vngo_ = -9999;
}

bool LingDriver::parse()
{
  msgstream::FormatLing f;
  if ( !rawParser_.parse(&f) ) { return false; }

  bool ok = false;
  int val = pbtools::toQString(f.vngo()).toInt(&ok);
  if ( !ok ) { return false; }

  vngo_ = val;
  dt_ = QDateTime::currentDateTimeUtc();

  return true;
}

bool LingDriver::getMessage(tlg::MessageNew* msg)
{
  return mkMsg(msg, dt_, QString::number(vngo_).toUtf8());
}

} // meteo
