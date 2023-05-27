#include "firloader.h"

#include <qmenu.h>

#include <cross-commons/debug/tmap.h>

#include <sql/nspgbase/tsqlquery.h>
#include <meteo/commons/global/global.h>
#include <meteo/commons/ui/map/view/mapwindow.h>
#include <meteo/commons/global/weatherloader.h>
#include <meteo/commons/ui/map/layermrl.h>
#include <meteo/commons/ui/map/document.h>
#include <meteo/commons/ui/map/weather.h>
#include <meteo/commons/ui/map/geopolygon.h>

#include "ui_firloader.h"

namespace meteo {
namespace map {

FirLoader::FirLoader( MapWindow* w )
  : MapWidget(w),
  ui_(new Ui::FirLoader)
{
  ui_->setupUi(this);
  QObject::connect( ui_->loadfir, SIGNAL( clicked() ), this, SLOT( slotLoadFirFile() ) );
}

FirLoader::~FirLoader()
{
  delete ui_; ui_ = 0;
}

void FirLoader::slotLoadFirFile()
{
  TSqlQuery sql( global::dbMeteo() );
  sql.setQuery("SELECT st_astext(fir_area) FROM sprinf.firs WHERE fir_area IS NOT NULL ");
  sql.exec();
  Layer* l = new Layer( mapdocument(), QObject::tr("Районы полетной информации") );
  for ( int i = 0, sz = sql.size(); i < sz; ++i ) {
    QString strarea = sql.value( i, 0 );
    strarea.replace("POLYGON((","");
    strarea.replace("))","");
    QStringList list = strarea.split(',');
    GeoVector gv;
    for ( int j = 0, jsz = list.size(); j < jsz; ++j ) {
      QStringList list2 = list[j].split(' ');
      GeoPoint gp = GeoPoint::fromDegree( list2[1].toDouble(), list2[0].toDouble() );
      gv.append(gp);
    }
    GeoPolygon* gp = new GeoPolygon(l);
    gp->setPenColor(Qt::blue);
    gp->setSkelet(gv);
  }
}

}
}
