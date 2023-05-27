#include "cityaction.h"

#include "../mapview.h"
#include "../mapscene.h"

#include <cross-commons/debug/tlog.h>
#include <commons/geobasis/geopoint.h>
#include <meteo/commons/global/global.h>

namespace meteo {
namespace map {

CityAction::CityAction( MapScene* scene )
  : Action(scene, "cityaction" ),
    btn_(new QToolButton),
    widget_(0)
{
  btnitem_ = scene_->addWidget(btn_);
  item_ = btnitem_;
  btn_->setCheckable(true);
  btn_->setStyleSheet(
        "QToolButton{"
        "background-color: white;"
        "border: 1px solid rgb(56,123,248);"
        "}"
        "QToolButton:checked {"
        "background-color: #ffeba0;"
        "}"
        );
  btn_->setIcon( QIcon(":/meteo/icons/map/city.png") );
  btn_->setToolTip( QObject::tr("Населенные пункты") );
  btnitem_->setZValue(15000);
  connect(btn_, SIGNAL(toggled(bool)), SLOT(slotShow(bool)));
}

void CityAction::slotShow(bool on)
{
  if ( false == hasView() ) {
    return;
  }
  if( 0 == widget_ ){
    widget_ = new CityWidget( view() );
    widgets_.append(widget_);
    widget_->hide();
    connect(widget_, SIGNAL(closeWidget()), btn_, SLOT(click()));
  }
  widget_->setVisible(on);
}

}
}
