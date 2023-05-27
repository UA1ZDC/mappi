#include "createcutwidget.h"
#include "verticalcutaction.h"
#include "ui_createcutwidgeet.h"

#include <cross-commons/debug/tlog.h>

#include <qobject.h>
#include <qwidget.h>

#include <meteo/commons/global/dateformat.h>
#include <meteo/commons/global/global.h>
#include <meteo/commons/proto/vcut_settings.pb.h>
#include <meteo/commons/services/sprinf/sprinfservice.h>
#include <commons/textproto/pbtools.h>
#include <commons/textproto/tprototext.h>
#include <meteo/commons/ui/custom/stationwidget.h>
#include <meteo/commons/ui/mainwindow/mainwindow.h>
#include <meteo/commons/ui/mainwindow/mdisubwindow.h>
#include <meteo/commons/ui/mainwindow/widgethandler.h>
#include <meteo/commons/ui/map/view/actions/layersaction.h>
#include <meteo/commons/ui/map/weather.h>
#include <meteo/commons/proto/weather.pb.h>
#include <meteo/commons/proto/meteo.pb.h>


namespace meteo {

static const int kTimeout = 240000;
static const QString kTitle = QObject::tr("Вертикальный разрез атмосферы");

QString vcConfFile() { return QDir::homePath() + "/." + MnCommon::applicationName() + "/vcut.conf"; }

CreateCutWidget::CreateCutWidget(meteo::map::MapWindow* parent )
  : QDialog(parent->mainwindow()),
    vdoc_(nullptr),
    resp_(nullptr),
    ui_(new Ui::Form),
    mainwindow_(parent->mainwindow()),
    window_(parent),
    scene_( nullptr),
    station_widget_(nullptr),
    act_(nullptr),
    aeroTable_(nullptr)
{
  ui_->setupUi(this);
  ui_->startDate->setDisplayFormat(meteo::dtHumanFormatDateOnly);
  ui_->startTime->setDisplayFormat(meteo::dtHumanFormatTimeOnly);
  ui_->endDate->setDisplayFormat(meteo::dtHumanFormatDateOnly);
  ui_->endTime->setDisplayFormat(meteo::dtHumanFormatTimeOnly);

  ui_->centerCombo->addItem(QObject::tr("<нет доступных центров>"), -1);

  QDateTime dt = QDateTime::currentDateTimeUtc();
  dt.setTime(QTime(dt.time().hour(), 0));

  ui_->startDate->setDate(dt.addDays(-1).date());
  ui_->startTime->setTime(dt.time());
  ui_->endDate->setDate(dt.date());
  ui_->endTime->setTime(dt.time());

  cutType_ = map::kSpaceType;
  srvType_ = kSrcData;
  ui_->centerCombo->setEnabled(false);

  sdt_ = QDateTime( ui_->startDate->date(), ui_->startTime->time() );
  edt_ = QDateTime( ui_->endDate->date(), ui_->endTime->time() );

  QObject::connect( ui_->runBtn, SIGNAL(clicked(bool)), SLOT(slotRun()) );

  QObject::connect( ui_->centerCombo, SIGNAL(currentIndexChanged(int)), SLOT(slotCenterChanged(int )) );
  QObject::connect( ui_->serviceCombo, SIGNAL(currentIndexChanged(int)), SLOT(slotServiceChanged(int )) );
  QObject::connect( ui_->vcutTypeCombo, SIGNAL(currentIndexChanged(int)), SLOT(slotTypeChanged(int )) );

  QObject::connect( ui_->startTime, SIGNAL(timeChanged( const QTime& )), SLOT(slotStartDtChanged()) );
  QObject::connect( ui_->startDate, SIGNAL(dateChanged( const QDate& )), SLOT(slotStartDtChanged()) );

  QObject::connect( ui_->endTime, SIGNAL(timeChanged( const QTime& )), SLOT(slotEndDtChanged()) );
  QObject::connect( ui_->endDate, SIGNAL(dateChanged( const QDate& )), SLOT(slotEndDtChanged()) );
  QObject::connect( ui_->placesTree, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)), SLOT( slotEditName(QTreeWidgetItem*,int)));
  QObject::connect( ui_->placesTree->model(), SIGNAL(rowsInserted(const QModelIndex &, int, int)), this, SLOT(onItemMoved()));

  addStWidget();
  ui_->addBtn->show();
  ui_->removeBtn->show();

  loadSettings();
  slotStartDtChanged();
  initMenu(window_);
}

CreateCutWidget::~CreateCutWidget()
{

  delete ui_;
  ui_ = nullptr;
  if (nullptr != resp_) {
      delete resp_;
      resp_ = nullptr;
    }

  if (nullptr != act_) {
      delete act_;
      act_ = nullptr;
    }
  station_widget_ = nullptr;
  removeSceneAct();
}

bool CreateCutWidget::addStWidget(){

  QToolBar * tb = window_->addToolBar(QObject::tr("Текущая станция"));
  station_widget_ = new StationWidget();

  station_widget_->setObjectName("StationsWdgt");

  station_widget_->setIsAltitudeVisible(false);
  tb->addWidget(station_widget_);
  QList<meteo::sprinf::MeteostationType> stationsToLoad;
  stationsToLoad << meteo::sprinf::MeteostationType::kStationAero
                 << meteo::sprinf::MeteostationType::kStationAeromob;
  if ( !station_widget_->loadStation( stationsToLoad ) ) {
      error_msg << station_widget_->lastError();
    }

  QObject::connect( ui_->addBtn, SIGNAL(clicked(bool)), SLOT(slotAddPlace())  );
  QObject::connect( ui_->removeBtn, SIGNAL(clicked(bool)), SLOT(slotRemovePlace()) );
  QObject::connect( station_widget_, &StationWidget::signalOnMap, this, &CreateCutWidget::slotMapBtnToggled);

  station_widget_->show();

  return true;
}


void CreateCutWidget::setVisible(bool visible)
{
  QDialog::setVisible(visible);
}

void CreateCutWidget::setCutDoc( map::VProfileDoc *doc )
{
  vdoc_ = doc;
}

void CreateCutWidget::setCutWindow( meteo::map::MapWindow* w )
{
  window_ = w;
}

void CreateCutWidget::slotTypeChanged(int index)
{
  cutType_ = static_cast<map::Type>(index);
  QTreeWidget* tree = ui_->placesTree;
  if(0 != tree->topLevelItemCount()) {
      QTreeWidgetItem* item = tree->topLevelItem(0)->clone();
      tree->clear();
      tree->addTopLevelItem(item);
    }
  if ( ServiceType::kFieldData == srvType_ ) {
      fillCenterList();
    }
  updateDate();
  if ( nullptr != act_) {
      if ( map::Type::kTimeType == cutType_ ) {
          act_->setWorkMode(map::TraceAction::WorkMode::DotMode);
        }
      else {
          act_->setWorkMode( map::TraceAction::WorkMode::TraceMode );
        }
    }
}

void CreateCutWidget::slotServiceChanged(int index)
{
  QApplication::setOverrideCursor(Qt::WaitCursor);
  srvType_ = ( index == 0 ) ? (kSrcData) : (kFieldData);
  bool fieldService = ( kFieldData == srvType_ );
  ui_->centerCombo->setEnabled(fieldService);
  if(true == fieldService){
     slotRequestFields();
     fillCenterList();
    }
  updateDate();

  QApplication::restoreOverrideCursor();
}

void CreateCutWidget::slotWindowClosed()
{
  window_ = nullptr;
  close();
  deleteLater();
}


void CreateCutWidget::slotActionDestroyed()
{
  scene_ = nullptr;
  act_   = nullptr;
}

void CreateCutWidget::slotCenterChanged(int index)
{
  int center = ui_->centerCombo->itemData(index,Qt::UserRole).toInt();
  if ( center > 0 ) {
      center_ = center;
    }
  updateDate();
}

void CreateCutWidget::slotStartDtChanged()
{
  QApplication::setOverrideCursor(Qt::WaitCursor);
  ui_->endDate->setMinimumDate(ui_->startDate->date());
  ui_->startDate->setMaximumDate(ui_->endDate->date());
  if ( ui_->startDate->date() == ui_->endDate->date()) {
      ui_->endTime->setMinimumTime(ui_->startTime->time());
      ui_->startTime->setMaximumTime(ui_->endTime->time());
    }
  else {
      ui_->endTime->clearMinimumTime();
      ui_->startTime->clearMaximumTime();
    }
  sdt_ = QDateTime( ui_->startDate->date(), ui_->startTime->time() );
  edt_ = QDateTime( ui_->endDate->date(), ui_->endTime->time() );
  slotRequestFields();
  fillCenterList();
  updateDate();

  QApplication::restoreOverrideCursor();
}

void CreateCutWidget::slotEndDtChanged()
{
  QApplication::setOverrideCursor(Qt::WaitCursor);
  ui_->endDate->setMinimumDate(ui_->startDate->date());
  ui_->startDate->setMaximumDate(ui_->endDate->date());
  if ( ui_->startDate->date() == ui_->endDate->date()) {
      ui_->endTime->setMinimumTime(ui_->startTime->time());
      ui_->startTime->setMaximumTime(ui_->endTime->time());
    }
  else {
      ui_->endTime->clearMinimumTime();
      ui_->startTime->clearMaximumTime();
    }
  sdt_ = QDateTime( ui_->startDate->date(), ui_->startTime->time() );
  edt_ = QDateTime( ui_->endDate->date(), ui_->endTime->time() );
  slotRequestFields();
  fillCenterList();
  updateDate();

  QApplication::restoreOverrideCursor();
}

void CreateCutWidget::slotRequestFields()
{
  if (ServiceType::kFieldData != srvType_) {
      ui_->centerCombo->blockSignals(true);
      ui_->centerCombo->clear();
      ui_->centerCombo->addItem(QObject::tr("<нет доступных центров>"), -1);
      ui_->centerCombo->blockSignals(false);
      updateDate();
      return;
    }
  QTime timer;
  timer.start();
  field::DataRequest req;
  req.set_date_start( sdt_.toString(Qt::ISODate).toUtf8().constData() );
  req.set_date_end( edt_.toString(Qt::ISODate).toUtf8().constData() );
  for ( const int descr: { 10009, 10004, 10051, 12101, 12103, 12108, 11003, 11004, 7004, 20010, 20011, 20012, 20013 } ) {
      req.add_meteo_descr(descr);
    }
  auto control = std::unique_ptr<rpc::Channel>(global::serviceChannel(settings::proto::kField));
  if ( nullptr == control ) {
      error_log.msgBox() << msglog::kNoConnect.arg(global::serviceTitle(settings::proto::kField));
      QApplication::restoreOverrideCursor();
      return;
    }
  if ( nullptr != resp_) {
      delete resp_;
      resp_ = nullptr;
    }
  resp_ = control->remoteCall(&field::FieldService::GetFieldsForDate, req, kTimeout);
  if ( nullptr == resp_ ) {
      error_log.msgBox() << "Нет ответа от сервиса";
      return;
    }
  debug_log << QObject::tr( "Запрос GetFieldsForDate выполнялся ")<<timer.elapsed()<<"мc";//<<QString::fromStdString(resp_->DebugString());
}

void CreateCutWidget::slotEditName(QTreeWidgetItem* item, int column)
{

  if ( kPlaceName == column ) {
      if ( true == item->text( kNumber ).isEmpty() ) {
          Qt::ItemFlags tmp = item->flags();
          item->setFlags(tmp | Qt::ItemIsEditable);
          ui_->placesTree->editItem(item,column);
          item->setFlags(tmp);
        }
    }
}

void CreateCutWidget::fillCenterList()
{
  ui_->centerCombo->blockSignals(true);
  QApplication::setOverrideCursor(Qt::WaitCursor);
  ui_->centerCombo->clear();
  if ( nullptr == resp_ ) {
      ui_->centerCombo->addItem(QObject::tr("<нет доступных центров>"), -1);
      QApplication::restoreOverrideCursor();
      ui_->centerCombo->blockSignals(false);
      slotCenterChanged(0);
      return;
    }
  if ( 0 == resp_->date_size() ) {
      ui_->centerCombo->addItem(QObject::tr("<нет доступных центров>"), -1);
      QApplication::restoreOverrideCursor();
      ui_->centerCombo->blockSignals(false);
      slotCenterChanged(0);
      return;
    }
  int dsz = 0;
  dsz = resp_->date_size();

  QMap< int, QPair<int, QString> > centers;
  for (int i = 0; i<dsz;++i) {
      field::ManyFieldsForDate date = resp_->date(i);
      if (0 == date.zonds_size()) {
          ui_->centerCombo->addItem(QObject::tr("<нет доступных центров>"), -1);
          QApplication::restoreOverrideCursor();
          ui_->centerCombo->blockSignals(false);
          slotCenterChanged(0);
          return;
        }
      for (int j = 0, sz = date.zonds_size(); j<sz; ++j) {
          field::FieldId id = date.zonds(j).id();
          if (false == id.has_priority()) {
              continue;
            }
          if (false == id.has_center()) {
              continue;
            }
          if (centers.contains(id.center())) {
              continue;
            }
          int level_count = date.zonds(j).levels_size();
          if ( 2 > level_count) {
              continue;
            }
          QString name;
          if (id.has_short_name()) {
              name = QString::fromStdString(id.short_name());
          }
          else if (id.has_name()) {
              name = QString::fromStdString(id.name());
          }
          QPair<int, QString> priorAndName;
          priorAndName.first = id.priority();
          priorAndName.second = name +QString(" (%1 ур.)").arg(level_count);
          centers.insert(id.center(), priorAndName);
        }

      if ( ( dsz < resp_->date_size())
           && (true == centers.isEmpty())) {
          ++dsz;
        }
    }
  if (0 == centers.size()) {
      ui_->centerCombo->addItem(QObject::tr("<нет доступных центров>"), -1);
    }
  QMap<int, QPair<int, QString>>::iterator it;
  int i = 0;
  for (it = centers.begin(); it!=centers.end();++it) {
      ui_->centerCombo->addItem(it.value().second,i);
      ui_->centerCombo->setItemData(i, it.key(), Qt::UserRole);
      ++i;
    }
  slotCenterChanged(0);
  ui_->centerCombo->blockSignals(false);
  QApplication::restoreOverrideCursor();
}


void CreateCutWidget::loadSettings()
{
  ui_->placesTree->clear();
  proto::VCutSettings opt;
  TProtoText::fromFile(vcConfFile(), &opt);
  for ( int i=0,isz=opt.place_size(); i<isz; ++i ) {
      const surf::Point& p = opt.place(i);
      zond::PlaceData data;
      data.setCoord(GeoPoint(p.fi(), p.la(),p.height()));
      if ( p.has_index() ) {
          data.setIndex( QString::fromStdString(p.index()) );
        }
      if ( p.has_name() ) {
          data.setRuName( pbtools::toQString(p.name()) );
        }
      addPlace(data);
    }
  ui_->serviceCombo->setCurrentIndex(opt.src());
  ui_->vcutTypeCombo->setCurrentIndex(opt.type());
}

void CreateCutWidget::saveSettings() const
{
  proto::VCutSettings opt;
  QTreeWidget* tree = ui_->placesTree;
  for ( int i=0; i<tree->topLevelItemCount(); ++i ) {
      QTreeWidgetItem* item = tree->topLevelItem(i);
      double fi = item->data(kFi,Qt::UserRole).toDouble();
      double la = item->data(kLa,Qt::UserRole).toDouble();
      double alt = item->data(kAlt,Qt::UserRole).toDouble();
      surf::Point* p = opt.add_place();
      p->set_fi(fi);
      p->set_la(la);
      p->set_height(alt);
      p->set_name(item->text(kPlaceName).toStdString());
      p->set_index(item->text(kNumber).toStdString());
    }
  opt.set_src(ui_->serviceCombo->currentIndex());
  opt.set_type(ui_->vcutTypeCombo->currentIndex());
  TProtoText::toFile(opt, vcConfFile());
}

void CreateCutWidget::slotTraceItemAdded(int /*num*/, const GeoPoint& coord)
{
  zond::PlaceData data;
  data.setCoord(coord);
  addPlace(data);
}

void CreateCutWidget::slotTraceItemChanged(int num, const GeoPoint& coord)
{
  QTreeWidgetItem* item = ui_->placesTree->topLevelItem(num);
  if ( nullptr == item ) {
      return;
    }

  item->setText( kPlaceCoord, coord.toString(false,"%1,%2") );
  item->setData( kLa, Qt::UserRole, coord.la() );
  item->setData( kFi, Qt::UserRole, coord.fi() );
  item->setData( kAlt, Qt::UserRole, coord.alt() );
}

void CreateCutWidget::slotTraceItemRemoved(int num)
{
  QTreeWidget* tree = ui_->placesTree;
  if ( num > tree->topLevelItemCount() )
    {
      return;
    }

  QTreeWidgetItem* item = tree->topLevelItem(num);
  if ( nullptr == item )
    {
      return;
    }
  delete item;
}

void CreateCutWidget::slotClearAll()
{
  ui_->placesTree->clear();
}

void CreateCutWidget::addSceneAct(map::MapView* view)
{
  if ( nullptr == view ) {
    return;
    }
  if ( nullptr == view->mapscene() ) {
    return;
  }

  scene_ = view->mapscene();
  if ( nullptr == act_ ){
    act_ = new map::VerticalCutAction(scene_);
  }

  QObject::connect( act_, SIGNAL( destroyed(QObject*) ), SLOT( slotActionDestroyed()) );
  scene_->addAction(act_);
  if ( map::Type::kTimeType == cutType_ ) {
      act_->setWorkMode(map::TraceAction::WorkMode::DotMode);
    }
  else {
      act_->setWorkMode( map::TraceAction::WorkMode::TraceMode );
    }
  QObject::connect( act_, SIGNAL(itemAdded(int,const GeoPoint&)), SLOT(slotTraceItemAdded(int,const GeoPoint&)) );
  QObject::connect( act_, SIGNAL(changeItem(int,const GeoPoint&)), SLOT(slotTraceItemChanged(int, const GeoPoint&)) );
  QObject::connect( act_, SIGNAL(itemRemoved(int)), SLOT(slotTraceItemRemoved(int)) );
  QObject::connect( act_, SIGNAL(removeTrace()), SLOT(slotClearAll()) );

  QObject::connect( this, SIGNAL(placeRemoved(int)), act_, SLOT(slotItemRemove(int)));
  QObject::connect( this, SIGNAL(placeChanged(int, GeoPoint, QString)), act_, SLOT(slotPlaceChanged(int, GeoPoint, QString)) );

  updatePath();
}

void CreateCutWidget::removeSceneAct()
{
  if ( nullptr != act_   ) {
      disconnect(act_);
      act_->disconnect(this);
      act_->slotRequestCoord(false);
    }
  if ( nullptr != scene_ ) {
      map::Action* a = scene_->takeAction(map::VerticalCutAction::kName);
      if ( nullptr != a ) {
          delete a;
        }
      scene_ = nullptr;
      act_   = nullptr;
    }
  if ( nullptr != station_widget_ ) {
   debug_log<<"station_widget_->disconnect(this)"<< QObject::disconnect( station_widget_, SIGNAL(changed()), this,SLOT(slotAddPlace()) );


    }
}

void CreateCutWidget::updatePath()
{
  if ( nullptr == act_ ) {
      return;
    }
  act_->blockSignals(true);

  act_->slotRemovePath();

  QTreeWidget* tree = ui_->placesTree;
  for ( int i=0,isz=tree->topLevelItemCount(); i<isz; ++i ) {
      QTreeWidgetItem* item = tree->topLevelItem(i);
      double fi = item->data(kFi,Qt::UserRole).toDouble();
      double la = item->data(kLa,Qt::UserRole).toDouble();
      double alt = item->data(kAlt,Qt::UserRole).toDouble();
      map::TraceItem* node = act_->addNode(meteo::GeoPoint(fi,la,alt));
      node->updateText(item->text(kPlaceName));
      node->setShowDistance(false);
      if ( false == item->text(kNumber).isEmpty() ) {
          node->setFlag(QGraphicsItem::ItemIsMovable, false);
        }
    }

  act_->blockSignals(false);
}


bool foSortPoTime(const zond::PlaceData& p1, const zond::PlaceData& p2)
{
  return p1.dt() < p2.dt();
}

void CreateCutWidget::slotRun()
{
  if ( nullptr == vdoc_ ) {
      return;
    }
  vdoc_->clear();
  QApplication::setOverrideCursor(Qt::WaitCursor);
  setEnabled(false);
  profilesData_.clear();
  if ( !getProfiles( &profilesData_ ) ) {
      vdoc_->clear();
      QApplication::restoreOverrideCursor();
      warning_log.msgBox() << QObject::tr( "Не выбраны станции для посторения вертикального разреза" );
      setEnabled(true);
      return;
    }
  if ( 1 >= profilesData_.size() ) {
      QApplication::restoreOverrideCursor();
      if ( cutType_ == map::Type::kSpaceType ) {
          warning_log.msgBox() << QObject::tr( "Недостаточно данных для построения вертикального разреза. Добавьте больше станций" );
        }
      else if ( cutType_ == map::Type::kTimeType ) {
          warning_log.msgBox() << QObject::tr( "Недостаточно данных для построения вертикального разреза. Добавьте больше сроков" );
        }
      setEnabled(true);
      return;
    }
  meteo::map::proto::Map map;
  map.mutable_document()->mutable_doc_center()->set_lat_deg(25000);
  map.mutable_document()->mutable_doc_center()->set_lon_deg(35000);
  switch (cutType_){
    case map::Type::kSpaceType:
      map.set_profile( meteo::map::proto::ProfileType::kSpace );
      map.set_hour(0);
      break;
    case map::Type::kTimeType:
      map.set_profile(meteo::map::proto::ProfileType::kTimeList);
      map.set_hour(0);
      //map.set_hour(( edt_.toMSecsSinceEpoch() - sdt_.toMSecsSinceEpoch())/1000/3600 );
      break;
    case map::Type::kSpaceTimeType:
      break;
    }
  //  map.set_hour(0);
  switch (srvType_) {
    case kSrcData:
      map.set_source( meteo::map::proto::DataSource::kSurface );
      break;
    case kFieldData:
      map.set_source( meteo::map::proto::DataSource::kField );
      map.set_center( center_ );
      break;
    }

  map.clear_point();
  QDateTime dt = edt_;
  for (int i = 0, sz=profilesData_.size(); i<sz; ++i) {
      meteo::surf::Point* p = map.add_point();
      p->set_name( profilesData_.at(i).name().toStdString() );
      p->set_index( profilesData_.at(i).index().toStdString() );
      p->set_fi( profilesData_.at(i).coord().fi() );
      p->set_la( profilesData_.at(i).coord().la() );
      p->set_height(profilesData_.at(i).coord().alt() );
      p->set_model( profilesData_.at(i).model());
      p->set_date_time( profilesData_.at(i).dt().toString(Qt::ISODate).toStdString() );
      if(dt!=profilesData_.at(i).dt()){
          dt = profilesData_.at(i).dt();
        }
    }
  map.set_datetime( dt.toString(Qt::ISODate).toUtf8().constData() );
  meteo::map::Weather w;
  if(false == w.buildVProfile( vdoc_, map )){
      warning_log.msgBox() << QObject::tr( "Недостаточно данных для построения вертикального разреза." );
      setEnabled(true);
      QApplication::restoreOverrideCursor();
      return;
    }

  if(nullptr != window_ && nullptr != window_->document()){
      window_->document()->setName(makeTitle(profilesData_));
      QList<map::Incut*> list = window_->document()->incuts();
      if( 0 != list.size() ) {
          setIncut(list[0],map);
        }
    }

  setEnabled(true);
  QApplication::restoreOverrideCursor();
  saveSettings();
  window_->loadBaseLayersVisibilitySettings();
}

QString CreateCutWidget::makeTitle(const QList<zond::PlaceData>& profiles) const
{
  if ( profiles.size() == 0 ) {
      return kTitle;
    }

  QString title = kTitle + " ";
  switch (cutType_){
    case map::Type::kSpaceType:
      title += QObject::tr("(пространственный) за %1 (UTC)")
          .arg(profiles.first().dt().toString("dd MMM yyyy hh:mm"));
      break;
    case map::Type::kTimeType:
      title += QObject::tr("%1 с %2 по %3 (UTC)")
          .arg(profiles.first().name())
          .arg(profiles.first().dt().toString("dd MMM yyyy hh:mm"))
          .arg(profiles.last().dt().toString("dd MMM yyyy hh:mm"));
      break;
    case map::Type::kSpaceTimeType:
      title += QObject::tr("(пространственно-временной) с %2 по %3 (UTC)")
          .arg(profiles.first().dt().toString("dd MMM yyyy hh:mm"))
          .arg(profiles.last().dt().toString("dd MMM yyyy hh:mm"));
      break;
    }

  return title;
}


bool CreateCutWidget::setIncut(meteo::map::Incut *incut, const map::proto::Map &map)
{
  if(!incut) return false;
  incut->setAllLabelVisible(false);

  incut->setAuthor( true );
  switch (srvType_) {
    case kSrcData:
      incut->setLabel("center", tr("По данным зондирования") );
      break;
    case kFieldData:
      incut->setLabel("center", tr("Центр: %1").arg(ui_->centerCombo->currentText()));
      break;
    }
  if(1 > map.point_size()) {
      return false;
    }

  switch (cutType_){
    case map::Type::kSpaceType:
      incut->setMapName(kTitle+tr(" (пространственный)"));
      for(int i = 0; map.point_size() > i ; ++i){
          const meteo::surf::Point& p = map.point(i);
          GeoPoint gp = GeoPoint(p.fi(),p.la());
          QString stname = tr("%1 (%2) ")
              .arg(QString::fromStdString(p.name()))
              .arg(gp.toString(false, "%1%2"));
          incut->setLabel(QString("station%1").arg(i),stname);
        }
      incut->setDate( true );
      break;
    case map::Type::kTimeType:
      {
        incut->setMapName(kTitle+tr(" (временной)"));
        const meteo::surf::Point& p = map.point(0);
        GeoPoint gp = GeoPoint(p.fi(),p.la());
        QString stname = tr("%1 (%2) ")
            .arg(QString::fromStdString(p.name()))
            .arg(gp.toString(false, "%1%2"));
        incut->setLabel(QString("station"),stname);
        for(int i = 0; map.point_size() > i ; ++i){
            incut->setLabel(QString("tdate%1").arg(i),QString::fromStdString(map.point(i).date_time()));
          }
      }
      break;
    case map::Type::kSpaceTimeType:
      incut->setMapName(kTitle+tr(" (пространственно-временной)"));
      for(int i = 0; map.point_size() > i ; ++i){
          const meteo::surf::Point& p = map.point(i);
          GeoPoint gp = GeoPoint(p.fi(),p.la());
          QString stname = tr("%1 (%2) %3")
              .arg(QString::fromStdString(p.name()))
              .arg(gp.toString(false, "%1%2"))
              .arg(QString::fromStdString(p.date_time()));
          incut->setLabel(QString("station%1").arg(i),stname);
        }
      break;
    }

  return true;
}


int CreateCutWidget::addPlace(const zond::PlaceData& data)
{
  QTreeWidget* tree =  ui_->placesTree;
  if ( map::Type::kTimeType == cutType_ ) {
      ui_->placesTree->clear();
    }
  QTreeWidgetItem* item = new QTreeWidgetItem(tree);
  if ( nullptr == item ) {
      return -1;
    }
  item->setFlags(item->flags() & ~Qt::ItemFlag::ItemIsDropEnabled);
  int num = tree->indexOfTopLevelItem(item);
  zond::PlaceData pd = data;
  pd.setName(makePlaceName(data, num));
  fillItem(item,pd);
  return num;
}

void CreateCutWidget::fillItem(QTreeWidgetItem* item, const zond::PlaceData& data)
{
  QTreeWidget* tree =  ui_->placesTree;
  if ( ( nullptr == tree )
       || ( nullptr == item ) ){
      return;
    }

  item->setText(kPlaceName, data.name());
  item->setText(kNumber, data.index());
  item->setText(kPlaceCoord, data.coord().toString(false,"%1,%2"));
  item->setData(kFi, Qt::UserRole, data.coord().fi());
  item->setData(kLa, Qt::UserRole, data.coord().la());
  item->setData(kAlt, Qt::UserRole, data.coord().alt());
  tree->resizeColumnToContents(kPlaceName);
  tree->resizeColumnToContents(kNumber);
  tree->resizeColumnToContents(kPlaceCoord);
  item->setText(kDate, QObject::tr("Нет данных"));

  emit placeChanged(tree->indexOfTopLevelItem(item), data.coord(), data.name());
}

surf::DateReply* CreateCutWidget::getZondDates(QString index)
{
  auto ctrl = std::unique_ptr<meteo::rpc::Channel>(meteo::global::serviceChannel( meteo::settings::proto::kSrcData ));
  if ( (nullptr != ctrl) && (false == index.isEmpty()) ) {
      surf::DataRequest req;
      req.add_station(index.toStdString());
      req.set_date_start(sdt_.toString(Qt::ISODate).toStdString());
      req.set_date_end(edt_.toString(Qt::ISODate).toStdString());
      req.add_type(surf::kAeroFix);
      surf::DateReply* reply = ctrl->remoteCall(&surf::SurfaceService::GetZondDates, req, 120000);
      return reply;
    }
  return nullptr;
}
/*
field::DateReply* CreateCutWidget::getFieldDates(QTreeWidgetItem *item)
{
  field::DataRequest request;
  request.add_center(center_);
  surf::Point* p = request.add_coords();
  p->set_fi(item->data( kFi,Qt::UserRole).toDouble() );
  p->set_la(item->data( kLa,Qt::UserRole).toDouble() );
  p->set_height(item->data( kAlt,Qt::UserRole).toDouble() );
  p->set_index(item->text(kNumber).toStdString());
  request.set_only_last(true);
  request.set_need_field_descr(true);
  request.set_date_start( sdt_.toString(Qt::ISODate).toStdString() );
  request.set_date_end( edt_.toString(Qt::ISODate).toStdString() );
  rpc::Channel* ctrl_field = global::serviceChannel(settings::proto::kField);
  if(nullptr == ctrl_field ) {
      delete ctrl_field;
      return nullptr;
    }
  field::DateReply* reply = ctrl_field->remoteCall(&field::FieldService::GetFieldDates, request, 240000);
  delete ctrl_field;
  return reply;
}
*/
int CreateCutWidget::setPlaceData(QTreeWidgetItem* item, const zond::PlaceData& data)
{
  int num = 0;
  if ( nullptr == item ) {
      num = addPlace(data);
    }
  else {
      QTreeWidget* tree = ui_->placesTree;
      num = tree->indexOfTopLevelItem(item);
      zond::PlaceData pd = data;
      pd.setName(makePlaceName(data, num));
      fillItem(item,pd);
    }
  return num;
}

bool CreateCutWidget::getProfiles(QList<zond::PlaceData>* aprofile)
{
  aprofile->clear();
  QTreeWidget* tree = ui_->placesTree;
  if(nullptr == ui_ ) return false;
  for ( int i=0,isz=tree->topLevelItemCount(); i<isz; ++i ) {
      QTreeWidgetItem* item = tree->topLevelItem(i);
      if(nullptr != item)  {
          zond::PlaceData pd;
          double fi = item->data(kFi,Qt::UserRole).toDouble();
          double la = item->data(kLa,Qt::UserRole).toDouble();
          double alt = item->data(kAlt,Qt::UserRole).toDouble();
          int model = item->data(kDate, Qt::UserRole).toInt();
          pd.setCoord(meteo::GeoPoint(fi,la,alt));
          pd.setName(item->text(0));
          pd.setIndex(item->text(1));
          pd.setModel(model);
          QDateTime dt = QDateTime::fromString(item->text(3), "yyyy-MM-ddThh:ss:mm");
          if(dt.isValid()){
              pd.setDt(dt);
            }
          aprofile->append(pd);
        }
    }
  return (0 < aprofile->size() );
}

QString CreateCutWidget::makePlaceName(const zond::PlaceData& data, int num) const
{
  if ( !data.ruName().isEmpty() ) {
      return data.ruName();
    }
  else if ( !data.enName().isEmpty() ) {
      return data.enName();
    }
  return QObject::tr("Точка %1").arg(num);
}

void CreateCutWidget::slotAddPlace()
{
  if ( nullptr == station_widget_ ) {
    return;
  }
  zond::PlaceData data;
  data.setCoord( station_widget_->coord() );
  data.setRuName( station_widget_->ruName() );
  data.setEnName( station_widget_->enName() );
  data.setIndex( station_widget_->stationIndex() );
  for ( int i = 0, sz = ui_->placesTree->topLevelItemCount(); i < sz; ++i ) {
      if ( data.coord().toString(false,"%1,%2") == ui_->placesTree->topLevelItem(i)->text(2) ) {
          return;
        }
    }
  int num = addPlace(data);
  data.setName(makePlaceName(data, num));
  if ( nullptr != act_ ) {
    act_->blockSignals(true);
    if ( map::Type::kTimeType == cutType_) {
      act_->clearTrace();
    }
    bool move = true;
    if ( false  == station_widget_->stationIndex().isEmpty() ) {
      move = false;
    }
    act_->appendNode(data.coord(), data.name(), move );
    act_->blockSignals(false);
  }
  if ( nullptr != station_widget_ && nullptr != station_widget_->getAssociatedAction() ) {
    station_widget_->getAssociatedAction()->slotRemoveTrace();
  }
  updateDate();
  saveSettings();
}

void CreateCutWidget::slotRemovePlace()
{
  QTreeWidget* tree = ui_->placesTree;
  QTreeWidgetItem* item = tree->currentItem();
  int num = tree->indexOfTopLevelItem(item);
  if ( nullptr == item ) {
      return;
    }
  delete item;
  emit placeRemoved(num);
  saveSettings();
}

void CreateCutWidget::initMenu(meteo::map::MapWindow* w)
{
  Q_UNUSED(w);
  if ( nullptr != w ) {
          auto path = QList<QPair<QString, QString>>
              ({QPair<QString, QString>("table", QObject::tr("Задачи"))});
          auto title = QPair<QString, QString>("createaerotablevertical", QObject::tr("Данные зондирования"));
          QAction* act = w->addActionToMenu(title, path);

          QObject::connect( act, SIGNAL( triggered() ), this, SLOT( slotOpenAeroTable() ) );

          aeroTable_ = new meteo::aero::TabAeroTableWidget(w);
          QObject::connect(this, SIGNAL( currentDataChanged() ), this, SLOT( slotFillAeroTable() ) );
    }
}

//! Таблица с данными зондирования
void CreateCutWidget::slotOpenAeroTable()
{
  if( ( nullptr == aeroTable_ )
      && ( nullptr != scene_ ) ) {
      meteo::map::MapWindow* mainwidget = scene_->mapview()->window();
      if ( nullptr == mainwidget ) {
          return;
        }
      aeroTable_ = new meteo::aero::TabAeroTableWidget(mainwidget);
      QObject::connect(this, SIGNAL(currentDataChanged()), this, SLOT(slotFillAeroTable()));
    }
  slotFillAeroTable();
  if( nullptr!= aeroTable_ ){
      aeroTable_->show();
    }
}

void CreateCutWidget::slotFillAeroTable(){
  QList<zond::PlaceData> profilesData;
  if(getProfiles(&profilesData)){
      if(0 < profilesData.size() && nullptr != aeroTable_ ){
          aeroTable_->addAeroTable(profilesData);
        } else {
          error_log.msgBox()<< QObject::tr("Данные не получены.\nПопробуйте изменить параметры запроса и нажать кнопку Загрузить");
        }
    }
}
/*
void CreateCutWidget::updateFieldDate(){
  QTreeWidget* tree = ui_->placesTree;

  switch(cutType_){
    case map::kSpaceTimeType:
      break;
    case map::kSpaceType:
      for ( int i=0,isz=tree->topLevelItemCount(); i<isz; ++i ) {
          QTreeWidgetItem* item = tree->topLevelItem(i);
          field::DateReply* reply = getFieldDates(item);
          if (nullptr == reply || 0 == reply->date_size()) {
              item->setText(kDate, QString("Нет данных"));
              continue;
            }
          for(int j = 0, dsz = reply->date_size(); j<dsz; ++j){
                      item->setText(kDate, QString("Нет данных"));
                      QString date = QString::fromStdString(resp_->date(j).forecast_start());
                      item->setText(kDate, date);
                      //item->setData(kDate, Qt::UserRole, date);
                }
            }
      break;
    case map::kTimeType:{
        if (0 == tree->topLevelItemCount()) break;
        QTreeWidgetItem* item = tree->topLevelItem(0)->clone();
        field::DateReply* reply = getFieldDates(item);
        tree->clear();
        if(nullptr != reply || 0 == reply->date_size()) {
            item->setText(kDate, QString("Нет данных"));
            tree->addTopLevelItem(item);
            break;
          }
        for (int i=0,sz=reply->date_size(); i<sz; ++i) {
            QTreeWidgetItem* newitem = item->clone();
            //field::DateReply* reply = getFieldDates(item);
                    newitem->setText(kDate, QString::fromStdString(reply->date(i)));
                    //newitem->setData(kDate, Qt::UserRole, QString::fromStdString(reply->date(i)));
                    debug_log<<reply->date(i);
                    tree->addTopLevelItem(newitem);
                  }
              }
      break;
    }


}
*/
int  CreateCutWidget::currentCenter(){
  if(nullptr == ui_|| nullptr == ui_->centerCombo ) return -1;
  return ui_->centerCombo->currentData(Qt::UserRole).toInt();
}


void CreateCutWidget::updateFieldDate(){
  QTreeWidget* tree = ui_->placesTree;
  switch(cutType_){
    case map::kSpaceTimeType:
      break;
    case map::kSpaceType:
      for ( int i=0,isz=tree->topLevelItemCount(); i<isz; ++i ) {
          QTreeWidgetItem* item = tree->topLevelItem(i);
          if (nullptr == resp_ || 0 == resp_->date_size()) {
              item->setText(kDate, QString("Нет данных"));
              continue;
            }
          bool has_data = false;
          for(int j = 0, dsz = resp_->date_size(); j<dsz; ++j){
              item->setText(kDate, QString("Нет данных"));
              for (int k = 0, zsz = resp_->date(j).zonds_size(); k<zsz;++k) {
                  if ( 2 > resp_->date(j).zonds(k).levels_size() ) { continue;}
                  if (center_ == resp_->date(j).zonds(k).id().center()) {
                      QString date = QString::fromStdString(resp_->date(j).forecast_start());
                      item->setText(kDate, date);
                      item->setData(kDate, Qt::UserRole, resp_->date(j).zonds(k).id().model());
                      has_data = true;
                      break;
                    }
                }
              if(true == has_data) break;
            }
        }
      break;
    case map::kTimeType:{
        if (0 == tree->topLevelItemCount()) break;
        QTreeWidgetItem* item = tree->topLevelItem(0)->clone();
        tree->clear();
        if(nullptr == resp_ || 0 == resp_->date_size()) {
            item->setText(kDate, QString("Нет данных"));
            tree->addTopLevelItem(item);
            break;
          }
        for (int i=0,sz=resp_->date_size(); i<sz; ++i) {
            QTreeWidgetItem* newitem = item->clone();
            for (int j =0, szz = resp_->date(i).zonds_size(); j<szz; ++j) {
                if (2 > resp_->date(i).zonds(j).levels_size()) { continue; }
                if (center_ == resp_->date(i).zonds(j).id().center()) {
                    newitem->setText(kDate, QString::fromStdString(resp_->date(i).forecast_start()));
                    newitem->setData(kDate, Qt::UserRole, resp_->date(i).zonds(j).id().model());
                    tree->addTopLevelItem(newitem);
                  }
              }
          }
        if (0 == tree->topLevelItemCount() && nullptr != item ) {
            item->setText(kDate, QString("Нет данных"));
            tree->addTopLevelItem(item);

          }
      }
      break;
    }
}

void CreateCutWidget::updateSrfDate(){

  QTreeWidget* tree = ui_->placesTree;

  switch(cutType_){
    case map::kSpaceType:
      for ( int i=0,isz=tree->topLevelItemCount(); i<isz; ++i ) {
          QTreeWidgetItem* item = tree->topLevelItem(i);
          surf::DateReply* reply = getZondDates(item->text(kNumber));
          if (nullptr == reply || 0 == reply->date_size()) {
              item->setText(kDate, QString("Нет данных"));
              continue;
            }
          item->setText(kDate, QString::fromStdString(reply->date(0)));
          delete reply;
        }
      break;
    case map::kTimeType:{
        if (0 == tree->topLevelItemCount()) { break; }
        QTreeWidgetItem* item = tree->topLevelItem(0)->clone();
        tree->clear();
        surf::DateReply* reply = getZondDates(item->text(kNumber));
        if(nullptr == reply || 0 == reply->date_size()) {
            item->setText(kDate, QString("Нет данных"));
            tree->addTopLevelItem(item);
            delete reply;
            break;
          }
        for (int i=0,sz=reply->date_size();i<sz;++i) {
            QTreeWidgetItem* newitem = item->clone();
            newitem->setText(kDate, QString::fromStdString(reply->date(i)));
            tree->addTopLevelItem(newitem);
          }
        delete reply;
      }
      break;
    case map::kSpaceTimeType:
      break;
    }

}


void CreateCutWidget::updateDate()
{
  switch (srvType_) {
    case kSrcData:
      updateSrfDate();
      break;
    case kFieldData:
      updateFieldDate();
      break;
    }
}

void CreateCutWidget::slotMapBtnToggled(bool enable)
{
  if (true == enable ) {
    auto scene = this->station_widget_->getAssociatedScene();
    addSceneAct(scene->mapview());
  }
}

void CreateCutWidget::onItemMoved()
{
  QTreeWidget* tree = ui_->placesTree;
  for ( int i = 0; i < tree->topLevelItemCount(); ++i ){
    auto item = tree->topLevelItem(i);

    auto name = item->text(kPlaceName);
    GeoPoint coord;
    coord.setFi(item->data(kFi, Qt::UserRole).toFloat());
    coord.setLa(item->data(kLa, Qt::UserRole).toFloat());
    coord.setAlt(item->data(kAlt, Qt::UserRole).toFloat());

    emit placeChanged(i, coord, name);
  }
}


} // meteo
