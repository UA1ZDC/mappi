#include "testmain.h"
#include "taskservice.pb.h"
#include <unistd.h>

#include <meteo/commons/settings/settings.h>
#include <meteo/commons/global/global.h>
#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/paths.h>
#include <qcoreapplication.h>
#include <meteo/commons/settings/settings.h>

extern int msleep(int msecs);

int getRandom(){
  unsigned int r = rand();
  return (r % 1000) + 1;
}

void TestMain::initTestCase()
{
  TAPPLICATION_NAME("meteo");
  meteo::gSettings(meteo::global::Settings::instance());
  if ( false == meteo::global::Settings::instance()->load() ){
    QFAIL("FAILED LOAD SETTINGS");
  }
  subthread_ = new ServerThread();
  QObject::connect(subthread_, &QThread::finished, this, &TestMain::onSubthreadExit );

  subthread_->start();
  int retry = 10;
  while ( false == subthread_->isServerStarted() ){
    msleep(1000);
    --retry;
    debug_log << retry;
    QVERIFY(0 != retry);
  }
}

void TestMain::onSubthreadExit() {
  warning_log << "ПОДПРОЦЕСС ЗАВЕРШИЛСЯ!";

}

///
/// \brief TestMain::testFinish100
/// Тестируем работу сервиса, 100% запросов должны быть выполнены успешно
///
void TestMain::testFinish100()
{
  meteo::rpc::Channel* channel = meteo::global::serviceChannel(serviceAddress);
  QVERIFY( nullptr != channel );

  for (int i = 0; i < statsRetry; ++i ) {
    debug_log << i << "OUT OF" << statsRetry;
    meteo::test::proto::TestMessage req;
    req.set_count(getRandom());
    int timeout = 1100;
    meteo::test::proto::TestMessage *resp = channel->remoteCall(&meteo::test::proto::TestService::testreq, req, timeout );
    QVERIFY(nullptr != resp);
    QCOMPARE(req.count(), resp->count());
    delete resp;
  }
  delete channel;
}

///
/// \brief TestMain::testFinishTimeouts
/// Тестируем работу, часть запросов завершается по таймауту
///
void TestMain::testFinishTimeouts()
{
  meteo::rpc::Channel* channel = meteo::global::serviceChannel(serviceAddress);
  QVERIFY( nullptr != channel );

  int droppedCnt = 0;
  for (int i = 0; i < statsRetry; ++i ) {
    debug_log << i << "OUT OF" << statsRetry;
    meteo::test::proto::TestMessage req;
    req.set_count(getRandom());
    int timeout = 800;
    meteo::test::proto::TestMessage *resp = channel->remoteCall(&meteo::test::proto::TestService::testreq, req, timeout );
    if ( nullptr != resp ) {
      QCOMPARE(req.count(), resp->count());
      delete resp;
    }
    else {
      ++droppedCnt;
    }
  }

  QVERIFY(droppedCnt != 0);

  for (int i = 0; i < statsRetry; ++i ) {
    debug_log << i << "OUT OF" << statsRetry;
    meteo::test::proto::TestMessage req;
    req.set_count(getRandom());
    int timeout = 1100;
    meteo::test::proto::TestMessage *resp = channel->remoteCall(&meteo::test::proto::TestService::testreq, req, timeout );
    QVERIFY(nullptr != resp);
    QCOMPARE(req.count(), resp->count());
    delete resp;
  }
  delete channel;
}


QTEST_MAIN(TestMain)
