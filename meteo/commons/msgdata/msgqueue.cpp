#include "msgqueue.h"

#include <climits>
#include <qcryptographichash.h>

#include <cross-commons/debug/tlog.h>

#include <meteo/commons/msgparser/common.h>
#include <meteo/commons/msgparser/msgmetainfo.h>
#include <meteo/commons/msgparser/tlgfuncs.h>

namespace meteo {

MsgQueue::MsgQueue()
  : mutex_(QMutex::Recursive)
{
  counter_ = 0;
}

MsgQueueItem MsgQueue::head(MsgStage stage) const
{
  QMutexLocker lock(&mutex_);

  if ( size(stage) == 0 ) { return MsgQueueItem(); }

  int prior = -1;
  foreach ( int p, index_[stage].keys() ) {
    if ( index_[stage][p].size() != 0 ) {
      prior = p;
      break;
    }
  }

  if ( -1 == prior ) { return MsgQueueItem(); }

  return MsgQueueItem(const_cast<MsgQueue*>(this), index_[stage][prior].first());
}

MsgQueueItem MsgQueue::item(int id) const
{
  return MsgQueueItem(const_cast<MsgQueue*>(this), id);
}

MsgQueueItem MsgQueue::append(const tlg::MessageNew& msg, MsgStage stage)
{
  QMutexLocker lock(&mutex_);

  if ( size() == INT_MAX ) { return MsgQueueItem(); }

  if ( INT_MAX == counter_ ) { counter_ = 0; } else { ++counter_; }

  int prior = tlg::priority(msg);

  msg_[counter_] = msg;
  lifeCycle_[counter_].append(stage);
  index_[stage][prior].append(counter_);

  return MsgQueueItem(this, counter_);
}

void MsgQueue::remove(const MsgQueueItem& item)
{
  remove(item.id());
}

void MsgQueue::remove(int id)
{
  mutex_.lock();

  for ( int i=kNone; i<kMaxMsgStage; ++i ) {
    MsgStage ms = static_cast<MsgStage>(i);
    foreach ( int prior, index_[ms].keys() ) {
      if ( 0 != index_[ms][prior].removeAll(id) ) {
        break;
      }
    }
  }
  lifeCycle_.remove(id);
  msg_.remove(id);

  mutex_.unlock();
}

int MsgQueue::size() const
{
  QMutexLocker lock(&mutex_);

  return msg_.size();
}

int MsgQueue::size(MsgStage stage) const
{
  mutex_.lock();

  int sz = 0;
  foreach ( int p, index_[stage].keys() ) {
    sz += index_[stage].value(p).size();
  }

  mutex_.unlock();

  return sz;
}

void MsgQueue::clear()
{
  mutex_.lock();

  msg_.clear();
  for ( int i=kNone; i<kMaxMsgStage; ++i ) {
    index_[static_cast<MsgStage>(i)].clear();
  }
  lifeCycle_.clear();

  mutex_.unlock();
}

MsgStage MsgQueue::stage(int id) const
{
  QMutexLocker lock(&mutex_);

  if ( !lifeCycle_.contains(id) || lifeCycle_[id].size() == 0 ) { return kNone; }

  return lifeCycle_[id].first();
}

tlg::MessageNew MsgQueue::msg(int id) const
{
  QMutexLocker lock(&mutex_);

  if ( !msg_.contains(id) ) { return tlg::MessageNew(); }

  return msg_[id];
}

bool MsgQueue::setMsg(int id, const tlg::MessageNew& msg)
{
  QMutexLocker lock(&mutex_);

  if ( !msg_.contains(id) ) { return false; }

  msg_[id].CopyFrom(msg);

  return true;
}

bool MsgQueue::nextStage(int id)
{
  QMutexLocker lock(&mutex_);

  if ( !msg_.contains(id) ) { return false; }

  int prior = tlg::priority(msg_[id]);

  if ( lifeCycle_[id].size() != 0 ) {
    MsgStage stage = lifeCycle_[id].takeFirst();
    index_[stage][prior].removeOne(id);
  }

  if ( lifeCycle_[id].size() == 0 ) {
    remove(id);
//    debug_log << QObject::tr("обработка завершена #%1 ").arg(id);
    return false;
  }
  else {
    MsgStage stage = lifeCycle_[id].first();
    index_[stage][prior].append(id);
//    debug_log << QObject::tr("следующий этап #%1 для #%2 ").arg(stage).arg(id);
  }

  return true;
}

bool MsgQueue::contains(int id) const
{
  QMutexLocker lock(&mutex_);

  return msg_.contains(id);
}

void MsgQueue::appendStage(int id, MsgStage stage, bool unique)
{
  mutex_.lock();

  if ( !unique || !lifeCycle_[id].contains(stage) ) {
    lifeCycle_[id].append(stage);
//    debug_log << QObject::tr("добавлен этап #%1 для элемента #%2 ").arg(stage).arg(id);
  }

  mutex_.unlock();
}

//
// MsgQueueItem
//

MsgStage MsgQueueItem::stage() const
{
  if ( !isValid() ) { return kNone; }

  return queue_->stage(queueId_);
}

void MsgQueueItem::appendStage(MsgStage stage, bool unique)
{
  if ( !isValid() ) { return; }

  queue_->appendStage(queueId_, stage, unique);
}

bool MsgQueueItem::nextStage()
{
  if ( !isValid() ) { return false; }

  return queue_->nextStage(queueId_);
}

const tlg::MessageNew& MsgQueueItem::msg() const
{
  if ( !isValid() || cached_ ) { return cachedMsg_; }

  cachedMsg_ = queue_->msg(queueId_);

  return cachedMsg_;
}

bool MsgQueueItem::setMsg(const tlg::MessageNew& msg)
{
  if ( !isValid() ) { return false; }

  cachedMsg_.CopyFrom(msg);

  return queue_->setMsg(queueId_, msg);
}

bool MsgQueueItem::isValid() const
{
  if ( 0 == queue_ ) { return false; }
  if ( -1 == queueId_ ) { return false; }

  return queue_->contains(queueId_);
}

MsgInfo::MsgInfo(const tlg::MessageNew& msg)
{
  MsgMetaInfo info(msg);
  dt_ = info.calcConvertedDt();

  md5_ = tlg::md5hash(msg);

  for ( int i=0,isz=msg.metainfo().distibuted_size(); i<isz; ++i ) {
    distributed_ << QString::fromUtf8(msg.metainfo().distibuted(i).c_str());
  }
}

} // meteo
