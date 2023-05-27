#ifndef METEO_COMMONS_MSGDATA_MSGRECEIVER_H
#define METEO_COMMONS_MSGDATA_MSGRECEIVER_H

#include <qset.h>
#include <qmap.h>
#include <qobject.h>
#include <qstringlist.h>
#include <qmetatype.h>

#include <meteo/commons/proto/msgparser.pb.h>
#include <meteo/commons/global/appstatusthread.h>


class QTimer;

namespace meteo {

class MsgInfo;
class MsgQueue;
class MsgQueueItem;
}
namespace meteo {
class Dbi;
class GridFs;
class DbiQuery;

//! Класс MsgReceiver отвечает за усвоение ГМИ (запись в БД и ФС).
class MsgReceiver : public QObject
{
  Q_OBJECT

  enum ret_value{
    true_res,
    false_res,
    error_res,
  };

public:
  explicit MsgReceiver(QObject* parent = 0);
  virtual ~MsgReceiver();

  void setMsgPath(const QString& path) { tlgPath_ = path; }
  void setMsgQueue(MsgQueue* queue) { queue_ = queue; }
  void setStatusSender(AppStatusThread* status);
  void setSelfIds(const QStringList& ids) { selfIds_ = ids; }

signals:
  void msgProcessed(int id); //!< \deprecated
  void msgProcessed(int id, int64_t ptkppId); //!< \deprecated
  void msgProcessedError(int id, const QString& error); //!< \deprecated
  void msgProcessed(int id, bool ok, const QString& errorText);
  void msgProcessed(int id, const QString& errorText, qint64 ptkppId, bool duplicate);

public slots:
  void slotInit();
  void slotRun();

private:
  //! Запивывает телеграмму в файл
  bool saveToFile(const tlg::MessageNew& msg, const QString& path, const QString& fileName);

  QString generatePath(const tlg::MessageNew& msg) const;
  QString generateFileName(const tlg::MessageNew& msg, const QString& dt, const QString& suff = QString()) const;

  bool getGridFsDb(GridFs *gridfs, Dbi *db);
  ret_value checkDupl(Dbi *db, MsgQueueItem *item);
  void setMetric(const tlg::MessageNew& msg);

private:
  // параметры
  QString tlgPath_;
  QStringList selfIds_;

  // данные
  MsgQueue* queue_;

  int64_t id_ = -1;

  qint64 dup_ = 0;
  qint64 writeToDb_ = 0;
  QMap<std::string,int> paramId_;
  QMap<std::string,qint64> count_;
  int skipCount = 0;

  int dbErrorTimeCount_ = 0; //Время, в течении которого БД была недоступна
  static const int maxDbErrorTime = 60000; //60s, максимальное время в течении которого БД была не доступна. Превышение этого времени приводит к пробросу всех телеграмм без записи на следующий этап
  // служебные
  bool run_       = false;

  QTimer* timer_  = nullptr;
  AppStatusThread* status_ = nullptr;
};

}

Q_DECLARE_METATYPE( int64_t )

#endif
