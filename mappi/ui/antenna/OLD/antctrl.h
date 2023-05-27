#ifndef MAPPI_UI_ANTENNACTRL_ANTCTRL_H
#define MAPPI_UI_ANTENNACTRL_ANTCTRL_H

#include <qtreewidget.h>
#include <qwidget.h>
#include <qtablewidget.h>
#include <QTableWidgetItem>

#include <mappi/proto/reception.pb.h>
#include <mappi/schedule/schedule.h>
#include <mappi/schedule/session.h>
// #include <mappi/device/antenna/prbl-ant.h>
// #include <mappi/device/antenna/mantcoord.h>
#include <mappi/ui/scheduletable/scheduletable.h>

class QTimer;

namespace Ui {
class AntCtrl;
}


namespace mappi {
namespace receive {

class PrblAnt;
class AntProgon;
enum Column {
  kStateCol    = 0,
  kAosCol      = 1,
  kLosCol      = 2,
  kNameCol     = 3,
  kCulminCol   = 4,
  kDurationCol = 5,
  kDirectCol   = 6,
  kRevol       = 7,
};

enum CurrentTask {
  kAzimutFix,
  kElevationFix,
  kRun,
  kSetPosition,
  kNone
};

class AntCtrl : public QWidget {
  Q_OBJECT

public:
  AntCtrl(QWidget* parent = 0, Qt::WindowFlags fl = 0);
  ~AntCtrl();

public slots:

  void setPort(const QString& port);
  void powerChanged(bool state);//int state);
  void getCurrentCoords();

  void setAzimut(bool isSet);
  void setElevation(bool isSet);
  void setRun(bool isSet);
  void setNextValue();

  void slotOnCoordsTimer();

private:

  void enableButtons(bool enable);
  void followForSatellite();
  bool setStartPosition(const schedule::Session& session, bool wait);
  void setNextPosition();
  bool getTrackSatellite (const schedule::Session& session);


private slots:
  void slotFollow(bool isSet);
  void slotNextSession();
  void slotNextPosition();
  void slotUpdate();
  void slotVisibleTableBox();
  void slotSetPosition(bool isSet);
  void slotStopTimer();
  void slotShowCorr(bool isSet);
  void slotSetCorr();
  void slotChangedNear(std::shared_ptr<schedule::Session> near);



private:
  Ui::AntCtrl* _ui;

  //PrblAnt* _ant;
  //AntProgon* _progon;

  ScheduleTable* _scheduleTable;

  std::shared_ptr<schedule::Session> _near = nullptr;

  QTimer* _updateTimer;
  QTimer* _nextPositionTimer;
  QTimer* _timer;
  QTimer* _coordsTimer;
  QTimer* _stopTimer;

  mappi::conf::Reception _conffollow;
  meteo::GeoPoint _stationCoord;    //!< координаты пункта приема
  //MnDevice::MAntCoord* _track = 0;
  int64_t _trackIdx = 0;	//!< Текущий номер точки траектории
  QString _tle; //!< TLE файл

  // текущее положение антенны
  float _azCur = 0; // азимут
  float _elCur = 0; // угол места

  // целевое положение антенны
  float _azDest = 0; // азимут
  float _elDest = 0; // угол места
  mappi::conf::Reception _conf;

  CurrentTask _curTask;

  bool  _showComplitedSeans; //!< Флаг показать/скрыть выполненые сеансы
};

}
}

#endif
