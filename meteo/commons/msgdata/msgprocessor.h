#ifndef METEO_COMMONS_MSGDATA_MSGPROCESSOR_H
#define METEO_COMMONS_MSGDATA_MSGPROCESSOR_H

#include <bson.h>

#include <qobject.h>

class QTimer;

namespace meteo {
namespace tlg {

class MessageNew;

} // tlg
} // meteo

namespace meteo {

class MsgQueue;

class MsgProcessor : public QObject
{
  Q_OBJECT

public:
  explicit MsgProcessor(QObject* parent = nullptr);

  void setWmoId(const QString& id) { wmoId_ = id; }
  void setHmsId(const QString& id) { hmsId_ = id; }

  void setQueue(MsgQueue* queue) { queue_ = queue; }

signals:
  void msgProcessed(int id);

public slots:
  void slotInit();
  void slotRun();

private:
  bool processHmiRequest(const tlg::MessageNew& msg);
  bool processHmiData(const tlg::MessageNew& msg);
  bool processConfirmMsg(const tlg::MessageNew& msg);

  bool queryFromAhd(const QString& ahd, bson_t* q, bson_t* opts) const;

private:
  // данные
  QString wmoId_;
  QString hmsId_;

  MsgQueue* queue_  = nullptr;

  // служебные
  bool run_       = false;
  QTimer* timer_  = nullptr;
};

} // meteo

#endif // METEO_COMMONS_MSGDATA_MSGPROCESSOR_H
