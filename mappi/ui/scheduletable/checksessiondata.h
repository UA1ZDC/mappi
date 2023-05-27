#ifndef MAPPI_UI_SCHEDULETABLE_CHECKSESSIONDATA_H
#define MAPPI_UI_SCHEDULETABLE_CHECKSESSIONDATA_H

#include <qobject.h>

#include <mappi/schedule/schedule.h>

namespace mappi {

class CheckSessionData : public QObject {
  Q_OBJECT
  using Session = schedule::Session;
public:
  struct SessionState {
    unsigned long int revol;
    QString satName;
    bool state;
  };

public:
  CheckSessionData(QObject* parent = nullptr) : QObject(parent) { }
  ~CheckSessionData() { }
  void setSessionList(QList<Session> lst);
  void run();
  void interrupt();
  bool isWorking() const;
signals:
  void complete(const SessionState& state);
  void finished();

private:



  QList<Session> lst_;
  bool working_;
  bool interrupt_;
};

} // mappi

#endif // MAPPI_UI_SCHEDULETABLE_CHECKSESSIONDATA_H
