#pragma once

#include "antenna.h"
#include "script.h"
#include "catcher.h"
#include "direct.h"
#include "tracker.h"
#include "monkeytest.h"
#include <qlist.h>


namespace mappi {

namespace antenna {

/**
 * Цепочка сценариев, позволяет:
 * - исключить конфликт при запуске сценариев (чтобы одновременно не были запущено сразу несколько сценариев);
 * - запустить последовательность сценариев (сценарий, который не завершается, должен стоять в цепочки последним).
 *
 * !!! Цепочку нельзя использовать для автоматического режима.
 */
class ScriptChain :
  public QObject
{
  Q_OBJECT
public :
  explicit ScriptChain(Antenna* antenna);
  virtual ~ScriptChain();

  void appendCatcher();
  void appendDirect(float azimut_deg, float elevat_deg, float azimut_speed_deg, float elevat_speed_deg, bool alreadyCorrect = false);
  void appendDirect(const MnSat::TrackTopoPoint& point, bool alreadyCorrect = false);
  void appendMonkeyTest(float dsa, float dse);
  void appendTracker(const QList<MnSat::TrackTopoPoint>& track, const QDateTime& timeStamp);
  void appendTracker(const QList<MnSat::TrackTopoPoint>& track);

  void run();
  void terminate();

  [[deprecated("Use TrackTopoPoint or 4-argument version instead.")]]
  void appendDirect(float azimut_deg, float elevat_deg, bool alreadyCorrect = false){
      appendDirect(azimut_deg, elevat_deg, 0, 0, alreadyCorrect);
  }

signals :
  void completed();     // сценарий заверщен
  void failed();        // ошибка при выполнении сценария

private slots :
  void onFinished();

private :
  Antenna* antenna_;    // QObject

  Catcher* catcher_;
  Tracker* tracker_;
  Direct* direct_;
  MonkeyTest* monkey_;

  QList<Script::id_t> chain_;
};

}

}
