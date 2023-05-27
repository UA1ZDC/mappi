#include "sxf.h"

#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/paths.h>
#include <meteo/commons/ui/map/map.h>
#include <meteo/commons/ui/map/layer.h>
#include <meteo/commons/ui/map/loader.h>
#include <meteo/commons/ui/map/dataexchange/sxftomap.h>
#include <meteo/commons/global/global.h>
#include <meteo/commons/global/log.h>
#include <commons/textproto/tprototext.h>
#include <meteo/commons/proto/sxfloader.pb.h>

#define CONF_FILE MnCommon::etcPath("meteo") + "/sxfloader.conf"

namespace {
  const QString kLoaderType("SXF");

  static bool res = meteo::map::Loader::instance()->registerDataHandler(kLoaderType, &meteo::map::sxf::loadGeoData );
}

namespace meteo {
  namespace map {

namespace sxf {
  bool readFile(const QString& fileName, QByteArray *ba)
  {
    QFile file(fileName);
    if(!file.open(QIODevice::ReadOnly)) {
      error_log << msglog::kFileOpenError.arg(fileName).arg(file.error());
      return false;
    }
    *ba = file.readAll();
    
    file.close();
    
    return true;
  }

  bool readConfig(meteo::map::sxf::SxfLoaderConf* conf)
  {
    if (0 == conf) {
      return false;
    }
    
    QFile file(CONF_FILE);
    if ( !file.open(QIODevice::ReadOnly) ) {
      error_log << msglog::kFileOpenError.arg(CONF_FILE).arg(file.error());
      return false;
    }
  
    QString text = QString::fromUtf8(file.readAll());
    file.close();
  
    if ( !TProtoText::fillProto(text, conf) ) {
      error_log << msglog::kFileReadFailed.arg(CONF_FILE).arg(file.error());
      return false;
    }

    return true;
  }

  bool loadGeoData( Document* doc )
  {
    if ( 0 == doc ) return false;
    meteo::map::sxf::SxfLoaderConf conf;
    if (!readConfig(&conf)) {
      return false;
    }

    meteo::SxfToMap sxfmap;

    QByteArray rscData;
    if (!readFile(MnCommon::projectPath() + QString::fromStdString(conf.rsc_path()), &rscData)) {
    return false;
  }

  if (!sxfmap.setRscData(rscData)) {
    return false;
  }
    
  QByteArray sxfData;
  if (!readFile(MnCommon::projectPath() + QString::fromStdString(conf.sxf_path()), &sxfData)) {
    return false;
  }

  if (!sxfmap.readSxf(sxfData)) {
    return false;
  }

  QStringList layerNums;
  for (int i = 0; i < conf.layers_size(); i++) {
    layerNums << QString::number(conf.layers(i));
  }
  //  var(layerNums);
  if (!sxfmap.import(layerNums, doc ) ) {
    return false;
  }
  
  return true;
}



}
  }
}
