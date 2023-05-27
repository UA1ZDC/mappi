#include "scalewidget.h"
#include "ui_scalewidget.h"

#include <cross-commons/debug/tlog.h>

namespace meteo {
namespace map {

ScaleWidget::ScaleWidget(QWidget *parent, int curZoom, int minScale, int maxScale, int step) :
  QWidget(parent),
  ui_(new Ui::ScaleWidget)
{
  ui_->setupUi(this);
  step_ = step;
  setWindowFlags(Qt::FramelessWindowHint | Qt::Tool);
  setAttribute(Qt::WA_TranslucentBackground);
  QPalette palette = this->palette();
  palette.setColor( QPalette::Window, Qt::transparent );
  ui_->zoomSlider->setMaximum(maxScale);
  ui_->zoomSlider->setMinimum(minScale);
  ui_->zoomSlider->setValue(curZoom);
  setGeometry(0,0,32,240);
  connect(ui_->zoomSlider, SIGNAL(sliderMoved(int)), this, SLOT(slotZoom(int)));
  connect(ui_->zoomInButton, SIGNAL(clicked()), this, SLOT(slotZoomIn()));
  connect(ui_->zoomOutButton, SIGNAL(clicked()), this, SLOT(slotZoomOut()));
}

ScaleWidget::~ScaleWidget()
{
  delete ui_;
  ui_ = 0;
}

void ScaleWidget::setZoom(int zoom)
{
  ui_->zoomSlider->setValue(zoom);
}

void ScaleWidget::slotZoomIn()
{
  int val = ui_->zoomSlider->value();
  int newVal = val - step_;
  if (newVal < ui_->zoomSlider->minimum())
  {
    return;
  }
  ui_->zoomSlider->setValue(newVal);
  emit zoom(ui_->zoomSlider->value());
}

void ScaleWidget::slotZoomOut()
{
  int val = ui_->zoomSlider->value();
  int newVal = val + step_;
  if (newVal > ui_->zoomSlider->maximum())
  {
    return;
  }
  ui_->zoomSlider->setValue(newVal);
  emit zoom(ui_->zoomSlider->value());
}

void ScaleWidget::slotZoom(int z)
{
  if (z > ui_->zoomSlider->maximum())
  {
    return;
  }
  if (z < ui_->zoomSlider->minimum())
  {
    return;
  }
  emit zoom(z);
}

}
}

