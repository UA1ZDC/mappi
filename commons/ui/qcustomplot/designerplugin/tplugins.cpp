#include "tplugins.h"

#include <qplugin.h>

#include <tplugin.h>
#include <qcustomplot.h>


DESIGNER_IMPLEMENT( QCustomPlot, "<qcustomplot.h>" )

TPlugins::TPlugins( QObject* p )
  : QObject(p)
{
  APPEND_NEW_PLUGIN( widgets_, QCustomPlot );

}

Q_EXPORT_PLUGIN2( tplugin, TPlugins )
