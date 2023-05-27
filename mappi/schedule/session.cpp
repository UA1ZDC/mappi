#include "session.h"


namespace mappi {

namespace schedule {

void Session::toProto(const Session& other, conf::Session* session)
{
  session->set_satellite(other.data().satellite.toStdString());
  session->set_aos(SessionData::dateToString(other.data().aos).toStdString());
  session->set_los(SessionData::dateToString(other.data().los).toStdString());
  session->set_elevat_max(other.data().elevatMax);
  session->set_revol(other.data().revol);
  session->set_direction(other.data().direction);
  session->set_confl_state(other.data().conflState);
}

void Session::fromProto(const conf::Session& other, Session* session)
{
  session->data_.satellite = other.satellite().c_str();
  session->data_.aos = SessionData::dateFromString(other.aos().c_str());
  session->data_.los = SessionData::dateFromString(other.los().c_str());
  session->data_.elevatMax = other.elevat_max();
  session->data_.revol = other.revol();
  session->data_.direction = other.direction();
  session->data_.conflState = other.confl_state();
}

Session::Session()
{
}

Session::Session(const SessionData& data) :
  data_(data)
{
}

Session::Session(const conf::Session& other)
{
  Session::fromProto(other, this);
}

Session::~Session()
{
}

int Session::secsToAos(const QDateTime& dt /*=*/) const
{
  return dt.secsTo(data_.aos);
}

int Session::msecsToAos(const QDateTime& dt /*=*/) const
{
  return dt.msecsTo(data_.aos);
}

int Session::secsToLos(const QDateTime& dt /*=*/) const
{
  return dt.secsTo(data_.los);
}

int Session::msecsToLos(const QDateTime& dt /*=*/) const
{
  return dt.msecsTo(data_.los);
}

int Session::duration() const
{
  return data_.aos.secsTo(data_.los);
}

bool Session::isIgnored() const
{
   switch (data_.conflState) {
     case conf::kNormalState :
     case conf::kSelfState :
     case conf::kUserSelfState :
       return false;

     default :
       break;
   }

   return true;
}

Session::stage_t Session::stage(const QDateTime& dt /*=*/) const
{
  if (0 < dt.secsTo(data_.aos))
    return Session::WAITING;

  return (0 < dt.secsTo(data_.los) ? Session::PROCEED : Session::COMPLETED);
}

bool Session::operator ==(const Session& other) const
{
  return (data_ == other.data_);
}

bool Session::operator <(const Session& other) const
{
  return (data_ < other.data_);
}

bool Session::operator !=(const Session& other) const
{
  return (data_ != other.data_);
}

}

}
