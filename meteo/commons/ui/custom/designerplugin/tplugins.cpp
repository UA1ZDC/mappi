#include "tplugins.h"

#include <qplugin.h>

#include "tplugin.h"

#include <meteo/commons/ui/custom/geopointeditor.h>
#include <meteo/commons/ui/custom/docoptions.h>
#include <meteo/commons/ui/custom/clicklabel.h>
#include <meteo/commons/ui/custom/textposdlg.h>
#include <meteo/commons/ui/custom/fontwidget.h>
#include <meteo/commons/ui/custom/alignbutton.h>
#include <meteo/commons/ui/custom/symbolbtn.h>
#include <meteo/commons/ui/custom/colorbtn.h>
#include <meteo/commons/ui/custom/iconsetbtn.h>
#include <meteo/commons/ui/custom/multichoosebtn.h>
#include <meteo/commons/ui/custom/timesheet.h>
#include <meteo/commons/ui/custom/stationwidget.h>
#include <meteo/commons/ui/custom/fontcombobox.h>
#include <meteo/commons/ui/custom/multichoosewgt.h>
#include <meteo/commons/ui/custom/timesheetwgt.h>
#include <meteo/commons/ui/custom/choosefile.h>
#include <meteo/commons/ui/custom/choosedir.h>
#include <meteo/commons/ui/custom/tlgeditor.h>
#include <meteo/commons/ui/custom/coordedit.h>
#include <meteo/commons/ui/custom/ahdwidget.h>
#include <meteo/commons/ui/custom/preparewidget.h>
#include <meteo/commons/ui/custom/placewidget.h>
#include <meteo/commons/ui/custom/resizedcombo.h>
#include <meteo/commons/ui/custom/intervaledit.h>
#include <meteo/commons/ui/custom/selectlevel.h>
#include <meteo/commons/ui/custom/sloywidget.h>
#include <meteo/commons/ui/custom/zondwidget.h>
#include <meteo/commons/ui/custom/gradienteditor.h>
#include <meteo/commons/ui/custom/selectstation.h>
#include <meteo/commons/ui/custom/longintspinbox.h>
#include <meteo/commons/ui/custom/forecastformwidget.h>
#include <meteo/commons/ui/custom/forecastwidgetbutton.h>
#include <meteo/commons/ui/custom/forecastwidgetitem.h>
#include <meteo/commons/ui/custom/forecastwidgetinput.h>
#include <meteo/commons/ui/custom/forecastwidgetresultbox.h>
#include <meteo/commons/ui/custom/calendarbtn.h>
#include <cross-commons/app/paths.h>

DESIGNER_IMPLEMENT( GeoPointEditor, "<meteo/commons/ui/custom/geopointeditor.h>" )
DESIGNER_IMPLEMENT( DocOptions, "<meteo/commons/ui/custom/docoptions.h>" )
DESIGNER_IMPLEMENT( ClickLabel, "<meteo/commons/ui/custom/clicklabel.h>" )
DESIGNER_IMPLEMENT( TextPosdlg, "<meteo/commons/ui/custom/textposdlg.h>" )
DESIGNER_IMPLEMENT( FontComboBox, "<meteo/commons/ui/custom/fontcombobox.h>" )
DESIGNER_IMPLEMENT( FontWidget, "<meteo/commons/ui/custom/fontwidget.h>" )
DESIGNER_IMPLEMENT( AlignButton, "<meteo/commons/ui/custom/alignbutton.h>" )
DESIGNER_IMPLEMENT( SymbolButton, "<meteo/commons/ui/custom/symbolbtn.h>" )
DESIGNER_IMPLEMENT( ColorButton, "<meteo/commons/ui/custom/colorbtn.h>" )
DESIGNER_IMPLEMENT( IconsetButton, "<meteo/commons/ui/custom/iconsetbtn.h>" )
DESIGNER_IMPLEMENT( MultiChooseBtn, "<meteo/commons/ui/custom/multichoosebtn.h>" )
DESIGNER_IMPLEMENT( Timesheet, "<meteo/commons/ui/custom/timesheet.h>" )
DESIGNER_IMPLEMENT( StationWidget, "<meteo/commons/ui/custom/stationwidget.h>" )
DESIGNER_IMPLEMENT( MultiChooseWgt, "<meteo/commons/ui/custom/multichoosewgt.h>" )
DESIGNER_IMPLEMENT( TimesheetWgt, "<meteo/commons/ui/custom/timesheetwgt.h>" )
DESIGNER_IMPLEMENT( ChooseFile, "<meteo/commons/ui/custom/choosefile.h>" )
DESIGNER_IMPLEMENT( ChooseDir, "<meteo/commons/ui/custom/choosedir.h>" )
DESIGNER_IMPLEMENT( TlgEditor, "<meteo/commons/ui/custom/tlgeditor.h>" )
DESIGNER_IMPLEMENT( CoordEdit, "<meteo/commons/ui/custom/coordedit.h>" )
DESIGNER_IMPLEMENT( AhdWidget, "<meteo/commons/ui/custom/ahdwidget.h>" )
DESIGNER_IMPLEMENT( PrepareWidget, "<meteo/commons/ui/custom/preparewidget.h>" )
DESIGNER_IMPLEMENT( PlaceWidget, "<meteo/commons/ui/custom/placewidget.h>" )
DESIGNER_IMPLEMENT( ResizedComboBox, "<meteo/commons/ui/custom/resizedcombo.h>" )
DESIGNER_IMPLEMENT( DateIntervalEdit, "<meteo/commons/ui/custom/intervaledit.h>" )
DESIGNER_IMPLEMENT( SelectLevel, "<meteo/commons/ui/custom/selectlevel.h>" )
DESIGNER_IMPLEMENT( SloyWidget, "<meteo/commons/ui/custom/sloywidget.h>" )
DESIGNER_IMPLEMENT( ZondWidget, "<meteo/commons/ui/custom/zondwidget.h>" )
DESIGNER_IMPLEMENT( GradientWidget, "<meteo/commons/ui/custom/gradienteditor.h>" )
DESIGNER_IMPLEMENT( SelectStation, "<meteo/commons/ui/custom/selectstation.h>" )
DESIGNER_IMPLEMENT( LongIntSpinBox, "<meteo/commons/ui/custom/longintspinbox.h>" )
DESIGNER_IMPLEMENT( ForecastWidgetButton, "<meteo/commons/ui/custom/forecastwidgetbutton.h>" )
DESIGNER_IMPLEMENT( ForecastWidgetItem, "<meteo/commons/ui/custom/forecastwidgetitem.h>" )
DESIGNER_IMPLEMENT( ForecastWidgetInput, "<meteo/commons/ui/custom/forecastwidgetinput.h>" )
DESIGNER_IMPLEMENT( ForecastWidgetResultBox, "<meteo/commons/ui/custom/forecastwidgetresultbox.h>" )
DESIGNER_IMPLEMENT( ForecastFormWidget, "<meteo/commons/ui/custom/forecastformwidget.h>" )
DESIGNER_IMPLEMENT( CalendarButton, "<meteo/commons/ui/custom/calendarbtn.h>" )


TPlugins::TPlugins( QObject* p )
  : QObject(p)
{
  TAPPLICATION_NAME("meteo");
  APPEND_NEW_PLUGIN( widgets_, GeoPointEditor);
  APPEND_NEW_PLUGIN( widgets_, DocOptions);
  APPEND_NEW_PLUGIN( widgets_, ClickLabel);
  APPEND_NEW_PLUGIN( widgets_, TextPosdlg);
  APPEND_NEW_PLUGIN( widgets_, FontComboBox);
  APPEND_NEW_PLUGIN( widgets_, FontWidget);
  APPEND_NEW_PLUGIN( widgets_, AlignButton);
  APPEND_NEW_PLUGIN( widgets_, SymbolButton);
  APPEND_NEW_PLUGIN( widgets_, ColorButton);
  APPEND_NEW_PLUGIN( widgets_, IconsetButton);
  APPEND_NEW_PLUGIN( widgets_, MultiChooseBtn);
  APPEND_NEW_PLUGIN( widgets_, Timesheet);
  APPEND_NEW_PLUGIN( widgets_, StationWidget);
  APPEND_NEW_PLUGIN( widgets_, MultiChooseWgt);
  APPEND_NEW_PLUGIN( widgets_, TimesheetWgt);
  APPEND_NEW_PLUGIN( widgets_, ChooseFile);
  APPEND_NEW_PLUGIN( widgets_, ChooseDir);
  APPEND_NEW_PLUGIN( widgets_, TlgEditor);
  APPEND_NEW_PLUGIN( widgets_, CoordEdit);
  APPEND_NEW_PLUGIN( widgets_, AhdWidget);
  APPEND_NEW_PLUGIN( widgets_, PrepareWidget);
  APPEND_NEW_PLUGIN( widgets_, PlaceWidget);
  APPEND_NEW_PLUGIN( widgets_, ResizedComboBox);
  APPEND_NEW_PLUGIN( widgets_, DateIntervalEdit);
  APPEND_NEW_PLUGIN( widgets_, SelectLevel);
  APPEND_NEW_PLUGIN( widgets_, SloyWidget);
  APPEND_NEW_PLUGIN( widgets_, ZondWidget);
  APPEND_NEW_PLUGIN( widgets_, GradientWidget);
  APPEND_NEW_PLUGIN( widgets_, SelectStation );
  APPEND_NEW_PLUGIN( widgets_, LongIntSpinBox );
  APPEND_NEW_PLUGIN( widgets_, ForecastWidgetItem );
  APPEND_NEW_PLUGIN( widgets_, ForecastWidgetButton );
  APPEND_NEW_PLUGIN( widgets_, ForecastWidgetResultBox );
  APPEND_NEW_PLUGIN( widgets_, ForecastWidgetInput );
  APPEND_NEW_PLUGIN( widgets_, ForecastFormWidget );
  APPEND_NEW_PLUGIN( widgets_, CalendarButton );
}
