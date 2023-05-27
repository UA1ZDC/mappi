#include "datamodel/wgtdocviewModel.h"
#include <cross-commons/debug/tlog.h>
#include <meteo/commons/rpc/rpc.h>
#include <meteo/commons/global/global.h>
#include <meteo/commons/proto/meteo.pb.h>
#include <qtemporaryfile.h>
#include <qurl.h>
#include <qapplication.h>

#include <qregexp.h>
#include <qprogressdialog.h>

#include <meteo/commons/global/common.h>
#include <meteo/commons/ui/conditionmaker/templates/conditiongenericprivate.h>

#include "custom/wgtdocviewpanel.h"

namespace meteo {
namespace documentviewer {


bool DataModel::preloadDocuments(const QStringList documents)
{
  int total = documents.size();
  int loaded = 0;
  QProgressDialog progressDialog(QObject::tr("Загрузка документов"), QObject::tr("Отменить"), 0, total );
  progressDialog.setModal(true);
  progressDialog.setWindowModality(Qt::WindowModality::ApplicationModal);
  progressDialog.setWindowTitle(QString(" "));
  progressDialog.show();
  for ( auto id: documents ){
    ++loaded;
    progressDialog.setValue(loaded);
    QApplication::processEvents();
    auto document = leftPanel_->getDocumentById(id);
    this->data_.loadFile(document->getClass(), document->getFileId(), true);
    if (progressDialog.wasCanceled() ){
      return false;
    }
  }
  return true;
}

bool DataModel::premakeTransparentImages(const QStringList documents)
{
  int total = documents.size();
  int loaded = 0;
  QProgressDialog progressDialog(QObject::tr("Подготовка изображений"), QObject::tr("Отметить"), 0, total );
  progressDialog.setModal(true);
  progressDialog.setWindowModality(Qt::WindowModality::ApplicationModal);
  progressDialog.setWindowTitle(QString());
  progressDialog.show();
  for ( auto id: documents ){
    ++loaded;
    progressDialog.setValue(loaded);
    QApplication::processEvents();
    auto document = leftPanel_->getDocumentById(id);
    this->getTransparentImage(document->getClass(), document->getFileId() );
    if (progressDialog.wasCanceled() ){
      return false;
    }
  }
  return true;
}

void DataModel::slotSaveAnimation(const QStringList documents, const QString& outputFileName, int delay )
{  
  QApplication::setOverrideCursor(Qt::WaitCursor);
  if ( false == this->preloadDocuments(documents) ){
    QApplication::restoreOverrideCursor();
    return;
  }
  QStringList paths;
  for ( auto id: documents ){
    auto document = leftPanel_->getDocumentById(id);
    paths << this->data_.getFsFilePath(document->getClass(), document->getFileId());
  }
  this->data_.generageGifFromImages(paths, outputFileName, delay);  
  QApplication::restoreOverrideCursor();
}

DataModel::DataModel(DocumentHeader::DocumentClass type):
  QObject (),
  documentClass_( type ),
  dtBegin_( QDateTime::fromMSecsSinceEpoch(0) ),
  dtEnd_( QDateTime::fromMSecsSinceEpoch(0) )
{
  QObject::connect( &data_, SIGNAL( signalFileLoaded(const QList<QImage>&) ),
                    this, SLOT( slotFileLoaded(const QList<QImage>&) ) );
  QObject::connect( &data_, &DataService::signalListLoaded,
                    this, &DataModel::slotListLoaded );
  QObject::connect( &data_, &DataService::signalFileChanged,
                    this, &DataModel::slotFileChanged );
}

DataModel::~DataModel()
{
  for ( auto doc: this->filteredList_ ){
    delete doc;
  }
  this->filteredList_.clear();
}

void DataModel::slotFileLoaded( const QList<QImage>& images )
{  
  emit signalFileLoaded(images);
}


void DataModel::slotFileChanged( const DocumentUID& uid )
{  
  if ( uid == this->lastLoadedDocument_ ) {
    switch (this->documentClass_) {
    case DocumentHeader::kFormalDocument:{      
      if ( uid == this->lastLoadedDocument_ ) {
        this->loadDocument(uid);        
      }
      break;
    }
    case DocumentHeader::kMapImage:{
      debug_log << QObject::tr("NOT IMPLEMENTED");
      break;
    }
    case DocumentHeader::kFax:{
      debug_log << QObject::tr("NOT IMPLEMENTED");
      break;
    }
    case DocumentHeader::kLocalDocument:{
      if ( uid == this->lastLoadedDocument_ ) {
        this->loadDocument( uid );
      }
      break;
    }    
    case DocumentHeader::kSateliteDocument:{
      debug_log << QObject::tr("NOT IMPLEMENTED");
      break;
    }
    case DocumentHeader::kAmbiguous:
      error_log << QObject::tr("Неопределенный тип докуемнта");
      break;
    }
  }
}

const QImage& DataModel::getImage( DocumentHeader* document )
{
  return this->getImage( document->getClass(), document->getFileId() );
}

const QImage& DataModel::getImage( DocumentHeader::DocumentClass type, const QString& fileId )
{
  return data_.getFile(type, fileId);
}

void DataModel::setDateRange(const QDateTime& dtBegin, const QDateTime& dtEnd )
{  
  if ( dtBegin == this->dtBegin_ && dtEnd == this->dtEnd_ ){
    emit signalListLoaded( &this->filteredList_ );
  }
  else {
    this->dtBegin_ = dtBegin;
    this->dtEnd_ = dtEnd;
    QApplication::setOverrideCursor(Qt::WaitCursor);
    this->data_.loadList( documentClass_, dtBegin_, dtEnd_ );
    QApplication::restoreOverrideCursor();
  }
}


void DataModel::printCurrentDocument( DocumentHeader::DocumentClass docClass, const QString& fileName )
{  
  this->data_.printDocument(docClass, fileName);
}

void DataModel::filterFaxTT( Condition tt)
{
  this->filterFaxTT_ = tt;
  this->filterDocuments();
  emit signalListLoaded( & this->filteredList_ );
}

void DataModel::filterFaxII( Condition ii)
{
  this->filterFaxII_ = ii;
  this->filterDocuments();
  emit signalListLoaded( & this->filteredList_ );
}

void DataModel::filterFaxAA( Condition aa)
{
  this->filterFaxAA_ = aa;
  this->filterDocuments();
  emit signalListLoaded( & this->filteredList_ );
}

void DataModel::filterFaxCCCC( Condition cccc)
{
  this->filterFaxCCCC_ = cccc;
  this->filterDocuments();
  emit signalListLoaded( & this->filteredList_ );
}

void DataModel::openDocument( DocumentHeader* doc )
{
  this->data_.openDocument( doc->getClass(), doc->getFileId() );
}

void DataModel::openDocument( DocumentHeader::DocumentClass docClass, const QString& docname )
{
  this->data_.openDocument( docClass, docname );
}

void DataModel::cleanFilters()
{
  this->filterFaxTT_ = Condition();
  this->filterFaxAA_ = Condition();
  this->filterFaxCCCC_ = Condition();
  this->filterFaxII_ = Condition();

  this->filterSateliteTT_ = Condition();
  this->filterSateliteAA_ = Condition();
  this->filterSateliteCCCC_ = Condition();
  this->filterSateliteII_ = Condition();

  this->filterMapImageName_ = Condition();
  this->filterMapImageHour_ = Condition();
  this->filterMapImageCenter_ = Condition();
  this->filterMapImageModel_ = Condition();
  this->filterMapImageJobName_ = Condition();
}

void DataModel::setDocumentClass( DocumentHeader::DocumentClass docClass )
{  
  this->cleanFilters();

  if ( this->documentClass_ != docClass ) {
    this->documentClass_ = docClass;
    this->refresh();
  } else {
    emit signalListLoaded( &this->filteredList_ );
  }
}

void DataModel::refresh(  )
{  
  QApplication::setOverrideCursor(Qt::WaitCursor);
  this->data_.loadList( documentClass_, dtBegin_, dtEnd_ );
  QApplication::restoreOverrideCursor();
}

void DataModel::loadDocument( const DocumentUID uid )
{
  QApplication::setOverrideCursor(Qt::WaitCursor);
  this->lastLoadedDocument_ = uid;
  this->data_.loadFile( uid );
  QApplication::restoreOverrideCursor();
}


QImage DataModel::makeTransparentFax( const QImage& original, const QColor &color )
{
  QImage image(original.convertToFormat(QImage::Format_ARGB32));
  for ( int i = 0; i < image.width(); ++i ) {
    for ( int j = 0; j < image.height(); ++j ){
      QColor clr( image.pixel(i,j) );
//      if ( image.pixelColor( i, j ) == color ){
      if ( clr == color ){
//        image.setPixelColor(i, j, Qt::transparent );
        image.setPixel( i, j, QColor(Qt::transparent).rgba() );
      }
    }
  }
  return image;
}

QImage DataModel::makeTransparentSateliteImage(const QImage& original, TransparencyMode transp, int value )
{
  QImage image(original.convertToFormat(QImage::Format_ARGB32));

  QVector<QVector<int> > black;
  QVector<QVector<int> > white;

  const int w = image.width(),
            h = image.height();
  switch (transp) {
  case kTransparencyNone:{
    return original;
  }
  case kTransparencyBlack:{
    black.resize(w);
    for (int i = 0; i < w; ++i) {
      black[i].resize(h);
    }

    int corr_coef = qRound(2.55 * value);
    for (int i = 0; i < w; ++i) {
      for (int j = 0; j < h; ++j) {
        int alpha = qGray(image.pixel(i,j));
        if (alpha < corr_coef) {
          alpha = 0;
        }
        else {
          alpha = 255;
        }
        black[i][j] = alpha;
      }
    }
    break;
  }
  case kTransparencyWhite:{
    white.resize(w);
    for (int i = 0; i < w; ++i) {
      white[i].resize(h);
    }

    int corr_coef = qRound(2.55 * value);
    for (int i = 0; i < w; ++i) {
      for (int j = 0; j < h; ++j) {
        int alpha = 255 - qGray(image.pixel(i,j));
        if (alpha < corr_coef) {
          alpha = 0;
        }
        else {
          alpha = 255;
        }
        white[i][j] = alpha;
      }
    }
    break;
  }
  }

  for (int i = 0, w = image.width(); i < w; ++i) {
    for (int j = 0, h = image.height(); j < h; ++j) {
      QColor color(image.pixel(i,j));
      if (!black.isEmpty() && !white.isEmpty()) {
        color.setAlpha(qMin(black[i][j], white[i][j]));
      }
      else if (!black.isEmpty()) {
        color.setAlpha(black[i][j]);
      }
      else if (!white.isEmpty()) {
        color.setAlpha(white[i][j]);
      }
      image.setPixel(i,j, color.rgba());
    }
  }

  return image;
}

void DataModel::setTransparencySettings( TransparencyMode mode, int value )
{
  this->currentTransparencyMode_ = mode;
  this->currentTransparencyValue_ = value;
  this->transparentSateliteImages_.clear();
  this->transparentMapImages_.clear();
}

QImage DataModel::getTransparentImage( DocumentHeader::DocumentClass docClass, const QString& imageId )
{
  DocumentUID uid(docClass, imageId);
  auto image = this->getImage( docClass, imageId );
  if ( true == image.isNull() ){
    return QImage();
  }

  switch ( docClass ){
  case DocumentHeader::kFax:{
    if ( false == this->transparentFaxImages_.contains(uid) ) {
      QImage resultImage = this->makeTransparentFax(image, Qt::white);
      this->transparentFaxImages_[uid] = resultImage;
      return resultImage;
    }
    else {
      return this->transparentFaxImages_[uid];
    }
  }
  case DocumentHeader::kFormalDocument:{
    return image;
  }
  case DocumentHeader::kLocalDocument:{
    return image;
  }
  case DocumentHeader::kMapImage:{
    //return image;
    if ( false == this->transparentMapImages_.contains(uid) ){
      auto grayscaleImage = image.convertToFormat(QImage::Format_Grayscale8);
      auto transparentGrayScaleImage = this->makeTransparentSateliteImage(grayscaleImage, this->currentTransparencyMode_, this->currentTransparencyValue_);
      QImage resultImage = image.copy();
      resultImage.setAlphaChannel(transparentGrayScaleImage.alphaChannel());
      this->transparentMapImages_[uid] = resultImage;
      return resultImage;
    }
    else {
      return this->transparentMapImages_[uid];
    }
  }
  case DocumentHeader::kSateliteDocument:{
    if ( false == this->transparentSateliteImages_.contains(uid) ) {
      auto resultImage = this->makeTransparentSateliteImage( image, this->currentTransparencyMode_, this->currentTransparencyValue_ );
      this->transparentSateliteImages_[uid] = resultImage;
      return resultImage;
    }
    else {
      return this->transparentSateliteImages_[uid];
    }
  }
  case DocumentHeader::kAmbiguous:
    error_log << QObject::tr("Неопределенный тип докуемнта");
    break;
  }
  return QImage();
}

void DataModel::loadDocument( const QString& fileName )
{
  this->loadDocument( DocumentUID( documentClass_, fileName ) );
}

void DataModel::filterDocuments()
{
  QApplication::setOverrideCursor(Qt::WaitCursor);
  static const QString errorDocumentIncorrect = QObject::tr("Ошибка: неверный класс документа");
  for ( auto doc : filteredList_ ) {
    delete doc;
  }
  this->filteredList_.clear();
  auto docClass = this->getDocumentClass();

  switch (docClass) {
  case DocumentHeader::kFormalDocument: {
    for ( auto document: this->completeList_ ) {
      auto formalDocument = static_cast<DocumentFormal*>(document);
      if ( nullptr != formalDocument ) {
        bool ok = ( true == filterFormalDocumentName_.isNull() || true == filterFormalDocumentName_.isMatch(formalDocument->getName() ) );
        ok &= ( true == filterFormalDocumentHour_.isNull() || true == filterFormalDocumentHour_.isMatch(formalDocument->getHour()) ) ;
        ok &= ( true == filterFormalDocumentCenter_.isNull() || true == filterFormalDocumentCenter_.isMatch(formalDocument->getCenter()) );
        ok &= ( true == filterFormalDocumentJobName_.isNull() || true == filterFormalDocumentJobName_.isMatch(formalDocument->getJobName()) ) ;
        ok &= ( true == filterFormalDocumentModel_.isNull() || true == filterFormalDocumentModel_.isMatch(formalDocument->getModel()) );
        if ( true == ok ) {
          auto formalDocument_new = new DocumentFormal( formalDocument->getName(),
                                                        formalDocument->getDt(),
                                                        formalDocument->getFullHour(),
                                                        formalDocument->getCenter(),
                                                        formalDocument->getModel(),
                                                        formalDocument->getJobName(),
                                                        formalDocument->getFileId());
          this->filteredList_ << formalDocument_new;
        }
      }
      else {
        error_log << errorDocumentIncorrect;
      }
    }
    break;
  }
  case DocumentHeader::kMapImage: {
    for ( auto document: this->completeList_ ){
      auto mapDocument = static_cast<DocumentMapImage*>(document);
      if ( nullptr != mapDocument ){
        if ( ( true == filterMapImageName_.isNull() || true == filterMapImageName_.isMatch(mapDocument->getName()) ) &&
             ( true == filterMapImageHour_.isNull() || true == filterMapImageHour_.isMatch(mapDocument->getHour()) ) &&
             ( true == filterMapImageCenter_.isNull() || true == filterMapImageCenter_.isMatch(mapDocument->getCenter()) ) &&
             ( true == filterMapImageJobName_.isNull() || true == filterMapImageJobName_.isMatch(mapDocument->getJobName()) ) &&
             ( true == filterMapImageModel_.isNull() || true == filterMapImageModel_.isMatch(mapDocument->getModel()) ) ) {
          auto mapDocument_new = new DocumentMapImage( mapDocument->getName(),
                                                       mapDocument->getDt(),
                                                       mapDocument->getFullHour() ,
                                                       mapDocument->getCenter(),
                                                       mapDocument->getModel(),
                                                       mapDocument->getJobName(),
                                                       mapDocument->getFileId() );
          this->filteredList_ << mapDocument_new;
        }
      }
      else {
        error_log << errorDocumentIncorrect;
      }
    }
    break;
  }
  case DocumentHeader::kFax:{
    for ( auto document: this->completeList_ ) {
      auto faxDocument = static_cast<DocumentFax*>(document);
      if ( nullptr != faxDocument ){
        if ( ( true == filterFaxTT_.isNull() || true == filterFaxTT_.isMatch(faxDocument->getT1T2()) ) &&
             ( true == filterFaxAA_.isNull() || true == filterFaxAA_.isMatch(faxDocument->getA1A2()) ) &&
             ( true == filterFaxCCCC_.isNull() || true == filterFaxCCCC_.isMatch(faxDocument->getCCCC()) ) &&
             ( true == filterFaxII_.isNull() || true == filterFaxII_.isMatch(faxDocument->getII()) )){
          auto faxDocument_new = new DocumentFax( faxDocument->getId(),
                                   faxDocument->getDt(),
                                   faxDocument->getFileId(),
                                   faxDocument->getT1T2(),
                                   faxDocument->getA1A2(),
                                   faxDocument->getCCCC(),
                                   faxDocument->getII()
                                   );
          this->filteredList_ << faxDocument_new;
        }
      }
      else {
        error_log << errorDocumentIncorrect;
      }
    }
    break;
  }
  case DocumentHeader::kSateliteDocument:{
    for ( auto document: this->completeList_ ){
      auto sateliteDocument = static_cast<DocumentSatelite*>(document);
      if ( nullptr != sateliteDocument ){
        if ( ( true == filterSateliteTT_.isNull() || true == filterSateliteTT_.isMatch(sateliteDocument->getT1T2()) ) &&
             ( true == filterSateliteAA_.isNull() || true == filterSateliteAA_.isMatch(sateliteDocument->getA1A2()) ) &&
             ( true == filterSateliteCCCC_.isNull() || true == filterSateliteCCCC_.isMatch(sateliteDocument->getCCCC()) ) &&
             ( true == filterSateliteII_.isNull() || true == filterSateliteII_.isMatch(sateliteDocument->getII()) ) ){
          auto sateliteDocument_new = new DocumentSatelite( sateliteDocument->getId(),
                                                            sateliteDocument->getFileId(),
                                                            sateliteDocument->getDt(),
                                                            sateliteDocument->getT1T2(),
                                                            sateliteDocument->getA1A2(),
                                                            sateliteDocument->getCCCC(),
                                                            sateliteDocument->getII() );
          this->filteredList_ << sateliteDocument_new;
        }
      }
      else {
        error_log << errorDocumentIncorrect;
      }
    }
    break;
  }
  case DocumentHeader::kLocalDocument:{
    //NOTHING
    break;
  }
  case DocumentHeader::kAmbiguous:
    error_log << QObject::tr("Неопределенный тип докуемнта");
    break;
  }
  QApplication::restoreOverrideCursor();
}


  const Condition DataModel::getFilterMapImageName() const
  {
    return this->filterMapImageName_;
  }

  const Condition DataModel::getFilterMapImageHour() const
  {
    return this->filterMapImageHour_;
  }

  const Condition DataModel::getFilterMapImageCenter() const
  {
    return this->filterMapImageCenter_;
  }

  const Condition DataModel::getFilterMapImageModel() const
  {
    return this->filterMapImageModel_;
  }

  const Condition DataModel::getFilterMapImageJobName() const
  {
    return this->filterMapImageJobName_;
  }

  void DataModel::filterMapImageName( Condition name )
  {
    this->filterMapImageName_ = name;
    this->filterDocuments();
    emit signalListLoaded(&this->filteredList_);
  }

  void DataModel::filterMapImageHour( Condition hour )
  {
    this->filterMapImageHour_ = hour;
    this->filterDocuments();
    emit signalListLoaded(&this->filteredList_);
  }

  void DataModel::filterMapImageCenter( Condition center )
  {
      this->filterMapImageCenter_ = center;
      this->filterDocuments();    
    emit signalListLoaded(&this->filteredList_);
  }

  void DataModel::filterMapImageModel( Condition model )
  {
      this->filterMapImageModel_ = model;
      this->filterDocuments();
    emit signalListLoaded(&this->filteredList_);
  }

  void DataModel::filterMapImageJobName( Condition jobname )
  {
      this->filterMapImageJobName_ = jobname;
      this->filterDocuments();

    emit signalListLoaded(&this->filteredList_);
  }

  void DataModel::filterSateliteTT( Condition tt)
  {
      this->filterSateliteTT_ = tt;
      this->filterDocuments();

    emit signalListLoaded(&this->filteredList_);
  }

  void DataModel::filterSateliteAA( Condition aa)
  {
      this->filterSateliteAA_ = aa;
      this->filterDocuments();

    emit signalListLoaded(&this->filteredList_);
  }

  void DataModel::filterSateliteCCCC( Condition cccc)
  {
      this->filterSateliteCCCC_ = cccc;
      this->filterDocuments();

    emit signalListLoaded(&this->filteredList_);
  }

  void DataModel::filterSateliteII( Condition ii)
  {
      this->filterSateliteII_ = ii;
      this->filterDocuments();

    emit signalListLoaded(&this->filteredList_);
  }

  const Condition DataModel::getFilterSateliteTT() const
  {
    return this->filterSateliteTT_;
  }

  const Condition DataModel::getFilterSateliteAA() const
  {
    return this->filterSateliteAA_;
  }

  const Condition DataModel::getFilterSateliteCCCC() const
  {
    return this->filterSateliteCCCC_;
  }

  const Condition DataModel::getFilterSateliteII( ) const
  {
    return this->filterSateliteII_;
  }

void DataModel::slotListLoaded( const QList<DocumentHeader*>* list)
{
  this->completeList_.clear();
  this->completeList_.append(*list);  
  this->filterDocuments();    
  emit signalListLoaded( &this->filteredList_ );

}

const Condition DataModel::getFilterFaxTT( ) const
{
  return this->filterFaxTT_;
}

const Condition DataModel::getFilterFaxAA( ) const
{  
  return this->filterFaxAA_;
}

const Condition DataModel::getFilterFaxCCCC( ) const
{
  return this->filterFaxCCCC_;
}

const Condition DataModel::getFilterFaxII( ) const
{  
  return this->filterFaxII_;
}

const Condition DataModel::getFilterFormalDocumentName() const
{
  return this->filterFormalDocumentName_;
}

const Condition DataModel::getFilterFormalDocumentHour() const
{
  return this->filterFormalDocumentHour_;
}

const Condition DataModel::getFilterFormalDocumentCenter() const
{
  return this->filterFormalDocumentCenter_;
}

const Condition DataModel::getFilterFormalDocumentModel() const
{
  return this->filterFormalDocumentModel_;
}

const Condition DataModel::getFilterFormalDocumentJobName() const
{
  return this->filterFormalDocumentJobName_;
}

void DataModel::filterFormalDocumentName( Condition name )
{
  this->filterFormalDocumentName_ = name;
  this->filterDocuments();
  emit signalListLoaded(&this->filteredList_);
}

void DataModel::filterFormalDocumentHour( Condition hour )
{
  this->filterFormalDocumentHour_ = hour;
  this->filterDocuments();
  emit signalListLoaded(&this->filteredList_);
}

void DataModel::filterFormalDocumentCenter( Condition center )
{
  this->filterFormalDocumentCenter_ = center;
  this->filterDocuments();
  emit signalListLoaded(&this->filteredList_);
}

void DataModel::filterFormalDocumentModel( Condition model )
{
  this->filterFormalDocumentModel_ = model;
  this->filterDocuments();
  emit signalListLoaded(&this->filteredList_);
}

void DataModel::filterFormalDocumentJobName( Condition jobname )
{
  this->filterFormalDocumentJobName_ = jobname;
  this->filterDocuments();
  emit signalListLoaded(&this->filteredList_);
}


}
}
