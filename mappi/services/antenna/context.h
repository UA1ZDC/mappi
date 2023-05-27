#pragma once

#include <cross-commons/debug/tlog.h>
#include <sat-commons/satellite/satellite.h>
#include <mappi/schedule/session.h>
#include <mappi/device/antenna/antenna.h>
#include <mappi/device/antenna/direct.h>
#include <mappi/device/antenna/tracker.h>
#include <mappi/device/antenna/monkeytest.h>


namespace mappi {

namespace antenna {

class Context :
  public QObject
{
  Q_OBJECT
public :
  explicit Context(QObject* parent = nullptr);
  virtual ~Context();

  bool init();

  QDateTime now() const;
  bool isReplay() const;

  QString lastError();
  void setLastError(const QString& msg);
  bool hasError() const;

public :
  Antenna* antenna;     // QObject
  Direct* direct;       // QObject
  Tracker* tracker;     // QObject
  MonkeyTest* mTest;    // QObject
  Configuration conf;

  // предварительный сеанс
  schedule::Session sessionOriginal_prep;
  schedule::Session session_prep;
  conf::TleItem tleItem_prep;

  // исполнительный сеанс
  schedule::Session sessionOriginal;
  schedule::Session session;
  conf::TleItem tleItem;

  Satellite satellite;
  QList<MnSat::TrackTopoPoint> track;

private :
  QString error_;
};

}

}
