#ifndef AUTOMAP_H
#define AUTOMAP_H

#include "joblist.h"
#include "automapmodel.h"
#include <QSignalMapper>


namespace Ui {
  class AutoMap;
}

namespace meteo {

namespace map {

class CursorGuard {
  Q_DISABLE_COPY(CursorGuard)

public :
  CursorGuard();
  ~CursorGuard();
};


class AutoMap :
  public QWidget
{
  Q_OBJECT
public :
  explicit AutoMap(QWidget* parent = nullptr);
  virtual ~AutoMap();

protected :
  virtual void closeEvent(QCloseEvent* event);
  virtual void timerEvent(QTimerEvent* event);

private slots :
  void load();
  void save();

  void jobAdd();
  void jobCopy();
  void jobEdit();
  void jobEdit(const QModelIndex& index);
  void jobDelete();

  void slotOnDocListInit();

  void mapOpen(const QString& name);
  void docOpen(const QString& name);
  void docDownload(const QString& name);
  void slotRunJob(const QString& name);

private :
  void initAction();
  rpc::Channel* channel();
  QModelIndex currentIndex();
  void setCurrentIndex(int row);
  void setIndexWidget(int row);

private :
  Ui::AutoMap* ui_;
  int timerSortId_;
  int timerWaitId_;

  JobList* jobList_;
  AutoMapModel* model_;
  AutoMapSortProxyModel* proxyModel_;

  QSignalMapper* smMap_;
  QSignalMapper* smDoc_;
  QSignalMapper* smDownload_;
  QSignalMapper* smRunJob_;

  QAction* jobAdd_;
  QAction* jobEdit_;
  QAction* jobCopy_;
  QAction* jobDelete_;
  QAction* jobSave_;
  QActionGroup* jobGroup_;
};

}

}

#endif
