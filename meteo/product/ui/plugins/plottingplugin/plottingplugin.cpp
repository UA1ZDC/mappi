#include "plottingplugin.h"

#include <meteo/commons/proto/map_isoline.pb.h>
#include <meteo/commons/global/gradientparams.h>
#include <meteo/commons/global/global.h>
#include <meteo/commons/ui/map/view/mapwindow.h>
#include <meteo/commons/ui/map/view/mapview.h>

#include <meteo/commons/ui/plottingwidget/plottingwidget.h>

namespace meteo{
namespace map{

PlottingPlugin::PlottingPlugin()
  : ActionPlugin("plottingaction")
{
}

PlottingPlugin::~PlottingPlugin()
{
}

Action* PlottingPlugin::create( MapScene* scene ) const
{
  PlottingAction* a = new PlottingAction(scene);
  return a;
}

PlottingAction::PlottingAction( MapScene* scene )
  : Action( scene, "plottingaction" ),
  plotwidget_(nullptr)
{
  MapWindow* window = view()->window();
//  QStringList path = QStringList()
//    << QObject::tr("Наноска")
//    << QObject::tr("Изолинии")
//    ;
  auto path = QList<QPair<QString, QString>>
      ({QPair<QString, QString>("deposition", QObject::tr("Наноска")),
        QPair<QString, QString>("isolines", QObject::tr("Изолинии"))});
  GradientParams gradparams( meteo::global::kIsoParamPath() );
  const proto::FieldColors& fieldcolors = gradparams.protoParams();
  for(int i = 0, sz = fieldcolors.color_size(); i < sz; ++i ) {
    const proto::FieldColor& isoline = fieldcolors.color(i);
    auto menuId = QString("plotting%1").arg(isoline.descr());
    auto menuItem = QPair<QString, QString>(menuId, QString::fromStdString( isoline.name() ));
//    QAction* a = window->addActionToMenu( QString::fromStdString( isoline.name() ), path );
    QAction* a = window->addActionToMenu( menuItem, path );
    QObject::connect( a, SIGNAL( triggered() ), this, SLOT( slotOpenPlottingWidget() ) );
  }
}

PlottingAction::~PlottingAction()
{
}

void PlottingAction::slotOpenPlottingWidget()
{
  QAction* a = qobject_cast<QAction*>( sender() );
  if ( nullptr == a ) {
    return;
  }
  QString isoname( a->text() );
  isoname.replace("&", "");

  bool needUpdate = false;

  if ( nullptr == plotwidget_ ) {
    plotwidget_ = new PlottingWidget( view()->window() );
    plotwidget_->setLastDataDate(isoname);
    if (isoname == plotwidget_->currentIsoName()) {
      needUpdate = true;
    }
  }

  plotwidget_->setCurrentIsoDescriptor(isoname);

  if (needUpdate) {
    plotwidget_->updateFields();  
  }
  
  plotwidget_->show();
  plotwidget_->showNormal();
}

}
}


