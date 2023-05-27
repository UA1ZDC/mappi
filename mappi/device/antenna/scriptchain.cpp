#include "scriptchain.h"
#include <qthread.h>
#include <QMetaEnum>


namespace mappi {

namespace antenna {

ScriptChain::ScriptChain(Antenna* antenna) :
    QObject(antenna),
  antenna_(antenna),
  catcher_(new Catcher(this)),
  tracker_(new Tracker(this)),
  direct_(new Direct(this)),
  monkey_(new MonkeyTest(this))
{
  QObject::connect(catcher_, &Catcher::finished, this, &ScriptChain::onFinished);
  QObject::connect(tracker_, &Tracker::finished, this, &ScriptChain::onFinished);
  QObject::connect(direct_, &Direct::finished, this, &ScriptChain::onFinished);
  QObject::connect(monkey_, &MonkeyTest::finished, this, &ScriptChain::onFinished);
}

ScriptChain::~ScriptChain()
{
  antenna_ = nullptr;
}

void ScriptChain::appendCatcher()
{
  chain_.append(Script::CATCHER);
}

void ScriptChain::appendDirect(float azimut_deg, float elevat_deg, float azimut_speed_deg,
                  float elevat_speed_deg, bool alreadyCorrect /*=false*/){
    direct_->setPosition(azimut_deg, elevat_deg, azimut_speed_deg, elevat_speed_deg, alreadyCorrect);
    chain_.append(Script::DIRECT);
}
void ScriptChain::appendDirect(const MnSat::TrackTopoPoint& point, bool alreadyCorrect /*=false*/){
    direct_->setPosition(point, alreadyCorrect);
    chain_.append(Script::DIRECT);
}

void ScriptChain::appendMonkeyTest(float dsa, float dse)
{
  monkey_->setDSA(dsa);
  monkey_->setDSE(dse);
  chain_.append(Script::MONKEY_TEST);
}

void ScriptChain::appendTracker(const QList<MnSat::TrackTopoPoint>& track, const QDateTime& timeStamp)
{
  tracker_->setTrack(track, timeStamp);
  chain_.append(Script::TRACKER);
}

void ScriptChain::appendTracker(const QList<MnSat::TrackTopoPoint>& track)
{
  tracker_->setTrack(track);
  chain_.append(Script::TRACKER);
}

void ScriptChain::run()
{
  onFinished();
}

void ScriptChain::terminate()
{
  catcher_->terminate();
  tracker_->terminate();
  direct_->terminate();
  monkey_->terminate();

  chain_.clear();
}

void ScriptChain::onFinished()
{
  if (chain_.isEmpty()) {
    emit completed();   // emit signal
    return ;
  }

  Script* script = nullptr;
  switch (chain_.takeFirst()) {
    case Script::CATCHER :     script = catcher_; break ;
    case Script::TRACKER :     script = tracker_; break ;
    case Script::DIRECT :      script = direct_;  break ;
    case Script::MONKEY_TEST : script = monkey_;  break ;

    default :
      break ;
  }

  if (script != nullptr) {
    QThread::usleep((1000 - QTime::currentTime().msec()) * 1000);       // msec -> 000
    if (script->run(antenna_)) {
      info_log << QObject::tr("run script: %1")
        .arg(QMetaEnum::fromType<mappi::antenna::Script::id_t>().valueToKey(script->id()));

      return ;
    }
  }

  emit failed();        // emit signal
}

}

}
