#pragma once

#include <qstringlist.h>


namespace meteo {

namespace auth {

class Gecos
{
public :
  Gecos();
  Gecos(const QString& surname, const QString& name, const QString& patron);
  ~Gecos();

  inline QString surname() const { return surname_; }
  inline void setSurname(const QString& v) { surname_ = v; }

  inline QString name() const { return name_; }
  inline void setName(const QString& v) { name_ = v; }

  inline QString patron() const { return patron_; }
  inline void setPatron(const QString& v) { patron_ = v; }

  QString fullName() const;

  void appendOther(const QString& v);
  QString other() const;

  inline void setRoomNumber(const QString& v) { roomNumber_ = v; }
  inline QString roomNumber() const { return roomNumber_; }

  inline void setWorkNumber(const QString& v) { workNumber_ = v; }
  inline QString workNumber() const { return workNumber_; }

  void fromString(const QString& v);
  QString toString() const;

private :
  QString surname_;
  QString name_;
  QString patron_;

  QString roomNumber_;
  QString workNumber_;

  QStringList other_;
};

}

}
