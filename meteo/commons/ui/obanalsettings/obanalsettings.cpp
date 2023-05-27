#include "obanalsettings.h"
#include "ui_obanalsettings.h"

#include "obanaloptions.h"
#include "obanalresults.h"

#include <cross-commons/app/paths.h>
#include <cross-commons/debug/tlog.h>

#include <commons/textproto/tprototext.h>

#include <meteo/commons/planner/timesheet.h>

#include <meteo/commons/proto/cron.pb.h>
#include <meteo/commons/proto/obanal.pb.h>
#include <meteo/commons/ui/custom/filedialogrus.h>

#include <QAction>
#include <QCloseEvent>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QMenu>
#include <QMessageBox>
#include <QRegExp>
#include <QString>
#include <QStringList>
#include <QTableWidgetItem>


namespace {

QString settingsHumanName() { return QString::fromUtf8("Задания объективного анализа"); }
QString settingsDirPath() { return MnCommon::etcPath() + "cron.d"; }
QString settingsFileName() { return QDir(settingsDirPath()).absoluteFilePath("obanal.tasks"); }
QString cronTasksFileName() { return QDir(settingsDirPath()).absoluteFilePath("runobanal.custom.conf"); }

QString cronTaskUidKey() { return "--tasks-uid"; }
QString cronFileKey() { return "--file"; }

}

//using namespace maslo;

namespace meteo {

ObanalSettings::ObanalSettings(QWidget* parent) :
  QWidget(parent),
  ui_(new Ui::ObanalSettings()),
  isChanged_(false)
{
  setObjectName(::settingsHumanName());
  ui_->setupUi(this);

  ui_->appPathLineEdit->setText(MnCommon::binPath() + "novost.obanal");
  ObanalResults* obres = new ObanalResults(this);
  ui_->resultsTab->layout()->addWidget(obres);

  ui_->taskTableWidget->setStyleSheet("QTableView::item { height: 36px; }");

  connect(ui_->applyButton, SIGNAL(clicked()), SLOT(slotApply()));
  connect(ui_->cancelButton, SIGNAL(clicked()), SLOT(close()));

  connect(ui_->selectPathButton, SIGNAL(clicked()), SLOT(slotSelectPath()));

  connect(ui_->addButton, SIGNAL(clicked()), SLOT(slotAdd()));
  connect(ui_->editButton, SIGNAL(clicked()), SLOT(slotEdit()));
  connect(ui_->removeButton, SIGNAL(clicked()), SLOT(slotRemove()));

  connect(ui_->taskTableWidget->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)), SLOT(slotEnableActions()));
  connect(ui_->appPathLineEdit, SIGNAL(editingFinished()), SLOT(slotVerifyPath()));

  ui_->taskTableWidget->installEventFilter(this);
  ui_->taskTableWidget->viewport()->installEventFilter(this);
}

ObanalSettings::~ObanalSettings()
{
  delete ui_;
  ui_ = 0;
}

int ObanalSettings::init()
{
  bool ok = load();
  updateTasksTable();

  if (ui_->taskTableWidget->rowCount() > 0) {
    ui_->taskTableWidget->selectRow(0);
  }
  else {
    ui_->taskTableWidget->selectionModel()->clearSelection();
  }

  return (ok ? 0 : -1);
}

void ObanalSettings::closeEvent(QCloseEvent* event)
{
  if (isChanged_ == true) {
    QMessageBox::StandardButton answer = QMessageBox::warning(this, QString::fromUtf8("Настройки изменены"),
                                                                    QString::fromUtf8("Несохранённые изменения будут потеряны. Сохранить изменения?"),
                                                                    QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
                                                                    QMessageBox::Cancel);
    switch (answer) {
      case QMessageBox::Cancel:
          event->ignore();
        return;
      case QMessageBox::Yes:
          if (save() == false) {
            event->ignore();
          }
        break;
      case QMessageBox::No:
      default:
        break;
    }
  }
  QWidget::closeEvent(event);
}

bool ObanalSettings::eventFilter(QObject* object, QEvent* event)
{
  if (ui_->taskTableWidget != 0) {
    if (object == ui_->taskTableWidget) {
      switch (event->type()) {
        case QEvent::ContextMenu: {
            QMenu menu;
            QAction add(QString::fromUtf8("Добавить"), &menu);
            QAction edit(QString::fromUtf8("Редактировать"), &menu);
            QAction remove(QString::fromUtf8("Удалить"), &menu);

            QList<QAction*> actions;
            actions.append(&add);
            const QPoint& pos = static_cast<QContextMenuEvent*>(event)->pos();
            QTableWidgetItem* current = ui_->taskTableWidget->itemAt(ui_->taskTableWidget->viewport()->mapFromGlobal(ui_->taskTableWidget->mapToGlobal(pos)));
            QSharedPointer<obanalsettings::Task> a;
            if (current != 0  && allTasks_.contains(current->row()) == true) {
              a = allTasks_[current->row()];
              actions.append(&edit);
              actions.append(&remove);
            }

            QAction* answer = menu.exec(actions, ui_->taskTableWidget->mapToGlobal(pos));
            if (answer == &add) {
              slotAdd();
            }
            else if (answer == &edit) {
              slotEdit(current->row());
            }
            else if (answer == &remove) {
              slotRemove(current->row());
            }
          }
          return true;
        case QEvent::KeyPress: {
            QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
            switch (keyEvent->key()) {
              case Qt::Key_Delete:
                slotRemove();
              break;
              case Qt::Key_Return:
              case Qt::Key_Enter:
                slotEdit();
              break;
              default:
              break;
            }
          }
          return true;
        default:
          break;
      }
    }
    else if (object == ui_->taskTableWidget->viewport()) {
      switch (event->type()) {
        case QEvent::MouseButtonDblClick: {
            const QPoint& pos = static_cast<QMouseEvent*>(event)->pos();
            QTableWidgetItem* current = ui_->taskTableWidget->itemAt(pos);
            if (current != 0 && allTasks_.contains(current->row()) == true) {
              slotEdit(current->row());
            }
          }
          return true;
        default:
          break;
      }
    }
  }
  return QWidget::eventFilter(object, event);
}

void ObanalSettings::slotApply()
{
  if (save() == true) {
    close();
  }
}

bool ObanalSettings::load()
{
  meteo::cron::Settings cronConfig;
  bool ok = TProtoText::fromFile(::cronTasksFileName(), &cronConfig);
  if (ok == true) {
    for (int i = 0, sz = cronConfig.run_app_size(); i < sz; ++i) {
      if (ui_->appPathLineEdit->text().isEmpty()) {
        if (cronConfig.run_app(i).has_path()) {
          ui_->appPathLineEdit->setText(QString::fromStdString(cronConfig.run_app(i).path()));
        }
      }

      int taskuid = -1;
      for(int j = 0, jsz = cronConfig.run_app(i).arg_size(); j < jsz; ++j) {
        if (QString::fromStdString(cronConfig.run_app(i).arg(j).key()) == ::cronTaskUidKey()) {
          taskuid = QString::fromStdString(cronConfig.run_app(i).arg(j).value()).toInt(&ok);
          if (ok == true && cronConfig.run_app(i).has_timesheet()) {
            tasksTimesheets_.insert(taskuid, QString::fromStdString(cronConfig.run_app(i).timesheet()));
            break;
          }
        }
      }
    }
  }

  obanalsettings::Settings tasksConfig;
  ok = TProtoText::fromFile(::settingsFileName(), &tasksConfig);
  if (ok == true) {
    for (int i = 0, sz = tasksConfig.task_size(); i < sz; ++i) {
      QSharedPointer<obanalsettings::Task> task(new obanalsettings::Task(tasksConfig.task(i)));
      allTasks_.insert(task->uid(), task);
    }
  }
  return ok;
}

void ObanalSettings::updateTasksTable()
{
  for (int i = 0, sz = ui_->taskTableWidget->rowCount(); i < sz; ++i) {
    ui_->taskTableWidget->removeRow(i);
  }
  ui_->taskTableWidget->setRowCount(0);

  typedef QMap<int, QSharedPointer<obanalsettings::Task> >::const_iterator Iter;
  for (Iter it = allTasks_.constBegin(), end = allTasks_.constEnd(); it != end; ++it) {
    addTaskEntry(*(it.value()));
  }
  ui_->taskTableWidget->resizeColumnsToContents();
}

void ObanalSettings::addTaskEntry(const obanalsettings::Task& config)
{
  int row = ui_->taskTableWidget->rowCount();
  ui_->taskTableWidget->setRowCount(row+1);

  QTableWidgetItem* item = new QTableWidgetItem(taskTitle(config));
  ui_->taskTableWidget->setItem(row, TITLE, item);

  QPair<QString, QString> textAndTooltip = taskLevels(config);
  item = new QTableWidgetItem(textAndTooltip.first);
  if (textAndTooltip.second.isEmpty() == false) {
    item->setToolTip(textAndTooltip.second);
  }
  item->setTextAlignment(Qt::AlignCenter);
  ui_->taskTableWidget->setItem(row, LEVELS, item);

  textAndTooltip = taskDataTypes(config);
  item = new QTableWidgetItem(textAndTooltip.first);
  if (textAndTooltip.second.isEmpty() == false) {
    item->setToolTip(textAndTooltip.second);
  }
  item->setTextAlignment(Qt::AlignCenter);
  ui_->taskTableWidget->setItem(row, DATA_TYPES, item);

  textAndTooltip = taskStations(config);
  item = new QTableWidgetItem(textAndTooltip.first);
  if (textAndTooltip.second.isEmpty() == false) {
    item->setToolTip(textAndTooltip.second);
  }
  item->setTextAlignment(Qt::AlignCenter);
  ui_->taskTableWidget->setItem(row, STATIONS, item);

  if (tasksTimesheets_.contains(config.uid())) {
    textAndTooltip = taskNextStart(tasksTimesheets_.value(config.uid()));
  }
  else {
    textAndTooltip = qMakePair(QString("−"),
                               QString::fromUtf8("Расписание не задано"));
  }
  item = new QTableWidgetItem(textAndTooltip.first);
  if (textAndTooltip.second.isEmpty() == false) {
    item->setToolTip(textAndTooltip.second);
  }
  item->setTextAlignment(Qt::AlignCenter);
  ui_->taskTableWidget->setItem(row, NEXT_START, item);
}

QString ObanalSettings::taskTitle(const obanalsettings::Task& config) const
{
  QString result;
  switch(config.params().type()) {
    case obanalsettings::kSurface:
        result = QString::fromUtf8("Анализ приземных данных");
      break;
    case obanalsettings::kAero:
        result = QString::fromUtf8("Анализ аэрологических данных");
      break;
    case obanalsettings::kRadar:
        result = QString::fromUtf8("Анализ радиолокационных данных");
      break;
    case obanalsettings::kOcean:
        result = QString::fromUtf8("Анализ морских данных");
      break;
    case obanalsettings::kGrib:
        result = QString::fromUtf8("Анализ GRIB");
      break;
    default:
        result = QString::fromUtf8("Объективный анализ");
      break;
  }
  return result;
}

QPair<QString, QString> ObanalSettings::taskLevels(const obanalsettings::Task& config) const
{
  QString value;
  QString tooltip;
  if (config.params().type() == obanalsettings::kSurface) {
    value = QString("−");
  }
  else {
    int sz = config.params().level_size();
    if (sz == 0 ||
        (sz == 1 && config.params().level(0) == -1)) {
      value = QString::fromUtf8("Все");
    }
    else {
      value = QString::number(sz);
      QStringList vlist;
      for (int i = 0; i < sz; ++i) {
        vlist.append(QString::number(config.params().level(i)));
      }
      tooltip = vlist.join(", ");
    }
  }
  return qMakePair(value, tooltip);
}

QPair<QString, QString> ObanalSettings::taskDataTypes(const obanalsettings::Task& config) const
{
  QString value;
  QString tooltip;
  int sz = config.params().descr_size();
  if (sz == 0 ||
      (sz == 1 && config.params().descr(0) == -1)) {
    value = QString::fromUtf8("Все");
  }
  else {
    value = QString::number(sz);
    QStringList vlist;
    for (int i = 0; i < sz; ++i) {
      vlist.append(QString::number(config.params().descr(i)));
    }
    tooltip = vlist.join(", ");
  }
  return qMakePair(value, tooltip);
}

QPair<QString, QString> ObanalSettings::taskStations(const obanalsettings::Task& config) const
{
  QString value;
  QString tooltip;
  int sz = config.params().center_size();
  if (sz == 0 ||
      (sz == 1 && config.params().center(0) == -1)) {
    value = QString::fromUtf8("Все");
  }
  else {
    value = QString::number(sz);
    QStringList vlist;
    for (int i = 0; i < sz; ++i) {
      vlist.append(QString::number(config.params().center(i)));
    }
    tooltip = vlist.join(", ");
  }
  return qMakePair(value, tooltip);
}

QPair<QString, QString> ObanalSettings::taskNextStart(const QString& timesheet) const
{
  QDateTime currentDt = QDateTime::currentDateTime();
  QDateTime nextStart = getNextStart(timesheet);

  QString interval;
  if (nextStart >= currentDt) {
    int secs = currentDt.secsTo(nextStart);
    int days = secs / (60*60*24);
    secs = secs % (60*60*24);
    int hours = secs / (60*60);
    secs = secs % (60*60);
    int mins = secs / 60 + 1;

    interval = QString::fromUtf8("Через%1%2%3");
    if (days > 0) {
      interval = interval.arg(QString::fromUtf8(" %1 дн").arg(days));
    }
    else {
      interval = interval.arg("");
    }

    if (hours > 0) {
      interval = interval.arg(QString::fromUtf8(" %1 ч").arg(hours));
    }
    else {
      interval = interval.arg("");
    }

    interval = interval.arg(QString::fromUtf8(" %1 мин").arg(mins));
  }
  else {
    debug_log << "current =" << currentDt.toString(Qt::ISODate)
              << "next =" << nextStart.toString(Qt::ISODate);
  }

  return qMakePair(interval, nextStart.toString("dd.MM.yyyy hh:mm"));
}

QDateTime ObanalSettings::getNextStart(const QString& timesheet) const
{
  QDateTime currentDt = QDateTime::currentDateTime();

  QDate resultDate = currentDt.date();

  Timesheet ts = Timesheet::fromString(timesheet);
  QRegExp reEveryN("\\*\\/([\\d]{1,2})");
  QRegExp reCustom("[\\d]{1,2}(,[\\d]{1,2})*");

  // get date:
  QList<int> availableDD,
             availableMM,
             availableDoW;

  if (ts.day() != "*" &&
      reCustom.indexIn(ts.day()) > -1) {
    foreach (const QString& each, reCustom.cap().split(',')) {
      availableDD.append(each.toInt());
    }
  }
  if (ts.month() != "*" &&
      reCustom.indexIn(ts.month()) > -1) {
    foreach (const QString& each, reCustom.cap().split(',')) {
      availableMM.append(each.toInt());
    }
  }
  if (ts.dayOfWeek() != "*" &&
      reCustom.indexIn(ts.dayOfWeek()) > -1) {
    foreach (const QString& each, reCustom.cap().split(',')) {
      availableDoW.append(each.toInt());
    }
  }

  while (true) {
    bool checkDD = availableDD.isEmpty() || availableDD.contains(resultDate.day());
    bool checkMM = availableMM.isEmpty() || availableMM.contains(resultDate.month());
    bool checkDoW = availableDoW.isEmpty() || availableDoW.contains(resultDate.dayOfWeek());

    if (checkDD && checkMM && checkDoW) {
      break;
    }
    else {
      resultDate = resultDate.addDays(1);
    }
  }

  // get time:
  QList<int> available_hh;
  QList<int> available_mm;

  if (ts.hour() == "*" ||
      reEveryN.indexIn(ts.hour()) > -1) {
    int step = ts.hour() == "*" ? 1
                                : reEveryN.cap(1).toInt();
    if (step > 0) {
      for (int i = 0; i < 24; i += step) {
        available_hh.append(i);
      }
    }
  }
  else if (reCustom.indexIn(ts.hour()) > -1) {
    bool ok = false;
    foreach (const QString& each, reCustom.cap().split(',')) {
      int i = each.toInt(&ok);
      if (ok == true && available_hh.contains(i) == false) {
        available_hh.append(i);
      }
    }
  }

  if (ts.minute() == "*" ||
      reEveryN.indexIn(ts.minute()) > -1) {
    int step = ts.minute() == "*" ? 1
                                  : reEveryN.cap(1).toInt();
    if (step > 0) {
      for (int i = 0; i < 60; i += step) {
        available_mm.append(i);
      }
    }
  }
  else if (reCustom.indexIn(ts.minute()) > -1) {
    bool ok = false;
    foreach (const QString& each, reCustom.cap().split(',')) {
      int i = each.toInt(&ok);
      if (ok == true && available_mm.contains(i) == false) {
        available_mm.append(i);
      }
    }
  }

  if (available_hh.isEmpty() || available_mm.isEmpty()) {
    QDateTime result = QDateTime(resultDate, QTime(0,0));
    while (result <= currentDt) {
      result = result.addDays(1);
    }
    return result;
  }

  while (true) {
    QListIterator<int> hhit(available_hh);
    while (hhit.hasNext()) {
      int hh = hhit.next();
      QListIterator<int> mmit(available_mm);
      while (mmit.hasNext()) {
        int mm = mmit.next();
        QDateTime dt(resultDate, QTime(hh, mm));
        if (dt > currentDt) {
          return dt;
        }
      }
    }
    resultDate = resultDate.addDays(1);
  }
  return QDateTime();
}

bool ObanalSettings::save()
{
  if (isChanged_ == false) {
    return true;
  }

  if (verifyPath(ui_->appPathLineEdit->text()) == false) {
    QMessageBox::warning(this, QString::fromUtf8("Ошибка"),
                         QString::fromUtf8("Некорректный путь к исполняемому файлу приложения"));
    slotVerifyPath();
    return false;
  }

  typedef QMap<int, QSharedPointer<obanalsettings::Task> >::const_iterator Iter;
  obanalsettings::Settings config;
  for (Iter it = allTasks_.constBegin(), end = allTasks_.constEnd(); it != end; ++it) {
    obanalsettings::Task* entry = config.add_task();
    entry->CopyFrom(*(it.value()));
  }

  bool ok = false;
  if (config.task_size() > 0) {
    ok = TProtoText::toFile(config, ::settingsFileName());
  }
  else {
    ok = QFile::exists(::settingsFileName()) ? QFile::remove(::settingsFileName()) : true;
  }

  if (ok == true) {
    ok = makeCronTasks();
    if (ok == true) {
      isChanged_ = false;
      QMessageBox::information(this, QString::fromUtf8("Настройка заданий объективного анализа"),
                               QString::fromUtf8("Настройки успешно сохранены"));
    }
  }

  if (ok == false) {
    QMessageBox::warning(this, QString::fromUtf8("Ошибка"), QString::fromUtf8("Не удалось сохранить настройки заданий объективного анализа"), QMessageBox::Ok);
  }

  return ok;
}

bool ObanalSettings::makeCronTasks() const
{
  typedef QMap<int, QSharedPointer<obanalsettings::Task> >::const_iterator Iter;

  QFile cronSettingsFile(::cronTasksFileName());
  if (cronSettingsFile.exists() == false) {
    if(cronSettingsFile.open(QIODevice::WriteOnly) == false) {
      error_log << cronSettingsFile.errorString();
      return false;
    }
    cronSettingsFile.close();
  }

  meteo::cron::Settings cronSettings;
  std::string appath = ui_->appPathLineEdit->text().toStdString();
  for (Iter it = allTasks_.constBegin(), end = allTasks_.constEnd(); it != end; ++it) {
    if (tasksTimesheets_.contains(it.key())) {
      meteo::cron::RunApp* app = cronSettings.add_run_app();
      app->set_timesheet(tasksTimesheets_.value(it.key()).toStdString());
      app->set_path(appath);
      QString name = ui_->taskTableWidget->item(it.key(), TITLE)->text();
      if (name.isEmpty() == false) {
        app->set_name(name.toStdString());
      }
      meteo::cron::RunApp_Arg* arg = app->add_arg();
      arg->set_key(::cronFileKey().toStdString());
      arg->set_value(::settingsFileName().toStdString());
      arg = app->add_arg();
      arg->set_key(::cronTaskUidKey().toStdString());
      arg->set_value(QString::number(it.key()).toStdString());
    }
  }

  bool result = false;
  if (cronSettings.run_app_size() > 0) {
    result = TProtoText::toFile(cronSettings, ::cronTasksFileName());
  }
  else {
    result = QFile::exists(::cronTasksFileName()) ? QFile::remove(::cronTasksFileName())
                                                  : true;
  }
  return result;
}

void ObanalSettings::slotAdd()
{
  meteo::ObanalOptions dlg;
  if (dlg.exec() == QDialog::Accepted) {
    QSharedPointer<meteo::obanalsettings::Task> newtask(new meteo::obanalsettings::Task(dlg.config()));
    int index = ui_->taskTableWidget->rowCount();
    newtask->set_uid(index);
    allTasks_.insert(index, newtask);
    tasksTimesheets_.insert(index, dlg.timesheet());
    addTaskEntry(*newtask);
    setChange();
    ui_->taskTableWidget->selectRow(index);
  }
}

void ObanalSettings::slotEdit()
{
  if (ui_->taskTableWidget->selectionModel()->hasSelection() == false) {
    return;
  }

  int row = ui_->taskTableWidget->selectedItems().first()->row();
  slotEdit(row);
}

void ObanalSettings::slotEdit(int row)
{
  if (allTasks_.contains(row) == true) {
    QString* timesheet = tasksTimesheets_.contains(row) ? &tasksTimesheets_[row] : 0;
    ObanalOptions dlg;
    dlg.init(allTasks_[row].data(), timesheet);
    if (dlg.exec() == QDialog::Accepted) {
      obanalsettings::Task edited = dlg.config();
      obanalsettings::Task& original = *(allTasks_[row]);
      original.mutable_params()->Swap(edited.mutable_params());
      tasksTimesheets_[row] = dlg.timesheet();

      setChange();
      ui_->taskTableWidget->selectRow(row);
    }
  }
}

void ObanalSettings::slotRemove()
{
  if (ui_->taskTableWidget->selectionModel()->hasSelection() == false) {
    return;
  }

  int row = ui_->taskTableWidget->selectedItems().first()->row();
  slotRemove(row);
}

void ObanalSettings::slotRemove(int row)
{
  if (   allTasks_.contains(row) == true
      && tasksTimesheets_.contains(row) == true) {
    if (QMessageBox::information(this, QString::fromUtf8("Подтверждение удаления"),
                                 QString::fromUtf8("Удалить задание?"),
                                 QMessageBox::Yes | QMessageBox::No,
                                 QMessageBox::No) == QMessageBox::Yes) {
      for (int i = row+1, sz = allTasks_.count(); i < sz; ++i) {
        qSwap(allTasks_[i-1], allTasks_[i]);
        int uid = allTasks_[i-1]->uid();
        allTasks_[i-1]->set_uid(allTasks_[i]->uid());
        allTasks_[i]->set_uid(uid);
      }
      for (int i = row+1, sz = tasksTimesheets_.count(); i < sz; ++i) {
        qSwap(tasksTimesheets_[i-1], tasksTimesheets_[i]);
      }

      allTasks_.remove(allTasks_.count()-1);
      tasksTimesheets_.remove(tasksTimesheets_.count()-1);
      setChange();

      if (ui_->taskTableWidget->rowCount() > 0) {
        if (row < ui_->taskTableWidget->rowCount()) {
          ui_->taskTableWidget->selectRow(row);
        }
        else {
          ui_->taskTableWidget->selectRow(ui_->taskTableWidget->rowCount()-1);
        }
      }
    }
  }
}

void ObanalSettings::slotSelectPath()
{
  ui_->appPathLineEdit->setText(meteo::FileDialog::getOpenFileName(this, QString::fromUtf8("Выберите файл..."),
                                                             qApp->applicationDirPath()));
  slotVerifyPath();
}

void ObanalSettings::slotVerifyPath()
{
  QString appPath = ui_->appPathLineEdit->text();
  QPalette palette;
  if (appPath.isEmpty() == true ||
      verifyPath(appPath) == true) {
    palette.setColor(QPalette::Text,Qt::black);
  }
  else {
    palette.setColor(QPalette::Text,Qt::red);
  }
  ui_->appPathLineEdit->setPalette(palette);
}

bool ObanalSettings::verifyPath(const QString& path) const
{
  return path.isEmpty() == false &&
         QFile::exists(path) == true;
}

void ObanalSettings::slotEnableActions()
{
  bool rowSelected = ui_->taskTableWidget->selectionModel()->hasSelection();
  ui_->editButton->setEnabled(rowSelected);
  ui_->removeButton->setEnabled(rowSelected);
}

void ObanalSettings::setChange()
{
  updateTasksTable();
  isChanged_ = true;
}

} // meteo
