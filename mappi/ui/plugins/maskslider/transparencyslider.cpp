#include "transparencyslider.h"
#include "ui_transparencyslider.h"

#include <meteo/commons/ui/map/document.h>

#include <mappi/ui/satelliteimage/satelliteimage.h>
#include <mappi/ui/satlayer/satlayer.h>

namespace meteo {
namespace map{

const QString TransparencySlider::kAll = "all";

TransparencySlider::TransparencySlider(MapWindow *window) :
  MapWidget(window),
  ui(new Ui::TransparencySlider)
{
  ui->setupUi(this);
  ui->lockButton->setIcon(QIcon(":/mappi/icons/unlocked.png"));
  ui->lockButton->setEnabled(false);
  sldr_ = new DoubleSlider(Qt::Horizontal, this);
  sldr_->setRange(-1,256);
  sldr_->setValues(-1,256);
  sldr_->setStyleSheet("QSlider::handle {image: url(:/mappi/icons/handlerblack.png);}");
  sldr_->setSingle(false);
  ui->groupBox->layout()->addWidget(sldr_);
  ui->groupBox->layout()->addWidget(new QLabel("Белый"));
  sldr_->setTracking(false);
  QObject::connect(sldr_, SIGNAL(positionsChanged(int,int)), this, SLOT(slotRepaint()));
  QObject::connect(sldr_, SIGNAL(sliderReleased()),SLOT(slotRepaint()));
  QObject::connect(mapdocument()->eventHandler(),
                   &EventHandler::layerChanged,
                   this,
                   &TransparencySlider::slotLayerChanged);

  connect(ui->applianceBox, SIGNAL(activated(int)), this, SLOT(slotListChanged()));
  connect(ui->lockButton, SIGNAL(clicked(bool)), this, SLOT(slotLockLayer()));

  setWindowTitle("Маскирование");
  ui->applianceBox->addItem("Все изображения", kAll);
}

TransparencySlider::~TransparencySlider()
{
  delete ui;
  ui = nullptr;
  delete sldr_;
  sldr_ = nullptr;
}

int TransparencySlider::blackValue() const
{
  return sldr_->minimumValue();
}

int TransparencySlider::whiteValue() const
{
  return sldr_->maximumValue();
}

void TransparencySlider::slotRepaint()
{
  Document* doc = mapdocument();
  if ( kAll == ui->applianceBox->currentData().toString()) {
    foreach ( Layer* layer, doc->layers() ) {
      SatLayer* l = maplayer_cast<SatLayer*>(layer);
      if ( nullptr == l ) { continue; }
      SatelliteImage* obj = l->currentObject();
      if ( nullptr == obj ) { continue; }
      allBlack_ = sldr_->minimumPosition();
      allWhite_ = sldr_->maximumPosition();
      obj->setColorToHide(allBlack_, allWhite_);
      l->repaint();
    }
  }
  else {
    Layer* l = doc->layerByUuid(ui->applianceBox->currentData().toString());
    SatLayer* s = maplayer_cast<SatLayer*>(l);
    if ( nullptr == l ) { return; }
    SatelliteImage* obj = s->currentObject();
    if ( nullptr == obj ) { return ; }
    obj->setColorToHide(sldr_->minimumPosition(), sldr_->maximumPosition());
    l->repaint();
  }
}

void TransparencySlider::slotLayerChanged(Layer* layer, int ch)
{
  SatLayer* sl = maplayer_cast<SatLayer*>(layer);
  if ( nullptr == sl )  { return; }
  SatelliteImage* obj = sl->currentObject();
  if ( nullptr == obj ) { return; }

  if ( (int)LayerEvent::ObjectChanged == ch ) {
    //добавление
    bool found = false;
    for ( int i = 0, sz = ui->applianceBox->count(); i < sz; ++i ) {
      if ( ui->applianceBox->itemData(i) == layer->uuid() ) {
        found = true;
        if ( true == obj->isLocked() ) {
          ui->applianceBox->setItemIcon(i, QIcon(":/mappi/icons/locked.png"));
          if ( ui->applianceBox->currentData() == layer->uuid()) {
            ui->lockButton->setIcon(QIcon(":/mappi/icons/locked.png"));
            ui->groupBox->setEnabled(false);
          }
        }
        else {
          ui->applianceBox->setItemIcon(i, QIcon());
          if ( ui->applianceBox->currentData() == layer->uuid()) {
            ui->lockButton->setIcon(QIcon(":/mappi/icons/unlocked.png"));
            ui->groupBox->setEnabled(true);
          }
        }
        break;
      }
    }
    if ( false == found ) {
      ui->applianceBox->addItem(layer->name(), layer->uuid());
      obj->setColorToHide(sldr_->minimumPosition(), sldr_->maximumPosition());
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

void TransparencySlider::slotListChanged()
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
      ui->groupBox->setEnabled(false);
    }
    else {
      ui->lockButton->setIcon(QIcon(":/mappi/icons/unlocked.png"));
      ui->groupBox->setEnabled(true);
    }
    sldr_->setMinimumPosition(obj->blackMask());
    sldr_->setMaximumPosition(obj->whiteMask());
  }
  else {
    ui->lockButton->setIcon(QIcon(":/mappi/icons/unlocked.png"));
    ui->applianceBox->setItemIcon(ui->applianceBox->currentIndex(), QIcon());
    ui->lockButton->setEnabled(false);
    ui->groupBox->setEnabled(true);
    sldr_->setMinimumPosition(allBlack_);
    sldr_->setMaximumPosition(allWhite_);
  }
}

void TransparencySlider::slotLockLayer()
{
  Layer* l = mapdocument()->layerByUuid(ui->applianceBox->currentData().toString());
  SatLayer* sl = maplayer_cast<SatLayer*>(l);
  SatelliteImage* obj = sl->currentObject();
  obj->setLocked(!obj->isLocked());
  if ( true == obj->isLocked()) {
    ui->lockButton->setIcon(QIcon(":/mappi/icons/locked.png"));
    ui->applianceBox->setItemIcon(ui->applianceBox->currentIndex(), QIcon(":/mappi/icons/locked.png"));
  }
  else {
    ui->lockButton->setIcon(QIcon(":/mappi/icons/unlocked.png"));
    ui->applianceBox->setItemIcon(ui->applianceBox->currentIndex(), QIcon());
  }
}

} //map
} //meteo
