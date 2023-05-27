#ifndef TESTMAIN_H
#define TESTMAIN_H

#include <qobject.h>
#include <QtTest/QtTest>
#include <meteo/commons/rpc/autotest/rpctimeout/serverthread.h>

class TestMain : public QObject
{
  Q_OBJECT
private slots:
  void initTestCase();

  void testFinish100();

  void testFinishTimeouts();

  void onSubthreadExit();

private:
  ServerThread* subthread_ = nullptr;

  int statsRetry = 25;
};

#endif // TESTMAIN_H
