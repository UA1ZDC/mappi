#ifndef MAPPI_SERVICES_FILESERVICE_FILESERVICECLIENT_FILESERVICECLIENT_H
#define MAPPI_SERVICES_FILESERVICE_FILESERVICECLIENT_FILESERVICECLIENT_H

#include <qobject.h>
#include <qsharedpointer.h>
#include <qdatetime.h>

#include <mappi/global/global.h>
#include <mappi/settings/mappisettings.h>
#include <mappi/proto/fileservice.pb.h>

namespace mappi {

struct FileServiceResponse {
  explicit FileServiceResponse( const QString& fn = QString::null, int sz = 0, bool ok = false ) :
             filename(fn),
             size(sz),
             result(ok) {}

  QString filename;
  int size;
  bool result;
};

class FileServiceClient : public QObject
{
  Q_OBJECT

public:
  explicit FileServiceClient( QObject* parent = 0 );
  ~FileServiceClient();

  void setFileName(const QString& fileName) { clientFileName_ = fileName; }
  void setRewrite(bool rewrite) { rewrite_ = rewrite; }

  void answerReceived(proto::ExportResponse* resp);
  bool requestFile(const QString& filename, bool rewrite = false);
  bool conenctToFileService();
  QString getClientFilename() {return clientFileName_;}
  bool isSuccessful() { return success_;}

public slots:
  void slotRun();

signals:
  void fileResponseStart( ::mappi::FileServiceResponse resp );
  void fileResponseFinish( ::mappi::FileServiceResponse resp );
  void complete( const QString& filename, int percent );
  void fileNotFound();
  void connectionFailure();
  void requestFailure();
  void responseTimeout();

protected:
  virtual void timerEvent(QTimerEvent *);

private:
  void receiveStartChunk(const proto::ExportResponse* chunk);
  void receiveDataChunk(const proto::ExportResponse* chunk);
  void receiveFinishChunk(const proto::ExportResponse* chunk);

  void writeDataChunks();
  void writeFinishChunk();

private:
  meteo::rpc::Channel* channel_;
  QFile* output_;

  bool rewrite_ = false;
  bool success_ = false;

  QString clientFileName_;

  int64_t totalReceivedBytes_;
  QSharedPointer<proto::ExportResponse> startChunk_;
  QSharedPointer<proto::ExportResponse> finishChunk_;
  QList<QSharedPointer<proto::ExportResponse> > dataChunks_;

  QDateTime lastActive_;
};

} // mappi



#endif // MAPPI_SERVICES_FILESERVICE_FILESERVICECLIENT_FILESERVICECLIENT_H
