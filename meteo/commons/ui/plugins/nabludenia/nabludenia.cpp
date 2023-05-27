#include "nabludenia.h"
#include "ui_nabludenia.h"

#include <qfile.h>
#include <qhash.h>

#include <sql/psql/psqlquery.h>

#include <cross-commons/app/paths.h>

#include <meteo/commons/global/dateformat.h>
#include <meteo/commons/global/dateformat.h>
#include <commons/textproto/tprototext.h>

#include <meteo/commons/global/global.h>
#include <meteo/commons/proto/field.pb.h>
#include <meteo/commons/proto/puanson.pb.h>
#include <meteo/commons/proto/surface_service.pb.h>
#include <meteo/commons/ui/mainwindow/widgethandler.h>
#include <meteo/commons/ui/map/customevent.h>
#include <meteo/commons/ui/map/layerpunch.h>
#include <meteo/commons/ui/map/puanson.h>
#include <meteo/commons/ui/map/view/mapview.h>
#include <meteo/commons/ui/map/view/mapwindow.h>
#include <meteo/commons/ui/map/view/widgets/layerswidget.h>
#include <meteo/commons/ui/map/weather.h>
#include <meteo/commons/ui/map/layermenu.h>
#include <sql/nosql/nosqlquery.h>

namespace meteo {
namespace map {

static constexpr int kSyncallTimeout = 100000;
static const QString kTermAll = QObject::tr("Все сроки");
static const QString kTermStandart = QObject::tr("Стандартные сроки");

enum {
  Id            = 0,
  Date          = 1,
  Level          = 2,
  CountPoint    = 3,
  ExistData     = 4,
  Center        = 5,
  Model         = 6,
  Hour          = 7,
  NetStep       = 8,
  ForecastBeg   = 9,
  ForecastEnd   = 10
};

Nabludenia::Nabludenia( MapWindow* view )
  : MapWidget(view),
    ui_( new Ui::Nabludenia ),
    termsall_(false)
{
  ui_->setupUi(this);

  menutermtype_ = new QMenu(this);

  QAction* termstandard = menutermtype_->addAction(kTermStandart);
  QObject::connect(termstandard, &QAction::triggered, this, &Nabludenia::slotTermStandard);

  QAction* termall = menutermtype_->addAction(kTermAll);
  QObject::connect( termall, &QAction::triggered, this, &Nabludenia::slotTermAll );

  QFile styleFile(":/style.css");
  styleFile.open(QIODevice::ReadOnly);
  QString style = styleFile.readAll();
  ui_->treeWidget->setStyleSheet(style);
  ui_->treeWidget->header()->setMinimumSectionSize(30);

  ui_->treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);
  ui_->treeWidget->hideColumn(Id);
  hideGribColumns();


  ui_->dateEdit->setDate(QDateTime::currentDateTimeUtc().date());

  ui_->lblterms->setText(kTermStandart);

  if ( nullptr != mapdocument() && nullptr != mapdocument()->eventHandler() ) {
      mapdocument()->eventHandler()->installEventFilter(this);
    }

  punchlibrary_ = WeatherLoader::instance()->punchlibrary();
  punchmaps_ = WeatherLoader::instance()->punchmaps();

  ui_->cmbTemplate->clear();
  for( auto it: punchmaps_) {
      ui_->cmbTemplate->addItem( QString::fromStdString(it.title() ) );
    }
  turnSignals();

  if ( 0 < ui_->cmbTemplate->count() ) {
      ui_->cmbTemplate->setCurrentIndex(0);
    }

  setCurrentSource( currentSource() );
  slotTermStandard();

  ui_->dateEdit->setDisplayFormat(meteo::dtHumanFormatDateOnly);
}

Nabludenia::~Nabludenia()
{
  delete menutermtype_;
  menutermtype_ = nullptr;
  delete ui_;
  ui_ = nullptr;
}

void Nabludenia::turnSignals()
{
  QObject::connect(ui_->dateEdit, SIGNAL( dateTimeChanged(QDateTime) ), this, SLOT(slotDateTimeChanged() ) );
  QObject::connect(ui_->closeBtn, SIGNAL( clicked() ), this, SLOT( close() ) );
  QObject::connect(ui_->addBtn, SIGNAL( clicked() ), this, SLOT( slotAddLayerBtn() ) );
  QObject::connect(ui_->delBtn, SIGNAL( clicked() ), this, SLOT(  slotRemoveLayer() ) );
  QObject::connect( ui_->treeWidget, SIGNAL( currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)),
                    this, SLOT( slotItemSelectionChanged(QTreeWidgetItem*) ) );
  QObject::connect(ui_->cmbTemplate, SIGNAL( currentIndexChanged( const QString& ) ),
                   this, SLOT(slotPunchmapChanged( const QString& ) ) );
  QObject::connect(ui_->cmbSource, SIGNAL(currentIndexChanged( int ) ), this, SLOT(slotSourceChanged( int ) ) );
  QObject::connect( ui_->treeWidget, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slotContextMenu(QPoint)));
  QObject::connect( ui_->selectlevel, SIGNAL( levelChanged( int, int ) ), this, SLOT( slotLevelChanged( int, int ) ) );
  QObject::connect( ui_->cmbterm, SIGNAL( currentIndexChanged( int ) ), this, SLOT( slotTermChanged( int ) ) );
  QObject::connect(ui_->treeWidget, SIGNAL(itemSelectionChanged()), this, SLOT(slotChangeSelection()));
  QObject::connect( ui_->btnterm, SIGNAL( clicked() ), this, SLOT( slotBtntermClicked() ) );
  QObject::connect(ui_->treeWidget, SIGNAL( itemDoubleClicked(QTreeWidgetItem *, int ) ), this, SLOT( slotDoubleClicked(QTreeWidgetItem *, int) ) );
}

void Nabludenia::muteSignals()
{
  QObject::disconnect( ui_->dateEdit, SIGNAL( dateTimeChanged( QDateTime ) ), this, SLOT(slotDateTimeChanged() ) );
  QObject::disconnect( ui_->closeBtn, SIGNAL( clicked() ), this, SLOT( close() ) );
  QObject::disconnect( ui_->addBtn, SIGNAL( clicked() ), this, SLOT( slotAddLayerBtn() ) );
  QObject::disconnect( ui_->delBtn, SIGNAL( clicked() ), this, SLOT(  slotRemoveLayer() ) );
  QObject::disconnect( ui_->treeWidget, SIGNAL( currentItemChanged( QTreeWidgetItem*,QTreeWidgetItem* ) ),
                       this, SLOT(slotItemSelectionChanged( QTreeWidgetItem*) ) );
  QObject::disconnect( ui_->cmbTemplate, SIGNAL( currentIndexChanged( const QString& ) ),
                       this, SLOT(slotPunchmapChanged( const QString& ) ) );
  QObject::disconnect( ui_->cmbSource, SIGNAL( currentIndexChanged( int ) ), this, SLOT(slotSourceChanged( int ) ) );
  QObject::disconnect( ui_->treeWidget, SIGNAL( customContextMenuRequested( QPoint ) ), this, SLOT( slotContextMenu(QPoint) ) );
  QObject::disconnect( ui_->selectlevel, SIGNAL( levelChanged( int, int ) ), this, SLOT( slotLevelChanged( int, int ) ) );
  QObject::disconnect( ui_->cmbterm, SIGNAL( currentIndexChanged( int ) ), this, SLOT( slotTermChanged( int ) ) );
  QObject::disconnect( ui_->treeWidget, SIGNAL( itemSelectionChanged() ), this, SLOT( slotChangeSelection() ) );
  QObject::disconnect( ui_->btnterm, SIGNAL( clicked() ), this, SLOT( slotBtntermClicked() ) );
  QObject::disconnect(ui_->treeWidget, SIGNAL( itemDoubleClicked(QTreeWidgetItem *, int ) ), this, SLOT( slotDoubleClicked(QTreeWidgetItem *, int) ) );
}

void Nabludenia::setCurrentMap(const QString& title)
{
  proto::DataSource cursrc = proto::kAuto;
  QString id;
  for ( const proto::Map& map: punchmaps_ ) {
      if ( title != QString::fromStdString(map.title() ) ) {
          continue;
        }
      currentmap_.CopyFrom(map);
      if ( map.data_size() > 0 ) {
          id = QString::fromStdString( map.data(0).template_name() );
          if( true == map.data(0).has_source() ) {
              cursrc = map.data(0).source();
            }
        }
      break;
    }

  if ( true == id.isEmpty() ) {
      error_log.msgBox() << QObject::tr("Не указан знак наноски");
      return;
    }

  if ( false == punchlibrary_.contains(id) ) {
      error_log.msgBox() << QObject::tr("Не найден шаблон %1").arg(id);
      return;
    }

  int indx = ui_->cmbTemplate->findText(title);
  if ( -1 == indx ) {
      error_log << QObject::tr("Не найден шаблон %1").arg(id);
      return;
    }
  if ( indx != ui_->cmbTemplate->currentIndex() ) {
      ui_->cmbTemplate->blockSignals(true);
      ui_->cmbTemplate->setCurrentIndex(indx);
      ui_->cmbTemplate->blockSignals(false);
    }

  ui_->selectlevel->blockSignals(true);
  switch ( currentmap_.data_type() ) {
    case surf::kOceanType:
      ui_->selectlevel->setViewMode(SelectLevel::kOcean);
      break;
    case surf::kAeroType:
      ui_->selectlevel->setViewMode(SelectLevel::kAero);
      break;
    case surf::kSynopType:
    default:
      ui_->selectlevel->setViewMode(SelectLevel::kNoMode);
      break;
    }
  ui_->selectlevel->blockSignals(false);

  QWidget::setWindowTitle(QObject::tr("Наноска %1").arg(title));
  QWidget::adjustSize();

  const meteo::puanson::proto::Puanson& punch = punchlibrary_[id];
  setCurrentPunch(punch);

  /* if ( proto::kAuto != cursrc ) {
    ui_->cmbSource->setDisabled(true);
  } else {
    ui_->cmbSource->setDisabled(false);
  }*/
  setCurrentSource(cursrc);
  loadAvailableData();
}

void Nabludenia::setCurrentPunch( const meteo::puanson::proto::Puanson& punch )
{
  currentpunch_ = punch;
  showPunch(currentpunch_);
}

void Nabludenia::setCurrentSource(proto::DataSource source)
{
  muteSignals();
  switch( source ) {
    case proto::kField:
      ui_->cmbSource->setCurrentIndex(1);
      break;
    case proto::kGribSource:
      ui_->cmbSource->setCurrentIndex(2);
      break;
    case proto::kSurface:
      ui_->cmbSource->setCurrentIndex(0);
      break;
    default:
      ui_->cmbSource->setCurrentIndex(3);
      break;
    }

  turnSignals();

  if ( proto::kSurface == currentSource() ) {
      ui_->cmbterm->setDisabled(true);
    }
  else {
      ui_->cmbterm->setEnabled(true);
    }
}

proto::DataSource Nabludenia::currentSource() const
{
  proto::DataSource cs = proto::kSurface;
  switch (ui_->cmbSource->currentIndex()) {
    case 1:
      cs = proto::kField;
      break;
    case 2:
      cs = proto::kGribSource;
      break;
    case 0:
      cs = proto::kSurface;
      break;
    default:
      cs = proto::kAuto;
      break;
    }

  return cs;
}

void Nabludenia::loadAvailableData()
{
  ui_->treeWidget->clear();
  hideGribColumns();
  if ( 0 == currentmap_.data_size() ) {
      error_log << QObject::tr("Неверный шаблон наноски. Не указан ни один тип данных");
      return;
    }
  switch( currentSource() ) {
    case proto::kField:
      showGribColumns();
      ui_->treeWidget->hideColumn(ExistData);
      loadAvailableField();
      break;
    case proto::kGribSource:
      showGribColumns();
      loadAvailableGrib();
      break;
    case proto::kSurface:
    default:
      ui_->treeWidget->showColumn(CountPoint);
      loadAvailableSurface();
      break;
    }
  updateItems();
}

void Nabludenia::loadAvailableGrib()
{
  proto::WeatherLayer info;
  info.CopyFrom(currentmap_.data(0));
  info.set_mode(proto::kPuanson);
  info.set_source(proto::kGribSource);
  int cl = currentLevel();
  if(0 < cl){
    info.set_level(cl);
  }
  info.set_type_level(currentTypeLevel());
  int ctrm = currentTerm();
  if ( -1 != ctrm ) {
      info.set_hour(ctrm);
    }
  auto reply = sendGribRequest();
  if ( nullptr == reply ) {
      QMessageBox::warning(
            this,
            QObject::tr("Ошибка при получении данных"),
            QObject::tr("Данные от сервиса данных не получены. Попробуйте еще раз или перезапустите сервис данных."),
            QObject::tr("Закрыть")
            );
      return;
    }

  for ( const grib::GribInfo& ginfo: reply->info() ) {
      QString date_str = QString::fromStdString(ginfo.date());
      QDateTime dt = QDateTime::fromString(date_str, Qt::ISODate);
      if ( true == termsall_ || 0 == dt.time().minute() ) {
          proto::WeatherLayer l_info;
          l_info.CopyFrom(info);
          l_info.set_datetime(ginfo.date());
          l_info.set_hour(ginfo.hour().hour());//      optional HourType hour    = 6; //!< Срок
          l_info.set_center(ginfo.center());
          l_info.set_center_name(ginfo.center_name());
          l_info.set_model(ginfo.model());
          l_info.set_level(ginfo.level());
          l_info.set_type_level(ginfo.level_type());//у GRIB для этого большая табличка
          l_info.mutable_meteo_descr()->CopyFrom(ginfo.param());
          //l_info.set_data_size(ginfo.param_size());
          if(ginfo.hour().has_dt1()){
            l_info.set_forecast_beg(ginfo.hour().dt1());//TODO нужно понять сюда ли записывать dt1 dt2 timerange
          }
          if(ginfo.hour().has_dt2()){
            l_info.set_forecast_end(ginfo.hour().dt2());
          }
          if(ginfo.has_net_type()){
              l_info.set_net_type(ginfo.net_type());
            }
          if(ginfo.has_step_lat()){
              l_info.set_step_lat(ginfo.step_lat());
            }
          if(ginfo.has_step_lon()){
              l_info.set_step_lon(ginfo.step_lon());
            }
          //param      = 7; //!< Номер параметра - сейчас это дескриптор bufr

          QByteArray barr;
          global::protomsg2arr(l_info, &barr);

          QTreeWidgetItem* item = new QTreeWidgetItem(ui_->treeWidget);
          item->setData(Date, Qt::UserRole, barr);
          item->setText(Date, meteo::dateToHumanTimeShort(dt));
          item->setText(Level, QString::number(l_info.level()));
          item->setText(Center, QString::fromStdString(l_info.center_name()));
          item->setText(Model, QString::number(l_info.model()));
          item->setText(Hour, QString::number(l_info.hour()));
          QString netStep = "Не указан";
          if(l_info.has_net_type()){
              netStep = obanal::netTypetoStr(::NetType(l_info.net_type()));
            }
          if(ginfo.has_step_lat()&&ginfo.has_step_lon()){
            netStep =  stepsToStr(ginfo.step_lat(),ginfo.step_lon());
            }
          item->setText(NetStep,netStep);
          if(l_info.has_forecast_beg()){
            ui_->treeWidget->showColumn(ForecastBeg);
            QString datef = QString::fromStdString(l_info.forecast_beg());
            QDateTime dtf = QDateTime::fromString(datef, Qt::ISODate);
            item->setText(ForecastBeg,meteo::dateToHumanTimeShort(dtf));
          }
          if(l_info.has_forecast_end()){
            ui_->treeWidget->showColumn(ForecastEnd);
            QString datef = QString::fromStdString(l_info.forecast_end());
            QDateTime dtf = QDateTime::fromString(datef, Qt::ISODate);
            item->setText(ForecastEnd,meteo::dateToHumanTimeShort(dtf));
          }

          //item->setText(CountPoint,  QString::number(ginfo.param_size()));
          QStringList params;
          for(int i=0;i<ginfo.param_size();++i)
            {
              params << TMeteoDescriptor::instance()->name(ginfo.param(i));
            }
          item->setText(ExistData, params.join(";"));
        }
    }
  delete reply;

  for(auto i: { Date, CountPoint, Center, Model }) {
      ui_->treeWidget->resizeColumnToContents(i);
    }
}


void Nabludenia::loadAvailableSurface()
{
  proto::WeatherLayer info;
  info.CopyFrom(currentmap_.data(0));
  info.set_mode(proto::kPuanson);
  info.set_source(proto::kSurface);
  info.set_center(global::kCenterWeather);
  info.set_model(global::kModelSurface);
  int cl = currentLevel();
  if(0 < cl){
    info.set_level(cl);
  }
  info.set_type_level(currentTypeLevel());
  info.set_center_name(global::kCenterWeatherName.toStdString());

  auto reply = sendSurfaceRequest();
  if ( nullptr == reply ) {
      QMessageBox::warning(
            this,
            QObject::tr("Ошибка при получении данных"),
            QObject::tr("Данные от сервиса данных не получены. Попробуйте еще раз или перезапустите сервис данных."),
            QObject::tr("Закрыть")
            );
      return;
    }
  for ( const surf::CountData& data: reply->data() ) {
      QString date_str = QString::fromStdString(data.date());
      QDateTime dt = QDateTime::fromString(date_str, Qt::ISODate);
      if ( true == termsall_ || 0 == dt.time().minute() ) {
          proto::WeatherLayer l_info;
          l_info.CopyFrom(info);
          l_info.set_datetime(data.date());
          l_info.set_hour(0);
         // l_info.set_data_size(data.count());

          QByteArray barr;
          global::protomsg2arr(l_info, &barr);

          QTreeWidgetItem* item = new QTreeWidgetItem(ui_->treeWidget);
          item->setData(Date, Qt::UserRole, barr);
          item->setText(Level, QString::number(l_info.level()));
          item->setText(Date, meteo::dateToHumanTimeShort(dt));
          if ( SelectLevel::kNoMode != ui_->selectlevel->viewMode() && -1 < data.level() ) {
            l_info.set_level(data.level());
            ui_->treeWidget->showColumn(Level);
          } else {
            ui_->treeWidget->hideColumn(Level);
          }
          item->setText(CountPoint, QString::number(data.count())+" ("+QString::number(data.cover()*100)+"%)");
        }
    }
  delete reply;

  for(auto i: { Date, CountPoint, Center, Model }) {
      ui_->treeWidget->resizeColumnToContents(i);
    }
}

void Nabludenia::loadAvailableField()
{
  proto::WeatherLayer info;
  info.CopyFrom(currentmap_.data(0));
  info.set_mode(proto::kPuanson);
  info.set_source(proto::kField);
  int cl = currentLevel();
  if(0 <= cl){
    info.set_level(cl);
  }
  info.set_type_level(currentTypeLevel());

  int ctrm = currentTerm();
  if ( -1 != ctrm ) {
      info.set_hour(ctrm);
    }

  auto reply = std::unique_ptr<meteo::field::DataDescResponse>( sendFieldRequest() );
  if ( nullptr == reply ) {
      QMessageBox::warning(
            this,
            QObject::tr("Ошибка при получении данных"),
            QObject::tr("Данные от сервиса полей не получены. Попробуйте еще раз или перезапустите сервис полей."),
            QObject::tr("Закрыть")
            );
      return;
    }
  QList<int> descrs;

  typedef struct Key {
    int center;
    int model;
    int hour;
    int net_type;
    bool operator==(const Key& other) const {
      return this->center == other.center &&
          this->model == other.model &&
          this->hour == other.hour &&
          this->net_type == other.net_type;
    }
    //Не используется, но нужен для компиляции
    bool operator<(const Key& other) const   {
      if ( this->center != other.center ) {
          return this->center < other.center ;
        }
      if ( this->model != other.model  ){
          return this->model < other.model;
        }
      if ( this->hour != other.hour ){
          return this->hour < other.hour;
        }
      if ( this->net_type != other.net_type ) {
          return this->net_type < other.net_type;
        }
      return false;
    }
  } Key;
  QMap<QDateTime, QMap<Key, const field::DataDesc*>> terms;

  for ( int i = 0; i < reply->descr_size(); ++i  ){
      const field::DataDesc*  desc = reply->mutable_descr(i);
      auto dt = NosqlQuery::datetimeFromString(desc->date());
      if ( false == dt.isValid() ) {
          warning_log << QObject::tr("Неверный формат даты = %1").arg(QString::fromStdString(desc->date()));
          continue;
        }
      QMap<Key,const field::DataDesc* >& map = terms[dt];
      Key mapKey;// = {};
      mapKey.center = desc->center();
      mapKey.hour = desc->hour();
      mapKey.model = desc->model();
      mapKey.net_type = desc->net_type();
      if ( false == map.contains(mapKey) ){
          map.insert( mapKey, desc);
        }
      else {
          auto target = map[mapKey];
          if ( target->count_point() < desc->count_point() ){
              map.insert( mapKey, desc);
            }
        }
    }


  for ( auto it = terms.cbegin(); it != terms.cend(); ++it ) {
      QDateTime dt = it.key();
      for ( auto desc: it.value() ) {
          if ( 0 == dt.time().minute() ) {
              proto::WeatherLayer layer;
              layer.CopyFrom(info);
              layer.set_center(desc->center());
              layer.set_model(desc->model());
              layer.set_datetime(dt.toString(Qt::ISODate).toStdString());

              //layer.set_data_size(desc->count_point());
              layer.set_center_name(desc->center_name());
              layer.set_hour(desc->hour() / 3600);
              if(desc->has_net_type()){
                  layer.set_net_type(desc->net_type());
                }

              if(desc->has_dt1()){
                layer.set_forecast_beg(desc->dt1());//TODO нужно понять сюда ли записывать dt1 dt2 timerange
              }
              if(desc->has_dt2()){
                layer.set_forecast_end(desc->dt2());
              }
              /*if(ginfo.has_step_lat()){
                  layer.set_step_lat(ginfo.step_lat());
                }
              if(ginfo.has_step_lon()){
                  layer.set_step_lon(ginfo.step_lon());
                }*/

              QDateTime itemdt = PsqlQuery::datetimeFromString(layer.datetime());
              QByteArray barr;
              global::protomsg2arr(layer, &barr);

              QTreeWidgetItem* item = new QTreeWidgetItem(ui_->treeWidget);
              item->setText(Id, QString::fromStdString(desc->id()));
              item->setData(Date, Qt::UserRole, barr);
              item->setText(Date, meteo::dateToHumanTimeShort(itemdt));
              item->setText(CountPoint, QString::number(layer.data_size()));
              item->setText(Center, QString::fromStdString(layer.center_name()));
              item->setText(Model, QString::number(layer.model()));
              item->setText(Hour, QString::number(layer.hour()));

              if(SelectLevel::kNoMode != ui_->selectlevel->viewMode() && 100 == desc->level_type()){
                ui_->treeWidget->showColumn(Level);
                item->setText(Level, QString::number(desc->level()));
              } else {
                ui_->treeWidget->hideColumn(Level);
              }


              QString netStep = "Не указан";
              if(layer.has_net_type()){
                netStep = obanal::netTypetoStr(::NetType(layer.net_type()));
                }
              if(layer.has_step_lat()&&layer.has_step_lon()){
                netStep =  stepsToStr(layer.step_lat(),layer.step_lon());
                }
              item->setText(NetStep,netStep);

              if(layer.has_forecast_beg()){
                ui_->treeWidget->showColumn(ForecastBeg);
                QString datef = QString::fromStdString(layer.forecast_beg());
                QDateTime dtf = QDateTime::fromString(datef, Qt::ISODate);
                item->setText(ForecastBeg,meteo::dateToHumanTimeShort(dtf));
              }
              if(layer.has_forecast_end()){
                ui_->treeWidget->showColumn(ForecastEnd);
                QString datef = QString::fromStdString(layer.forecast_end());
                QDateTime dtf = QDateTime::fromString(datef, Qt::ISODate);
                item->setText(ForecastEnd,meteo::dateToHumanTimeShort(dtf));
              }

            }
        }
    }

  for ( const auto& it: { Date, CountPoint, Center, Model } ) {
      ui_->treeWidget->resizeColumnToContents(it);
    }
  for ( int i = 0, sz = currentpunch_.rule_size(); i < sz; ++i ) {
      currentpunch_.mutable_rule(i)->set_visible( descrs.contains(currentpunch_.rule(i).id().descr() ) );
    }
}

void Nabludenia::hideGribColumns(){
  ui_->treeWidget->hideColumn(Level);
  ui_->treeWidget->hideColumn(Center);
  ui_->treeWidget->hideColumn(Model);
  ui_->treeWidget->hideColumn(Hour);
  ui_->treeWidget->hideColumn(NetStep);
  ui_->treeWidget->hideColumn(ForecastBeg);
  ui_->treeWidget->hideColumn(ForecastEnd);
  ui_->treeWidget->hideColumn(ExistData);
}

void Nabludenia::showGribColumns(){
  ui_->treeWidget->hideColumn(CountPoint);
  ui_->treeWidget->showColumn(Level);
  ui_->treeWidget->showColumn(ExistData);
  ui_->treeWidget->showColumn(Center);
  ui_->treeWidget->showColumn(Model);
  ui_->treeWidget->showColumn(Hour);
  ui_->treeWidget->showColumn(NetStep);
  ui_->treeWidget->showColumn(ForecastBeg);
  ui_->treeWidget->showColumn(ForecastEnd);
}


QString Nabludenia::stepsToStr(int32_t stepLat,int32_t stepLon){

  QString str1 = "~";
  QString str2 = "~";
  if (stepLat != std::numeric_limits<int32_t>::max()){
   str1 = QString("%1").arg(stepLat/1000.,0,'f',2);
  } else {
    str1 = "~";
  }
  if (stepLon != std::numeric_limits<int32_t>::max()){
   str2 = QString("%1").arg(stepLon/1000.,0,'f',2);
  } else {
    str2 = "~";
  }

   return QString("%1x%2").arg(str1).arg(str2);
}


bool Nabludenia::eventFilter(QObject *watched, QEvent* event)
{
  if ( meteo::map::LayerEvent::LayerChanged == event->type() ) {
      LayerEvent* e = reinterpret_cast<LayerEvent*>(event);
      if ( LayerEvent::Added == e->changeType() || LayerEvent::Deleted == e->changeType() ) {
          updateItems();
        }
    }
  return MapWidget::eventFilter(watched, event);
}

void Nabludenia::closeEvent(QCloseEvent* event)
{
  /*  Document* doc = mapdocument();
  if ( nullptr != doc && nullptr != doc->eventHandler() ) {
    doc->eventHandler()->postEvent( new CustomEvent(CustomEvent::ClearGeoPoints) );
  }*/
  MapWidget::closeEvent(event);
}

int Nabludenia::currentLevel() const
{
  int lvl = -1;
  if ( 0 == currentmap_.data_size() ) {
      return 0;
    }

  if ( true == currentmap_.data(0).has_level() ) {
      lvl = currentmap_.data(0).level();
    }

  if ( -1 == lvl ) {
      return ui_->selectlevel->level();
    }

  return lvl;
}

int Nabludenia::currentTypeLevel() const
{
  int tplvl = -1;

  if ( 0 == currentmap_.data_size() ) {
      return 0;
    }

  if ( true == currentmap_.data(0).has_level() ) {
      tplvl = currentmap_.data(0).type_level();
    }

  if ( -1 == tplvl ) {
      tplvl = ui_->selectlevel->typeLevel();
    }

  return tplvl;
}

int Nabludenia::currentTerm() const
{
  if ( 0 == ui_->cmbterm->currentIndex() ) {
      return -1;
    }
  else if ( 1 == ui_->cmbterm->currentIndex() ) {
      return 0;
    }
  QString txt = ui_->cmbterm->currentText();
  if ( true == txt.isEmpty() ) {
      return 0;
    }
  return txt.toInt();
}

meteo::surf::CountDataReply* Nabludenia::sendSurfaceRequest()
{
  meteo::surf::DataRequest request;

  QDate date = ui_->dateEdit->date();
  request.set_date_start(QString("%1 %2").arg(date.toString("yyyy-MM-dd"))
                         .arg(QString("00:00:00")).toStdString());
  request.set_date_end(QString("%1 %2").arg(date.toString("yyyy-MM-dd"))
                       .arg(QString("23:59:59")).toStdString());
  /* int ctrm = currentTerm();
  if ( -1 != ctrm ) {
    request.add_hour(ctrm * 3600);
  }*/

  int level = -1;
  int type_level = -1;

  for ( const proto::WeatherLayer& l: currentmap_.data() ) {
      for ( const int dtype: l.data_type() ) {
          request.add_type(dtype);
        }
      if ( true == l.has_query_type() ) {
          request.set_query_type(l.query_type());
        }
      if ( true == l.has_type_level() ) {
          type_level = l.type_level();
        }
      if ( true == l.has_level() && 0 < l.level()) {
          level = l.level();
        }
    }
  if ( (-1 == level || -1 == type_level)
       && SelectLevel::kNoMode != ui_->selectlevel->viewMode() ) {
      level = ui_->selectlevel->level();
      type_level = ui_->selectlevel->typeLevel();
    }
  request.set_type_level(type_level);
  if ( -1 == level ||100 != type_level) {
      request.clear_level_p();
    }
  else {
      request.set_level_p(level);
    }


  for ( const puanson::proto::CellRule& rule: currentpunch_.rule() ) {
      request.add_meteo_descrname( rule.id().name() );
    }

  meteo::rpc::Channel* ch = meteo::global::serviceChannel(meteo::settings::proto::kSrcData);
  if ( nullptr == ch ) {
      error_log.msgBox() << QObject::tr("Не удалось установить соединение с сервисом данных");
      return nullptr;
    }
  QApplication::setOverrideCursor(Qt::WaitCursor);
  auto reply = ch->remoteCall( &meteo::surf::SurfaceService::GetDataCount, request, kSyncallTimeout );
  delete ch;
  QApplication::restoreOverrideCursor();
  if ( nullptr == reply ) {
      error_log.msgBox() << QObject::tr("Ответ от сервиса данных не получен.");
    }
  return reply;
}

meteo::surf::GribDataAvailableReply* Nabludenia::sendGribRequest()
{
  meteo::surf::DataRequest request;

  QDate date = ui_->dateEdit->date();
  request.set_date_start(QString("%1 %2").arg(date.toString("yyyy-MM-dd"))
                         .arg(QString("00:00:00")).toStdString());
  request.set_date_end(QString("%1 %2").arg(date.toString("yyyy-MM-dd"))
                       .arg(QString("23:59:59")).toStdString());
  int ctrm = currentTerm();
  if ( -1 != ctrm ) {
      request.set_hour(ctrm * 3600);
    }

  int level = -1;
  int type_level = -1;

  for ( const proto::WeatherLayer& l: currentmap_.data() ) {
      for ( const int dtype: l.data_type() ) {
          request.add_type(dtype);
        }
      if ( true == l.has_query_type() ) {
          request.set_query_type(l.query_type());
        }
      if ( true == l.has_type_level() ) {
          type_level = l.type_level();
        }
      if ( true == l.has_level() ) {
          level = l.level();
        }
    }

  if ( -1 == level || -1 == type_level ) {
      level = ui_->selectlevel->level();
      type_level = ui_->selectlevel->typeLevel();
    }
  request.set_type_level(type_level);
  if ( -1 == level ) {
      request.clear_level_p();
    }
  else {
      request.set_level_p(level);
    }
  for ( const puanson::proto::CellRule& rule: currentpunch_.rule() ) {
      request.add_meteo_descrname( rule.id().name() );
      request.add_meteo_descr( rule.id().descr() );
    }

  /*for ( const puanson::proto::CellRule& rule: currentpunch_.rule() ) {
    request.add_meteo_descrname( rule.id().name() );
  }*/

  meteo::rpc::Channel* ch = meteo::global::serviceChannel(meteo::settings::proto::kSrcData);
  if ( nullptr == ch ) {
      error_log.msgBox() << QObject::tr("Не удалось установить соединение с сервисом данных");
      return nullptr;
    }
  //GribDataRequest
  QApplication::setOverrideCursor(Qt::WaitCursor);
  auto reply = ch->remoteCall( &meteo::surf::SurfaceService::GetAvailableGrib, request, kSyncallTimeout );
  delete ch;
  QApplication::restoreOverrideCursor();
  if ( nullptr == reply ) {
      error_log.msgBox() << QObject::tr("Ответ от сервиса данных не получен.");
    }
  return reply;
}

meteo::field::DataDescResponse* Nabludenia::sendFieldRequest()
{
  meteo::field::DataRequest request;
  QDate date = ui_->dateEdit->date();
  request.set_date_start(QString("%1 %2").arg(date.toString("yyyy-MM-dd"))
                         .arg(QString("00:00:00")).toStdString());
  request.set_date_end(QString("%1 %2").arg(date.toString("yyyy-MM-dd"))
                       .arg(QString("23:59:59")).toStdString());

  int ctrm = currentTerm();
  if ( -1 != ctrm ) {
      request.add_hour(ctrm * 3600);
    }
  int level = -1;
  int type_level = -1;
  for ( const proto::WeatherLayer& layer: currentmap_.data() ) {
      if ( true == layer.has_type_level() ) {
          type_level = layer.type_level();
        }
      if ( true == layer.has_level() ) {
          level = layer.level();
        }
    }
  if( -1 == level || -1 == type_level ) {
      level = ui_->selectlevel->level();
      type_level = ui_->selectlevel->typeLevel();
    }

  request.add_type_level(type_level);
  if ( -1 == level ) {
      request.clear_level();
    }
  else {
      request.add_level(level);
    }
  for ( const puanson::proto::CellRule& rule: currentpunch_.rule() ) {
      request.add_meteo_descr(rule.id().descr());
    }
  meteo::rpc::Channel* ctrl = meteo::global::serviceChannel(meteo::settings::proto::kField);
  if ( nullptr == ctrl ) {
      error_log.msgBox() << QObject::tr("Не удалось установить соединение с сервисом полей");
      return nullptr;
    }
  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
  meteo::field::DataDescResponse* reply = ctrl->remoteCall(&meteo::field::FieldService::GetAvailableData, request, kSyncallTimeout);
  delete ctrl;
  qApp->restoreOverrideCursor();
  if ( nullptr == reply ) {
      error_log.msgBox() << QObject::tr("Ответ от сервиса полей не получен.");
    }
  return reply;
}

void Nabludenia::slotDateTimeChanged()
{
  loadAvailableData();
}

void Nabludenia::slotTermChanged(int)
{
  loadAvailableData();
}

void Nabludenia::slotContextMenu(QPoint point)
{
  QMenu* menu = new QMenu(this);

  QAction* add = menu->addAction(QIcon(":/meteo/icons/tools/plus.png"), QObject::tr("Добавить слой"));
  add->setIconVisibleInMenu(true);
  QObject::connect(add, &QAction::triggered, this, &Nabludenia::slotAddLayerBtn);

  QAction* del = menu->addAction(QIcon(":/meteo/icons/tools/minus.png"), QObject::tr("Удалить слой"));
  del->setIconVisibleInMenu(true);
  QObject::connect(del, &QAction::triggered, this, &Nabludenia::slotRemoveLayer);

  QTreeWidgetItem* item = ui_->treeWidget->currentItem();
  if(nullptr == item ) { return; }

  if ( true == hasLayer(item) ) {
      add->setDisabled(true);
      del->setDisabled(false);
    }
  else {
      add->setDisabled(false);
      del->setDisabled(true);
    }

  Layer* layer = layerByItem(item);
  if(nullptr != layer) {
      LayerMenu* lm = layer->layerMenu();
      lm->setLayersForRemove(QList<Layer*>() << layer);
      lm->addActions(menu);
    }
  menu->exec(ui_->treeWidget->mapToGlobal(point));

  delete menu;
}

void Nabludenia::slotDoubleClicked(QTreeWidgetItem * item, int){
  if ( nullptr == item ) {
      return;
    }
  if ( hasLayer(item)) {
      slotRemoveLayer();
      return;
    }
  slotAddLayerBtn();
}

void Nabludenia::slotAddLayerBtn()
{
  QTreeWidgetItem* item = ui_->treeWidget->currentItem();
  if ( nullptr == item ) {
      return;
    }
  qApp->setOverrideCursor(Qt::WaitCursor);

  //meteo::puanson::proto::Puanson punch;
  QByteArray barr = item->data( Date, Qt::UserRole ).toByteArray();
  proto::WeatherLayer info;
  global::arr2protomsg(barr, &info);

  Weather weather;
  if( nullptr == weather.buildPuansonLayer( mapdocument(), info ) ) {
      error_log << QObject::tr("Не удалось создать слой с наноской");
    }
  qApp->restoreOverrideCursor();
}

void Nabludenia::slotRemoveLayer()
{
  QTreeWidgetItem* item = ui_->treeWidget->currentItem();
  if(nullptr != item) {
      Layer* l = layerByItem(item);
      if(nullptr != l) {
          delete l;
        }
    }
}

bool Nabludenia::hasLayer(QTreeWidgetItem* item)
{
  bool result = false;
  if ( nullptr == item || nullptr == mapdocument() ) {
      return result;
    }

  proto::WeatherLayer info;
  QByteArray arr = item->data(Date, Qt::UserRole).toByteArray();
  global::arr2protomsg(arr, &info);

  for(const Layer* l: mapdocument()->layers()) {
      if(l->isEqual(info)) {
          result = true;
          break;
        }
    }
  return result;
}

bool Nabludenia::hasLayer( Layer* layer )
{
  return window()->document()->layers().contains(layer);
}

void Nabludenia::updateItems()
{
  if ( true == hasLayer(ui_->treeWidget->currentItem() ) ) {
      ui_->addBtn->setDisabled(true);
      ui_->delBtn->setDisabled(false);
    }
  else {
      ui_->addBtn->setDisabled(false);
      ui_->delBtn->setDisabled(true);
    }

  QTreeWidgetItem* item = nullptr;
  for ( int i = 0, sz = ui_->treeWidget->topLevelItemCount(); i < sz; ++i ) {
      item = ui_->treeWidget->topLevelItem(i);
      if ( nullptr == item) {
          continue;
        }

      if ( true == hasLayer(item) ) {
          for ( int j = 0, jsz = ui_->treeWidget->columnCount(); j < jsz; ++j ) {
              item->setBackgroundColor(j, Qt::yellow);
            }
        }
      else {
          for(int j = 0, jsz = ui_->treeWidget->columnCount(); j < jsz; ++j) {
              item->setBackgroundColor(j, Qt::white);
            }
        }
    }
}

void Nabludenia::slotItemSelectionChanged( QTreeWidgetItem* item )
{
  if ( true == hasLayer(item) ) {
      ui_->addBtn->setDisabled(true);
      ui_->delBtn->setDisabled(false);
    }
  else {
      ui_->addBtn->setDisabled(false);
      ui_->delBtn->setDisabled(true);
    }
}

void Nabludenia::slotChangeSelection()
{
  if ( false == ui_->treeWidget->selectionModel()->hasSelection() ) {
      return;
    }
  QTreeWidgetItem* current = ui_->treeWidget->selectedItems().first();
  if ( nullptr == current ) {
      return;
    }
  switch(currentSource()) {
    case proto::kField:
      sendSelectPointsEvent(current->text(Id));
      break;
    case proto::kGribSource:
    case proto::kSurface: {
        QDateTime dt = QDateTime::fromString( current->text(Date), "yyyy-MM-dd hh:mm");
        if ( true == dt.isValid() ) {
            sendSelectPointsEvent(dt);
          }
      }
      break;
    default:
      break;
    }
}

void Nabludenia::sendSelectPointsEvent(const QString& fieldId) const
{
  Document* doc = mapdocument();
  if ( nullptr == doc || nullptr == doc->eventHandler() ) {
      return;
    }
  field::SimpleDataRequest* req = new field::SimpleDataRequest();
  req->add_id(fieldId.toStdString());

  CustomEvent* ev = new CustomEvent(CustomEvent::SelectGeoPointsByField);

  ev->setOwnedMessage(req);
  doc->eventHandler()->postEvent(ev);
}

void Nabludenia::sendSelectPointsEvent(const QDateTime& srok) const
{
  Document* doc = mapdocument();
  if ( nullptr == doc || nullptr == doc->eventHandler() ) {
      return;
    }

  surf::DataRequest* req = new surf::DataRequest();
  req->set_date_start(srok.toString(Qt::ISODate).toStdString());
  for ( const proto::WeatherLayer& layer: currentmap_.data() ) {
      for ( int dtype: layer.data_type() ) {
          req->add_type(dtype);
        }
    }
  if( req->type_size() == 0 ) {
      req->add_type(currentmap_.data_type());
    }

  CustomEvent* ev = new CustomEvent(CustomEvent::SelectGeoPointsBySurface);

  ev->setOwnedMessage(req);
  doc->eventHandler()->postEvent(ev);
}

void Nabludenia::showPunch(const meteo::puanson::proto::Puanson& punch)
{
  meteo::map::Puanson puansonEx;
  puansonEx.setPunch(punch);
  puansonEx.loadTestMeteoData();
  QPixmap myPix( QSize(100, 100));
  myPix.fill(Qt::white);
  puansonEx.setScreenPos( QPoint( myPix.width()/2, myPix.height()/2 ) );
  QPainter painter(&myPix);

  QList<QRect> list = puansonEx.boundingRect(QTransform());
  if ( 0 == list.size() ) {
      return;
    }
  QRect r = list[0];
  puansonEx.render( &painter, r, QTransform() );
  ui_->lblTemplate->setPixmap(myPix);
}

Layer* Nabludenia::layerByItem( QTreeWidgetItem* item )
{
  if ( nullptr == item || nullptr == mapdocument() ) {
      return nullptr;
    }
  proto::WeatherLayer info;
  QByteArray barr = item->data(Date, Qt::UserRole).toByteArray();
  global::arr2protomsg(barr, &info);
  Layer* layer = nullptr;
  QList<Layer*> list = mapdocument()->layers();
  for( auto l : list ) {
      if ( true == l->isEqual(info) ) {
          layer = l;
          break;
        }
    }

  return layer;
}

void Nabludenia::slotPunchmapChanged( const QString& title )
{
  setCurrentMap(title);
}

void Nabludenia::slotSourceChanged( int indx )
{
  Q_UNUSED(indx);
  setCurrentSource(currentSource());
  loadAvailableData();
}

void Nabludenia::slotLevelChanged( int lvl, int type ) {
  Q_UNUSED(lvl);
  Q_UNUSED(type);
  loadAvailableData();
}

void Nabludenia::slotBtntermClicked()
{
  menutermtype_->exec(QWidget::mapToGlobal(ui_->btnterm->geometry().bottomLeft()));
}

void Nabludenia::slotTermStandard()
{
  QAction* a = qobject_cast<QAction*>(sender());
  if(nullptr == a || a->text() == ui_->lblterms->text()) { return; }

  ui_->lblterms->setText(a->text());
  termsall_ = false;

  loadAvailableData();
}

void Nabludenia::slotTermAll()
{
  QAction* a = qobject_cast<QAction*>(sender());
  if(nullptr == a || a->text() == ui_->lblterms->text()) { return; }

  ui_->lblterms->setText(a->text());
  termsall_ = true;

  loadAvailableData();
}

}
}
