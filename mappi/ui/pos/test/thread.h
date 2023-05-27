#ifndef THREAD_H
#define THREAD_H

#include <qbytearray.h>
#include <qthread.h>

#include <meteo/commons/rpc/rpc.h>
#include <mappi/proto/fileservice.pb.h>

class Thread : public QThread
{
  Q_OBJECT
  public:
    Thread( const std::string& path );
    ~Thread();

    const QByteArray& data() const { return arr_; }
    bool result() const { return result_; }

    void slotResponse( mappi::proto::ExportResponse* response );

  protected:
    void run();

  private:
    bool result_ = false;
    std::string path_;
    QByteArray arr_;
  meteo::rpc::Channel* ch_ = 0;
};

#endif
