#ifndef METEO_COMMONS_UI_OBANALSETTINGS_OBANALSETTINGS_H
#define METEO_COMMONS_UI_OBANALSETTINGS_OBANALSETTINGS_H

#include <QMap>
#include <QSharedPointer>
#include <QWidget>

class QCloseEvent;
class QDateTime;
class QEvent;
template <typename K, typename V> class QPair;
class QString;

namespace Ui {
  class ObanalSettings;
}

namespace meteo {
namespace obanalsettings {
  class Task;
} // obanalsettings
} // novost

namespace meteo {

class ObanalSettings : public QWidget
{
  Q_OBJECT

  enum Column {
    TITLE = 0,
    LEVELS,
    DATA_TYPES,
    STATIONS,
    NEXT_START
  };

public:
  explicit ObanalSettings(QWidget* parent = 0);
  ~ObanalSettings();

  int init();

private slots:
  void slotApply();
  void slotAdd();
  void slotEdit();
  void slotEdit(int row);
  void slotRemove();
  void slotRemove(int row);
  void slotEnableActions();
  void slotSelectPath();
  void slotVerifyPath();

private:
  bool save();
  bool load();
  bool makeCronTasks() const;
  bool verifyPath(const QString& path) const;

  void setChange();
  void updateTasksTable();
  void addTaskEntry(const meteo::obanalsettings::Task& config);

  QString taskTitle(const meteo::obanalsettings::Task& config) const;
  QPair<QString, QString> taskLevels(const meteo::obanalsettings::Task& config) const;
  QPair<QString, QString> taskDataTypes(const meteo::obanalsettings::Task& config) const;
  QPair<QString, QString> taskStations(const meteo::obanalsettings::Task& config) const;
  QPair<QString, QString> taskNextStart(const QString& timesheet) const;

  QDateTime getNextStart(const QString& timesheet) const;

  void closeEvent(QCloseEvent* event);
  bool eventFilter(QObject* object, QEvent* event);

private:
  Ui::ObanalSettings* ui_;
  bool isChanged_;

  QMap<int, QSharedPointer<meteo::obanalsettings::Task> > allTasks_;
  QMap<int, QString> tasksTimesheets_;

};

} //meteo

#endif // METEO_COMMONS_UI_OBANALSETTINGS_OBANALSETTINGS_H
