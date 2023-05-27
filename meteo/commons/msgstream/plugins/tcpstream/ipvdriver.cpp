#include "ipvdriver.h"

#include <commons/textproto/pbtools.h>
#include <commons/textproto/tprototext.h>

#include <meteo/commons/proto/msgparser.pb.h>

#include <meteo/commons/msgparser/common.h>
#include <meteo/commons/msgparser/tlgparser.h>
#include <meteo/commons/msgparser/rawparser.pb.h>

#include "tcpstream.pb.h"

namespace meteo {

IPVDriver::IPVDriver(AppStatusThread* status)
  : RawDataDriver(status)
{
  QString file = ":/meteo/msgstream/formats/ipv.conf";

  msgparser::RawBinFormat format;
  if ( !TProtoText::fromFile(file, &format) ) {
    error_log << QObject::tr("Не удалось загрузить описание формата из файла '%1'").arg(file);
  }

  rawParser_.setFormat(format);
}

bool IPVDriver::parse()
{
  if ( !rawParser_.parse(&wind_) ) { return false; }

  dt_ = QDateTime::currentDateTimeUtc();
  return true;
}

bool IPVDriver::getMessage(tlg::MessageNew* msg)
{
  QString text;
  text += QString::number(wind_.inst_speed() * 0.01) + ";";
  text += QString::number(wind_.inst_angle()) + ";";
  text += QString::number(wind_.cur_speed() * 0.01) + ";";
  text += QString::number(wind_.cur_angle()) + ";";
  text += QString::number(wind_.avg2m_speed() * 0.01) + ";";
  text += QString::number(wind_.avg2m_angle()) + ";";
  text += QString::number(wind_.avg10m_speed() * 0.01) + ";";
  text += QString::number(wind_.avg10m_angle()) + ";";
  text += QString::number(wind_.max_speed() * 0.01) + ";";
  text += QString::number(wind_.max10_speed() * 0.01) + ";";

  return mkMsg(msg, dt_, text.toUtf8());
}

} // meteo
