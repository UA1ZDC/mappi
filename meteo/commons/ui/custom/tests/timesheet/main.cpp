#include <tdebug.h>

#include "ui_form.h"

#include <tdebug.h>

#include <qapplication.h>
#include <qtextcodec.h>

#include <meteo/commons/ui/custom/timesheet.h>

int testForm( QApplication* app )
{
  Ui::Form* tb = new Ui::Form;
  QWidget* widget = new QWidget;
  tb->setupUi(widget);
  widget->show();
  tb->ts->setTimesheet(QObject::tr("19,30,45 12,18,22 * * *"));
//  tb->ts->setTimesheet(QObject::tr("* 12,18,22 * * *"));
//  tb->ts->setTimesheet(QObject::tr("19,30,45 * * * *"));

  debug_log << tb->ts->timesheet();
  app->setActiveWindow(widget);
  return app->exec();
}

int main( int argc, char** argv )
{
  //QTextCodec::setCodecForCStrings( QTextCodec::codecForLocale() );
  QApplication app( argc, argv );
  return testForm(&app);
}


