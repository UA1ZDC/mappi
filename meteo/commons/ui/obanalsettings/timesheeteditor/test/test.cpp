#include <QApplication>
#include <QTextCodec>

#include <cross-commons/app/paths.h>
#include <cross-commons/debug/tlog.h>

#include <meteo/commons/planner/timesheet.h>
#include <meteo/novost/ui/obanalsettings/timesheeteditor/timesheeteditor.h>

using namespace meteo;

int main(int argc, char* argv[])
{
  TAPPLICATION_NAME("meteo");

  QApplication app(argc, argv);
  QTextCodec::setCodecForCStrings(QTextCodec::codecForLocale());

  TimesheetEditor ts;
  QObject::connect(&ts, SIGNAL(finished(int)), &app, SLOT(quit()), Qt::QueuedConnection);

  ts.init(Timesheet::fromString("* */5 * 3,4 5"));
  if (ts.exec() == QDialog::Accepted) {
    debug_log << ts.timesheet().toString();
  }

  return app.exec();
}
