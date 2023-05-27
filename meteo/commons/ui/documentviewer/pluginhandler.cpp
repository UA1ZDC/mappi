#include <qaction.h>

#include "pluginhandler.h"
#include <qmdisubwindow.h>
#include <meteo/commons/ui/mainwindow/mainwindow.h>
#include <meteo/commons/global/log.h>

namespace meteo {
namespace documentviewer {

PluginHandler::PluginHandler(meteo::app::MainWindow *mw):
  mw_(mw),
  faxWidget_(nullptr),
  localDocWidget_(nullptr),
  sateliteImageWidget_(nullptr),
  mapImageWidget_(nullptr)
{
  static const QString& faxImages = QObject::tr("faximages");
  QAction* faxImagesAction = mw_->findAction(faxImages);
  if ( nullptr == faxImagesAction ){
    warning_log << meteo::msglog::kTargetMenyNotFound.arg(faxImages);
  }
  else {
    faxImagesAction->setDisabled(false);
    faxImagesAction->setIcon(QIcon(":/meteo/icons/fax-toolbar.png"));
    QObject::connect(faxImagesAction, &QAction::triggered, this, &PluginHandler::slotOpenFaxWindow);
  }

  static const QString& sateliteImages = QObject::tr("sateliteimages");
  QAction* sateliteImagesAction = mw_->findAction(sateliteImages);
  if ( nullptr == sateliteImagesAction ){
    warning_log << meteo::msglog::kTargetMenyNotFound.arg(sateliteImages);
  }
  else {
    sateliteImagesAction->setDisabled(false);
    sateliteImagesAction->setIcon(QIcon(":/meteo/icons/satellite.png"));
    QObject::connect(sateliteImagesAction, &QAction::triggered, this, &PluginHandler::slotOpenSateliteImagesWindow);
  }

  static const QString& mapImages = QObject::tr("mapimages");
  QAction* mapImagesAction = mw_->findAction(mapImages);
  if ( nullptr == mapImagesAction ){
    warning_log << meteo::msglog::kTargetMenyNotFound.arg(mapImages);
  }
  else {
    mapImagesAction->setDisabled(false);
    mapImagesAction->setIcon(QIcon(":/meteo/icons/map.png"));
    QObject::connect(mapImagesAction, &QAction::triggered, this, &PluginHandler::slotOpenMapImages);
  }


  static const QString& localImages = QObject::tr("localimages");
  QAction* localimagesAction = mw_->findAction(localImages);
  if ( nullptr == localimagesAction ){
    warning_log << meteo::msglog::kTargetMenyNotFound.arg(localImages);
  }
  else {
    localimagesAction->setDisabled(false);
    localimagesAction->setIcon(QIcon(":/meteo/icons/open-map.png"));
    QObject::connect(localimagesAction, &QAction::triggered, this, &PluginHandler::slotOpenLocalDocsWindow);
  }

  static const QString& formalDocs = QObject::tr("formaldocuments");
  QAction* formalDocsAction = mw_->findAction(formalDocs);
  if ( nullptr == formalDocsAction ){
    warning_log << meteo::msglog::kTargetMenyNotFound.arg(formalDocs);
  }
  else {
    formalDocsAction->setDisabled(false);
    QObject::connect(formalDocsAction, &QAction::triggered, this, &PluginHandler::slotOpenFormalDocuments );
  }

}

PluginHandler::~PluginHandler()
{
  if (nullptr != faxWidget_) {
    delete faxWidget_; faxWidget_ = nullptr;
  }
}

void PluginHandler::slotWidgetDesctroyed()
{
  auto sender = QObject::sender();
  if ( sender == faxWidget_ ) {
    faxWidget_ = nullptr;
  }
  else if ( sender == localDocWidget_ ){
    localDocWidget_ = nullptr;
  }
  else if ( sender == sateliteImageWidget_ ){
    sateliteImageWidget_ = nullptr;
  }
  else if ( sender == mapImageWidget_ ) {
    mapImageWidget_ = nullptr;
  }
}

void PluginHandler::slotOpenFaxWindow()
{
  QAction* a = qobject_cast<QAction*>(sender());

  if ( nullptr == a ) { return; }

  if ( nullptr == faxWidget_ ) {
    faxWidget_ = new WgtDocView( WgtDocView::DocumentViewerStates::kStateDefaultFaxPage );
    mw_->toMdi(faxWidget_);
    connect( faxWidget_, &WgtDocView::destroyed, this, &PluginHandler::slotWidgetDesctroyed );
  }

  faxWidget_->show();
  faxWidget_->setFocus();
}

void PluginHandler::slotOpenLocalDocsWindow()
{
  QAction* a = qobject_cast<QAction*>(sender());

  if ( nullptr == a ) { return; }

  if ( nullptr == localDocWidget_ ) {
    localDocWidget_ = new WgtDocView( WgtDocView::DocumentViewerStates::kStateDefaultLocalPage );
    mw_->toMdi(localDocWidget_);
    connect( localDocWidget_, &WgtDocView::destroyed, this, &PluginHandler::slotWidgetDesctroyed );
  }

  localDocWidget_->show();
  localDocWidget_->setFocus();
}

void PluginHandler::slotOpenSateliteImagesWindow()
{
  QAction* a = qobject_cast<QAction*>(sender());

  if ( nullptr == a ) { return; }

  if ( nullptr == sateliteImageWidget_ ) {
    sateliteImageWidget_ = new WgtDocView( WgtDocView::DocumentViewerStates::kStateDefaultSatelitePage );
    mw_->toMdi(sateliteImageWidget_);
    connect( sateliteImageWidget_, &WgtDocView::destroyed, this, &PluginHandler::slotWidgetDesctroyed );
  }

  sateliteImageWidget_->show();
  sateliteImageWidget_->setFocus();
}

void PluginHandler::slotOpenMapImages()
{
  QAction* a = qobject_cast<QAction*>(sender());

  if ( nullptr == a ) { return; }

  if ( nullptr == mapImageWidget_ ) {
    mapImageWidget_ = new WgtDocView( WgtDocView::DocumentViewerStates::kStateDefaultMapImagePage );
    mw_->toMdi(mapImageWidget_);
    connect( mapImageWidget_, &WgtDocView::destroyed, this, &PluginHandler::slotWidgetDesctroyed );
  }

  mapImageWidget_->show();
  mapImageWidget_->setFocus();
}

void PluginHandler::slotOpenFormalDocuments()
{
  QAction* a = qobject_cast<QAction*>(sender());

  if ( nullptr == a ) { return; }

  if ( nullptr == mapImageWidget_ ) {
    mapImageWidget_ = new WgtDocView( WgtDocView::DocumentViewerStates::kStateDefaultFormalDocPage );
    mw_->toMdi(mapImageWidget_);
    connect( mapImageWidget_, &WgtDocView::destroyed, this, &PluginHandler::slotWidgetDesctroyed );
  }

  mapImageWidget_->show();
  mapImageWidget_->setFocus();
}

}
}
