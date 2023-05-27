#ifndef APPLOGWATCHER_H
#define APPLOGWATCHER_H

#include <qobject.h>
#include <qmap.h>
#include <qpair.h>

#include <meteo/commons/proto/appconf.pb.h>

namespace google
{
namespace protobuf
{
class Closure;
}
}

class QFile;
class QTimer;

namespace meteo
{
namespace app
{
typedef QPair<AppOutReply*, google::protobuf::Closure*> OutSubscribe;

class AppLogWatcher : public QObject
{
  Q_OBJECT
  public:
    AppLogWatcher( QObject* parent = 0, int32_t sendsecs = 1 );
    ~AppLogWatcher();

    bool addSubscribe( const QString& filename, const OutSubscribe& subscribe );
    void rmSubscribe( const OutSubscribe& subscribe );

    void setOutFiles( const QStringList& outfilelist );

  private:
    QMap< QString, QFile* > files_;
    QMap< QFile*, OutSubscribe > subscribes_;
    QTimer* timer_;
    QByteArray chunk_;
    int32_t sendsecs_;

  private:
    void sendLastLogs( QFile* file, const OutSubscribe& subscribe );

  private slots:
    void slotTimeout();
    void slotFileChanged( QFile* file );

  private:
    bool wasdisconnected_ = false;
};

}
}

#endif
