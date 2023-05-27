#include "tplugins.h"

#include <qplugin.h>

#include "tplugin.h"

#include <meteo/commons/ui/map/view/custom/faxviewer.h>
DESIGNER_IMPLEMENT( FaxViewer, "<meteo/commons/ui/map/view/custom/faxviewer.h>" )

TPlugins::TPlugins( QObject* p )
  : QObject(p)
{
  APPEND_NEW_PLUGIN( widgets_, FaxViewer );
}

Q_EXPORT_PLUGIN2( tplugin, TPlugins )
