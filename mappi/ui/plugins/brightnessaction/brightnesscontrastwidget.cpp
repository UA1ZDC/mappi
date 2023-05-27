#include "brightnesscontrastwidget.h"
#include "ui_brightnesscontrastwidget.h"

#include <meteo/commons/ui/map/document.h>

#include <mappi/ui/satelliteimage/satelliteimage.h>
#include <mappi/ui/satlayer/satlayer.h>

namespace meteo {
namespace map {

const QString BrightnessContrastWidget::kAll = "all";

BrightnessContrastWidget::BrightnessContrastWidget(MapWindow* window) :
  MapWidget(window),
  ui(new Ui::BrightnessContrastWidget),
  brightnessSlider_(new DoubleSlider(Qt::Horizontal, this)),
  contrastSlider_(new DoubleSlider(Qt::Horizontal, this))
{
  ui->setupUi(this);
  ui->lockButton->setIcon(QIcon(":/mappi/icons/unlocked.png"));
  ui->lockButton->setEnabled(false);
  ui->brightnessBox->layout()->addWidget(brightnessSlider_);
  ui->contrastBox->layout()->addWidget(contrastSlider_);
  brightnessSlider_->setSingle(true);
  brightnessSlider_->setMinimum(-100);
  brightnessSlider_->setMaximum(100);
  brightnessSlider_->setMaximumPosition(-1);
  brightnessSlider_->setMinimumPosition(-100);
  contrastSlider_->setSingle(true);
  contrastSlider_->setMinimum(-200);
  contrastSlider_->setMaximum(200);
  contrastSlider_->setMaximumPosition(100);
  contrastSlider_->setMinimumPosition(-200);

  brightnessSlider_->setStyleSheet("QSlider::handle {image: url(:/mappi/icons/handlerblack.png);}");
  contrastSlider_->setStyleSheet("QSlider::handle {image: url(:/mappi/icons/handlerblack.png);}");

  connect(brightnessSlider_, SIGNAL(maximumPositionChanged(int)), this, SLOT(slotRepaint()));
  connect(contrastSlider_, SIGNAL(maximumPositionChanged(int)), this, SLOT(slotRepaint()));

  connect(ui->applianceBox, SIGNAL(activated(int)), this, SLOT(slotListChanged()));
  connect(ui->lockButton, SIGNAL(clicked(bool)), this, SLOT(slotLockLayer()));

//  QObject::connect(ui->brightnessSlider, SIGNAL(valueChanged(int)), this, SLOT(slotRepaint()));
//  QObject::connect(ui->brightnessSlider, SIGNAL(sliderMoved(int)), this, SLOT(slotRepaint()));
//  QObject::connect(ui->brightnessSlider, SIGNAL(sliderReleased()),this, SLOT(slotRepaint()));
//  QObject::connect(ui->contrastSlider, SIGNAL(valueChanged(int)), this, SLOT(slotRepaint()));
//  QObject::connect(ui->contrastSlider, SIGNAL(sliderMoved(int)), this, SLOT(slotRepaint()));
//  QObject::connect(ui->contrastSlider, SIGNAL(sliderReleased()),SLOT(slotRepaint()));
  QObject::connect(mapdocument()->eventHandler(),
                   &EventHandler::layerChanged,
                   this,
                   &BrightnessContrastWidget::slotLayerChanged);
  setWindowTitle("Яркость и контраст");

  ui->applianceBox->addItem("Все изображения", kAll);
}

BrightnessContrastWidget::~BrightnessContrastWidget()
{
  delete ui;
}

void BrightnessContrastWidget::slotRepaint()
{
  Document* doc = mapdocument();
  if ( kAll == ui->applianceBox->currentData().toString()) {
    foreach ( Layer* layer, doc->layers() ) {
      SatLayer* l = maplayer_cast<SatLayer*>(layer);
      if ( nullptr == l ) { continue; }
      SatelliteImage* obj = l->currentObject();
      if ( nullptr == obj ) { continue; }
      allContrast_ = contrastSlider_->maximumPosition();
      allBrightness_ = brightnessSlider_->maximumPosition();
      obj->setBrightness(allBrightness_);
      obj->setContrast(allContrast_);
      l->repaint();
    }
  }
  else {
    Layer* l = doc->layerByUuid(ui->applianceBox->currentData().toString());
    SatLayer* s = maplayer_cast<SatLayer*>(l);
    if ( nullptr == l ) { return; }
    SatelliteImage* obj = s->currentObject();
    if ( nullptr == obj ) { return ; }
    obj->setBrightness(brightnessSlider_->maximumPosition());
    obj->setContrast(contrastSlider_->maximumPosition());
    l->repaint();
  }
}

void BrightnessContrastWidget::slotLayerChanged(Layer *layer, int ch)
{
  SatLayer* sl = maplayer_cast<SatLayer*>(layer);
  if ( nullptr == sl )  { return; }
  SatelliteImage* obj = sl->currentObject();
  if ( nullptr == obj ) { return; }

  if ( (int)LayerEvent::ObjectChanged == ch) {
    //добавление
    bool found = false;
    for ( int i = 0, sz = ui->applianceBox->count(); i < sz; ++i ) {
      if ( ui->applianceBox->itemData(i) == layer->uuid() ) {
        found = true;
        if ( true == obj->isLocked() ) {
          ui->applianceBox->setItemIcon(i, QIcon(":/mappi/icons/locked.png"));
          if ( ui->applianceBox->currentData() == layer->uuid() ) {
            ui->lockButton->setIcon(QIcon(":/mappi/icons/locked.png"));
            ui->brightnessBox->setEnabled(false);
            ui->contrastBox->setEnabled(false);
          }
        }
        else {
          ui->applianceBox->setItemIcon(i, QIcon());
          if ( ui->applianceBox->currentData() == layer->uuid() ) {
            ui->lockButton->setIcon(QIcon(":/mappi/icons/unlocked.png"));
            ui->brightnessBox->setEnabled(true);
            ui->contrastBox->setEnabled(true);
          }
        }
        break;
      }
    }
    if ( false == found ) {
      ui->applianceBox->addItem(layer->name(), layer->uuid());
    //применение текущих параметров к новому слою, если выбран пункт Все изображения
      obj->setBrightness(brightnessSlider_->maximumPosition());
      obj->setContrast(contrastSlider_->maximumPosition());
    }
  }
  else if ( (int)LayerEvent::Deleted == ch ) {
    for ( int i = 0, sz = ui->applianceBox->count(); i < sz; ++i ) {
      if ( layer->uuid() == ui->applianceBox->itemData(i) ) {
        ui->applianceBox->removeItem(i);
        break;
      }
    }
  }
}

void BrightnessContrastWidget::slotListChanged()
{
  QString data = ui->applianceBox->currentData().toString();
  if ( kAll != data ) {
    ui->lockButton->setEnabled(true);
    Layer* l = mapdocument()->layerByUuid(data);
    SatLayer* sl = maplayer_cast<SatLayer*>(l);
    SatelliteImage* obj = sl->currentObject();
    if ( true == obj->isLocked() ) {
      ui->lockButton->setIcon(QIcon(":/mappi/icons/locked.png"));
      ui->applianceBox->setItemIcon(ui->applianceBox->currentIndex(), QIcon(":/mappi/icons/locked.png"));
      ui->brightnessBox->setEnabled(false);
      ui->contrastBox->setEnabled(false);
    }
    else {
      ui->lockButton->setIcon(QIcon(":/mappi/icons/unlocked.png"));
      ui->brightnessBox->setEnabled(true);
      ui->contrastBox->setEnabled(true);
    }
    brightnessSlider_->setMaximumPosition(obj->brightness());
    contrastSlider_->setMaximumPosition(obj->contrast());
  }
  else {
    ui->lockButton->setIcon(QIcon(":/mappi/icons/unlocked.png"));
    ui->applianceBox->setItemIcon(ui->applianceBox->currentIndex(), QIcon());
    ui->lockButton->setEnabled(false);
    ui->brightnessBox->setEnabled(true);
    ui->contrastBox->setEnabled(true);
    brightnessSlider_->setMaximumPosition(allBrightness_);
    contrastSlider_->setMaximumPosition(allContrast_);
  }
}

void BrightnessContrastWidget::slotLockLayer()
{
  Layer* l = mapdocument()->layerByUuid(ui->applianceBox->currentData().toString());
  SatLayer* sl = maplayer_cast<SatLayer*>(l);
  SatelliteImage* obj = sl->currentObject();
  obj->setLocked(!obj->isLocked());
  if ( true == obj->isLocked()) {
    ui->lockButton->setIcon(QIcon(":/mappi/icons/locked.png"));
    ui->applianceBox->setItemIcon(ui->applianceBox->currentIndex(), QIcon(":/mappi/icons/locked.png"));
    ui->brightnessBox->setEnabled(false);
    ui->contrastBox->setEnabled(false);
  }
  else {
    ui->lockButton->setIcon(QIcon(":/mappi/icons/unlocked.png"));
    ui->applianceBox->setItemIcon(ui->applianceBox->currentIndex(), QIcon());
    ui->brightnessBox->setEnabled(true);
    ui->contrastBox->setEnabled(true);
  }
}

}
}

