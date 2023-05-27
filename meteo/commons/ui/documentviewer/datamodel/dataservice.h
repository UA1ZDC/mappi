#ifndef METEO_PRODUCT_UI_PLUGINS_DOCUMENTVIEWERPLUGIN_DATAMODEL_DATASERVICE_H
#define METEO_PRODUCT_UI_PLUGINS_DOCUMENTVIEWERPLUGIN_DATAMODEL_DATASERVICE_H

#include <qobject.h>
#include <qtemporaryfile.h>
#include <qtemporarydir.h>
#include <qhash.h>
#include <qimage.h>
#include <meteo/commons/ui/documentviewer/datamodel/docviewtypes.h>
#include <qfilesystemwatcher.h>
#include <qprinter.h>
#include <meteo/commons/proto/weather.pb.h>
#include <meteo/commons/global/global.h>

namespace meteo {
namespace documentviewer {

/*
 * Загрузка данных из сервисов
*/

class DataService : public QObject
{
  Q_OBJECT
public:
  DataService();
  virtual ~DataService() override;
  const QImage& getFile( DocumentHeader::DocumentClass type, const QString& fileName );

  void loadFile( DocumentUID uid, bool preloadOnly = false );
  void loadFile( DocumentHeader::DocumentClass type, const QString& fileName, bool preloadOnly = false );
  void loadList( DocumentHeader::DocumentClass type, const QDateTime& dtStart, const QDateTime& dtEnd );  
  void printDocumentFromImage( const QString& path );
  void generageGifFromImages(const QStringList& files, const QString& outputFileName, int delay);  
  QString getFsFilePath( DocumentHeader::DocumentClass docClass,  const QString& fileName );
  void printDocument( DocumentHeader::DocumentClass docClass, const QString& fileName );

  void openDocument( DocumentHeader::DocumentClass docClass, const QString& fileId );

signals:  
  void signalFileLoaded( const QList<QImage>& fileImage );
  void signalListLoaded( const QList<DocumentHeader*> *document );
  void signalFileChanged( DocumentUID docuid );
private slots:
  void slotFileChanged( const QString& filePath );
  void slotDocumentChanged( DocumentUID docuid );

private:
  enum PrintSettings {
    kPrintSettingsInitialized,
    kPrinterName,
    kOutputFileName,
    kPageWidth,
    kPageHeight,
    kPageLeftField,
    kPageTopField,
    kPageRightField,
    kPageBottomField,
    kPageUnit,
    kPageSize,
    kPageOrientation,
    kPrinterColorMode   
  };
  void clearDocuments();
  void loadWeatherImage(const QString& mapFileId, bool preloadOnly );
  void loadWeatherDocument(const QString& mapFileId, bool preloadOnly );
  void loadFax( const QString& faxid, bool preloadOnly );
  void loadLocalFile( const QString& fileName, bool preloadOnly );
  void loadSateliteImageFile( const QString& fileid, bool preloadOnly );
  void loadMapImageFile( const QString& fileid, bool preloadonly );
  void loadFormalDocumentFile( QString fileid, bool preloadonly );

  const QList<QImage>& loadPdfDocument( const QString& pdf );
  void removeCachedFile( const QString& fileName );

  void loadAndEmitFaxList( const QDateTime& beginDt, const QDateTime& endDt );
  void loadAndEmitSateliteImageList( const QDateTime& beginDt, const QDateTime& endDt );
  void loadAndEmitMapImagesList( const QDateTime& beginDt, const QDateTime& endDt );
  void loadAndEmitFormalDocumentsList( const QDateTime& beginDt, const QDateTime& endDt );

  const QString copyFileToTemporary( const QString& fileName );
  const QString& generatePdfFromOdt(const QString& fileName);  

  void printLocalDocument( const QString& path );
  void printDocumentFromImage( const QImage& page, const QString& docType );
  void loadPrintSettings(QPrinter * printer, QString docType);
  void addPrintSettings(QPrinter * printer, QString docType);
  void savePrintSettings();
  bool loadFaxList(const QDateTime& dtStart, const QDateTime& dtEnd);
  bool loadMapImageList( const QDateTime& beginDt, const QDateTime& endDt );
  bool loadFormalDocumentList( const QDateTime& beginDt, const QDateTime& endDt );
  static void printOfficeDocument( const QString& path );

  QImage loadFax(const QString& faxid);
  QImage loadSateliteImage(const QString& imageid);
  const QImage& loadLocalImage( const QString& imagePath );
  void importFormalDocument( DocumentUID uid );

  bool tryConnect();
  void openDocument(const QString& fsFileName);

  QHash<QString, QTemporaryFile*> clonedDocuments_;
  QHash<DocumentUID, QTemporaryFile*> loadedDocuments_;
  QHash<DocumentUID, QImage> loadedImages_;

  QStringList generatedDocuments_;
  QStringList loadedFaxes_;

  QList<DocumentHeader*> allDocuments_;
  QFileSystemWatcher fileWatcher_;

  bool printSettingsMapLoadedCheck_ = false;
  //QHash<QString, QHash<PrintSettings, QVariant>> - не сериализуется, проэтому int!
  QHash<QString, QHash<int, QVariant>> printSettingsMap_;

  meteo::rpc::Channel* documentChannel_ = nullptr;

  QHash<QString, QImage> loadedLocalImages_;
  QHash<QString, QList<QImage>> loadedLocalPdf_;
  QStringList documentsToRemove_;
  QTemporaryDir *temporaryDir_;


};

}
}
#endif
