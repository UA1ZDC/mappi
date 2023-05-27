#ifndef METEO_SBOR_UI_MSGLOADER_LOADER_H
#define METEO_SBOR_UI_MSGLOADER_LOADER_H

#include <qobject.h>
#include <qdir.h>
#include <qdatetime.h>
#include <meteo/commons/rpc/rpc.h>

class QDateTime;

namespace meteo {
namespace msgcenter {
class ProcessMsgReply;
}


class Loader : public QObject
{
  Q_OBJECT
public:
  explicit Loader(QObject *parent = nullptr);
  ~Loader();

signals:
  void signalAddDirs(QString);
  void signalAddFiles(QString);
  void signalTableItemIncrement(int);
  void signalUpdateDT(QDateTime,QDateTime);
  void signalFileIncrement();
  void finished();

public slots:
  void slotSetYMD(int year, int month, int day);
  void slotSetMaskFileList(QStringList maskFileList);
  void slotSetTemplList(QStringList templList);
  void slotSetDir(QDir dir);
  void slotRun();
  void slotStop();

private:
  void recursiveLoadFromDir(const QDir& directory);
  void fileHandler(const QString& absolutePath);
  bool createConnection();
  bool checkFileName(QString& name);
  void imageTlg(const QString& absolutePath);

private slots:
  void slotProcessed(msgcenter::ProcessMsgReply* r);

private:
  QDir dir_;
  rpc::Channel* msgServ_ = nullptr;
  QStringList maskFileList_;
  QStringList templList_;
  QDateTime dts_;
  QDateTime dte_;
  int year_ = 0;
  int month_ = 0;
  int day_ = 0;
  bool run_ = false;
};

}
#endif // METEO_SBOR_UI_MSGLOADER_LOADER_H
