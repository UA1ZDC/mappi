#ifndef METEO_MAP_VIEW_ACTIONS_FieldsAnalyseAction_H
#define METEO_MAP_VIEW_ACTIONS_FieldsAnalyseAction_H

#include "action.h"
#include <qpoint.h>
#include <qdialog.h>
#include <qgraphicsproxywidget.h>
#include <commons/geobasis/geopoint.h>

#include <meteo/commons/global/global.h>
#include <commons/obanal/tfield.h>

#include <meteo/commons/rpc/rpc.h>
#include <meteo/commons/proto/surface_service.pb.h>
#include <meteo/commons/services/sprinf/sprinfservice.h>
#include <meteo/commons/proto/field.pb.h>

class QLabel;
class QToolButton;
class QGraphicsProxyWidget;


class TObanalDb;

namespace meteo {
namespace map {

class MapView;

class FieldsAnalyseAction : public Action
{
  Q_OBJECT
  public:
    FieldsAnalyseAction( MapScene* scene );
    ~FieldsAnalyseAction();

   void addActionsToMenu( Menu* menu ) const ;

protected:

  private:

  void createServices();
  rpc::Channel* ctrl_field;

  TObanalDb *_db;

private slots:
  void calcTraject();
  void calcGrad();



};
}
}

#endif
