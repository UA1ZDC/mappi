#include "createoceandiagwidget.h"

#include "ui_createoceandiagwidget.h"

#include <meteo/commons/global/dateformat.h>
#include <cross-commons/debug/tlog.h>
#include <meteo/commons/ui/mainwindow/widgethandler.h>
#include <meteo/commons/ui/custom/stationwidget.h>
#include <meteo/commons/global/global.h>
#include <meteo/commons/ui/custom/filterheader.h>
#include <meteo/commons/ui/map/view/actions/traceaction.h>
#include <meteo/commons/settings/settings.h>

#define start_time QElapsedTimer _timer__name; _timer__name.start()
#define reset_time(text) debug_log << text << _timer__name.restart() << "msec"

namespace meteo {

namespace odiag {

static const QString kTitle = QObject::tr("Диаграмма состояния моря");
static const QString kName = QObject::tr("ocean_diag_action");
static const QString kNameStationWgt = QObject::tr("ocean");

CreateOceanDiagWidget::CreateOceanDiagWidget( meteo::map::MapWindow* window )
  : QDialog(window->mainwindow()),
    window_(window),
    odiagdoc_(nullptr),
    odiagdata_(nullptr),
    ui_(new Ui::CreateOceanDiagForm),
    station_widget_(nullptr),
    current_item_(nullptr),
    //data_(nullptr),
    watchedEventHandler_(nullptr)
{
  ui_->setupUi(this);
  ui_->departureDate->setDisplayFormat(meteo::dtHumanFormatDateOnly);
  ui_->departureTime->setDisplayFormat(meteo::dtHumanFormatTimeOnly);

  FilterHeader* fheader = new FilterHeader(ui_->spaceSrcPlaceTree);
  ui_->spaceSrcPlaceTree->setHeader(fheader);
  fheader->setSectionsClickable(true);
  ui_->spaceSrcPlaceTree->hideColumn(kId);
  ui_->spaceSrcPlaceTree->setSelectionMode(QAbstractItemView::SingleSelection);
  for (int idx = 0; idx < ui_->spaceSrcPlaceTree->columnCount(); idx++) {
    ui_->spaceSrcPlaceTree->resizeColumnToContents(idx);
  }

  gSettings(meteo::global::Settings::instance());
  meteo::global::Settings::instance()->load();
  GeoPoint gp;
  auto loc = meteo::global::Settings::instance()->location();
  ui_->point->setStation(QString::fromStdString(loc.index()),
			 pbgeopoint2geopoint(loc.coord()),
			 QString::fromStdString(loc.name()));
  
  ui_->spaceSrcPlaceTree->sortByColumn(kDistance, Qt::AscendingOrder);
  QDateTime dt = QDateTime::currentDateTime();
  dt.setTime(QTime(11, 0));

  // space type
  ui_->departureDate->setDate(dt.date());
  ui_->departureTime->setTime(dt.time());

  odiagdata_ = new meteo::odiag::OceanDiagData();

  type_ = kSensType;
  stype_ = kSrcData;

  ui_->angleSpin->setSuffix("\xC2\xB0");
  ui_->angleSpin->setMinimum(0.);
  ui_->angleSpin->setSingleStep(0.01);
  ui_->angleSpin->setEnabled(false);

  odiagdata_->setStartTime(dt);
  odiagdata_->setEndTime(dt.addDays(1));
  odiagdata_->setType(type_);
  odiagdata_->setServiceType(stype_);

  QObject::connect(ui_->spaceSrcPlaceTree, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)), SLOT(slotRun()));

  QObject::connect( ui_->departureTime, SIGNAL(timeChanged (const QTime &  )), SLOT(slotSdtCh()) );
  QObject::connect( ui_->departureDate, SIGNAL(dateChanged (const QDate &  )), SLOT(slotSdtCh()) );
  QObject::connect(ui_->point, SIGNAL(changed()), SLOT(slotCoordChanged()));

  addStWidget();

  createDocument();
  window_->loadBaseLayersVisibilitySettings();
  slotSdtCh();
}

CreateOceanDiagWidget::~CreateOceanDiagWidget()
{
  if (nullptr != odiagdoc_) {
      delete odiagdoc_;
      odiagdoc_ = nullptr;
    }

  if (nullptr != odiagdata_) {
      delete odiagdata_;
      odiagdata_ = nullptr;
    }

  if (nullptr != ui_) {
      delete ui_;
      ui_ = nullptr;
    }

  if (0 != oceanTable_) {
    delete oceanTable_;
    oceanTable_ = 0;
  }
  
  if (watchedEventHandler_ != nullptr) {
      this->removeEventFilter(watchedEventHandler_);
      watchedEventHandler_ = nullptr;
    }
}
bool CreateOceanDiagWidget::addStWidget(){
  QList<meteo::sprinf::MeteostationType> stations_to_load;
  stations_to_load << meteo::sprinf::MeteostationType::kStationOcean;
  
  QToolBar* tb = new QToolBar(this->window_);
  this->window_->addToolBar(tb);

  this->station_widget_ = new StationWidget(tb);
  tb->addWidget(this->station_widget_);
  this->station_widget_->loadStation(stations_to_load);
  //  station_widget_->hide(); //TODO сделать загрузку подвижных станций в базу!!!

  ui_->point->loadStation(stations_to_load);
  return true;
}


void CreateOceanDiagWidget::slotCoordChanged()
{
  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
  setEnabled(false);
  for ( int i=0,isz = ui_->spaceSrcPlaceTree->topLevelItemCount(); i<isz; ++i ) {
      const ocean::PlaceData& data = data_.value(ui_->spaceSrcPlaceTree->topLevelItem(i)->text(kId));
      ui_->spaceSrcPlaceTree->topLevelItem(i)->setText(kDistance,QString::number(data.coord().calcDistance(ui_->point->coord()),'f',0));
    }
  sortSrcDataTree();
  setEnabled(true);
  // QString pt = tr("Точка");
  // if(nullptr != station_widget_){
  //   pt = tr("%1(%2)")
  //              .arg(station_widget_->stationIndex())
  //              .arg(station_widget_->coord().toString(false, "%1%2"));
  //   }
  // emit stationChanged(station_widget_->coord(),pt,false);
  QApplication::restoreOverrideCursor();
}

void CreateOceanDiagWidget::sortSrcDataTree()
{
  ui_->spaceSrcPlaceTree->sortByColumn(ui_->spaceSrcPlaceTree->sortColumn(),
                                       ui_->spaceSrcPlaceTree->header()->sortIndicatorOrder());
  //  ui_->spaceSrcPlaceTree->sortItems(kDistance,Qt::AscendingOrder);
  for (int idx=0,isz = ui_->spaceSrcPlaceTree->topLevelItemCount(); idx<isz; ++idx ) {
      QTreeWidgetItem* item = ui_->spaceSrcPlaceTree->topLevelItem(idx);
      if (item->text(kPlaceName).contains(tr("Точка"))) {
          item->setText(kPlaceName, tr("Точка %1").arg(idx + 1));
        }
    }
}


void CreateOceanDiagWidget::setAngleSpin(float angle)
{
  disconnect(ui_->angleSpin,SIGNAL(valueChanged(double)),nullptr,nullptr);
  ui_->angleSpin->setEnabled(true);
  ui_->angleSpin->setMaximum(angle/M_PI*180.);
  ui_->angleSpin->setValue(angle/M_PI*180.);
  connect(ui_->angleSpin,SIGNAL(valueChanged(double)),SLOT(repaintChannel(double)));
}

void CreateOceanDiagWidget::repaintChannel(double angle)
{
  odiagdoc_->repaintChannel(angle*M_PI/180.);
}


void CreateOceanDiagWidget::slotSdtCh(){
  if (nullptr == odiagdata_) { return ; }
  odiagdata_->setStartTime(QDateTime(ui_->departureDate->date(),ui_->departureTime->time()));

  slotUpdateAvailableData();
}



void CreateOceanDiagWidget::createDocument()
{
  if (nullptr != odiagdoc_) {
      delete odiagdoc_;
      odiagdoc_ = nullptr;
    }
  odiagdoc_ = new OceanDiagDoc(window_->document(), window_->mapscene());
  odiagdoc_->showNoData();
  odiagdoc_->createEmptyBlank();



  if (window_ != nullptr) {
      map::Incut* i = nullptr;
      if ( 0 != window_->mapscene()->document()->incuts().size() ) {
          i = window_->mapscene()->document()->incuts()[0];
        }
      else {
          i = window_->mapscene()->document()->addIncut(kTopRight);
        }
      setIncut(i);
    }

  auto path = QList<QPair<QString, QString>> ({QPair<QString, QString>("table", "Задачи")});

  auto title = QPair<QString, QString> ("createoceantable", "Данные батиметрии");
  QAction* a = window_->addActionToMenu(title, path);
  QObject::connect(a,SIGNAL(triggered()),this,SLOT(slotOpenOceanTable()));
  
  
  watchedEventHandler_ = window_->document()->eventHandler();
  watchedEventHandler_->installEventFilter(this);

}

bool CreateOceanDiagWidget::eventFilter(QObject* watched, QEvent* event)
{
  if (watched == watchedEventHandler_) {
      if (event->type() == ::meteo::map::DocumentEvent::DocumentChanged) {
          ::meteo::map::DocumentEvent* de = static_cast<meteo::map::DocumentEvent*>(event);
          if (nullptr != de && de->changeType() == ::meteo::map::DocumentEvent::DocumentLoaded) {
              ::meteo::map::proto::Map info = window_->document()->info();
              if (info.mutable_document()->has_raw_data()) {
                  QByteArray zd(info.mutable_document()->raw_data().data(), info.mutable_document()->raw_data().size());
                  ocean::PlaceData pd;
                  pd << zd;
                  data_.insert("0", pd);
                  odiagdoc_->setData(&(data_["0"]));
		  emit currentDataChanged();
                }
            }
        }
    }
  return QDialog::eventFilter(watched, event);
}

bool CreateOceanDiagWidget::setIncut(meteo::map::Incut* incut)
{
  if(nullptr == incut ) return false;
  incut->setMapName(kTitle);
  incut->setWMOHeader(false);
  incut->setScale(false);
  if(nullptr == station_widget_) return false;
  if(false == odiagdoc_->hasData()){
      incut->setLabel("station", false );
      incut->setLabel("coords", false );
      return true;
    }
  //incut->setLabel("begdt",tr("C %1 по %2").arg(dtStart.toString("hh:mm dd.MM.yy"), dtEnd.toString("hh:mm dd.MM.yy")));
  incut->setLabel("station", tr("%1 (%2)")
           .arg(station_widget_->stationIndex())
           .arg(station_widget_->coord().toString(false, "%1%2")));

  return true;
}


QString CreateOceanDiagWidget::makePlaceName()
{
  QString name = QString();
  QString stName, stCoord;

  //const ocean::PlaceData& data = data_.value(current_item_->text(kId));

  stName = station_widget_->stationIndex();
  if(station_widget_->coord().isValid()){
      stCoord = station_widget_->coord().toString(true, "%1 %2 %3");
    }
  name = tr(" %1 (%2)").arg(stName).arg(stCoord);
  //name += ( !current_data_.dt().isValid() ) ? "" : QObject::tr(" за ") + current_data_.dt().toString("hh:mm MM-dd-yyyy");
  return name;
}


void CreateOceanDiagWidget::slotUpdateAvailableData()
{
  //  start_time;
  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

  QTreeWidget* tree = ui_->spaceSrcPlaceTree;

  current_item_ = nullptr;
  tree->clear();
  ui_->stationCount->setText(QObject::tr(" Нет станций "));

  odiagdata_->loadAvailableData(&data_);
  QMapIterator<QString, ocean::PlaceData> it(data_);
  int index = 0;
  while (it.hasNext()) {
    it.next();
    QTreeWidgetItem* item = new TreeWidgetNumberItem(tree);
    if (nullptr != item) {
      item->setText(kId, it.key());
      if (!it.value().name().isEmpty()) {
        item->setText(kPlaceName, it.value().name());
      }
      else {
        item->setText(kPlaceName, tr("Точка %1").arg(index + 1));
      }
      item->setText(kLat, it.value().coord().strLat());
      item->setText(kLon, it.value().coord().strLon());

      item->setText(kCount,QString::number(it.value().count()));
      item->setText(kLevel,QString::number(it.value().maxlevel()));
      item->setText(kDistance,QString::number(it.value().coord().calcDistance(ui_->point->coord()),'f',0));
      index++;
    }
  }
  if (data_.size() > 0) {
    ui_->stationCount->setText(QObject::tr("Всего станций: ").append(QString::number(data_.size())));
  }

  sortSrcDataTree();

  FilterHeader* fheader = static_cast<FilterHeader*>(tree->header());
  fheader->reloadFilters();

  for (int idx = 0; idx < tree->columnCount(); idx++) {
      tree->resizeColumnToContents(idx);
    }

  QApplication::restoreOverrideCursor();

  //    reset_time("loadGeoData station");
}


void CreateOceanDiagWidget::slotRun()
{
  if ( nullptr == odiagdoc_ ) { return ;}

  if (nullptr != current_item_) {
      for( int i = 0; i <= ui_->spaceSrcPlaceTree->columnCount(); i++ ){
          current_item_->setBackground(i, QBrush(QColor(0,0,0,255), Qt::NoBrush));
        }
    }

  QObject::connect(odiagdoc_,SIGNAL(PZK_Exist(float)),SLOT(setAngleSpin(float)));

  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
  setEnabled(false);
  //TODO передавать параметр _type сюда в getProfile для получения текущих или прогностических данных zond::PlaceData
  ocean::PlaceData* profileData = getProfile();
  if(nullptr != profileData) {

      if(odiagdoc_->create(profileData/*, type_*/)) {
          if (nullptr != window_ && nullptr != window_->document()) {
              QByteArray zd;
              (*profileData) >> zd;
              ::meteo::map::proto::Map info = window_->document()->info();
              //info.set_datetime(current_data_.dt().toString(Qt::ISODate).toStdString());
              info.set_datetime(QDateTime( ui_->departureDate->date(), ui_->departureTime->time()).toString(Qt::ISODate).toStdString());
              info.mutable_document()->set_raw_data(zd.data(), zd.size());
              window_->document()->setMap(info);
              //window_->document()->setProperty(info.document());
            }
        }
    }
  if (nullptr != current_item_) {
      for( int i = 0; i <= ui_->spaceSrcPlaceTree->columnCount(); i++ ){
          ui_->spaceSrcPlaceTree->currentItem()->setBackgroundColor(i, Qt::yellow);
        }

      if (nullptr != station_widget_) {
          const ocean::PlaceData& data = data_.value(current_item_->text(kId));
          station_widget_->blockSignals(true);
          station_widget_->setStation(data.name(), data.coord());
          station_widget_->blockSignals(false);
        }
    }
  if(true == odiagdoc_->hasData()){
      QString title = kTitle + makePlaceName();
      //window_->setWindowTitle(title);
      window_->mapscene()->document()->setName(title);

      QList<map::Incut*> list = window_->mapscene()->document()->incuts();
      if( 0 != list.size() ) {
          setIncut(list[0]);
        }
    }
  setEnabled(true);
  ui_->spaceSrcPlaceTree->setFocus();
  emit currentDataChanged();
  QApplication::restoreOverrideCursor();
  disconnect(odiagdoc_,SIGNAL(PZK_Exist(float)),nullptr,nullptr);
}

ocean::PlaceData* CreateOceanDiagWidget::getProfile()
{
  ocean::PlaceData* aprofile = nullptr;
  QTreeWidget* tree = ui_->spaceSrcPlaceTree;
  for ( int i=0,isz=tree->topLevelItemCount(); i<isz; ++i ) {
      QTreeWidgetItem* item = tree->topLevelItem(i);
      if (nullptr != item && item->isSelected())  {
          current_item_ = item;
          QString key = item->text(kId);
          odiagdata_->fillProfile(&key, &(data_[key]));
          aprofile = &(data_[key]);
          break;
        }
    }

  return aprofile;
}

void CreateOceanDiagWidget::slotClearAll()
{
  current_item_ = nullptr;
  ui_->spaceSrcPlaceTree->clear();
  ui_->stationCount->setText(QObject::tr(" Нет станций "));
}


void CreateOceanDiagWidget::slotOpenOceanTable()
{
  if(nullptr == oceanTable_) {
    oceanTable_ = new odiag::OceanTableWidget(this);
    QObject::connect(this,SIGNAL(currentDataChanged()), this, SLOT(slotFillOceanTable()));
  }
  slotFillOceanTable();
  oceanTable_->show();
}

void CreateOceanDiagWidget::slotFillOceanTable()
{
  if (nullptr != oceanTable_) {
    ocean::PlaceData* pdata =  getProfile();
    if (nullptr != pdata) {
      const QList<TMeteoData>& mdList = odiagdoc_->getMdList(pdata);
      oceanTable_->fillOceanTable(pdata->name() , mdList);
    }
  }
}


}
}
