#ifndef CROSS_COMMONS_APP_QOBJECTDELETER_H
#define CROSS_COMMONS_APP_QOBJECTDELETER_H

#include <qobject.h>

//! Класс QObjectDeleter это удобный класс для удаления объектов класса QObject
class QObjectDeleter
{
public:
  QObjectDeleter(QObject* object, bool deleteLater = true) :
    _object(object),
    _deleteLater(deleteLater)
  {}

  ~QObjectDeleter()
  {
    if ( _deleteLater ) {
      _object->deleteLater();
    }
    else {
      delete _object;
      _object = 0;
    }
  }

private:
  QObject* _object;
  bool _deleteLater;
};

#endif // CROSS_COMMONS_APP_QOBJECTDELETER_H

