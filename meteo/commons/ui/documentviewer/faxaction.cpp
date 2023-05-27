#include "faxaction.h"
#include "markerflagitem.h"

#include <QtGui>
#include <qtoolbutton.h>
#include <qevent.h>
#include <qgraphicssceneevent.h>
#include <qgraphicsproxywidget.h>
#include <qgraphicslinearlayout.h>
#include <qobject.h>
#include <qtoolbutton.h>
#include <qdebug.h>
#include <qpalette.h>
#include <qrgb.h>
#include <qtreewidget.h>
#include <qheaderview.h>
#include <qmessagebox.h>
#include <qvector.h>

#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/paths.h>
#include <commons/geobasis/projection.h>
#include <meteo/commons/ui/map/document.h>
#include <meteo/commons/ui/map/event.h>
#include <meteo/commons/global/global.h>

#include <meteo/commons/ui/map/view/widgetitem.h>
#include <meteo/commons/ui/map/document.h>
#include <meteo/commons/ui/map/geopixmap.h>
#include <meteo/commons/ui/map/view/mapscene.h>
#include <meteo/commons/ui/map/view/mapview.h>
#include <meteo/commons/ui/map/view/mapwindow.h>
#include <meteo/commons/ui/map/view/widgets/layerswidget.h>

#include <meteo/commons/ui/mainwindow/widgethandler.h>
#include <meteo/commons/ui/mainwindow/mainwindow.h>

#include "datamodel/wgtdocviewModel.h"
#include "wgtdocview/wgtdocview.h"
#include <meteo/commons/ui/documentviewer/wgtdocview/wgtdocview.h>
#include <meteo/commons/ui/documentviewer/custom/imagedisplaygraphicsitem.h>
#include <meteo/commons/ui/documentviewer/custom/animationgraphicsitem.h>

namespace meteo {
namespace map {

static const int markerItemSize = 10;

const QString Faxaction::faxActionName = QObject::tr("documentvieweraction");

static const QPair<QString, QString> kMenuItem[] = {
  QPair<QString, QString>( QObject::tr("pinningfax"), QObject::tr("Факсимильные карты") ),
  QPair<QString, QString>( QObject::tr("pinningsateliete"), QObject::tr("Спутниковые изображения") ),
  QPair<QString, QString>( QObject::tr("pinningmapimage"), QObject::tr("Растровые карты") )
};
static const auto kMenuPath = QList<QPair<QString, QString>>(
{
      QPair<QString, QString>( QObject::tr("deposition"), QObject::tr("Наноска") ),
      QPair<QString, QString>( QObject::tr("pinning")   , QObject::tr("Изображения"))
});

Faxaction::Faxaction(MapScene* scene ) :
    Action(scene, faxActionName ),
    source_(nullptr),
    currentImageName_(QString()),
    animationGraphicsItem_(nullptr)
{
  for ( int i = 0; i < markersCount; ++i ){
    markersSource_[i] = nullptr;
    markersTarget_[i] = nullptr;
  }

  {
    QAction* a = view()->window()->addActionToMenu( kMenuItem[0], kMenuPath );
    QObject::connect( a, SIGNAL( triggered() ), this, SLOT( slotOpenFaxWidget() ) );
    a->setData(QObject::tr("fax"));
  }

  {
    QAction* a = view()->window()->addActionToMenu( kMenuItem[1], kMenuPath );
    QObject::connect( a, SIGNAL( triggered() ), this, SLOT( slotOpenSateliteWidget() ) );
    a->setData(QObject::tr("satelite"));
  }
  {
    QAction* a = view()->window()->addActionToMenu( kMenuItem[2], kMenuPath );
    QObject::connect( a, SIGNAL( triggered() ), this, SLOT( slotOpenMapImageWidget() ) );
    a->setData(QObject::tr("mapimage"));
  }
}

void Faxaction::pitToView( ::meteo::ViewImagePreview *source, const QString& currentImageName, const QString& currentImageDisplayName)
{  
  //images_.clear();
  //imageToSource_.clear();
  //imageToTarget_.clear();
  //imageLayers_.clear();
  //displayNames_.clear();

  source_ = source;

  currentImageName_ = currentImageName;
  displayNames_[currentImageName] = currentImageDisplayName;
  displayNames_[currentImageName] = currentImageDisplayName;
  for ( int i = 0; i < markersCount; ++i) {
    markersTarget_[i] = new MarkerFlagItem(markerItemSize, i + 1);
    scene_->addItem(markersTarget_[i]);
    auto visibleRect = view()->mapToScene(view()->viewport()->geometry()).boundingRect();
    QPoint pnt( qRound(visibleRect.x() + visibleRect.width() / 2),
                qRound(visibleRect.y() + visibleRect.height() / 2) );

    markersTarget_[i]->setPos( QPoint( pnt.x() + 30 * ( i - 1 ),
                                   pnt.y() + 30 * ( i - 1 ) * ( i - 1 ) ) );
    markersTarget_[i]->setFlag(QGraphicsItem::ItemIsMovable);
  }
  for ( int i = 0; i < markersCount; ++i ){
    markersSource_[i] = new MarkerFlagItem(markerItemSize, i + 1);
    markersSource_[i]->setZValue(5);
    source->scene()->addItem(markersSource_[i]);

    auto visibleRect = source->getVisibleRect();
    QPoint pnt ( qRound( visibleRect.x() + visibleRect.width() / 2 ),
                 qRound( visibleRect.y() + visibleRect.height() / 2 )
          );

    markersSource_[i]->setPos( QPoint( pnt.x() + 30 * ( i - 1 ),
                                   pnt.y() + 30 * ( i - 1 ) * ( i - 1 ) ) );
    markersSource_[i]->setFlag(QGraphicsItem::ItemIsMovable);
  }

  imageToTarget_ = getTargetRect();
  targetImageRectSave();
  sourceImageRectSave();
  QObject::connect( source, SIGNAL(destroyed()), this, SLOT(slotSourceDestroyed()) );
  QObject::connect( scene_, SIGNAL(destroyed()), this, SLOT(slotSceneDestroyed()) );
  scene_->document()->eventHandler()->installEventFilter(this);
}

Faxaction::~Faxaction()
{
  for ( int i = 0; i < Faxaction::markersCount; ++i) {
    if ( nullptr != markersSource_[i] ) {
      delete markersSource_[i];
    }
    if ( nullptr != markersTarget_[i] ) {
      delete markersTarget_[i];
    }
  }
}

void Faxaction::mouseMoveEvent( QMouseEvent* event )
{
  Action::mouseMoveEvent(event);

  bool needRestore = true;
  for ( int i = 0; i < markersCount; ++i ){    
    if ( ( nullptr != markersTarget_[i] ) &&
         ( true == markersTarget_[i]->isUnderMouse() ) ){
      needRestore = false;
    }
  }

  if ( true == needRestore ){
    targetImageRectRestore();
  }
  else {
    targetImageRectSave();
  }
}


void Faxaction::setCurrentImageName(const QString& name, const QString& displayName)
{
  currentImageName_ = name;
  if ( false == displayName.isEmpty() ){
    displayNames_[currentImageName_] = displayName;
  }
  setMarkersPinned( true == imageLayers_.contains(name) );
  targetImageRectRestore();
  sourceImageRectRestore();
}

bool Faxaction::eventFilter(QObject *watched, QEvent *event)
{
  if ( nullptr == source_){
    return false;
  }
  if ( watched == scene_->document()->eventHandler() ) {
    if ( meteo::map::LayerEvent::LayerChanged == event->type() ){
      for ( auto imageName: imageLayers_.keys() ){
        auto layeruuid = imageLayers_[imageName];
        if ( ( nullptr != scene_ ) &&
             ( nullptr != scene_->document() ) &&
             ( false == scene_->document()->hasLayer(layeruuid) ) ){
          imageLayers_.remove(imageName);
          images_.remove(imageName);
          if ( currentImageName_ == imageName ) {
            setMarkersPinned(false);
          }
          emit signalImageLayerRemoved(imageName);
        }
      }
    }
  }
  return false;
}

void Faxaction::setImage( const QImage& image )
{
  images_[currentImageName_] = image;
  emit signalImageLayerAdded(currentImageName_);  
  fixImagePosition();
}

void Faxaction::mouseReleaseEvent(QMouseEvent* e)
{
  Action::mouseReleaseEvent(e);
  targetImageRectRestore();
}

void Faxaction::wheelEvent(QWheelEvent *e)
{
  Action::wheelEvent(e);
  targetImageRectRestore();
}

meteo::GeoVector Faxaction::getTargetRect()
{
  meteo::GeoVector polyTarget;
  if ( nullptr != scene_ &&
       nullptr != scene_->document() ) {
    for ( int i = 0; i < markersCount; ++i ) {
      if ( nullptr != markersTarget_[i] ) {
        polyTarget << scene_->document()->screen2coord( markersTarget_[i]->pos() );
      }
    }
  }
  return polyTarget;
}

QPolygon Faxaction::getSourceRect()
{
  QPolygon polyOriginal;  
  if ( nullptr != source_ ) {
    for ( int i = 0; i < meteo::map::Faxaction::markersCount; ++i ){
      if ( nullptr != markersSource_[i] ) {
        polyOriginal << source_->mapToImage( markersSource_[i]->pos() ).toPoint();
      }
    }
  }
  return polyOriginal;
}


void Faxaction::sourceImageRectSave()
{
  if ( false == source_->hasImages() ){
    return;
  }
  auto sourceRect = getSourceRect();
  auto oldSourceRecet = imageToSource_;
  if ( sourceRect != oldSourceRecet ) {
    imageToSource_ = sourceRect;
    fixImagePosition(false);
  }
}

void Faxaction::sourceImageRectRestore()
{
  auto sourceRect = imageToSource_;
  for ( int i = 0; i < markersCount; ++i ){
    if ( (nullptr != markersSource_[i]) &&
         ( sourceRect.size() > i )) {
      markersSource_[i]->setPos(source_->mapFromImage(sourceRect[i]));
    }
  }
  if ( nullptr != animationGraphicsItem_ ){
    animationGraphicsItem_->setRect( getSourceRect(), getTargetRect() );
  }
}

void Faxaction::targetImageRectSave()
{
  auto targetRect = getTargetRect();
  if ( targetRect != imageToTarget_ ){
    imageToTarget_ = targetRect;
    fixImagePosition(false);
  }
}

void Faxaction::targetImageRectRestore()
{
  auto geoVector = imageToTarget_;
  if ( ( nullptr != scene_ ) &&
       ( nullptr != scene_->document() ) ) {
    for ( int i = 0; i < markersCount; ++i ){
      if ( (geoVector.size() > i) &&
           (nullptr != markersTarget_[i]) ){
        QPointF newPoint = scene_->document()->coord2screen(geoVector[i]);
        markersTarget_[i]->setPos(newPoint);
      }
    }
  }
  if ( nullptr != animationGraphicsItem_ ){
    animationGraphicsItem_->setRect( getSourceRect(), getTargetRect() );
  }
}

void Faxaction::fixImagePosition( bool recreateLayer)
{
  if ( false == images_.contains(currentImageName_) ){
    //warning_log << QObject::tr("Не возможно восстановить положение изображения: нет изображения");
    return;
  }
  if ( (false == hasLayer(currentImageName_)) && (true == recreateLayer) ){
    removeLayer(currentImageName_);
  }

  meteo::map::Layer* l = layerByImageName(currentImageName_);
  if ( nullptr == l ){
    return;
  }
  meteo::map::GeoPixmap* gpix = nullptr;
  if (1 == l->objects().size() ){
    gpix = dynamic_cast<meteo::map::GeoPixmap*>( *l->objects().begin() );
  }
  if ( 0 == l->objects().size() ){
    gpix = new meteo::map::GeoPixmap(l);
  }
  if ( nullptr == gpix ){
    error_log << QObject::tr("Ошибка - не удается создать слой карты");
    return;
  }
  QPair< QPolygon, meteo::GeoVector > bunch;

  QPolygon polyOriginal;
  bunch.first = getSourceRect();
  bunch.second = getTargetRect();

  QImage image = images_[currentImageName_];
  gpix->setImage(image);
  gpix->bindScreenToCoord( bunch.first, bunch.second );
  setMarkersPinned(true);
}

void Faxaction::addAnimationLayer( const AnimationController &controller )
{
  if ( nullptr == animationGraphicsItem_ ) {
    animationGraphicsItem_ = new AnimationGraphicsItem( scene_, controller);
    QObject::connect( animationGraphicsItem_, &QObject::destroyed, this, &Faxaction::slotDestroyed );
  }
  animationGraphicsItem_->setRect( getSourceRect(), getTargetRect() );
}

void Faxaction::removeAnimationLayer()
{
  if ( nullptr != animationGraphicsItem_ ){
    delete animationGraphicsItem_;
    animationGraphicsItem_ = nullptr;
  }
}

void Faxaction::removeLayer( const QString& layername )
{
  if ( false == hasLayer(layername)) {
    return;
  }
  auto layeruuid = imageLayers_[layername];
  if ( ( nullptr != scene_ ) &&
       ( nullptr != scene_->document() ) &&
       ( true == scene_->document()->hasLayer(layeruuid) ) ){
    delete scene_->document()->layer(layeruuid);
    imageLayers_.remove(layername);
    setMarkersPinned(false);
    images_.remove(layername);
    emit signalImageLayerRemoved(layername);
  }
}

bool Faxaction::hasLayer(const QString& layerName)
{
  return  (true == imageLayers_.contains(layerName)) &&
      (true == scene_->document()->hasLayer(imageLayers_[layerName]) );
}


void Faxaction::slotSourceDestroyed()
{
  //animationGraphicsItem_ = nullptr;
  destroyMarkers();
  source_ = nullptr;
  //images_.clear();
  //imageToSource_.clear();
  //imageToTarget_.clear();
  //imageLayers_.clear();
  //displayNames_.clear();
  currentImageName_ = QString();
}

void Faxaction::slotSceneDestroyed()
{  
  destroyMarkers();
  animationGraphicsItem_ = nullptr;
  images_.clear();
  imageToSource_.clear();
  imageToTarget_.clear();
  imageLayers_.clear();
  displayNames_.clear();
}

void Faxaction::destroyMarkers()
{
  for ( int i = 0; i < markersCount; ++i ){
    if ( nullptr != markersSource_[i] ) {
      delete markersSource_[i];
      markersSource_[i] = nullptr;
    }
    if ( nullptr != markersTarget_[i] ) {
      delete markersTarget_[i];
      markersTarget_[i] = nullptr;
    }
  }
}

meteo::map::Layer* Faxaction::layerByImageName(const QString& layerName)
{
  meteo::map::Layer *layer = nullptr;
  if ( nullptr == scene_ ||
       nullptr == scene_->document() ){
    return nullptr;
  }

  if ( true == imageLayers_.contains(layerName) ){
    auto layeruuid = imageLayers_[layerName];
    if ( true == scene_->document()->hasLayer(layeruuid) ){
      return scene_->document()->layer(layeruuid);
    }
  }

  if ( nullptr == layer ){
    layer = new meteo::map::Layer( scene_->document(), displayNames_[currentImageName_] );
    imageLayers_.insert(layerName, layer->uuid());
  }
  return layer;
}

QDialog* Faxaction::toDialog(QWidget* wgt, QWidget* parent)
{
  QDialog * dlg = new QDialog(parent);
  delete dlg->layout();  
  dlg->setLayout(new QHBoxLayout());
  dlg->setWindowTitle(wgt->windowTitle());
  dlg->layout()->addWidget(wgt);
  wgt->setParent(dlg);
  dlg->showNormal();
  QObject::connect(dlg, SIGNAL(finished(int)), dlg, SLOT(deleteLater()) );
  return dlg;
}

void Faxaction::slotOpenFaxWidget()
{
  if ( nullptr == source_ ) {
    auto docview = new ::meteo::documentviewer::WgtDocView( ::meteo::documentviewer::WgtDocView::kStateFaxWithPinning );
    toDialog(docview, view()->window());
    docview->setPinningAction(this);    
    QObject::connect( source_, SIGNAL(destroyed()), this, SLOT(slotSourceDestroyed()) );
  }
  reEmitAddedItems();
}

void Faxaction::unpinAll()
{
  for ( auto key: imageLayers_.keys() ){
    auto layeruuid = imageLayers_[key];
    if ( nullptr == scene_ ||
         nullptr == scene_->document() ){
      return;
    }
    if ( true == scene_->document()->hasLayer(layeruuid) ){
      delete scene_->document()->layer(layeruuid);
    }
  }
}

void Faxaction::setMarkersPinned(bool isPinned)
{
  auto color = ( true == isPinned ) ? Qt::blue : Qt::red;
  for ( int i = 0; i < markersCount; ++i ){
    if ( nullptr != markersSource_[i] ){
      markersSource_[i]->setColor(color);
    }
    if ( nullptr != markersTarget_[i] ){
      markersTarget_[i]->setColor(color);
    }
  }
}


void Faxaction::setTargetRect( const meteo::GeoPoint points[markersCount] )
{
  meteo::GeoVector polyTarget;
  for ( int i = 0; i < markersCount; ++i ){
    polyTarget << points[i];
  }
  imageToTarget_ = polyTarget;
  targetImageRectRestore();
}

void Faxaction::setSourceRect( const QPoint points[markersCount] )
{
  QPolygon polySource;
  for ( int i = 0; i < markersCount; ++i ){
    polySource << points[i];
  }
  imageToSource_ = polySource;
  sourceImageRectRestore();
}

int Faxaction::getLayersCount()
{
  return imageLayers_.count();
}

void Faxaction::setLayerHidden( const QString& layerName, bool hidden )
{
  auto layer = layerByImageName(layerName);
  if ( nullptr != layer ){
    layer->setVisisble( ! hidden );
  }
}

void Faxaction::slotOpenSateliteWidget()
{
  if ( nullptr == source_ ) {
    auto docview = new ::meteo::documentviewer::WgtDocView( ::meteo::documentviewer::WgtDocView::kStateSateliteWithPinning );
    toDialog(docview, view()->window());
    docview->setPinningAction(this);
    QObject::connect( source_, SIGNAL(destroyed()), this, SLOT(slotSourceDestroyed()) );
  }
  reEmitAddedItems();
}

void Faxaction::slotOpenMapImageWidget()
{
  if ( nullptr == source_ ) {
    auto docview = new ::meteo::documentviewer::WgtDocView( ::meteo::documentviewer::WgtDocView::kStateMapImageWithPinning );
    toDialog(docview, view()->window());
    docview->setPinningAction(this);
    QObject::connect( source_, SIGNAL(destroyed()), this, SLOT(slotSourceDestroyed()) );
  }
  reEmitAddedItems();
}

void Faxaction::reEmitAddedItems()
{
  for ( auto item : images_.keys() )  {
    emit signalImageLayerAdded(item);
  }
}

void Faxaction::slotDestroyed( QObject* obj )
{
  if ( obj == animationGraphicsItem_ ) {
    animationGraphicsItem_ = nullptr;
  }
}

}
}
