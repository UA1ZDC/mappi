#ifndef SQL_NSPGBASE_TEST_THREADS_MAIN_H
#define SQL_NSPGBASE_TEST_THREADS_MAIN_H

#include <qobject.h>
#include <qcoreapplication.h>

class MainTest : public QObject
{
  Q_OBJECT
public:
  MainTest(int nThreads) : n_(nThreads) {}

public slots:
  void slotThreadFinished()
  {
    --n_;

    if ( 0 == n_ ) { qApp->quit(); }
  }

private:
  int n_;
};

#endif // SQL_NSPGBASE_TEST_THREADS_MAIN_H
