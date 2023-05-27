#include "bcmdriver.h"

#include <commons/textproto/pbtools.h>
#include <commons/textproto/tprototext.h>

#include <meteo/commons/proto/msgparser.pb.h>

#include <meteo/commons/msgparser/common.h>
#include <meteo/commons/msgparser/tlgparser.h>
#include <meteo/commons/msgparser/rawparser.pb.h>

#include "tcpstream.pb.h"

namespace meteo {

BCMDriver::BCMDriver(AppStatusThread* status)
  : RawDataDriver(status)
{
  QString file = ":/meteo/msgstream/formats/bcm.conf";

  msgparser::RawBinFormat format;
  if ( !TProtoText::fromFile(file, &format) ) {
    error_log << QObject::tr("Не удалось загрузить описание формата из файла '%1'").arg(file);
  }
  rawParser_.setFormat(format);
}

bool BCMDriver::parse()
{
  msgstream::FormatBCM f;
  if ( !rawParser_.parse(&f) ) { return false; }

  press_ = f.f_10000() * 10000 + f.f_1000() * 1000 + f.f_100() * 100 + f.f_10() * 10 + f.f_1() + f.f_01() * 0.1;
  dt_ = QDateTime::currentDateTimeUtc();

  return true;
}

bool BCMDriver::getMessage(tlg::MessageNew* msg)
{
  return mkMsg(msg, dt_, QString::number(press_).toUtf8());
}

} // meteo
