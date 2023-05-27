#ifndef COMMONS_METEO_DATA_TEST_TEST_H
#define COMMONS_METEO_DATA_TEST_TEST_H

#include <commons/meteo_data/tmeteodescr.h>

#include <qobject.h>
#include <qthread.h>


class Worker : public QObject
{
  Q_OBJECT
public:
  Worker(const QStringList& names, int idx) { _names = names; _idx = idx;}
  virtual ~Worker() {}
  
public slots:
  void doWork() {
    for (int idx = 0; idx < _names.size(); idx++) {
      QString complname;
      TMeteoDescriptor::instance()->isComponent(_names.at(idx), &complname);
      debug_log << _idx << _names.at(idx) << complname;
    }
  }

private:
  QStringList _names;
  int _idx;
  
};

class Controller : public QObject
{
  Q_OBJECT
public:
  Controller() {
  }
  virtual ~Controller() {
  }

  void createThread(const QStringList& names, int idx)
  {
    QThread* thread = new QThread;
    Worker *worker = new Worker(names, idx);
    worker->moveToThread(thread);
    connect(thread, &QThread::finished, worker, &QObject::deleteLater);
    connect(this, &Controller::operate, worker, &Worker::doWork);
    connect(thread, &QThread::started, worker, &Worker::doWork);
    thread->start();
  }

public slots:
  // void handleResults(const QString &);
signals:
  void operate(const QString &);
private:
  
};

#endif
