#ifndef JOBLIST_H
#define JOBLIST_H

#include <meteo/commons/global/global.h>
#include <meteo/commons/global/weatherloader.h>
#include <meteo/commons/proto/weather.pb.h>


namespace meteo {

namespace map {

class Period
{
public :
  Period(const proto::Job& job);
  ~Period();

  QDateTime nearest() const;
  QDateTime last() const;
  qint64 wait() const;

private :
  QDateTime alignDateTime(const QDateTime& cdt) const;

private :
  int32_t period_;
  int32_t await_;
  int32_t cron_;
  QDateTime dt_;
};


class JobWrapper
{
public :
  typedef enum {
    MAP_FORMED = 0,
    MAP_NOT_FORMED
  } state_t;

  typedef QSharedPointer<JobWrapper> ptr_t;

  static QString name(const proto::Job& job);
  static QString title(const proto::Job& job);
  static QString fileExt(const proto::Job& job);

public :
  JobWrapper(const proto::Job& job);
  ~JobWrapper();

  void updateState(rpc::Channel* ch);
  QByteArray download(rpc::Channel* ch);

  QString name() const;
  QString title() const;

  Period currentPeriod() const;

  const Period& fixedPeriod() const;
  void ajustFixedPeriod();

  state_t getState() const { return state_; }
  void setState(state_t v) { state_ = v; }

  proto::Job& raw() { return job_; }
  const proto::Job& raw() const { return job_; }

private :
  state_t state_;
  Period fixedPeriod_;
  proto::Job job_;
};


class JobList
{
public :
  JobList();
  ~JobList();

  // client stub
  void load(rpc::Channel* ch);
  void update(rpc::Channel* ch);
  void save(rpc::Channel* ch);

  JobWrapper::ptr_t get(int index);
  JobWrapper::ptr_t getByName(const QString& name, int* index = nullptr);

  int count() const { return data_.count(); }
  bool isChanged() const { return isChanged_; }

  void append(const proto::Job& job);

  void insert(int index, const proto::Job& job);
  void remove(int index);
  void update(int index, const proto::Job& job);
  void setEnabled(int index, bool enable);

private :
  bool isChanged_;
  QList<JobWrapper::ptr_t> data_;
  QSet<QString> dataCache_;
};

}

}

#endif // JOBLIST_H
