#include "elevation.h"

#include <qfile.h>

#include <cross-commons/app/paths.h>
#include <cross-commons/debug/tlog.h>

namespace meteo {

bool Elevation::init()
{
  QString path = MnCommon::sharePath("meteo") + QString("/landmask/globe/");
  for ( int i = 0; i < 16; ++i ) {
    pos_[i] = -1;
    value_[i] = -9999;
    tiles_[i].setFileName(path + kTileInfo[i].fileName);
  }

  for ( int i = 0; i < 16; ++i ) {
    if ( !tiles_[i].exists() ) {
      return false;
    }
  }

  return true;
}

int Elevation::value(double lat, double lon) const
{
  int x = (lon + 180) / 90;

  int idx = -1;
  if ( lat > 50 ) {
    idx = x;
  }
  else if ( lat > 0 ) {
    idx = 4 + x;
  }
  else if ( lat > -50 ) {
    idx = 8 + x;
  }
  else {
    idx = 12 + x;
  }

  if ( idx < 0 || idx > 15 ) {
    return -9999;
  }

  if ( !tiles_[idx].isOpen() && !tiles_[idx].open(QFile::ReadOnly) ) {
    error_log << QObject::tr("Не удалось открыть файл %1.").arg(tiles_[idx].fileName())
              << tiles_[idx].errorString();
    return -9999;
  }

  int i = qRound( (lon - kTileInfo[idx].begLon) / kLonStep );
  int j = kTileInfo[idx].rows - qRound( (lat - kTileInfo[idx].begLat) / kLatStep );

  uint pos = (j * kTileInfo[idx].columns + i) * 2;

  if ( pos == pos_[idx] ) {
    return value_[idx];
  }

  pos_[idx] = pos;
  tiles_[idx].seek(pos);
  tiles_[idx].read(reinterpret_cast<char*>(value_+idx), 2);

  return value_[idx];
}

} // meteo
