#ifndef MAPPI_FTPLOADER_RAWLOADER_H
#define MAPPI_FTPLOADER_RAWLOADER_H

#include <meteo/wrf/ftploader/ftploader.h>

namespace mappi {

class RawLoader : public ::meteo::FtpLoader
{
public:
  explicit RawLoader(QObject* parent = 0);

protected:
  virtual void downloadDone(const QString& tempName, const QString& origName);
};

} // mappi

#endif // MAPPI_FTPLOADER_RAWLOADER_H
