#include "incutparams.h"

#include "ui_incutsettings.h"

namespace meteo {
namespace map {

IncutParams::IncutParams( MapWindow* w )
  : MapWidget(w),
  ui_( new Ui::IncutSetup )
{
  ui_->setupUi(this);
}

IncutParams::~IncutParams()
{
}

}
}
