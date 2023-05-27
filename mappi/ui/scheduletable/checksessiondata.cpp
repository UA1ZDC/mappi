#include "checksessiondata.h"

#include <meteo/commons/rpc/rpc.h>
#include <meteo/commons/global/global.h>

#include <mappi/schedule/schedule.h>
#include <mappi/proto/sessiondataservice.pb.h>

namespace mappi {




void CheckSessionData::run() {




/*
  interrupt_ = false;
  working_ = true;
  auto* ch = meteo::global::serviceChannel(meteo::settings::proto::kDataControl);

  while(!lst_.empty() && !interrupt_) {
    bool good = true;
    auto session = lst_.takeLast();
    proto::ThematicData req;
    req.set_name("grayscale");
    QString tmp = session.aos.addSecs(-5 * 60).toString(Qt::ISODate).replace('T', ' ');
    req.set_date_start(tmp.toStdString());
    tmp = session.los.addSecs(5 * 60).toString(Qt::ISODate).replace('T', ' ');
    req.set_date_end(tmp.toStdString());
    req.set_satellite_name(session.name.toStdString());
    if (nullptr != ch) {
      auto* resp = ch->remoteCall(&mappi::proto::SessionDataService::GetAvailableThematic, req, 5000, true);
      if(nullptr != resp)
      {
        good &= (resp->themes_size() > 0);
        delete resp;
      }
      else good &= false;
    }
    SessionState state{ session.getRevol(), session.getNameSat(), good };
    Q_EMIT(complete(state));
  }
  working_ = false;
  Q_EMIT(finished());
  delete ch;
*/
}

bool CheckSessionData::isWorking() const { return working_; }

void CheckSessionData::interrupt() { interrupt_ = true; }

void CheckSessionData::setSessionList(QList<schedule::Session> lst) { lst_ = lst; }

} // mappi
