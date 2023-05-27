#ifndef METEO_COMMONS_MSGSTREAM_PLUGINS_TCPSTREAM_IPVDRIVER_H
#define METEO_COMMONS_MSGSTREAM_PLUGINS_TCPSTREAM_IPVDRIVER_H

#include <qdatetime.h>

#include <meteo/commons/msgstream/plugins/tcpstream/rawdatadriver.h>

#include "tcpstream.pb.h"


namespace meteo {

class IPVDriver : public RawDataDriver
{
public:
  explicit IPVDriver(AppStatusThread* status);

  virtual void setData(const QByteArray& data)    { rawParser_.setData(data); }
  virtual void appendData(const QByteArray& data) { rawParser_.appendData(data); }

  virtual bool parse();
  virtual bool getMessage(tlg::MessageNew* msg);

private:
  QDateTime dt_;
  msgstream::FormatIPV wind_;
};

} // meteo

#endif // METEO_COMMONS_MSGSTREAM_PLUGINS_TCPSTREAM_IPVDRIVER_H
