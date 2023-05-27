#ifndef SERVERTHREAD_H
#define SERVERTHREAD_H

#include <qthread.h>
#include <meteo/commons/rpc/autotest/rpctimeout/server.h>
#include <meteo/commons/rpc/server.h>

static const QString serviceAddress = QObject::tr("localhost:16994");

class ServerThread: public QThread
{
  Q_OBJECT
public:
  ServerThread();
  virtual ~ServerThread() override;

  bool isServerStarted();

private slots:
  void init();

private:
  meteo::test::TestService* service_ = nullptr;
  meteo::rpc::Server* server_ = nullptr;
};

#endif // SERVERTHREAD_H
