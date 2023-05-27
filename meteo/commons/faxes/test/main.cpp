#include <meteo/commons/faxes/imagemaker.h>

#include <cross-commons/app/paths.h>
#include <cross-commons/app/targ.h>
#include <cross-commons/debug/tlog.h>
#include <meteo/commons/global/global.h>
#include <meteo/commons/settings/settings.h>

#include <qcoreapplication.h>
#include <qtextcodec.h>
#include <qstring.h>
#include <cross-commons/app/options.h>

#include <meteo/commons/proto/msgcenter.pb.h>

#include <meteo/commons/faxes/packer.h>
#include <meteo/commons/faxes/extract.h>
#include <meteo/commons/faxes/tfaxdb.h>

#include <qhash.h>
#include <qdir.h>
#include <meteo/commons/proto/msgparser.pb.h>
#include <QTemporaryFile>


bool checkSegments (const QList<meteo::faxes::FaxSegment> &segments){
  for (int i = 1; ; ++i){
    bool ok = false;
    bool last = false;
    for (auto segment: segments){
      if (i == segment.segnum) {
        ok = true;
        last = ( meteo::faxes::FaxSegment::END == segment.segtype );
        break;
      }
    }
    if ( false == ok) {
      return false;
    }
    if ( true == last) {
      return true;
    }
  }
}

void processUndecodedFax(const QList<meteo::faxes::FaxSegment> &segments){
  if (segments.isEmpty() ) return;
  if (checkSegments(segments)) {
    QDateTime dt = QDateTime::currentDateTime();
    QString basePath = meteo::global::kFaxTiffPath + "/" + dt.date().toString(Qt::ISODate);
    QTemporaryFile tmpFile("this");
    tmpFile.setFileTemplate(QDir::tempPath() + "/fax_XXXXXX");
    if ( false == tmpFile.open()){
      error_log << QObject::tr("Невозможно создать tmp файл");
    }
    QString fileName = tmpFile.fileName();
    if ( false == meteo::faxes::Extractor::startExtracting(segments, fileName)){
      error_log << QObject::tr("Ошибка при сборке факса.");
      return;
    }
    QByteArray data = tmpFile.readAll();
    QFile fl("her.bmp");
    fl.open(QIODevice::WriteOnly);
    fl.write(data);
  }
}

int main(int argc, char* argv[])
{
  TAPPLICATION_NAME("meteo");
  meteo::global::setLogHandler();
  QCoreApplication app(argc, argv);
  ::setlocale(LC_NUMERIC, "C"); // локаль для записи координат в mongo (libbson)

  meteo::gSettings(new meteo::Settings());

  if (  meteo::gSettings()->load() == false) {
    error_log << QObject::tr("Не получилось загрузить настройки, выход");
    return EXIT_FAILURE;
  }
  meteo::faxes::Packer packer;
  QDir::setCurrent("/home/gotur/src/meteo/commons/faxes/test/tmp");
  QFile file;
 // file.setFileName("./test/test.bmp");
  file.setFileName("./test/Метеограммы - с 20:59 18.05.19 по 18:00 20.05.19 Санкт-Петербург (26063,  59°58' 0'' с.ш.  30°17'59'' в.д. 6 м.)_2019.05.20_16:16.jpeg");
  file.open(QIODevice::ReadOnly);
  QByteArray content = file.readAll();
  QList<QByteArray> messages;
  packer.createMessages(content,&messages,99999,meteo::faxes::FaxSegment::DOC);
  info_log << QObject::tr("Запуск...");
  int i=0;
  for(auto h:messages){
      QFile fl;
      QString fname = QString("%1.bmp")
          .arg(i);
      ++i;
      fl.setFileName(fname);
      fl.open(QIODevice::ReadWrite);
      fl.write(h);
      fl.close();
    }
  QMultiHash<int, meteo::faxes::FaxSegment> allSegments;
  for(auto h:messages){
      meteo::faxes::FaxSegment segment = meteo::faxes::Extractor::extractSegment(h);
      allSegments.insert(segment.faxid, segment);
    }
  while (false == allSegments.isEmpty()){
    int nextFaxId = allSegments.keys().first();
    auto segments = allSegments.values(nextFaxId);
    auto messages = allSegments.values(nextFaxId);
    processUndecodedFax(segments);
    allSegments.remove(nextFaxId);
  }
  return 0;

}

