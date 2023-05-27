#include "gecos.h"


namespace meteo {

namespace auth {

Gecos::Gecos()
{
}

Gecos::Gecos(const QString& surname, const QString& name, const QString& patron) :
  surname_(surname),
  name_(name),
  patron_(patron)
{
}

Gecos::~Gecos()
{
}

QString Gecos::fullName() const
{
  return QString("%1 %2 %3")
    .arg(surname_)
    .arg(name_)
    .arg(patron_);
}

void Gecos::appendOther(const QString& v)
{
  other_.append(v);
}

QString Gecos::other() const
{
  return other_.join(',');
}

void Gecos::fromString(const QString& v)
{
  QStringList gecos = v.split(',', QString::KeepEmptyParts);

  QStringList fullName = gecos.value(0, QString()).split(' ');
  surname_ = fullName.value(0, QString());
  name_ = fullName.value(1, QString());
  patron_ = fullName.value(2, QString());

  roomNumber_ = gecos.value(1, QString());
  workNumber_ = gecos.value(2, QString());

  for (int i = 4; i < gecos.size(); ++i)
    other_ << gecos.value(i);
}

QString Gecos::toString() const
{
  return QString("%1,%2,%3,%4")
    .arg(fullName())
    .arg(roomNumber())
    .arg(workNumber())
    .arg(other_.isEmpty() ? "" : other());
}

}

}
