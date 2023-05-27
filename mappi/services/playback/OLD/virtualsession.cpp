#include "virtualsession.h"

#include <mappi/schedule/schedulehelper.hpp>

using namespace mappi;
using namespace receive;

VirtualSession::VirtualSession()
{
}


VirtualSession::~VirtualSession()
{
}


// QDateTime VirtRecvServ::currentDt() const {
//   QDateTime dt = near().getAos().addSecs(-60);
//   return dt;
// }      

schedule::Session VirtualSession::getNextNear(const schedule::Schedule& sched)
{
  schedule::Session snear = near();
  if (snear.isDefaults()) {
    snear = SessionManager::getNextNear(sched);
  } else {
    QDateTime dt;
    //повторяем тот же сеанс, пока до него не дойдёт реальное время. Если убрать условие, то будем бежать вперед по расписанию по кругу. NOTE в гуи тогда не согласуется, надо разбираться
    // if (QDateTime::currentDateTimeUtc() < snear.getAos()) {
    //   dt = snear.getAos().addSecs(-10);
    // } else {
      dt = snear.getLos().addSecs(60);
      //    }
    snear = sched.nearSession(dt, true);
    debug_log << "near" << snear.getNameSat() << sched.countSession() << snear.isDefaults();

    //обновляем расписание полностью, если дошли до конца
    if (snear.isDefaults()) {
      recreateSchedule();
      
      return snear;
    }
  }

  //следующий запуск через 30 секунд
  setOffsetTime(QDateTime::currentDateTimeUtc().secsTo(snear.getAos().addSecs(-30)));
  debug_log << "near" << snear.getNameSat() << QDateTime::currentDateTimeUtc().secsTo(snear.getAos().addSecs(-30));
  
  return snear;
}

void VirtualSession::stopProcess()
{
  stopProcessWithDt();
}
