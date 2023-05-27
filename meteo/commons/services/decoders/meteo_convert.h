#ifndef METEO_COMMONS_SERVICES_DECODERS_METEO_CONVERT_H
#define METEO_COMMONS_SERVICES_DECODERS_METEO_CONVERT_H

#include <qlist.h>

class QByteArray;
class TMeteoData;

namespace meteo {
  namespace sigwx {
    class Header;
  }

  namespace decoders {
    bool toSigwxStructs(const TMeteoData& data, QList<QByteArray>* ba, QList<sigwx::Header>* header);
  }
}

#endif
