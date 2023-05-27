#ifndef MAPPI_THEMATIC_ALGS_TOFORMAT_H
#define MAPPI_THEMATIC_ALGS_TOFORMAT_H

#include <cross-commons/singleton/tsingleton.h>
#include <mappi/proto/thematic.pb.h>
#include <mappi/thematic/algs_calc/datastore.h>
#include <qmap.h>

#include <QSharedPointer>

namespace mappi {
  namespace to {

    class ThemAlg;

    typedef ThemAlg* (*CreateThem)(conf::ThemType, const std::string &, QSharedPointer<to::DataStore>&);

    class ThemFormat {
    public:
      ThemFormat();
      ~ThemFormat();

      bool registerThemHandler(conf::ThemType type, CreateThem func);
      ThemAlg* createThemAlg(conf::ThemType type, const std::string &themname, QSharedPointer<to::DataStore>& store) const;

    private:
      QMap<conf::ThemType, CreateThem> _themHandler;
    };

    namespace singleton {
      typedef TSingleton<mappi::to::ThemFormat> ThemFormat;
    }

  }
}


#endif 
