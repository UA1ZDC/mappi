#include "histogrameditor.h"
#include "ui_histogrameditor.h"

#include <qcolor.h>
#include <qpicture.h>
#include <qslider.h>
#include <qstyle.h>
#include <qevent.h>

#include <cross-commons/debug/tlog.h>
#include <meteo/commons/ui/map/event.h>
#include <mappi/ui/satelliteimage/satelliteimage.h>
#include <mappi/ui/satlayer/satlayer.h>
#include <mappi/proto/satelliteimage.pb.h>

namespace meteo {

HistogramEditor::HistogramEditor(map::MapWindow *window) :
  MapWidget(window)
{
  img_ = nullptr;
  originalImg_ =nullptr;
  ui_ = new Ui::HistogramEditor;
  ui_->setupUi(this);
  slider_ = new DoubleSlider(Qt::Horizontal,this);
  slider_->setStyleSheet("QSlider::handle {image: url(:/mappi/icons/handlerblack.png);}");
  ui_->doubleSlider->layout()->addWidget(slider_);
  slider_->setMaximum(255);
  slider_->setMinimum(0);
  slider_->setMaximumValue(255);
  slider_->setToolTip("Установка уровня яркости");
  fillLayersBox();
  QObject::connect(ui_->autoButton, SIGNAL(clicked()), SLOT(slotEqualization()));
  QObject::connect(ui_->returnButton, SIGNAL(clicked()), SLOT(slotReturnImageOnLayer()));
  QObject::connect(ui_->layersBox, SIGNAL(activated(const QString&)), SLOT(slotLoadImageFromlayer(const QString&)));
  QObject::connect(slider_, SIGNAL(positionsChanged(int,int)), SLOT(slotPositionChanged(int,int)));
  QObject::connect(slider_, SIGNAL(sliderReleased()), SLOT(slotHandleTransform()));
  QObject::connect(ui_->cancelButton, SIGNAL(clicked()), SLOT(slotCancelTransform()));
  map::Document* doc = mapdocument();
  if ( !doc->isStub() ) {
    doc->turnEvents();
    if ( 0 != doc->eventHandler()) {
      doc->eventHandler()->installEventFilter(this);
    }
  }
}

HistogramEditor::~HistogramEditor()
{
  delete ui_;
  ui_ = nullptr;
  delete slider_;
  slider_ = nullptr;
  delete img_;
  img_ = nullptr;
}

void HistogramEditor::fillLayersBox()
{
  ui_->layersBox->clear();
  ui_->layersBox->addItem("Все изображения","Ко всем");
  QList<map::Layer*> allLayers = mapdocument()->layers();
  foreach (map::Layer* l, allLayers) {
    if (meteo::kLayerSatellite == l->type()) {
      ui_->layersBox->addItem(l->name(),l->uuid());
    }
  }
  int index = ui_->layersBox->findData(uuidLayer_);
  if (-1 != index) {
    ui_->layersBox->setCurrentIndex(index);
  }
  ifAllImg();
}

void HistogramEditor::loadImageFromLayer()
{
  qApp->setOverrideCursor(Qt::WaitCursor);
  QString uuid = ui_->layersBox->currentData().toString();
  if ("Ко всем" == uuid) {
    uuidLayer_ = uuid;
  }
  else {
    ui_->fileNameLabel->setText( ui_->layersBox->currentText() );
    map::SatLayer* satLayer = maplayer_cast<map::SatLayer*>(mapdocument()->layerByUuid(uuid));
    if  ((mappi::conf::kFalseColor == satLayer->thematicType())
      || (mappi::conf::kGrayScale == satLayer->thematicType())) {
      uuidLayer_ = satLayer->uuid();
      map::SatelliteImage* objectImage = satLayer->currentObject();
      if (nullptr != objectImage) {
        originalImg_ = objectImage->img();
        img_ = new QImage(*originalImg_);
      }
      else {
        error_msg.operator <<("Нет изображения");
      }
    }
    else {
      error_msg.operator <<("Недопустимый тип тематической обработки");
    }
  }
  qApp->restoreOverrideCursor();
}

void HistogramEditor::createHistogram(QImage* image)
{
  qApp->setOverrideCursor(Qt::WaitCursor);
  int k = 256;
  histogram_.clear();
  histogram_.resize(k);

  for (int i = 0; i<histogram_.size(); i++) {
    histogram_[i] = 0;
  }
  if (nullptr != image) {
    for (int i =0; i < image->height(); i++) {
      for (int j =0; j < image->width(); j++) {
        QColor pixel = image->pixel(j,i);
        histogram_[pixel.lightness()] += 1;
      }
    }
    for (int i = 0; i<histogram_.size(); i++) {
      histogram_[i] = histogram_[i]/(image->height()*image->width());
    }
  }
  qApp->restoreOverrideCursor();
}


void HistogramEditor::showHistogram()
{
  qApp->setOverrideCursor(Qt::WaitCursor);
  QPixmap pixmap(256,100);
  QImage image(256,100,QImage::Format_ARGB32);
  QColor color(0,0,0);
  for (int i = 0; i < image.width();i++) {
    for (int j = 0; j< image.height();j++){
      image.setPixel(i,j,color.rgba());
    }
  }
  color.setGreen(0);
  color.setBlue(0);
  color.setRed(255);
  for (int i = 0; i < histogram_.size();i++) {
    for (int j = 0; j< 2000*histogram_[i] && j < 100;j++){
      image.setPixel(i,99-j,color.rgba());
    }
  }
  pixmap = QPixmap::fromImage(image);
  ui_->histogram->clear();
  ui_->histogram->setPixmap(pixmap);
  qApp->restoreOverrideCursor();
}

void HistogramEditor::transformHistogram(int min, int max)
{
  if ((min < 0) || (max > 255)) {
    error_msg.operator <<("Ошибка диапазона. Проверьте парметры");
    return;
  }
  if (min > max) {
    error_msg.operator <<("Ошибка диапазона. Проверьте парметры");
    return;
  }
  for (int i = 1; i < histogram_.size(); i++) {
    histogram_[i] = histogram_[i-1] + histogram_[i];
  }
  for (int i = 0; i < histogram_.size(); i++) {
    histogram_[i] = (max-min) * histogram_[i] + min;
  }
  if(nullptr != img_) {
    qApp->setOverrideCursor(Qt::WaitCursor);
    for (int i =0; i < img_->height(); i++) {
      for (int j =0; j < img_->width(); j++) {
        QColor pixel = img_->pixel(j,i);
        pixel.setHsl(pixel.hslHue(),pixel.hslSaturation(), histogram_[pixel.lightness()]);
        img_->setPixel(j,i,pixel.rgba());
      }
    }
  qApp->restoreOverrideCursor();
  }
  else {
    error_msg.operator <<("Нет изображения.");
  }
}

void HistogramEditor::showPreview()
{
  qApp->setOverrideCursor(Qt::WaitCursor);
  QPixmap preview(500,1000);
  if (nullptr != img_) {
   preview = QPixmap::fromImage(*img_);
   ui_->preview->setPixmap(preview.scaled(preview.width()/13,preview.height()/13));
  }
  qApp->restoreOverrideCursor();
}

void HistogramEditor::slotEqualization()
{
  if ("Ко всем" == uuidLayer_) {
    QList<map::Layer*> allLayers = mapdocument()->layers();
    foreach (map::Layer* l, allLayers) {
      if (meteo::kLayerSatellite == l->type()) {
        map::SatLayer* satLayer = maplayer_cast<map::SatLayer*>(l);
        uuidLayer_ = satLayer->uuid();
        map::SatelliteImage* objectImage = satLayer->currentObject();
        if (nullptr != objectImage) {
          originalImg_ = objectImage->img();
          img_ = new QImage(*originalImg_);
        }
        createHistogram(img_);
        transformHistogram(0,255);
        slotReturnImageOnLayer();
      }
    }
    uuidLayer_ = "Ко всем";
    slider_->setMaximumPosition(255);
    slider_->setMinimumPosition(0);
    ui_->minLabel->setText(QString::number(0));
    ui_->maxLabel->setText(QString::number(255));
  }
  else {
    transformHistogram(0,255);
    createHistogram(img_);
    showHistogram();
    showPreview();
    slider_->setMaximumPosition(255);
    slider_->setMinimumPosition(0);
    ui_->minLabel->setText(QString::number(0));
    ui_->maxLabel->setText(QString::number(255));
  }
}

void HistogramEditor::slotHandleTransform()
{
  if (true == positionChanged_) {
    if ("Ко всем" == uuidLayer_) {
      QList<map::Layer*> allLayers = mapdocument()->layers();
      foreach (map::Layer* l, allLayers) {
        if (meteo::kLayerSatellite == l->type()) {
          map::SatLayer* satLayer = maplayer_cast<map::SatLayer*>(l);
          uuidLayer_ = satLayer->uuid();
          map::SatelliteImage* objectImage = satLayer->currentObject();
          if (nullptr == objectImage) {
            continue;
          }
          originalImg_ = objectImage->img();
          img_ = new QImage(*originalImg_);
          int min = slider_->minimumValue();
          int max = slider_->maximumValue();
          createHistogram(img_);
          transformHistogram(min,max);
          slotReturnImageOnLayer();
        }
      }
      uuidLayer_ = "Ко всем";
    }
    else {
      int min = slider_->minimumValue();
      int max = slider_->maximumValue();
      transformHistogram(min,max);
      createHistogram(img_);
      showHistogram();
      showPreview();
    }
  }
  positionChanged_ = false;
}

void HistogramEditor::slotLoadImageFromlayer(const QString& string)
{
  Q_UNUSED(string);
  loadImageFromLayer();
  ifAllImg();
  if (img_ != nullptr) {
    createHistogram(img_);
    showHistogram();
    showPreview();
    slider_->setMaximumPosition(255);
    slider_->setMinimumPosition(0);
    ui_->minLabel->setText(QString::number(0));
    ui_->maxLabel->setText(QString::number(255));
  }
}

void HistogramEditor::slotReturnImageOnLayer()
{
  if (nullptr != originalImg_) {
    *originalImg_ = *img_;
    map::SatLayer* satlayer = maplayer_cast<map::SatLayer*>(mapdocument()->layerByUuid(uuidLayer_));
    
    // for ( int i = 0, sz = satlayer->objects().size(); i < sz; ++i ) {
    //   map::SatelliteImage* obj = mapobject_cast<map::SatelliteImage*>(satlayer->objects().at(i));
    //   obj->resetCache();
    // }
    foreach(meteo::map::Object* o, satlayer->objects()) {
      map::SatelliteImage* obj = mapobject_cast<map::SatelliteImage*>(o);
      obj->resetCache();
    }
    
    satlayer->repaint();
  }
  else {
    warning_msg.operator <<("Слой был удалён.");
  }
}

void HistogramEditor::slotCancelTransform()
{
  if (nullptr != originalImg_) {
    img_ = new QImage(*originalImg_);
    createHistogram(img_);
    showHistogram();
    showPreview();
  }
  else {
    warning_msg.operator <<("Ошибка. Возможно снимок был удалён.");
  }
}

void HistogramEditor::slotPositionChanged(int min, int max)
{
  ui_->minLabel->setText(QString::number(min));
  ui_->maxLabel->setText(QString::number(max));
  positionChanged_ = true;
}

bool HistogramEditor::eventFilter(QObject *obj, QEvent *event)
{
  Q_UNUSED( obj );
  if (map::LayerEvent::LayerChanged != event->type() ) {return false; }
  map::Document* doc = mapdocument();
  if ( 0 == doc ) {
    return false;
  }
  map::LayerEvent* ev = static_cast<map::LayerEvent*>(event);
  ui_->layersBox->blockSignals(true);
  fillLayersBox();
  ui_->layersBox->blockSignals(false);
  if ( map::LayerEvent::Deleted == ev->changeType() ) {
    if (uuidLayer_ == ev->layer()) {
      originalImg_ = new QImage;
      originalImg_ = nullptr;
    }
  }
  return false;
}

void HistogramEditor::ifAllImg()
{
  if ("Ко всем" == uuidLayer_) {
    ui_->cancelButton->setEnabled(false);
    ui_->returnButton->setEnabled(false);
    ui_->prevGroup->hide();
    ui_->histogram->hide();
    ui_->fileNameLabel->setText("Все изображения");
  }
  else {
    ui_->cancelButton->setEnabled(true);
    ui_->returnButton->setEnabled(true);
    ui_->prevGroup->show();
    ui_->histogram->show();
  }
}

} //meteo
