#include "automap.h"

#include "ui_automap.h"
#include "prerun.h"

#include "maptoolwidget.h"
#include <cross-commons/debug/tlog.h>
//#include <meteo/commons/global/log.h>
#include <meteo/commons/global/global.h>
#include <meteo/commons/ui/mainwindow/widgethandler.h>
#include <meteo/commons/ui/mainwindow/mainwindow.h>
#include <meteo/commons/ui/jobwidget/jobwidget.h>
#include <meteo/commons/ui/documentviewer/wgtdocview/wgtdocview.h>
#include <meteo/commons/ui/map/weather.h>
#include <QKeyEvent>
#include <QMessageBox>
#include <QToolBar>
#include <QtDebug>
#include <meteo/commons/services/map/tool/mapclient.h>


namespace meteo {

namespace map {

// class CursorGuard
CursorGuard::CursorGuard()
{
  QApplication::setOverrideCursor(Qt::WaitCursor);
}

CursorGuard::~CursorGuard()
{
  QApplication::restoreOverrideCursor();
}


// class AutoMap
AutoMap::AutoMap(QWidget* parent /*=*/)
  : QWidget(parent),
    ui_(new Ui::AutoMap),
    timerSortId_(-1),
    timerWaitId_(-1),
    jobList_(new JobList),
    model_(new AutoMapModel(jobList_, this)),
    proxyModel_(new AutoMapSortProxyModel(this)),
    smMap_(new QSignalMapper(this)),
    smDoc_(new QSignalMapper(this)),
    smDownload_(new QSignalMapper(this)),
    smRunJob_(new QSignalMapper(this))
{
  ui_->setupUi(this);
  setObjectName("automap");

  proxyModel_->setSourceModel(model_);
  proxyModel_->sort(AutoMapModel::TIME_T, Qt::AscendingOrder);

  ui_->tView_->setAlternatingRowColors(true);
  ui_->tView_->setSelectionMode(QAbstractItemView::SingleSelection);
  ui_->tView_->setSelectionBehavior(QAbstractItemView::SelectRows);

  ui_->tView_->setModel(proxyModel_);
  ui_->tView_->horizontalHeader()->setSectionResizeMode(AutoMapModel::VIEW, QHeaderView::Stretch);
  ui_->tView_->hideColumn(AutoMapModel::TIME_T);

  initAction();

#ifdef T_OS_ASTRA_LINUX
  QObject::connect(smMap_, SIGNAL(mapped(const QString&)), this, SLOT(mapOpen(const QString&)));
  QObject::connect(smDoc_, SIGNAL(mapped(const QString&)), this, SLOT(docOpen(const QString&)));
  QObject::connect(smDownload_, SIGNAL(mapped(const QString&)), this, SLOT(docDownload(const QString&)));
  QObject::connect(smRunJob_, SIGNAL(mapped(const QString&)), this, SLOT(slotRunJob(const QString&)));

#else
  QObject::connect(smMap_, SIGNAL(mappedString(const QString&)), this, SLOT(mapOpen(const QString&)));
  QObject::connect(smDoc_, SIGNAL(mappedString(const QString&)), this, SLOT(docOpen(const QString&)));
  QObject::connect(smDownload_, SIGNAL(mappedString(const QString&)), this, SLOT(docDownload(const QString&)));
  QObject::connect(smRunJob_, SIGNAL(mappedString(const QString&)), this, SLOT(slotRunJob(const QString&)));
#endif
  QObject::connect(ui_->tView_, SIGNAL(doubleClicked(const QModelIndex&)), this, SLOT(jobEdit(const QModelIndex&)));

  QTimer::singleShot(100, this, &AutoMap::load);
}

AutoMap::~AutoMap()
{
  delete ui_;
  ui_ = nullptr;

  delete jobList_;
  jobList_ = nullptr;
}

void AutoMap::initAction()
{
  QToolBar* toolBar = new QToolBar(this);
  ui_->horizontalLayout->addWidget(toolBar);

  jobGroup_ = new QActionGroup(this);
  jobGroup_->setEnabled(false);

  jobAdd_ = new QAction(QIcon(":/meteo/icons/plus.png"), tr("Добавить шаблон карты"), this);
  QObject::connect(jobAdd_, SIGNAL(triggered()), this, SLOT(jobAdd()));
  toolBar->addAction(jobAdd_);

  jobEdit_ = new QAction(QIcon(":/meteo/icons/edit.png"), tr("Обновить шаблон карты"), this);
  QObject::connect(jobEdit_, SIGNAL(triggered()), this, SLOT(jobEdit()));
  jobGroup_->addAction(jobEdit_);
  toolBar->addAction(jobEdit_);

  jobCopy_ = new QAction(QIcon(":/meteo/icons/jobs/copy.png"), tr("Копировать шаблон карты"), this);
  QObject::connect(jobCopy_, SIGNAL(triggered()), this, SLOT(jobCopy()));
  jobGroup_->addAction(jobCopy_);
  toolBar->addAction(jobCopy_);

  jobDelete_ = new QAction(QIcon(":/meteo/icons/jobs/trash.png"), tr("Удалить шаблон карты"), this);
  QObject::connect(jobDelete_, SIGNAL(triggered()), this, SLOT(jobDelete()));
  jobGroup_->addAction(jobDelete_);
  toolBar->addAction(jobDelete_);

  jobSave_ = new QAction(QIcon(":/meteo/icons/map/save.png"), tr("Сохранить изменения"), this);
  QObject::connect(jobSave_, SIGNAL(triggered()), this, SLOT(save()));
  toolBar->addAction(jobSave_);
}

rpc::Channel* AutoMap::channel()
{
  rpc::Channel* ch = global::serviceChannel(settings::proto::kMap);
  if (ch == nullptr) {
    error_log << msglog::kNoConnect.arg(global::serviceTitle(settings::proto::kMap));
    error_log.msgBox() << msglog::kNoConnect.arg(global::serviceTitle(settings::proto::kMap));
  }

  return ch;
}

QModelIndex AutoMap::currentIndex()
{
  return proxyModel_->mapToSource(ui_->tView_->currentIndex());
}

void AutoMap::setCurrentIndex(int row)
{
  if (0 <= row && row < jobList_->count()) {
    if (!jobGroup_->isEnabled())
      jobGroup_->setEnabled(true);

    ui_->tView_->setCurrentIndex(proxyModel_->mapFromSource(model_->index(row, 0)));
  } else
    jobGroup_->setEnabled(false);
}

void AutoMap::setIndexWidget(int row)
{
  QModelIndex index = proxyModel_->mapFromSource(model_->index(row, AutoMapModel::VIEW));
  MapToolWidget* widget = new MapToolWidget(ui_->tView_);
  // if (jobList_->get(row)->getState() == JobWrapper::MAP_NOT_FORMED)
  //   widget->setEnabled(false);

  QString name = jobList_->get(row)->name();
  smMap_->setMapping(widget, name);
  QObject::connect(widget, SIGNAL(mapOpen()), smMap_, SLOT(map()));

  smDoc_->setMapping(widget, name);
  QObject::connect(widget, SIGNAL(docOpen()), smDoc_, SLOT(map()));

  smDownload_->setMapping(widget, name);
  QObject::connect(widget, SIGNAL(docDownload()), smDownload_, SLOT(map()));

  smRunJob_->setMapping(widget, name);
  QObject::connect(widget, SIGNAL(runJob()), smRunJob_, SLOT(map()));

  ui_->tView_->setIndexWidget(index, widget);
}

void AutoMap::closeEvent(QCloseEvent* event)
{
  if (jobList_->isChanged()) {
    int res = QMessageBox::question(this,
                                    QObject::tr("Сохранение"),
                                    QObject::tr("Шаблоны карт изменены."),
                                    QObject::tr("Сохранить и закрыть"),
                                    QObject::tr("Закрыть"),
                                    QObject::tr("Отмена")
                                    );

    switch(res) {
      case 0 :
        save();

      case 1 :
        event->accept();
      return;

      case 2 :
        event->ignore();
      return;
    }
  }

  QWidget::closeEvent(event);
}

// update state
void AutoMap::timerEvent(QTimerEvent* event)
{
  if (event->timerId() == timerWaitId_ && timerSortId_ == -1) {
    JobWrapper::ptr_t job = jobList_->get(proxyModel_->mapToSource(proxyModel_->index(0, 0)).row());
    timerSortId_ = startTimer((job->currentPeriod().wait() + 30) * 1000);
    qDebug() << "wait:" << timerWaitId_ << job->title() <<  ":"
             << QDateTime::currentDateTimeUtc().toString("hh:mm:ss")
             << job->currentPeriod().nearest().toString("hh:mm:ss")
             << job->currentPeriod().wait();
    return ;
  }

  if (event->timerId() == timerSortId_) {
    CursorGuard guard;
    QScopedPointer<rpc::Channel> ch(channel());
    // jobList_->update(ch.data());
    for (int row = 0; row < jobList_->count(); ++row) {
      JobWrapper::ptr_t job = jobList_->get(row);
      job->updateState(ch.data());
    }

    model_->updatePeriod();

    ui_->tView_->setCurrentIndex(proxyModel_->index(0, 0));

    qDebug() << "sort:" << timerSortId_ << ":" << QDateTime::currentDateTimeUtc().toString("hh:mm:ss");
    killTimer(timerSortId_);
    timerSortId_ = -1;

    return ;
  }
}

void AutoMap::load()
{
  CursorGuard guard;
  QScopedPointer<rpc::Channel> ch(channel());
  jobList_->load(ch.data());
  if (0 < jobList_->count()) {
    model_->update();
    ui_->tView_->setCurrentIndex(proxyModel_->index(0, 0));

    for (int row = 0; row < model_->rowCount(); ++row) {
      setIndexWidget(row);
    }

    jobGroup_->setEnabled(true);
    timerWaitId_ = startTimer(10 * 1000);
  }

  ui_->tView_->horizontalHeader()->setSectionResizeMode(AutoMapModel::NAME, QHeaderView::ResizeToContents);
  ui_->tView_->horizontalHeader()->setSectionResizeMode(AutoMapModel::STATUS, QHeaderView::ResizeToContents);
}

void AutoMap::save()
{
  CursorGuard guard;
  QScopedPointer<rpc::Channel> ch(channel());
  jobList_->save(ch.data());
}

void AutoMap::jobAdd()
{
  JobWidget jobWidget(this);
  jobWidget.setWindowTitle(QObject::tr("Новое задание"));
  if (jobWidget.exec() == QDialog::Accepted) {
    proto::Job job = jobWidget.job();
    proto::Map map = jobWidget.map();
    job.mutable_map()->CopyFrom(map);

    int row = model_->rowCount();
    model_->insertRow(row);
    jobList_->append(job);

    setIndexWidget(row);
    setCurrentIndex(row);

    ui_->tView_->horizontalHeader()->setSectionResizeMode(AutoMapModel::NAME, QHeaderView::ResizeToContents);
  }
}

void AutoMap::jobCopy()
{
  int row = currentIndex().row();

  proto::Job job;
  job.CopyFrom(jobList_->get(row)->raw());
  job.set_name(QString::fromStdString(job.name()).replace(".job", "_copy.job").toStdString());
  job.set_title((QString::fromStdString(job.title()) + QObject::tr(" (копия)")).toStdString());

  JobWidget jobWidget(this);
  jobWidget.setWindowTitle(QObject::tr("Редактирование задания"));
  jobWidget.setMapId(QString::fromStdString(job.map_id()));
  jobWidget.setJob(job);
  if (jobWidget.exec() == QDialog::Accepted) {
    row++;
    model_->insertRow(row);
    jobList_->insert(row, job);
    model_->updatePeriod();

    setIndexWidget(row);
    setCurrentIndex(row);

    ui_->tView_->horizontalHeader()->setSectionResizeMode(AutoMapModel::NAME, QHeaderView::ResizeToContents);
  }
}

void AutoMap::jobEdit()
{
  int row = currentIndex().row();
  JobWrapper::ptr_t job = jobList_->get(row);

  JobWidget jobWidget(this);
  jobWidget.setWindowTitle(QObject::tr("Редактирование задания"));
  jobWidget.setMapId(QString::fromStdString(job->raw().map_id()));
  jobWidget.setJob(job->raw());
  if (jobWidget.exec() == QDialog::Accepted) {
    jobList_->update(row, jobWidget.job());
    model_->updatePeriod();
  }
}

void AutoMap::jobEdit(const QModelIndex& index)
{
  Q_UNUSED(index);
  jobEdit();
}

void AutoMap::jobDelete()
{
  QModelIndex index = currentIndex();
  int row = index.row();
  JobWrapper::ptr_t job = jobList_->get(row);

  int res = QMessageBox::question(this,
                                  QObject::tr("Внимание"),
                                  QObject::tr("Удалить задание %1?").arg(job->title()),
                                  QObject::tr("Да"),
                                  QObject::tr("Нет")
                                  );

  if (res == 1)
    return;

  QWidget* widget = ui_->tView_->indexWidget(index);
  smRunJob_->removeMappings(widget);
  smDownload_->removeMappings(widget);
  smDoc_->removeMappings(widget);
  smMap_->removeMappings(widget);

  jobList_->remove(row);
  model_->removeRow(row);
  model_->updatePeriod();

  setCurrentIndex(row == 0 ? 0 : row - 1);
}

void AutoMap::slotOnDocListInit()
{
  qApp->processEvents();

  JobWrapper::ptr_t jobWrapper = jobList_->get(currentIndex().row());
  const proto::Job& job = jobWrapper->raw();

  auto doc = qobject_cast<meteo::documentviewer::WgtDocView*>(sender());
  doc->setJobNameFilter(QString::fromStdString(job.title()),
                        QString::fromStdString(job.map().title())
                        );
}

void AutoMap::mapOpen(const QString& name)
{
  CursorGuard guard;
  proto::Document blank = meteo::global::lastBlankParams(proto::kGeoMap);
  QString loaderName = "ptkpp";
  if (blank.has_geoloader())
    loaderName = QString::fromStdString(blank.geoloader());

  auto mainwindow = WidgetHandler::instance()->mainwindow();
  auto window = new MapWindow(mainwindow, blank, loaderName, MnCommon::etcPath() + "/document.menu.conf" );
  window->addPluginGroup("meteo.map.common");
  window->addPluginGroup("meteo.map.weather");
  mainwindow->toMdi(window);

  int row = 0;
  JobWrapper::ptr_t jobWrapper = jobList_->getByName(name, &row);
  const proto::Job& job = jobWrapper->raw();

  proto::Map map;
  map.CopyFrom(job.map());

  proto::Document doc;
  doc.CopyFrom(window->document()->property());
  doc.mutable_map_center()->CopyFrom(job.document().map_center());
  doc.mutable_doc_center()->CopyFrom(job.document().doc_center());
  window->document()->setProperty(doc);

  Period period = jobWrapper->currentPeriod();
  map.set_datetime(period.last().addSecs(-(job.await() * 60)).toString(Qt::ISODate).toStdString());
  for (int i = 0, sz = map.data_size(); i < sz; ++i) {
    map.mutable_data(i)->set_center(job.center());
    if (job.hour_size() > 0)
      map.mutable_data(i)->set_hour(job.hour(0));

    map.mutable_data(i)->set_model(job.model());
    map.mutable_data(i)->set_source(meteo::map::proto::kAuto);
  }

  Weather weather;
  if (!weather.createMap(window->document(), map))
    error_log << QObject::tr("Карта не создана \n\t = %1 %2").arg(QString::fromStdString(map.Utf8DebugString()));

  setCurrentIndex(row);
}

void AutoMap::docOpen(const QString& name)
{
  int row = 0;
  jobList_->getByName(name, &row);

  auto mainwindow = WidgetHandler::instance()->mainwindow();
  auto doc = new meteo::documentviewer::WgtDocView(meteo::documentviewer::WgtDocView::DocumentViewerStates::kStateDefaultMapImagePage);
  mainwindow->toMdi(doc);
  connect(doc, SIGNAL(initialized()), this, SLOT(slotOnDocListInit()));

  setCurrentIndex(row);
}

void AutoMap::slotRunJob(const QString& jobname)
{
  meteo::map::Client client;

  int row = 0;
  jobList_->getByName(jobname, &row);
  JobWrapper::ptr_t jobWrapper = jobList_->get(row);
  proto::Job job = jobWrapper->raw();

  if(false == job.enabled())
  {
    QMessageBox dlg;
    dlg.setWindowTitle(QObject::tr("Внимание!"));
    dlg.setText(QObject::tr("Задание помечено как неактивное.\n Выполнить?"));
    dlg.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    dlg.setDefaultButton(QMessageBox::Yes);
    int ret = dlg.exec();
    if ( ret == QMessageBox::Yes) {
      job.set_enabled(true);
    }

    else {
      return;
    }
  }

  preRun *prerun = new preRun(jobname, this);

  if(QDialog::Rejected == prerun->exec())
  {
    return;

  }
  QList<proto::Map> maps = client.mapsForJob(  job );
  for ( auto map : maps )
  {
    map.set_job_title(job.title());
    map.set_name( job.map_id() );
    map.set_job_name( job.name() );
    job.mutable_map()->CopyFrom(map);
    if ( false == client.processJob(job) )
    {
      return;
    }
  }

}


void AutoMap::docDownload(const QString& name)
{
  int row = 0;
  JobWrapper::ptr_t jobWrapper = jobList_->getByName(name, &row);
  const proto::Job& job = jobWrapper->raw();

  QString ext = JobWrapper::fileExt(job);
  QString fileName = QString("%1_%2.%3")
                     .arg(jobWrapper->name())
                     .arg(jobWrapper->currentPeriod().last().toString(Qt::ISODate))
                     .arg(ext);

  QDir dir(QStandardPaths::writableLocation(QStandardPaths::DownloadLocation));
  QString path = QFileDialog::getSaveFileName(this,
                                              QObject::tr("Сохранить карту"),
                                              dir.absolutePath() + '/' + fileName,
                                              tr("Images (*.%1)").arg(ext)
                                              );

  if (path.isEmpty())
    return;

  CursorGuard guard;
  QScopedPointer<rpc::Channel> ch(channel());
  QByteArray buffer = jobWrapper->download(ch.data());
  if (!buffer.isEmpty()) {
    QFile file(path);
    if (file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
      file.write(buffer);
      file.flush();
      file.close();
    } else
      error_log << QObject::tr("Не удалось открыть файл для записи %1").arg(fileName);
  }

  setCurrentIndex(row);
}

}

}
