#ifndef MAPPI_UI_ANTENNACTRL_ANTCTRL_H
#define MAPPI_UI_ANTENNACTRL_ANTCTRL_H

#include <qtreewidget.h>
#include <qwidget.h>
#include <qtablewidget.h>
#include <QTableWidgetItem>

#include <mappi/proto/reception.pb.h>
// #include <mappi/schedule/schedule.h>
// #include <mappi/schedule/session.h>
// #include <mappi/schedule/scheduler.h>
#include <mappi/device/antenna/prbl-ant.h>
#include <mappi/device/antenna/smallant.h>
#include <mappi/device/antenna/mantcoord.h>
// #include <mappi/ui/scheduletable/scheduletable.h>

class QTimer;

namespace Ui {
  class SmallAntWidget;
}


namespace mappi {
namespace receive {

  class PrblAnt;
  class CafAnt;
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
    kAzimutFix = 0,
    kElevationFix = 1,
    kSetPosition = 2,
    kAzimutInc = 3,
    kAzimutDec = 4,
    kElevationInc = 5,
    kElevationDec = 6,
    kTravelPos = 7,
    kWorkPos = 8,
    kProgon = 9,
    kNone = 10
  };

  class SmallAntWidget : public QWidget {
    Q_OBJECT
    
  public:
    SmallAntWidget(QWidget* parent = 0, Qt::WindowFlags fl = 0);
    ~SmallAntWidget();

  public slots:
    
    void setPort(const QString& port);
    void powerChanged(bool state);//int state);
    void getCurrentCoords();
    
    // void setAzimut(bool isSet);
    // void setElevation(bool isSet);
    void setRun(bool isSet);
    void setNextValue();
    
    void slotOnCoordsTimer();
    void setCirclePoint();
    
  private:

    bool checkAnt();
    void enableButtons(bool enable);
    void disableRunButtons(bool azStep = false, bool elStep = false);
    // void followForSatellite();
    // bool setStartPosition(const schedule::Session& session, bool wait);
    // void setNextPosition();
    // bool getTrackSatellite (const schedule::Session& session);
	
    void keyPressEvent(QKeyEvent* event);
    bool checkKeyEvent(QKeyEvent *event);

    void setCircle();
    bool eventFilter(QObject* obj, QEvent* ev);
    void pointToAngles(int x, int y);
    void anglesToPoint(double az, double el, float* xres, float* yres);
  private slots:
    // void slotFollow(bool isSet);
    // void slotNextSession();
    // void slotNextPosition();

    void slotSetPosition(bool isSet);
    void slotStopTimer();
    // void slotShowCorr(bool isSet);
    // void slotSetCorr();

    void slotAzInc();
    void slotAzDec();
    void slotElInc();
    void slotElDec();
    void slotSetTravel();
    void slotSetWork();
    
  private:
    Ui::SmallAntWidget* _ui;

    //    SmallAnt* _ant;
    CafAnt* _ant;
    AntProgon* _progon;
    
    // QTimer* _updateTimer;
    // QTimer* _nextPositionTimer;
    QTimer* _timer;
    QTimer* _coordsTimer;
    QTimer* _stopTimer;
    
    // mappi::conf::Reception _conffollow;
    // meteo::GeoPoint _stationCoord;    //!< координаты пункта приема
    // MnDevice::MAntCoord* _track = 0;
    // int64_t _trackIdx = 0;	//!< Текущий номер точки траектории
    // QString _tle; //!< TLE файл
    
    // текущее положение антенны
    float _azCur = 0; // азимут
    float _elCur = 0; // угол места
    
    // целевое положение антенны
    float _azDest = 0; // азимут
    float _elDest = 0; // угол места
    mappi::conf::Reception _conf;
    
    CurrentTask _curTask = CurrentTask::kNone;

    QPixmap* _pix;
    float _scale_az = 1;
    float _scale_el = 1;
  };

}
}

#endif
