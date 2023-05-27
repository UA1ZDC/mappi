#ifndef METEO_COMMONS_MSGCENTER_H
#define METEO_COMMONS_MSGCENTER_H

#include <qobject.h>
#include <qtimer.h>
#include <meteo/commons/proto/msgparser.pb.h>

namespace meteo {

class MsgCenter;

class NotConfirmedChecker : public QObject
{
  Q_OBJECT
  public:
    NotConfirmedChecker( MsgCenter* srv );
    ~NotConfirmedChecker();

  private:
    void loadUnconfimedMessages();
    bool makeConfirmation(const tlg::MessageNew& msg, tlg::MessageNew* confirm);

  private slots:
    void slotTimeout();

  private:
    MsgCenter* service_ = nullptr;
    QTimer* tm_ = nullptr;
};

}

#endif
