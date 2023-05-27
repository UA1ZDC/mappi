#ifndef JOBWIDGET_H
#define JOBWIDGET_H

#include <qmap.h>
#include <qdialog.h>
#include <qmenu.h>
#include <qcheckbox.h>
#include <qwidgetaction.h>

#include "selectwidget.h"
#include "timesheet.h"

#include <meteo/commons/planner/timesheet.h>
#include <meteo/commons/global/weatherloader.h>
#include <meteo/commons/proto/weather.pb.h>

namespace Ui{
 class Job;
}

namespace meteo {
namespace map {

namespace automap {
enum Mode{
  Add = 0,
  Edit = 1
};
}

class JobWidget : public QDialog
{
  Q_OBJECT
  public:
    JobWidget(QWidget *parent = 0);
    ~JobWidget();
    proto::Job job();
    proto::Map map();
    void setMapId(const QString& name) { mapid_ = name; }
    void clearForm();
    void setJob(const proto::Job& job);
    void setMode( automap::Mode mode );
    Ui::Job* ui_;

  protected:
    void accept();

  private:
    QString mapid_;
    automap::Mode mode_;
    QMap<QString, QString> centers_;
    QMenu* hourMenu_;
    QList<QCheckBox*> checklist_;
    SelectWidget* addLayer_;
    TimeSheet* timeSheet_ = nullptr;
    QString sheet_ = "0 * * * *";
    int period_ = 60;
    int await_ = 0;
    int cron_ = 0;
    QString jobName_;
    QString mapName_;
    proto::Document doc_;
    proto::IncutParams incut_;
    proto::LegendParams legend_;
    void getMeteoCenters();
    void createMenu();
    bool checkHour(const proto::Job& job, int hour);
    void setMap(const proto::Map& map);
    void parseSheet();
    void incutInit();
    void legendInit();

  private slots:
    void slotChangeSize(const QString& str);
    void updateButton();
    void slotAddDate();
    void slotRemoveDate();
    void slotCenterChange(const QString& text);
    void slotHourButtonClicked();
    void slotCheckToggled(bool on);
    void chooseFile();
    void slotEditNameBtnClicked();

    void slotOkButton();
    void slotNoButton();

    void slotChangeStoreType();

    void slotNameChanged();
    void slotAddLayer();
    void slotEditLayer();
    void slotRemoveLayer();
    void slotItemChanged();

    void slotEditTimeSheet();

    void slotOnIncutVisible();
    void slotOnLegendVisible();
};

}
}

#endif // JOBWIDGET_H
