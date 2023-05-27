#ifndef METEO_PRODUCT_SERVICES_DOCUMENT_RUNJOB_STDINREADER_H
#define METEO_PRODUCT_SERVICES_DOCUMENT_RUNJOB_STDINREADER_H

#include <qobject.h>
#include <qbytearray.h>
#include <qfile.h>
#include <qtimer.h>
#include <qdatetime.h>

#include <meteo/commons/proto/weather.pb.h>
#include <meteo/commons/proto/document_service.pb.h>

namespace meteo {
namespace map {

class StdinReader : public QObject
{
  Q_OBJECT
  public:
    StdinReader();
    ~StdinReader();

    bool connectStdinout();

    bool parseAndRunJob();

    void runTestJob();

  private slots:
    void slotTimeout();

  private:
    bool parseJob( meteo::map::proto::Job* job );
    bool runJob( const meteo::map::proto::Job& job, meteo::map::proto::ExportResponse* result );
    bool saveFile( const meteo::map::proto::Job& job, const QByteArray& data, const QString& path );

  private:
    QByteArray stdinarr_;
    QTimer* tm_ = nullptr;
    QFile* in_ = nullptr;
    QFile* out_ = nullptr;
    QDateTime begdt_;
    int32_t jobsz = 0;
};

}
}

#endif
