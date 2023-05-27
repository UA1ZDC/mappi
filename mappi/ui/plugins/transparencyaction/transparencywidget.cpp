#include "transparencywidget.h"
#include "ui_transparencywidget.h"

#include <meteo/commons/ui/map/document.h>

#include <mappi/ui/satelliteimage/satelliteimage.h>
#include <mappi/ui/satlayer/satlayer.h>

namespace meteo {
namespace map {

const QString TransparencyWidget::kAll = "all";

TransparencyWidget::TransparencyWidget(MapWindow* window) :
  MapWidget(window),
  ui(new Ui::TransparencyWidget),
  transparencySlider_(new DoubleSlider(Qt::Horizontal, this))
{
  ui->setupUi(this);
  ui->lockButton->setIcon(QIcon(":/mappi/icons/unlocked.png"));
  ui->lockButton->setEnabled(false);
  ui->transparencyBox->layout()->addWidget(transparencySlider_);
  transparencySlider_->setSingle(true);
  transparencySlider_->setMinimum(0);
  transparencySlider_->setMaximum(255);
  transparencySlider_->setMaximumPosition(255);
  transparencySlider_->setStyleSheet("QSlider::handle {image: url(:/mappi/icons/handlerblack.png);}");

  connect(transparencySlider_, SIGNAL(maximumPositionChanged(int)), this, SLOT(slotRepaint()));

  connect(ui->applianceBox, SIGNAL(activated(int)), this, SLOT(slotListChanged()));
  connect(ui->lockButton, SIGNAL(clicked(bool)), this, SLOT(slotLockLayer()));

  QObject::connect(mapdocument()->eventHandler(),
                   &EventHandler::layerChanged,
                   this,
                   &TransparencyWidget::slotLayerChanged);
  setWindowTitle("Прозрачность");

  ui->applianceBox->addItem("Все изображения", kAll);
}

TransparencyWidget::~TransparencyWidget()
{
  delete ui;
}

void TransparencyWidget::slotRepaint()
{
  Document* doc = mapdocument();
  if ( kAll == ui->applianceBox->currentData().toString()) {
    foreach ( Layer* layer, doc->layers() ) {
      SatLayer* l = maplayer_cast<SatLayer*>(layer);
      if ( nullptr == l ) { continue; }
      SatelliteImage* obj = l->currentObject();
      if ( nullptr == obj ) { continue; }
      allTransparency_ = transparencySlider_->maximumPosition();
      obj->setTransparency(allTransparency_);
      l->repaint();
    }
  }
  else {
    Layer* l = doc->layerByUuid(ui->applianceBox->currentData().toString());
    SatLayer* s = maplayer_cast<SatLayer*>(l);
    if ( nullptr == l ) { return; }
    SatelliteImage* obj = s->currentObject();
    if ( nullptr == obj ) { return ; }
    obj->setTransparency(transparencySlider_->maximumPosition());
    l->repaint();
  }
}

void TransparencyWidget::slotLayerChanged(Layer *layer, int ch)
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
            ui->transparencyBox->setEnabled(false);
          }
        }
        else {
          ui->applianceBox->setItemIcon(i, QIcon());
          if ( ui->applianceBox->currentData() == layer->uuid() ) {
            ui->lockButton->setIcon(QIcon(":/mappi/icons/unlocked.png"));
            ui->transparencyBox->setEnabled(true);
          }
        }
        break;
      }
    }
    if ( false == found ) {
      ui->applianceBox->addItem(layer->name(), layer->uuid());
    //применение текущих параметров к новому слою, если выбран пункт Все изображения
      obj->setTransparency(transparencySlider_->maximumPosition());
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

void TransparencyWidget::slotListChanged()
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
      ui->transparencyBox->setEnabled(false);
    }
    else {
      ui->lockButton->setIcon(QIcon(":/mappi/icons/unlocked.png"));
      ui->transparencyBox->setEnabled(true);
    }
    transparencySlider_->setMaximumPosition(obj->transparency());
  }
  else {
    ui->lockButton->setIcon(QIcon(":/mappi/icons/unlocked.png"));
    ui->applianceBox->setItemIcon(ui->applianceBox->currentIndex(), QIcon());
    ui->lockButton->setEnabled(false);
    ui->transparencyBox->setEnabled(true);
    transparencySlider_->setMaximumPosition(allTransparency_);
  }
}

void TransparencyWidget::slotLockLayer()
{
  Layer* l = mapdocument()->layerByUuid(ui->applianceBox->currentData().toString());
  SatLayer* sl = maplayer_cast<SatLayer*>(l);
  SatelliteImage* obj = sl->currentObject();
  obj->setLocked(!obj->isLocked());
  if ( true == obj->isLocked()) {
    ui->lockButton->setIcon(QIcon(":/mappi/icons/locked.png"));
    ui->applianceBox->setItemIcon(ui->applianceBox->currentIndex(), QIcon(":/mappi/icons/locked.png"));
    ui->transparencyBox->setEnabled(false);
  }
  else {
    ui->lockButton->setIcon(QIcon(":/mappi/icons/unlocked.png"));
    ui->applianceBox->setItemIcon(ui->applianceBox->currentIndex(), QIcon());
    ui->transparencyBox->setEnabled(true);
  }
}

}
}
