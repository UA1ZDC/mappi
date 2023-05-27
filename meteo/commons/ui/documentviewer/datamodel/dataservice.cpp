#include "dataservice.h"

#include <meteo/commons/global/global.h>
#include <meteo/commons/rpc/rpc.h>
#include <cross-commons/debug/tlog.h>
#include <meteo/commons/global/global.h>
#include <meteo/commons/proto/document_service.pb.h>
#include <qprocess.h>
#include <qimage.h>
#include <qpainter.h>
#include <qfile.h>
#include <poppler/qt5/poppler-qt5.h>
#include <qprintdialog.h>
#include <qdatastream.h>
#include <qdesktopservices.h>
#include <qbuffer.h>
#include "datamodel/docviewtypes.h"
#include "datamodel/wgtdocviewModel.h"

namespace meteo {
namespace documentviewer {

static const QImage nullImage = QImage();
static const double resolution = 72 * 3;
static const int maxPagesNumber = 10;

static const QString kPrintSettingsFilename( ::QDir::homePath() + QString("/.meteo/meteoproduction/DocumentListPrintSettings") );
static const QString kTemporaryDirPath = QObject::tr("%1/document_viewer_XXXXXX").arg( QDir::tempPath() );

static const QString& kErrorMessageCanNotConnectToDocumentService = QObject::tr("Не удается установить соединение с сервисом формирования документов");

DataService::DataService()
{  
  QObject::connect(&this->fileWatcher_, SIGNAL(fileChanged(const QString&)), this, SLOT(slotFileChanged( const QString& )) );
  QObject::connect( this, &DataService::signalFileChanged, this, &DataService::slotDocumentChanged );
  this->temporaryDir_ = new QTemporaryDir(kTemporaryDirPath);
}

DataService::~DataService()
{
  this->savePrintSettings();  
  QStringList filesToDelete;
  while ( false == loadedDocuments_.isEmpty() ) {
    auto key = loadedDocuments_.keys().first();
    auto tmpfile = loadedDocuments_.value(key);
    loadedDocuments_.remove(key);
    tmpfile->remove();
    filesToDelete << tmpfile->fileName();
    delete tmpfile;
  }
  while ( false == clonedDocuments_.isEmpty() ){
    auto key = clonedDocuments_.keys().first();
    auto tmpfile = clonedDocuments_.value(key);
    clonedDocuments_.remove(key);
    tmpfile->remove();
    filesToDelete << tmpfile->fileName();
    delete tmpfile;
  }
  filesToDelete << generatedDocuments_;
  filesToDelete << loadedFaxes_;
  filesToDelete << documentsToRemove_;

  for ( auto fileName : filesToDelete ) {
    if ( fileName.isEmpty() ) continue;
    if ( true == QFile::exists(fileName) ){
      QFile file(fileName);
      if ( false == file.remove() ) {
        warning_log.msgBox() << QObject::tr("Ошибка при удалении файла");
      }
    }
  }
  this->clearDocuments();
  delete this->documentChannel_;
  delete this->temporaryDir_;
}

static void fillTemporaryGifFile( QTemporaryFile* tmpFile, const QList<QImage>& images )
{
  QDataStream ds(tmpFile);
  ds << images.size();

  for (int idx = 0; idx < images.size(); idx++) {
    QBuffer buf;
    images[idx].save(&buf, "png");
    //frames.at(idx)->pixmap().save(&buf, "png");

    int size = buf.size();
    ds << size;
    ds.writeRawData(buf.data().data(), size);
  }
}

void DataService::generageGifFromImages(const QStringList& files, const QString& outputFileName, int delay)
{
  static const QString stCreateGifProgramms = MnCommon::binPath("meteo") + "/meteo.createanimation";
  QList<QImage> images;
  for ( auto path : files ){
    images << this->loadLocalImage(path);
  }

  QTemporaryFile tmpFile(this->temporaryDir_->path() + QObject::tr("/gif_XXXXXX.gen"));
  if ( false == tmpFile.open() ) {
    error_log.msgBox() << QObject::tr("Не удается создать файл для генерации GIF");
    return;
  }

  fillTemporaryGifFile( &tmpFile, images );


  tmpFile.close();

  QProcess proc(this);
  QStringList arguments;
  arguments << (QString("-r ") + outputFileName)
      << (QString("-d ") + QString::number(delay))
      << tmpFile.fileName();

  proc.start(stCreateGifProgramms, arguments);

  if ( false == proc.waitForStarted() ) {
    error_log.msgBox() << QObject::tr("Ошибка запуска процесса для сохранения анимации %1").arg(stCreateGifProgramms);
    return ;
  }

  proc.waitForFinished(-1);
  if (0 != proc.exitCode()) {
    error_log.msgBox() << QObject::tr("Произошла ошибка при выполнении генерации изображения");
  }

  return;
}

const QImage& DataService::getFile( DocumentHeader::DocumentClass type, const QString& fileid )
{
  DocumentUID uid(type, fileid);
  static const QImage nullimage;
  this->loadFile(type, fileid, true);
  switch (type) {  
  case DocumentHeader::kFormalDocument:{
    //TODO = что тут делать?
    error_log << "TODO";
    break;
  }
  case DocumentHeader::kFax:{    
    if ( true == loadedImages_.contains( uid ) ){
      return loadedImages_[ uid ];
    }
    break;
  }
  case DocumentHeader::kMapImage:{
    if ( true == loadedImages_.contains(uid) ){
      return loadedImages_[uid];
    }
    break;
  }
  case DocumentHeader::kAmbiguous:
  case DocumentHeader::kLocalDocument:{
    error_log << QObject::tr("TODO");
    break;
  }
  case DocumentHeader::kSateliteDocument:{
    if ( true == loadedImages_.contains( uid ) ){
      return loadedImages_[ uid ];
    }
  }

  }
  return nullimage;
}

void DataService::slotFileChanged( const QString& path )
{  
  this->fileWatcher_.addPath(path);
  for ( auto documentuid : loadedImages_.keys() ){
    if ( documentuid.second == path ) {
      loadedImages_.remove(documentuid);
      emit signalFileChanged( documentuid );
    }
  }

  if ( true == clonedDocuments_.contains(path) ){
    auto clone = clonedDocuments_[path]    ;
    delete clone;
    clonedDocuments_.remove(path);
    emit signalFileChanged( DocumentUID(DocumentHeader::kLocalDocument, path) );
  }

  if ( true == loadedLocalPdf_.contains(path) ){
    loadedLocalPdf_.remove(path);
  }

  for ( auto uid : this->loadedDocuments_.keys() ){
    auto tmpFile = this->loadedDocuments_[uid];
    if ( 0 == tmpFile->fileName().compare(path) ){
      auto pdfPath = this->generatePdfFromOdt(tmpFile->fileName());
      emit signalFileChanged(uid);
      break;
    }
  }
}

void DataService::loadFile( DocumentUID uid, bool preloadOnly )
{
  this->loadFile(uid.first, uid.second, preloadOnly);
}

void DataService::loadFile( DocumentHeader::DocumentClass type, const QString& file, bool preloadOnly )
{
  switch (type)
  {
  case DocumentHeader::kFormalDocument:{
    this->loadFormalDocumentFile( file, preloadOnly );
    break;
  }
  case DocumentHeader::DocumentClass::kFax:{
    this->loadFax( file, preloadOnly );
    return;
  }
  case DocumentHeader::DocumentClass::kLocalDocument: {
    this->loadLocalFile(file, preloadOnly);
    break;
  }
  case DocumentHeader::kSateliteDocument:{
    this->loadSateliteImageFile(file, preloadOnly);
    break;
  }
  case DocumentHeader::DocumentClass::kMapImage:{
    this->loadMapImageFile( file, preloadOnly );
    break;
  }
  case DocumentHeader::DocumentClass::kAmbiguous:
    error_log << QObject::tr("Неопределенный тип докуемнта");
    break;
  }
}

void DataService::loadFormalDocumentFile( QString fileName, bool preloadOnly )
{
  DocumentUID uuid( DocumentHeader::kFormalDocument, fileName );
  //QString fsPath = this->getFsFilePath(DocumentHeader::kFormalDocument, fileName);

  if ( false == this->loadedDocuments_.contains(uuid) ){
    meteo::map::proto::ExportRequest req;
    if ( false == this->tryConnect() ){
      error_log.msgBox() << kErrorMessageCanNotConnectToDocumentService;
      return;
    }

    req.set_path(fileName.toStdString());
    req.set_format(meteo::map::proto::kOds);
    auto resp = std::unique_ptr<meteo::map::proto::ExportResponse> ( documentChannel_->remoteCall(&meteo::map::proto::DocumentService::GetDocument, req, 10000 ) );

    if ( nullptr == resp ){
      error_log.msgBox() << msglog::kServiceRequestFailed.arg(global::serviceTitle(settings::proto::kMap));
      return;
    }
    if ( false == resp->result() ){
      error_log.msgBox() << msglog::kServiceRequestFailed.arg(global::serviceTitle(settings::proto::kMap));
      error_log << resp->comment();
      return;
    }

    QByteArray data = QByteArray::fromStdString( resp->data() );

    static const QString odsFileTemplate = QObject::tr("%1/formal_documents/%2_XXXXXX.ods").arg(this->temporaryDir_->path());
    auto filePathTemplate = odsFileTemplate.arg(fileName.replace( '/', '_' ));
    QFileInfo fileInfo( filePathTemplate );
    if ( (false == fileInfo.dir().exists()) && (false == QDir("/").mkpath(fileInfo.dir().path())) ) {
      error_log.msgBox() << msglog::kFileNewFailed.arg(fileInfo.dir().path());
      return;
    }
    QTemporaryFile *outputTemporaryFile = new QTemporaryFile(filePathTemplate);
    if ( false == outputTemporaryFile->open() ){
      error_log.msgBox() << msglog::kDirCreateFailed.arg(outputTemporaryFile->fileName());
      delete outputTemporaryFile;
      return;
    }

    QFile ofile(outputTemporaryFile->fileName());
    if ( false == ofile.open(QIODevice::WriteOnly) ){
      error_log.msgBox() << meteo::msglog::kFileWriteFailed.arg(outputTemporaryFile->fileName()).arg(ofile.errorString());
      delete outputTemporaryFile;
      return;
    }    
    ofile.write(data);
    ofile.flush();    
    fileWatcher_.addPath(ofile.fileName());

    ofile.close();
    this->loadedDocuments_[uuid] = outputTemporaryFile;
  }

  if ( false == preloadOnly && true == this->loadedDocuments_.contains(uuid)){
    auto tmpFile = this->loadedDocuments_[uuid];
    QList<QImage> imagePages;
    auto pdfPath = this->generatePdfFromOdt(tmpFile->fileName());
    imagePages = this->loadPdfDocument(pdfPath);
    emit signalFileLoaded(imagePages);
  }
}

void DataService::loadMapImageFile( const QString& fileName, bool preloadOnly )
{  
  DocumentUID uuid( DocumentHeader::kMapImage, fileName );
  QString fsPath = this->getFsFilePath(DocumentHeader::kMapImage, fileName);

  if ( false == this->loadedImages_.contains(uuid) ){
    meteo::map::proto::ExportRequest req;
    if ( false == this->tryConnect() ){
      error_log.msgBox() << kErrorMessageCanNotConnectToDocumentService;
      return;
    }

    req.set_path(fileName.toStdString());
    req.set_format(meteo::map::proto::kPng);
    auto resp = std::unique_ptr<meteo::map::proto::ExportResponse> ( documentChannel_->remoteCall(&meteo::map::proto::DocumentService::GetDocument, req, 10000 ) );

    if ( nullptr == resp ){
      error_log.msgBox() << msglog::kServiceRequestFailed.arg(global::serviceTitle(settings::proto::kMap));
      return;
    }
    if ( false == resp->result() ){
      error_log.msgBox() << msglog::kServiceRequestFailed.arg(global::serviceTitle(settings::proto::kMap));
      error_log.msgBox() << resp->comment();
      return;
    }

    QByteArray data = QByteArray::fromStdString( resp->data() );

    QFile file(fsPath);
    QFileInfo fileInfo(fsPath);
    if ( true == DocumentHeader::getPdfSuffixes().contains(fileInfo.suffix()) ||
         true == DocumentHeader::getOfficeDocumentSuffixes().contains(fileInfo.suffix())) {
      QFile ofile(fsPath);
      if ( false == ofile.open(QIODevice::WriteOnly) ){
        error_log.msgBox() << meteo::msglog::kFileWriteFailed.arg(fsPath).arg(ofile.errorString());
        return;
      }
      ofile.write(data);
      ofile.flush();
      ofile.close();
    }
    else if ( true == DocumentHeader::getImageSuffixes().contains(fileInfo.suffix()) ){
      auto image  = QImage::fromData(data);
      this->loadedImages_[uuid] = image;
      QFileInfo fi(file.fileName());
      QDir dir = fi.absoluteDir();
      dir.mkdir(dir.absolutePath());
      if ( (true == file.open(QIODevice::WriteOnly)) && (true == image.save(&file)) ){
        this->documentsToRemove_ << fsPath;
      }
      else {
        error_log.msgBox() << msglog::kFileNewFailed.arg(fsPath) << file.errorString();
      }
    }
    else {
      error_log.msgBox() << QObject::tr("Неизвестный тип файла, не удается получить файл с сервиса");
      return;
    }
  }

  if ( false == preloadOnly ){
    QFileInfo fileInfo(fsPath);
    QList<QImage> imagePages;
    if ( true == DocumentHeader::getPdfSuffixes().contains(fileInfo.suffix()) ){
      imagePages = this->loadPdfDocument(fsPath);
    }
    else if ( true == DocumentHeader::getOfficeDocumentSuffixes().contains(fileInfo.suffix())) {
      auto pdfPath = this->generatePdfFromOdt(fsPath);
      imagePages = this->loadPdfDocument(pdfPath);
    }
    else if ( true == DocumentHeader::getImageSuffixes().contains(fileInfo.suffix()) ){
      imagePages << loadedImages_[uuid];
    }
    else {
      error_log.msgBox() << QObject::tr("Неизвестный тип файла, не удается получить файл с сервиса");
      return;
    }
    emit signalFileLoaded(imagePages);
  }
}

bool DataService::tryConnect()
{
  int cnt = 100;
  if ( cnt != 0 && ( nullptr == documentChannel_ || false == documentChannel_->isConnected() ) )  {
    --cnt;
    if ( nullptr != documentChannel_ ) {
      delete documentChannel_;
    }
    documentChannel_ = meteo::global::serviceChannel( meteo::settings::proto::kMap );
  }

  return nullptr != documentChannel_ && true == documentChannel_->isConnected();
}

QImage DataService::loadFax(const QString& faxid)
{
  if ( false == tryConnect() ) {
    error_log.msgBox() << kErrorMessageCanNotConnectToDocumentService;
    return QImage();
  }
  meteo::map::proto::FaxRequest request;
  request.set_header_only(false);
  auto faxGetRequestParam = request.add_faxes();
  faxGetRequestParam->set_id(faxid.toStdString());

  auto faxResponce = std::unique_ptr<meteo::map::proto::FaxReply> ( documentChannel_->remoteCall(&meteo::map::proto::DocumentService::GetFaxes, request, 30000 ) );
  if ( nullptr == faxResponce ) {
    return QImage();
  }

  QByteArray data_;
  if ( false == faxResponce->result() ){
    error_log << faxResponce->comment();
    return QImage();
  }

  if ( 1 != faxResponce->faxes_size() ){
    error_log.msgBox() << QObject::tr("Ошибка - ожидалось, что будет получен ровно 1 документ, получено %1")
                 .arg(faxResponce->faxes_size());
    return QImage();
  }
  auto singleFax = faxResponce->faxes(0);
  if ( false == singleFax.has_msg() ){
    error_log.msgBox() << QObject::tr("Ошибка - ожидалось, что поле msg будет заполнено");
    return QImage();
  }

  QByteArray data = QByteArray::fromStdString(singleFax.msg());

  return QImage::fromData(data,"TIFF");
}

QString DataService::getFsFilePath(DocumentHeader::DocumentClass docClass, const QString& fileName )
{
  switch (docClass) {
  case DocumentHeader::kFormalDocument:{
    DocumentUID uid(docClass, fileName);
    if ( true == this->loadedDocuments_.contains(uid) ){
      return this->loadedDocuments_[uid]->fileName();
    }
    else {
      warning_log << QObject::tr("Ошибка: документ не был загружен");
      return QString();
    }    
  }
  case DocumentHeader::kMapImage:{
    static const QString mapdir = QObject::tr("%1/map_images").arg(this->temporaryDir_->path());
    auto path = QObject::tr("%1/%2").arg(mapdir).arg( fileName );
    QFileInfo info(path);
    if( false == info.dir().exists() && false == QDir("/").mkpath( info.dir().path() ) ) {
      error_log.msgBox() << QObject::tr("Ошибка при создании каталога %1").arg(info.dir().path() );
      return QString();
    }
    return path;
  }
  case DocumentHeader::kLocalDocument:{
    return fileName;
  }
  case DocumentHeader::kFax:{
    static const QString faxesDir = QObject::tr("%1/faxes").arg(this->temporaryDir_->path());
    auto path = QObject::tr("%1/%2").arg(faxesDir).arg(fileName);
    QFileInfo info(path);
    if ( false == info.dir().exists() && false == QDir("/").mkpath( info.dir().path() ) ){
      error_log.msgBox() << QObject::tr("Ошибка при создании каталога %1").arg(info.dir().path() );
      return QString();
    }
    return path;
  }
  case DocumentHeader::kSateliteDocument:{
    static const QString sateliteDir = QObject::tr("%1/satelite_images").arg(this->temporaryDir_->path());
    auto path = QObject::tr("%1/%2").arg(sateliteDir).arg(fileName);
    QFileInfo info(path);
    if ( false == info.dir().exists() && false == QDir("/").mkpath( info.dir().path() ) ){
      return QString();
    }
    return path;
  }
  case DocumentHeader::kAmbiguous:
    error_log << QObject::tr("Неопределенный тип докуемнта");
    break;
  }
  error_log <<QObject::tr("Ошибка - этого не должно было никогда случиться");
  return QString();
}

void DataService::loadFax( const QString& faxid, bool preloadOnly )
{
  DocumentUID uid(DocumentHeader::kFax, faxid);
  if ( false == this->loadedImages_.contains(uid) ) {
    auto image = loadFax(faxid);
    if ( false == image.isNull() ) {
      auto faxpath = this->getFsFilePath(DocumentHeader::kFax, faxid);
      QFile file(faxpath);
      if ( (true == file.open(QIODevice::WriteOnly)) && (true == image.save(&file, "png")) ) {
        this->loadedImages_.insert( uid, image );
        this->loadedFaxes_ << faxid;
      }
      else {
        error_log.msgBox() << QObject::tr("Ошибка при создании файла на жестком диске: %1. Часть функций могут работать неправильно").arg(faxpath);
      }
    }
  }

  if ( false == preloadOnly) {
    QList<QImage> fax;
    fax << loadedImages_[uid];
    emit signalFileLoaded(fax);
  }
}

void DataService::loadSateliteImageFile( const QString& fileid, bool preloadOnly )
{
  DocumentUID uuid(DocumentHeader::kSateliteDocument, fileid);  
  if ( false == this->loadedImages_.contains(uuid) ||
       true == this->loadedImages_[uuid].isNull() ){
    auto image = this->loadSateliteImage(fileid);
    this->loadedImages_.insert( uuid, image );

    QString fsPath = this->getFsFilePath(DocumentHeader::kSateliteDocument, fileid);
    QFile file(fsPath);
    if ( (true == file.open(QIODevice::WriteOnly)) && (true == image.save(&file, "png")) ){
      this->documentsToRemove_ << fsPath;
    }
    else {
      error_log << QObject::tr("Ошибка при создании файла на жестком диске: %1. Часть функций могут работать неправильно").arg(fsPath);
    }
  }

  if ( false == preloadOnly ){
    QList<QImage> image;
    image << loadedImages_[uuid];
    emit signalFileLoaded(image);
  }
}

QImage DataService::loadSateliteImage(const QString& fileid)
{

  if ( false == tryConnect() ) {
    error_log.msgBox() << kErrorMessageCanNotConnectToDocumentService;
    return QImage();
  }
  meteo::map::proto::GetSateliteImageRequest request;
  request.set_gridfs_file_id(fileid.toStdString());

  auto faxResponce = std::unique_ptr<meteo::map::proto::GetSateliteImageReply> ( documentChannel_->remoteCall(&meteo::map::proto::DocumentService::GetSateliteImageFile, request, 10000 ) );
  if ( nullptr == faxResponce ) {
    error_log.msgBox() << meteo::msglog::kNoConnect.arg("Сервис формирования документов");
    return QImage();
  }

  QByteArray data_;
  if ( false == faxResponce->result() ){
    error_log.msgBox() << QObject::tr("Не найден файл изображения");
    error_log << faxResponce->comment();
    return QImage();
  }

  if ( true == faxResponce->has_data() ){
    QByteArray ba = QByteArray::fromRawData( faxResponce->data().c_str(), faxResponce->data().size() );
    return QImage::fromData(ba);
  }
  else {
    error_log.msgBox() << QObject::tr("Ошибка: ожидалось, что в ответе будет заполнено поле data или result установлен в false");
    return QImage();
  }  
}

void DataService::loadLocalFile(const QString& fileName, bool preloadOnly )
{
  QFileInfo info(fileName);
  auto suffix = info.suffix();
  if ( true == DocumentHeader::getImageSuffixes().contains(suffix) ){
    QList<QImage> imageList;
    imageList << this->loadLocalImage(fileName);
    if ( false == preloadOnly ) {
      emit signalFileLoaded(imageList);
    }
  }
  else if ( true == DocumentHeader::getOfficeDocumentSuffixes().contains(suffix) ){
    auto copyName = this->copyFileToTemporary(fileName);
    auto pdfName = this->generatePdfFromOdt(copyName);
    auto image = this->loadPdfDocument(pdfName);
    if ( false == preloadOnly ) {
      emit signalFileLoaded(image);
    }
  }
  else if ( true == DocumentHeader::getPdfSuffixes().contains(suffix) ){
    auto image = this->loadPdfDocument(fileName);
    if ( false == preloadOnly ) {
      emit signalFileLoaded(image);
    }
  }
  else  {
    error_log << QObject::tr("Неизвестный тип документа");
  }

  this->fileWatcher_.addPath(fileName);
}

void DataService::printDocumentFromImage(const QString& path )
{
  auto image = this->loadLocalImage(path);
  if ( false == image.isNull() ){
    this->printDocumentFromImage(image, "IMAGE");
  }
}


void DataService::savePrintSettings()
{
  // проверим наличие директории под настройки, если нет - создадим
  QFileInfo fi(kPrintSettingsFilename);
  QDir dir (fi.path());
  if ( false == dir.exists( dir.absolutePath() ) ) {
    if ( false == dir.mkpath( dir.absolutePath() ) ) {
      error_log.msgBox() << QObject::tr("не удалось создать каталог для сохранения настроек");
      return;
    }
  }

  QFile file(kPrintSettingsFilename);
  if ( file.open(QIODevice::WriteOnly) == false ) {
    error_log.msgBox() << QObject::tr("не удалось сохранить пользовательские настройки печати");
    return;
  }
  QDataStream out(&file);
  out << printSettingsMap_;
  file.close();
}

void DataService::loadPrintSettings(QPrinter * printer, QString docType)
{
  if ( false == printSettingsMapLoadedCheck_ ) {// если true, значит попытка подгрузки из файла уже была, неважно, успешная или нет
    printSettingsMapLoadedCheck_ = true;
    // загрузим из файла
    QFile file(kPrintSettingsFilename);
    if ( file.open(QIODevice::ReadOnly) == false ) {
      file.close();
      warning_log << QObject::tr("Не найден файл с пользовательскими настройками печати");
      return;
    }
    QDataStream in(&file);
    in >> printSettingsMap_;
    file.close();
  }

  if ( printSettingsMap_.contains(docType) == false ) {
    return;
  }

  printer->setPrinterName( printSettingsMap_[docType][kPrinterName].toString() );
  printer->setOutputFileName( printSettingsMap_[docType][kOutputFileName].toString() );

  //QPageSize::Unit unitsForPageSize =  static_cast <QPageSize::Unit> (intList.at(0));
  QPageSize::Unit unitsForPageSize = static_cast<QPageSize::Unit>(printSettingsMap_[docType][kPageUnit].toInt()) ;
  QPageLayout::Unit unitsForLayout = static_cast <QPageLayout::Unit> (unitsForPageSize);
  QPageLayout pageLayout = printer->pageLayout();// поскольку возвращается НЕ указатель, то поработаем с копией и в конце запишем этот layout в printer

  pageLayout.setUnits(unitsForLayout);
  QPageLayout::Orientation orientation =  static_cast <QPageLayout::Orientation> ( printSettingsMap_[docType][kPageOrientation].toInt() );
  pageLayout.setOrientation(orientation);
  QPrinter::ColorMode colorMode = static_cast <QPrinter::ColorMode> ( printSettingsMap_[docType][kPrinterColorMode].toInt() );
  printer->setColorMode(colorMode);

  QPageSize::PageSizeId pageSizeId = static_cast <QPageSize::PageSizeId> ( printSettingsMap_[docType][kPageUnit].toInt() );

  if ( pageSizeId == QPageSize::Custom ) { // размер страницы "вручную" задаём только если он не стандартный
    QSizeF size( printSettingsMap_[docType][kPageWidth].toDouble(), printSettingsMap_[docType][kPageHeight].toDouble());
    QPageSize pageSize(size, unitsForPageSize);//, QString(), QPageSize::ExactMatch);
    pageLayout.setPageSize(pageSize);
  }
  else {
    QPageSize pageSize(pageSizeId);
    pageLayout.setPageSize(pageSize);
  }

  pageLayout.setLeftMargin( printSettingsMap_[docType][kPageLeftField].toDouble() );
  pageLayout.setTopMargin( printSettingsMap_[docType][kPageTopField].toDouble() );
  pageLayout.setRightMargin( printSettingsMap_[docType][kPageRightField].toDouble() );
  pageLayout.setBottomMargin( printSettingsMap_[docType][kPageBottomField].toDouble() );

  printer->setPageLayout(pageLayout);
}


void DataService::addPrintSettings(QPrinter * printer, QString docType)
{
  /*
   * TODO:
   * printer->printerName() может быть пустым на DEBIAN. Проверить на ASTRA:
   * In short: The reason for the issue is that CUPS supports driverless printers (info), and Qt does not.
   * Qt uses CUPS API to return the list of printers in system (availablePrinterNames) without checking, but when it constructs QPrinterInfo,
   * it checks if the printer has a PPD driver.
   * If not, the printer is considered invalid, and Qt returns empty name for it.
  */
  printSettingsMap_[docType][kPrinterName] = printer->printerName();
  printSettingsMap_[docType][kOutputFileName] = printer->outputFileName();

  QPageSize::Unit units = static_cast <QPageSize::Unit> (printer->pageLayout().units());  

  printSettingsMap_[docType][kPageWidth] = printer->pageLayout().pageSize().size(units).width();
  printSettingsMap_[docType][kPageHeight] = printer->pageLayout().pageSize().size(units).height();
  printSettingsMap_[docType][kPageLeftField] = printer->pageLayout().margins(printer->pageLayout().units()).left();
  printSettingsMap_[docType][kPageRightField] = printer->pageLayout().margins(printer->pageLayout().units()).right();
  printSettingsMap_[docType][kPageTopField] = printer->pageLayout().margins(printer->pageLayout().units()).top();
  printSettingsMap_[docType][kPageBottomField] = printer->pageLayout().margins(printer->pageLayout().units()).bottom();

  printSettingsMap_[docType][kPageUnit] = printer->pageLayout().units();
  printSettingsMap_[docType][kPageSize] = printer->pageLayout().pageSize().id();
  printSettingsMap_[docType][kPageOrientation] = printer->pageLayout().orientation();
  printSettingsMap_[docType][kPrinterColorMode] = printer->colorMode();
}

void DataService::printDocumentFromImage( const QImage& page, const QString& docType )
{  
  static QMap <QString, int> copiesCountMap;//число копий не сохраняется, хранится только в сеансе

  QPrinter *printer = new QPrinter(QPrinter::HighResolution);
  loadPrintSettings(printer, docType);
  if (copiesCountMap.contains(docType) == true) {
    printer->setCopyCount(copiesCountMap.value(docType));
  }

  QPrintDialog printDialog(printer);


  if (printDialog.exec() == QDialog::Accepted) {
    addPrintSettings(printer, docType);
    copiesCountMap.insert(docType, printer->copyCount());


    QPainter painter(printer);

    // центровка изображения
    int trX = 0, trY = 0;
    QSize pixmapSize = page.size();
    QSize pageSize = printer->pageRect().size();
    pageSize.scale(pixmapSize, Qt::KeepAspectRatioByExpanding);
    if (pixmapSize.width() != pageSize.width()) {
      trX = ( pageSize.width() - pixmapSize.width() ) / 2;
      trX = abs(trX);
    }
    if (pixmapSize.height() != pageSize.height()) {
      trY = ( pageSize.height() - pixmapSize.height() ) / 2;
      trY = abs(trY);
    }

    QRect pageRect = printer->pageRect();
    painter.setViewport(0, 0, pageRect.width(), pageRect.height());


    painter.setWindow(0, 0, pageSize.width(), pageSize.height());

    painter.drawPixmap(trX, trY, QPixmap::fromImage(page));
  }
  delete printer;
}

const QList<QImage>& DataService::loadPdfDocument( const QString& document )
{
  static const QList<QImage> nullImageList;
  if ( false == loadedLocalPdf_.contains(document) ) {
    auto doc = std::unique_ptr<Poppler::Document>( Poppler::Document::load(document) );

    if ( nullptr == doc || true == doc->isLocked()) {
      error_log.msgBox() << QObject::tr("Ошибка при чтении PDF документа");
      return nullImageList;
    }    
    doc->setRenderHint(Poppler::Document::TextAntialiasing);
    QTime ttt; ttt.start();
    for ( int pageNumber = 0; (pageNumber < doc->numPages()) && ( pageNumber <= maxPagesNumber ) ; ++ pageNumber) {      
      auto pdfPage = std::unique_ptr<Poppler::Page>(doc->page(pageNumber));
      if ( nullptr != pdfPage ) {
        this->loadedLocalPdf_[document] << pdfPage->renderToImage(resolution,resolution);
      }
    }    
  }
  return loadedLocalPdf_[document];
}

void DataService::loadList( DocumentHeader::DocumentClass type, const QDateTime& dtStart, const QDateTime& dtEnd )
{  
  switch (type)
  {
  case DocumentHeader::kFormalDocument:{
    loadAndEmitFormalDocumentsList( dtStart, dtEnd );
    break;
  }
  case DocumentHeader::kMapImage:{
    loadAndEmitMapImagesList( dtStart, dtEnd );
    return;
  }
  case DocumentHeader::DocumentClass::kFax: {
    loadAndEmitFaxList( dtStart, dtEnd );
    return;
  }
  case DocumentHeader::DocumentClass::kLocalDocument:{
    this->clearDocuments();
    emit signalListLoaded( &allDocuments_ );
    return;
  }
  case DocumentHeader::DocumentClass::kSateliteDocument:{
    loadAndEmitSateliteImageList( dtStart, dtEnd );
    return;
  }
  case DocumentHeader::kAmbiguous:
    error_log << QObject::tr("Неопределенный тип докуемнта");
    break;
  }
}

void DataService::loadAndEmitFormalDocumentsList( const QDateTime& beginDt, const QDateTime& endDt )
{
  this->loadFormalDocumentList( beginDt, endDt );
  emit signalListLoaded( &allDocuments_ );
}

bool DataService::loadFormalDocumentList( const QDateTime& beginDt, const QDateTime& endDt )
{
  meteo::map::proto::MapRequest req;
  req.set_date_start( beginDt.toString(Qt::ISODate).toStdString() );
  req.set_date_end( endDt.toString(Qt::ISODate).toStdString() );
  //req.set_format(meteo::map::proto::kOds);
  this->clearDocuments();

  if ( false == tryConnect() ) {
    error_log.msgBox() << kErrorMessageCanNotConnectToDocumentService;
    return false;
  }
  auto resp = std::unique_ptr<meteo::map::proto::MapList> ( documentChannel_->remoteCall(&meteo::map::proto::DocumentService::GetAvailableDocuments, req, 10000 ) );

  if ( nullptr == resp ){
    error_log.msgBox() << meteo::msglog::kServiceAnswerFailed.arg("Сервис формирования документов");
    return false;
  }

  if ( false == resp->result() ){
    error_log.msgBox() << QObject::tr("При выполнении запроса произошла ошибка");
    error_log << resp->comment();
    return false;
  }

  for ( int i = 0; i < resp->map_size(); ++i ){    
    auto map = resp->map(i);
    if ( meteo::map::proto::kOds != map.format() && meteo::map::proto::kOdt != map.format() ) {
      continue;
    }    
    auto name = map.has_title()? QString::fromStdString(map.title()):
                                 QString::fromStdString(map.name());
    auto dt = NosqlQuery::datetimeFromString(map.datetime());
    int hour = map.hour();
    int center = map.center();
    int model = map.model();
    auto job = map.has_job_title() ? QString::fromStdString(map.job_title()):
                                     QString::fromStdString(map.job_name());

    auto fpath = QString::fromStdString(map.path());


    allDocuments_ << new DocumentFormal( name, dt, hour, center, model, job, fpath );
  }  
  return true;
}

void DataService::loadAndEmitMapImagesList( const QDateTime& beginDt, const QDateTime& endDt )
{
  this->loadMapImageList( beginDt, endDt );
  emit signalListLoaded( &allDocuments_ );
}

bool DataService::loadMapImageList( const QDateTime& beginDt, const QDateTime& endDt )
{
  meteo::map::proto::MapRequest req;
  req.set_date_start( beginDt.toString(Qt::ISODate).toStdString() );
  req.set_date_end( endDt.toString(Qt::ISODate).toStdString() );  
  this->clearDocuments();

  if ( false == tryConnect() ) {
    error_log.msgBox() << kErrorMessageCanNotConnectToDocumentService;
    return false;
  }
  auto resp = std::unique_ptr<meteo::map::proto::MapList> ( documentChannel_->remoteCall(&meteo::map::proto::DocumentService::GetAvailableDocuments, req, 10000 ) );

  if ( nullptr == resp ){
    error_log.msgBox()<< meteo::msglog::kServiceAnswerFailed.arg("Сервис формирования документов");
    return false;
  }

  if ( false == resp->result() ){
    error_log.msgBox() << QObject::tr("При выполнении запроса произошла ошибка");
    error_log << resp->comment();
    return false;
  }

  for ( int i = 0; i < resp->map_size(); ++i ){    
    auto map = resp->map(i);
    if ( meteo::map::proto::DocumentFormat::kBmp != map.format() &&
         meteo::map::proto::DocumentFormat::kJpeg != map.format() &&
         meteo::map::proto::DocumentFormat::kPng != map.format() ){
      continue;
    }
    auto title = map.has_title()? QString::fromStdString(map.title()):
                                  QString::fromStdString(map.name());
    auto dt = NosqlQuery::datetimeFromString(map.datetime());
    int hour = map.hour();
    int center = map.center();
    int model = map.model();    
    auto jobName = map.has_job_name()? QString::fromStdString(map.job_title()):
                                       QString::fromStdString(map.job_name());
    auto fpath = QString::fromStdString(map.path());


    allDocuments_ << new DocumentMapImage( title, dt, hour, center, model, jobName, fpath );
  }
  return true;
}

void DataService::loadAndEmitSateliteImageList( const QDateTime& beginDt, const QDateTime& endDt )
{
  meteo::map::proto::GetSateliteImageListRequest request;
  request.set_dt_begin(beginDt.toString(Qt::ISODate).toStdString());
  request.set_dt_end(endDt.toString(Qt::ISODate).toStdString());


  this->clearDocuments();
  if ( false == tryConnect() ) {
    error_log.msgBox() << kErrorMessageCanNotConnectToDocumentService;
    return;
  }

  auto resp = std::unique_ptr<meteo::map::proto::GetSateliteImageListReply> ( documentChannel_->remoteCall(&meteo::map::proto::DocumentService::GetSateliteImageList, request, 10000 ) );
  if ( nullptr == resp ) {
    error_log.msgBox() << meteo::msglog::kServiceAnswerFailed.arg("Сервис формирования документов");
    return;
  }
  if ( false == resp->result() ){
    error_log.msgBox() << QObject::tr("При выполнении запроса произошла ошибка.");
    error_log << resp->comment();
    return;
  }


  for ( int i = 0; i < resp->headers_size(); ++i ){
    auto singleHeader = resp->headers(i);
    auto tt = QObject::tr("%1%2")
        .arg(QString::fromStdString(singleHeader.t1()))
        .arg(QString::fromStdString(singleHeader.t2()));
    QString id = QString::fromStdString(singleHeader.id());
    QString filePath = QString::fromStdString(singleHeader.gridfs_id());
    auto dt = QDateTime::fromString(QString::fromStdString(singleHeader.dt()), Qt::ISODate);
    QString aa = QObject::tr("%1%2")
        .arg(QString::fromStdString(singleHeader.a1()))
        .arg(QString::fromStdString(singleHeader.a2()));
    QString cccc = QString::fromStdString(singleHeader.cccc());
    int ii = singleHeader.ii();
    DocumentSatelite *satelite = new DocumentSatelite(id, filePath, dt, tt, aa, cccc, ii );
    this->allDocuments_ << satelite;
  }  
  emit signalListLoaded( &this->allDocuments_ );
}

const QString DataService::copyFileToTemporary(const QString& fileName )
{
  if ( false == this->clonedDocuments_.contains(fileName) ) {
    QFileInfo info(fileName);
    QTemporaryFile *clonedFile = new QTemporaryFile(
          QObject::tr("%1/clone_XXXXXX.%2")
          .arg(this->temporaryDir_->path())
          .arg(info.suffix())
          );
    clonedFile->open();
    this->clonedDocuments_.insert( fileName, clonedFile );

    QFile inputFile(fileName);
    if ( false == inputFile.open(QIODevice::ReadOnly) ) {
      error_log.msgBox() << meteo::msglog::kFileNotFound;
      return QString();
    }
    auto data = inputFile.readAll();

    clonedFile->write(data);
    clonedFile->flush();    
  }
  return clonedDocuments_[fileName]->fileName();
}

bool DataService::loadFaxList(const QDateTime& dtStart, const QDateTime& dtEnd)
{
  this->clearDocuments();

  if ( false == tryConnect() ) {
    error_log.msgBox() << kErrorMessageCanNotConnectToDocumentService;
    return false;
  }
  meteo::map::proto::FaxRequest request;
  request.set_header_only(true);
  auto faxGetRequestParam = request.add_faxes();
  faxGetRequestParam->set_dt_start(dtStart.toString(Qt::ISODate).toStdString());
  faxGetRequestParam->set_dt_end(dtEnd.toString(Qt::ISODate).toStdString());

  auto faxResponce = std::unique_ptr<meteo::map::proto::FaxReply> ( documentChannel_->remoteCall(&meteo::map::proto::DocumentService::GetFaxes, request, 30000 ) );
  if ( nullptr == faxResponce ) {
    return false;
  }
  for ( int i = 0; i < faxResponce->faxes_size(); ++i) {
    auto singleFax = faxResponce->faxes(i);
    auto faxid = QString::fromStdString(singleFax.id());
    auto faxt1t2 = QString::fromStdString(singleFax.t1()) +
        QString::fromStdString(singleFax.t2());
    auto faxa1a2 = QString::fromStdString(singleFax.a1()) +
        QString::fromStdString(singleFax.a2());
    auto faxccc = QString::fromStdString(singleFax.cccc());
    auto dt = NosqlQuery::datetimeFromString(singleFax.dt_start());
    auto faxName = QObject::tr("Факс от %1 за %2")
        .arg(faxccc)
        .arg(dt.toString(Qt::ISODate));
    int ii = singleFax.ii();
    auto fax = new DocumentFax( faxName, dt, faxid, faxt1t2, faxa1a2, faxccc, ii );
    this->allDocuments_ << fax;
  }
  return true;
}

void DataService::loadAndEmitFaxList( const QDateTime& dtStart, const QDateTime& dtEnd )
{
  if ( false == this->loadFaxList(dtStart, dtEnd) ) {
    error_log.msgBox()<< meteo::msglog::kServiceAnswerFailed.arg("Сервис формирования документов");
  }
  emit signalListLoaded( &allDocuments_ );
}

void DataService::clearDocuments()
{
  while ( false == allDocuments_.isEmpty() )
  {
    auto doc = allDocuments_.first();
    allDocuments_.removeFirst();
    delete doc;
  }
}

void DataService::printOfficeDocument( const QString& path )
{  
  QStringList args;
  args << QObject::tr("-p")
       << path;
  if ( false == QProcess::startDetached(QObject::tr("soffice"), args ) ) {
    error_log.msgBox() << QObject::tr("Ошибка при запуске процесса soffice");
  }
}

void DataService::printDocument( DocumentHeader::DocumentClass docClass, const QString& fileName )
{
  switch (docClass) {
  case DocumentHeader::kFormalDocument:{
    auto fsFile = this->getFsFilePath(docClass, fileName );    
    printOfficeDocument(fsFile);
    break;
  }
  case DocumentHeader::kLocalDocument:{
    QFileInfo info(fileName);

    auto suffix = info.suffix();
    if ( true == DocumentHeader::getImageSuffixes().contains(suffix) ){
      this->printDocumentFromImage(fileName);
    }
    else if ( true == DocumentHeader::getOfficeDocumentSuffixes().contains(suffix) ){
      printOfficeDocument(fileName);
    }
    else if ( true == DocumentHeader::getPdfSuffixes().contains(suffix) ){
      this->openDocument(fileName);
    }
    else  {
      error_log << QObject::tr("Неизвестный тип документа");
    }
    break;
  }
  case DocumentHeader::kMapImage:
  case DocumentHeader::kFax:
  case DocumentHeader::kSateliteDocument:{
    DocumentUID uid(docClass, fileName);
    this->loadFax(fileName, true);
    QImage loadedFax = loadedImages_[uid];
    //loadedFax.invertPixels();
    this->printDocumentFromImage(loadedFax, "IMAGE");
    break;
  }
  case DocumentHeader::kAmbiguous:
    error_log << QObject::tr("Неопределенный тип докуемнта");
    break;
  }
}

void DataService::openDocument( DocumentHeader::DocumentClass docClass, const QString& fileId )
{
  this->openDocument(this->getFsFilePath( docClass, fileId ));
}

void DataService::openDocument(const QString& fsFileName)
{
  if ( false == fsFileName.isEmpty() ){
    auto url = QUrl::fromLocalFile( fsFileName );
    if ( false == QDesktopServices::openUrl(url) ) {
      error_log.msgBox() << QObject::tr("Не удается открыть URL: %1").arg(url.toString());
    }
  }
}

const QImage& DataService::loadLocalImage( const QString& imagePath )
{
  if ( false == loadedLocalImages_.contains(imagePath) ) {
    QFile file (imagePath);
    if ( false == file.open(QIODevice::ReadOnly) ) {
      error_log.msgBox() << meteo::msglog::kFileReadFailed
                            .arg(imagePath)
                            .arg(file.errorString());
      return nullImage;
    }
    loadedLocalImages_.insert( imagePath, QImage::fromData(file.readAll()) );
  }
  return loadedLocalImages_[imagePath];
}

const QString& DataService::generatePdfFromOdt(const QString& path )
{  
  static const QString& nullstr = QString();
  QFileInfo info(path);
  QString pdfFileName = QObject::tr("%1/pdf/%2%3")
      .arg(this->temporaryDir_->path())
      .arg(info.completeBaseName())
      .arg(".pdf");
  int index = generatedDocuments_.indexOf(pdfFileName);
  QFileInfo fileInfo(pdfFileName);
  if ( false == fileInfo.dir().exists() && false == QDir("/").mkpath(fileInfo.dir().path()) ) {
    error_log.msgBox() << QObject::tr("Ошибка при создании каталога для файла %1");
    return nullstr;
  }
  if ( -1 == index ) {
    qint64 auxNum = QDateTime::currentDateTime().toMSecsSinceEpoch();
    static const QString cmdTemplate = QObject::tr("libreoffice -env:UserInstallation=\"file:///%1/liboffice/LibO_Conversion_%2\" --headless --convert-to pdf  --outdir \"%4\" \"%3\"");
    QString cmd = cmdTemplate
        .arg(this->temporaryDir_->path())
        .arg(auxNum)
        .arg(path)
        .arg(fileInfo.dir().path());

    QProcess process;

    process.start(cmd);

    if ( false == process.waitForStarted() ) {
      error_log.msgBox() << QObject::tr("Ошибка при запуске процесса libreffoce");
      return nullstr;
    }

    if ( process.waitForFinished( ) == false) {
      error_log.msgBox() << QObject::tr("Ошибка при запуске процесса libreffoce");
      return nullstr;
    }

    generatedDocuments_.append(pdfFileName);
    index = generatedDocuments_.size() - 1;
  }  
  return generatedDocuments_[index];
}

void DataService::importFormalDocument( DocumentUID uid )
{
  static const QString& serviceSaveError = QObject::tr("При попытке соединения с сервосом произошла ошибка. Ваши изменения не будут сохранены!");
  if ( DocumentHeader::kFormalDocument != uid.first ) {
    error_log.msgBox() << QObject::tr("Ошибка во время вызова функции, ожидался формализованный документ");
    return;
  }
  auto fsPath = this->getFsFilePath( uid.first, uid.second );
  QFile file(fsPath);
  if ( false == file.open(QIODevice::ReadOnly)) {
    error_log.msgBox() << serviceSaveError;
    return;
  }
  auto data = file.readAll();

  if ( false == this->tryConnect() ) {
    error_log.msgBox() << serviceSaveError;
    return;
  }
  meteo::map::proto::ImportRequest req;
  req.set_path(uid.second.toStdString());
  req.set_data(data.toStdString());
  auto resp = std::unique_ptr<meteo::map::proto::Response> ( documentChannel_->remoteCall(&meteo::map::proto::DocumentService::ImportDocument, req, 10000 ) );
  if ( nullptr == resp ){
    error_log.msgBox() << serviceSaveError;
    return;
  }
}

void DataService::slotDocumentChanged( DocumentUID docuid )
{
  switch (docuid.first) {
  case DocumentHeader::DocumentClass::kFormalDocument:{    
    auto path = this->getFsFilePath(docuid.first, docuid.second);
    auto pdfPath = this->generatePdfFromOdt(path);
    this->generatedDocuments_.removeAll(pdfPath);
    this->loadedLocalPdf_.remove(pdfPath);

    this->importFormalDocument(docuid);
    break;
  }
  default:{
    break;
  }
  }
}


}
}
