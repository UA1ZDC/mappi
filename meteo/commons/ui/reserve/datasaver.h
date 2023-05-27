#ifndef METEO_UKAZ_RESERVE_DATASAVER_H
#define METEO_UKAZ_RESERVE_DATASAVER_H

#include <qobject.h>
#include <qstringlist.h>

namespace meteo{

class DataSaver : public QObject
{
  Q_OBJECT
public:
  DataSaver(const QStringList& savepath, const QString& pgflag, const QString& restorepath, const QString& host = 0);
  DataSaver(const QStringList& savepath, const QStringList& dbs, const QString& restorepath, const QString& host = 0);
  ~DataSaver();

  void setDropBeforeRestore(bool dbr) { dropBeforeRestore_ = dbr;}

private:
  void mongodump();
  void mongorestore();
  void make_tar();
  void extract();
  bool exec_app(const QString& app, const QStringList& arg);

  QStringList savePath_;
  QString pgFlag_;
  QStringList dbs_;
  QString workPath_;
  QString host_;
  bool dropBeforeRestore_ = false;

public slots:
  void save();
  void restore();
  void setRestoreDir(const QString& path){workPath_ = path;}

private slots:
  void slotReadError();
  void slotReadOutput();

signals:
  void newMessage(const QString& text);
  void saveComplete();
  void restoreComplete();
};

}

#endif // METEO_UKAZ_RESERVE_DATASAVER_H
