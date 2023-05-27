#ifndef TEST_H
#define TEST_H

#include <qobject.h>

class Test : public QObject
{
  Q_OBJECT
  public:
    Test(){}
    ~Test(){}

  public slots:
    void slotNotify( const char* name, const char* extra );
    void slotNotify( const char* name );
};

#endif
