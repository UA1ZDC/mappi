#include "ptkppgeo.h"

#include <qfile.h>
#include <qprocess.h>
#include <qdir.h>
#include <qvariant.h>
#include <qsettings.h>
#include <qfileinfo.h>
#include <qtextcodec.h>
#include <qpair.h>

#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/paths.h>
#include <commons/geobasis/geopoint.h>
#include <commons/geobasis/geovector.h>
#include <commons/geobasis/projection.h>
#include <meteo/commons/ui/map/geopolygon.h>
#include <meteo/commons/ui/map/map.h>
#include <meteo/commons/ui/map/layer.h>
#include <meteo/commons/ui/map/geotext.h>
#include <meteo/commons/ui/map/geopolygon.h>
#include <meteo/commons/ui/map/document.h>
#include <meteo/commons/ui/map/loader.h>
#include <meteo/commons/global/common.h>
#include <meteo/commons/fonts/weatherfont.h>
#include <commons/textproto/tprototext.h>
#include <meteo/commons/proto/map_isoline.pb.h>
#include <meteo/commons/global/global.h>

namespace {
  const QString kLoaderType("ptkpp");

  static bool res = meteo::map::Loader::instance()->registerDataHandler(kLoaderType, &meteo::map::ptkppgeo::loadGeoData );
}

namespace meteo {
namespace map {
namespace ptkppgeo {

QMap<QString, meteo::Property> geocolors;
QMap<QString, meteo::map::proto::CitySettings> citymap;

const QString kMapSettingsFile( MnCommon::etcPath("meteo") + "/map.settings.conf" );

meteo::map::proto::MapSettings mapSettingsOld()
{
  meteo::map::proto::MapSettings setts;
  QFile file(kMapSettingsFile);
  if ( false == file.open( QIODevice::ReadOnly ) ) {
    error_log << QObject::tr("Не могу открыть файл %1 для чтения")
      .arg(kMapSettingsFile);
    return setts;
  }
  QString str = QString::fromLocal8Bit( file.readAll() );
  meteo::map::proto::MapSettingsConfig mcfg;
  if ( false == TProtoText::fillProto( str, &mcfg) ) {
    error_log << QObject::tr("Не удалось загрузить настройки из файла %1")
      .arg(kMapSettingsFile);
  }
  setts.CopyFrom(mcfg.settings());
  return setts;
}

void loadMap( Document* doc, const char* arr )
{
  QList<Layer*> nonBase;
  for ( auto l : doc->layers() ) {
    nonBase.append(l);
  }
  doc->parseFromArray(arr);
  doc->setName("");
  for ( auto l : doc->layers() ) {
    if ( true == nonBase.contains(l) ) {
      continue;
    }
    l->setBase(true);
    l->setCache(true);
  }
  return;
}

bool loadGeoData( Document* doc )
{
  if ( 0 == doc ) {
    error_log << QObject::tr("Нулевой указатель");
    return false;
  }
  QString path = MnCommon::sharePath("meteo") + "/geo/sxf.map_ptkpp";
  QFile file(path);
  if ( false == file.exists() ) {
    error_log.msgBox() << QObject::tr("Не найден файл с географической основой %1")
      .arg(path);
    return false;
  }
  if ( false == file.open( QIODevice::ReadOnly ) ) {
    error_log.msgBox() << QObject::tr("Не удалось открыть файл для для чтения %1")
      .arg( file.fileName() );
    return false;
  }
  QByteArray arr = file.readAll();

  loadMap( doc, arr.data() );

  return true;
}

}
}
}
