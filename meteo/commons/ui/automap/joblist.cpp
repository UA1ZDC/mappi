#include "joblist.h"
#include <meteo/commons/proto/document_service.pb.h>


namespace meteo {

namespace map {

// class Period
Period::Period(const proto::Job& job) :
  period_(job.period() * 60),   // min -> sec
  await_(job.await() * 60),     // min -> sec
  cron_(job.cron_shift() * 60),
  dt_(QDateTime::currentDateTimeUtc())
{


}

Period::~Period()
{
}

QDateTime Period::nearest() const
{
  QDateTime cdt(dt_);
  if (period_ == 0)
    return cdt;

  cdt = cdt.addSecs(cron_);//cdt = cdt.addSecs(-await_);
  cdt = alignDateTime(cdt);
  cdt = cdt.addSecs(period_ + cron_);//cdt = cdt.addSecs(period_ + await_);
  cdt.setTimeSpec(Qt::UTC);

  return cdt;
}

QDateTime Period::last() const
{
  QDateTime cdt(dt_);
  if (period_ == 0)
    return cdt;

  cdt = cdt.addSecs(-cron_);//cdt = cdt.addSecs(-await_);
  cdt = alignDateTime(cdt);
  cdt = cdt.addSecs(cron_);//cdt = cdt.addSecs(await_);
  cdt.setTimeSpec(Qt::UTC);

  return cdt;
}

qint64 Period::wait() const
{
  return dt_.secsTo(nearest());
}

QDateTime Period::alignDateTime(const QDateTime& cdt) const
{
  int alignSec = (QTime(0, 0, 0).secsTo(cdt.time()) / period_) * period_;
  return QDateTime(cdt.date(), QTime(0, 0, 0).addSecs(alignSec));
}


// class JobWrapper
QString JobWrapper::name(const proto::Job& job)
{
  return QString::fromStdString(job.name()).remove(".job");
}

QString JobWrapper::title(const proto::Job& job)
{
  return QString("%1 %2")
      .arg(QString::fromStdString(job.map().title()))
      .arg(QString::fromStdString(job.title()));
}

QString JobWrapper::fileExt(const proto::Job& job) {
  switch (job.format()) {
    case proto::kJpeg : return "jpeg";
    case proto::kBmp : return "bmp";
    case proto::kPng : return "png";
    case proto::kSxf : return "sxf";
    case proto::kPtkpp : return "ptkpp";

    default :
      break ;
  }

  return "png";
}

JobWrapper::JobWrapper(const proto::Job& job) :
  state_(JobWrapper::MAP_NOT_FORMED),
  fixedPeriod_(job),
  job_(job)
{
}

JobWrapper::~JobWrapper()
{
}

void JobWrapper::updateState(rpc::Channel* ch)
{
  if (ch == nullptr)
    return ;

  proto::MapRequest req;
  req.set_job_name(job_.name());

  QString last = currentPeriod().last().toString(Qt::ISODate);
  req.set_date_start(last.toStdString());
  req.set_date_end(last.toStdString());

  QScopedPointer<proto::MapList> resp(ch->remoteCall(&proto::DocumentService::GetAvailableDocuments, req, 100000));
  if (resp.isNull()) {
    error_log << QObject::tr("Информация о документе недоступна");
    return;
  }

  if (!resp->result() || resp->map_size() == 0)
    state_ = JobWrapper::MAP_NOT_FORMED;
  else
    state_ = JobWrapper::MAP_FORMED;

}

QByteArray JobWrapper::download(rpc::Channel* ch)
{
  QByteArray data;

  if (ch == nullptr)
    return data;

  QString id;
  {
    proto::MapRequest req;
    req.set_job_name(job_.name());
    QScopedPointer<proto::MapList> resp(ch->remoteCall(&proto::DocumentService::GetAvailableDocuments, req, 100000));
    if (resp.isNull()) {
      error_log.msgBox() << QObject::tr("Сервис документов не доступен");
      return data;
    }

    if (!resp->result() || resp->map_size() == 0) {
      error_log.msgBox() << QObject::tr("Нет документов данного типа");
      return data;
    }

    id = QString::fromStdString(resp->map(0).idfile());
  }

  proto::ExportRequest req;
  req.set_idfile(id.toStdString());
  QScopedPointer<proto::ExportResponse> resp(ch->remoteCall(&proto::DocumentService::GetDocument, req, 100000));
  if (resp.isNull()) {
    error_log.msgBox() << QObject::tr("Сервис документов не доступен");
    return data;
  }

  if (!resp->result()) {
    error_log.msgBox() << QObject::tr("Не удалось получить документ");
    return data;
  }

  data.setRawData(resp->data().data(), resp->data().size());

  return data;
}

QString JobWrapper::name() const
{
  return JobWrapper::name(job_);
}

QString JobWrapper::title() const
{
  return JobWrapper::title(job_);
}

Period JobWrapper::currentPeriod() const
{
  return Period(job_);
}

const Period& JobWrapper::fixedPeriod() const
{
  return fixedPeriod_;
}

void JobWrapper::ajustFixedPeriod()
{
  fixedPeriod_ = Period(job_);
}


// class JobList
JobList::JobList() :
  isChanged_(false)
{
}

JobList::~JobList()
{
}

void JobList::load(rpc::Channel* ch)
{
  if (ch == nullptr)
    return ;

  QScopedPointer<proto::JobList> resp(ch->remoteCall(&proto::DocumentService::GetWeatherJobs, proto::JobList(), 20000));
  if (resp.isNull()) {
    error_log << QObject::tr("Список шаблонов карт недоступен");
    return;
  }

  data_.clear();
  dataCache_.clear();

  for (int i = 0, sz = resp->job_size(); i < sz; ++i) {
    const proto::Job& job(resp->job(i));
    data_.append(JobWrapper::ptr_t(new JobWrapper(job)));
    dataCache_.insert(JobWrapper::name(job));
  }
}

void JobList::update(rpc::Channel* ch)
{
  if (ch == nullptr)
    return ;

  proto::Dummy req;
  QScopedPointer<proto::JobList> resp(ch->remoteCall(&proto::DocumentService::GetLastJobs, req, 100000));
  if (resp.isNull()) {
    error_log << QObject::tr("Информация о последних выполненых заданиях недоступна");
    return;
  }

  for (int i = 0, sz = resp->job_size(); i < sz; ++i) {
    JobWrapper::ptr_t job = getByName(JobWrapper::name(resp->job(i)));
    // if (job.isNull()) {
    //   data_.append(new JobWrapper(job));
    //   dataCache_.insert(JobWrapper::name(job));
    // }

    job->updateState(ch);
  }
}

void JobList::save(rpc::Channel* ch)
{
  if (ch == nullptr)
    return ;

  proto::JobList newJobList;
  for (auto job : qAsConst(data_)) {
    proto::Job* item = newJobList.add_job();
    item->CopyFrom(job->raw());
  }

  QScopedPointer<proto::Response> resp(ch->remoteCall(&proto::DocumentService::SetWeatherJobs, newJobList, 100000));
  if (resp.isNull()) {
    error_log << QObject::tr("Не удалось передать изменения сервису");
    return;
  }

  if (!resp->result() ) {
    error_log << QObject::tr("При сохранении изменений произошла ошибка, повторите попытку");
  }

  isChanged_ = false;
}

JobWrapper::ptr_t JobList::get(int index)
{
  if (0 <= index && index < data_.size())
    return data_.at(index);

  return JobWrapper::ptr_t(nullptr);
}

JobWrapper::ptr_t JobList::getByName(const QString& name, int* index /*=*/)
{
  int res = 0;
  if (dataCache_.contains(name)) {
    for (auto job : qAsConst(data_)) {
      if (job->name() == name) {
        if (index)
          *index = res;

        return job;
      }

      res++;
    }
  }

  if (index)
    *index = res;

  return JobWrapper::ptr_t(nullptr);
}

void JobList::append(const proto::Job& job)
{
  data_.append(JobWrapper::ptr_t(new JobWrapper(job)));
  dataCache_.insert(JobWrapper::name(job));

  isChanged_ = true;
}

void JobList::insert(int index, const proto::Job& job)
{
  data_.insert(index, JobWrapper::ptr_t(new JobWrapper(job)));
  dataCache_.insert(JobWrapper::name(job));

  isChanged_ = true;
}

void JobList::remove(int index)
{
  if (0 <= index && index < data_.size()) {
    dataCache_.remove(data_.at(index)->name());
    data_.removeAt(index);

    isChanged_ = true;
  }
}

void JobList::update(int index, const proto::Job& job)
{
  if (0 <= index && index < data_.size()) {
    data_.at(index)->raw().CopyFrom(job);

    isChanged_ = true;
  }
}

void JobList::setEnabled(int index, bool enable)
{
  if (0 <= index && index < data_.size()) {
    data_.at(index)->raw().set_enabled(enable);

    isChanged_ = true;
  }
}

}

}
