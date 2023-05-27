#include "plugin.h"

#include <qdialog.h>

#include <cross-commons/debug/tlog.h>
#include <meteo/commons/ui/map/view/mapwindow.h>
#include <meteo/commons/ui/map/view/mapview.h>
#include <meteo/commons/ui/map/view/menu.h>

#include "ui_autocreatepattern.h"

#include <commons/textproto/tprototext.h>
#include <commons/textproto/pbtools.h>
#include <meteo/commons/proto/document_service.pb.h>
#include <meteo/commons/global/weatherloader.h>
#include <meteo/commons/global/global.h>
#include <QMap>

namespace meteo {
namespace map {

static const auto kMenuPath = QList<QPair<QString, QString>>(
{
      QPair<QString, QString>("document", QObject::tr("Документ"))
                                                             });

Plugin::Plugin()
  : ActionPlugin("autocreatepatternaction")
{
}

Plugin::~Plugin()
{
}

Action* Plugin::create( MapScene* scene ) const
{
  return new AutoCreatePattern(scene);
}

AutoCreatePattern::AutoCreatePattern( MapScene* scene )
  : Action( scene, "autocreatepatternaction" )
{
  insertaction_ = scene_->mapview()->window()->addActionToMenu({"autocreatepatternaction", QObject::tr("Создание шаблона карты")}, kMenuPath);
  QObject::connect(insertaction_,SIGNAL(triggered()),this,SLOT(slotAutoCreatePattern()) );
}

AutoCreatePattern::~AutoCreatePattern()
{
  delete jobWidget_;
  jobWidget_=0;
}

void AutoCreatePattern::addActionsToMenu(Menu *menu) const
{
  bool fl = false;
  for ( auto l : scene_->document()->layers() ) {
    if ( false == l->isBase() ) {
      fl = true;
      break;
    }
  }
  if ( false == fl ) {
    return;
  }
  else {
    menu->addDocumentAction(insertaction_);
  }
}

void AutoCreatePattern::saveJobOnServer()
{
  auto ch = meteo::global::serviceChannel(meteo::settings::proto::kMap);
  if ( nullptr == ch ) {
    error_log.msgBox() << msglog::kNoConnect.arg(meteo::global::serviceTitle(meteo::settings::proto::kMap));
    return;
  }
  auto jobs = WeatherLoader::instance()->mapjobs();
  proto::JobList jl;
  for ( auto job : jobs ) {
    auto j = jl.add_job();
    j->CopyFrom(job);
  }
  auto reply = ch->remoteCall( &meteo::map::proto::DocumentService::SetWeatherJobs, jl, 100000);
  delete ch;
  if ( nullptr == reply ) {
    error_log.msgBox() << QObject::tr("Не удалось отправить изменения на сервер");
  }
  delete reply;
}

void AutoCreatePattern::slotAutoCreatePattern()
{
  QString mapID;
  QDialog* dlg = new QDialog();
  Ui::AutoCreatePattern ui;
  ui.setupUi(dlg);
  proto::Map map_info = scene_->document()->info();
  proto::Map map;
  proto::Job job;

  for ( auto l : scene_->document()->layers()){
    if ( true == l->isBase() ) {
      continue;
    }
    auto layer = map.add_data();
    layer->CopyFrom(l->info());
    layer->clear_datetime();
    layer->clear_center();
    layer->clear_model();
    layer->clear_hour();
    layer->clear_data_size();
    layer->clear_center_name();
  }
  map.set_spline_koef(map_info.spline_koef());
  map.set_level(map_info.level());
  map.set_type_level(map_info.type_level());

  job.set_format(proto::kJpeg);
  job.add_hour(map_info.hour());

  if ( false == scene_->document()->activeLayer()->isBase() &&
       true == scene_->document()->activeLayer()->visible() ) {
    job.set_center(scene_->document()->activeLayer()->info().center());
    job.set_model(scene_->document()->activeLayer()->info().model());
  }
  else {
    for ( int i = 0, sz = scene_->document()->layers().size(); i < sz; ++i ) {
      auto l = scene_->document()->layers()[i];
      if ( false == l->isBase() &&
           true == l->visible() ) {
        job.set_center(l->info().center());
        job.set_model(l->info().model());
        break;
      }
    }
  }

  job.mutable_map()->CopyFrom(map);
  job.mutable_document()->CopyFrom(map_info.document());
  job.mutable_document()->clear_uuid();
  job.mutable_document()->clear_incut();
  job.mutable_document()->set_cache(true);
  job.set_period(60);

  int res = dlg->exec();
  if ( QDialog::Accepted == res ) {
    jobWidget_ = new JobWidget(dlg);
    jobWidget_->setWindowTitle(QObject::tr("Новое задание"));
    job.set_title(ui.jobNameDlg->text().toStdString());
    job.set_name(ui.jobNameDlg->text().toStdString());
    jobWidget_->setJob(job);
    if( QDialog::Accepted == jobWidget_->exec() ) {
      auto j = jobWidget_->job();
      j.set_enabled(true);
      WeatherLoader::instance()->addJobToLibrary(j);
      saveJobOnServer();
    }
  }
  delete jobWidget_; jobWidget_ = 0;
  delete dlg;
}

}
}
