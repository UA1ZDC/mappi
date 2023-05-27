#ifndef METEO_COMMONS_MSGDATA_MSGCREATER_H
#define METEO_COMMONS_MSGDATA_MSGCREATER_H

#include <qobject.h>

#include <meteo/commons/msgdata/msgqueue.h>

class QTimer;

namespace meteo {

class MsgCreater : public QObject
{
  Q_OBJECT

public:
  explicit MsgCreater(QObject* parent = nullptr);

signals:
  void msgProcessed(int id);

public slots:
  void slotInit();
  void slotRun();

private:
  // данные
  MsgQueue* queue_ = nullptr;

  // служебные
  bool run_ = false;

  QTimer* timer_ = nullptr;
};

} // meteo

#endif // METEO_COMMONS_MSGDATA_MSGCREATER_H
