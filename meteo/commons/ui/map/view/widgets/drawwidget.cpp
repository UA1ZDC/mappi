#include "drawwidget.h"
#include "../mapview.h"
#include "../mapscene.h"
#include "ui_drawtools.h"

#include <commons/geobasis/geopoint.h>

#include <meteo/commons/ui/map/geopolygon.h>

namespace meteo {
namespace map {

DrawWidget::DrawWidget(QWidget* parent) :
  MapWidget(parent),
  ui_(new Ui::DrawTools)
{
  QWidget* w = new QWidget();
  ui_->setupUi(w);
  addWidget(w);

  setTitle(QObject::tr("Рисование"));
  setPixmap(QPixmap(":/meteo/icons/map/paint.png"));

  setMinimumSize(QSize(250, 250));
  setMaximumSize(QSize(250, 250));
}

DrawWidget::~DrawWidget()
{
  delete ui_;
  ui_ = 0;
}


} //map
} //meteo
