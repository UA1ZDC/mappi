#ifndef MAPPI_SERVICES_SESSIONMANAGER_VIRTUALSESSION_H
#define MAPPI_SERVICES_SESSIONMANAGER_VIRTUALSESSION_H

#include <mappi/services/sessionmanager/sessionmanager.h>

//для иммитации приёма. время подстраивается под следующий сеанс, перерыв между ними 1 минута

namespace mappi {
  namespace receive {
    
    class VirtualSession : public SessionManager {
    public:
      VirtualSession();
      ~VirtualSession();
      
      //      QDateTime currentDt() const;
      virtual schedule::Session getNextNear(const schedule::Schedule& sched);

      virtual void stopProcess();
    };

  }
}

#endif
