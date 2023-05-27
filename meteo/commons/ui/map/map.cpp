#include "map.h"

#include <qglobal.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qsettings.h>

#include <cross-commons/debug/tlog.h>
#include <meteo/commons/global/common.h>

#include "layer.h"
#include "document.h"
#include "object.h"
#include "geopolygon.h"
#include "geotext.h"
#include "loader.h"

namespace meteo {
namespace map {

bool loadGeoLayers( Document* doc )
{
  if ( 0 == doc->projection() ) {
    error_log << QObject::tr("Невозможно загрузить слои географической основы. Параметры проекции не установлены.");
    return false;
  }
  if ( false == doc->property().has_geoloader() ) {
//    error_log << QObject::tr("Невозможно загрузить слои географической основы. Параметры загрузки не установлены.");
    return false;
  }
  bool res = meteo::map::Loader::instance()->handleData( QString::fromStdString( doc->property().geoloader() ), doc );
  return res;
}

}
}
