#ifndef METEO_COMMONS_MSGSTREAM_PLUGINS_TCPSTREAM_BCMDRIVER_H
#define METEO_COMMONS_MSGSTREAM_PLUGINS_TCPSTREAM_BCMDRIVER_H

#include <qdatetime.h>

#include <meteo/commons/msgstream/plugins/tcpstream/rawdatadriver.h>

namespace meteo {

class BCMDriver : public RawDataDriver
{
public:
  BCMDriver(AppStatusThread* status);

  virtual void setData(const QByteArray& data)    { rawParser_.setData(data); }
  virtual void appendData(const QByteArray& data) { rawParser_.appendData(data); }

  virtual bool parse();
  virtual bool getMessage(tlg::MessageNew* msg);

private:
  QDateTime dt_;
  double press_;
};

} // meteo

#endif // METEO_COMMONS_MSGSTREAM_PLUGINS_TCPSTREAM_BCMDRIVER_H
