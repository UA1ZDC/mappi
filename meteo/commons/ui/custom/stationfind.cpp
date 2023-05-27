#include "stationfind.h"
#include "ui_stationfind.h"

#include <qcompleter.h>
#include <qtableview.h>
#include <qtreeview.h>
#include <qheaderview.h>
#include <qstandarditemmodel.h>
#include <qsortfilterproxymodel.h>

#include <commons/textproto/pbcache.h>
#include <commons/textproto/pbtools.h>
#include <commons/textproto/tprototext.h>

#include <meteo/commons/global/global.h>
#include <meteo/commons/global/log.h>
#include <meteo/commons/proto/services.pb.h>
#include <meteo/commons/rpc/rpc.h>

Q_DECLARE_METATYPE( meteo::GeoPoint )
Q_DECLARE_METATYPE( meteo::surf::Point )



//TODO 

// bool StationWidget::isMapButtonVisible() const
// bool StationWidget::isMapButtonPressed() const
// bool StationWidget::isHorisontal() const
// void StationWidget::setMapButtonVisible(bool visible)
// void StationWidget::unPressMapButton()const

//QString StationWidget::placeName(){


static bool needToFillByFirstCompletion = false;

StationFind::StationFind(QWidget* parent, Qt::WindowFlags fl)
  : QWidget(parent, fl),
    ui_(new Ui::StationFind),
    model_(nullptr),
    completer_(nullptr)
{
  stationsLoaded_ = false;
  
  ui_->setupUi(this);

  // таймер для задержки сигналов для избежания дребезга
  timer_ = new QTimer(this);
  timer_->setSingleShot(true);
  QObject::connect( timer_,  SIGNAL(timeout()), this, SLOT(slotEmitChaged()) );

  // фильтр поиска
  filterModel_ = new QSortFilterProxyModel(this);
  filterModel_->setFilterRole(StationFind::kStationTypeRole);
  filterModel_->setFilterKeyColumn(kStationColumn);
  filterModel_->setSortRole(kStationRole);

  completer_ = new QCompleter(this);
  completer_->setCaseSensitivity(Qt::CaseInsensitive);
  completer_->setCompletionColumn(kStationColumn);
  completer_->setCompletionRole(StationFind::kCompleterRole);
  completer_->setFilterMode(Qt::MatchContains);
  completer_->setModel(filterModel_);
  StationFindPopup* pop = new StationFindPopup;
  completer_->setPopup(pop);
  ui_->station->setCompleter(completer_);

  QObject::connect( completer_, SIGNAL(activated(QModelIndex)), SLOT(slotSetupGui(QModelIndex)));
  QObject::connect( ui_->station, SIGNAL(returnPressed()), SLOT(slotUpdateByFirstCompletion()));
  QObject::connect( ui_->station, SIGNAL(textChanged(QString)), this, SLOT(slotResizeCompletionPopup()));
  QObject::connect( ui_->station, SIGNAL(textChanged(QString)), this, SLOT(slotEmitFizzBuzzChaged()));

  QObject::connect( ui_->coordEdit, SIGNAL(requestCoord(bool)), SIGNAL(mapBtnToggled(bool)));
  QObject::connect( ui_->coordEdit, SIGNAL(requestCoord(bool)), ui_->searchCheck, SLOT(setEnabled(bool)));
  QObject::connect( ui_->coordEdit, SIGNAL(tryChanged()), this, SLOT(slotClear()));
  QObject::connect( ui_->coordEdit, SIGNAL(tryChanged()), this, SLOT(slotEmitFizzBuzzChaged()));
  
  QObject::connect( ui_->addBtn, SIGNAL(clicked()), SLOT(slotAddStation()));
  QObject::connect( ui_->removeBtn, SIGNAL(clicked()), SLOT(slotDelStation()));
  QObject::connect( ui_->savedStation, SIGNAL(activated(int)), SLOT(slotChangeCurStation(int)));

  QObject::connect( ui_->filterBtn, SIGNAL(clicked(bool)), SLOT(slotShowFilterMenu()));
  
  QObject::connect( this, SIGNAL(add()), SLOT(slotUpdateComboVisible()));
  QObject::connect( this, SIGNAL(remove()), SLOT(slotUpdateComboVisible()));

  ui_->savedStation->view()->setTextElideMode(Qt::ElideRight);
  ui_->coordEdit->setMapButtonVisible(true);
  ui_->searchCheck->setEnabled(ui_->coordEdit->isMapButtonPressed());
  
  QObject::connect(pop, SIGNAL(signalSelectionChanged()),
		   this, SLOT(slotPopupIndexChanged()), Qt::DirectConnection);
  
  loadSavedStations();  
  //setHorisontal(false);
}

StationFind::~StationFind()
{
  saveStations();

  delete ui_;
  ui_ = nullptr;

  delete completer_;
  completer_ = nullptr;

  delete timer_;
  timer_ = nullptr;
}

//-----------------------------

//TODO не поняла с этим и curChanged и надо ли красным

void StationFind::slotEmitChaged()
{
  // 
  // проверяем - если данные введены, то красным подсвечивать не надо
  // если нет - то надо
  // 
  
  emit changed();
}

/**
 * слот, который запускает таймер на испускание сигнала об изменении
 */
void StationFind::slotEmitFizzBuzzChaged()
{
  checkInputed();

  // если установлена задержка на сигнал - то задерживаем его отправку по таймеру
  if ( isNeedDelay_ ) {
    timer_->start(1500);
  } else {
    slotEmitChaged();
  }
}

//! блокировка сигналов gui
void StationFind::beginUpdateGui()
{ 
  if ( is_signal_blocked_ == false ) {
    ui_->station->blockSignals(true);
    ui_->coordEdit->blockSignals(true);
    ui_->coordEdit->blockAllSignals(true);
    is_signal_blocked_ = true;
  }
}

//! разблокировка сигналов gui
void StationFind::endUpdateGui()
{
  if ( is_signal_blocked_ ) {
    ui_->station->blockSignals(false);
    ui_->coordEdit->blockSignals(false);
    ui_->coordEdit->blockAllSignals(false);
    is_signal_blocked_ = false;
  }
}

void StationFind::slotShowFilterMenu()
{
  stTypeMenu_.exec(mapToGlobal(ui_->filterBtn->geometry().bottomLeft()));
}

void StationFind::slotFilterChecked(bool checked)
{
  Q_UNUSED( checked );

  QAction* act = qobject_cast<QAction*>(sender());
  if ( nullptr == act ) { return; }

  QString pattern = "";
  QList<QAction*> list = stTypeMenu_.actions();
  for ( int i = 0, isz = list.size(); i < isz; ++i ) {
    if ( list.at(i)->isChecked() ) {
      pattern += QString::number(list.at(i)->data().toInt());
    }
  }

  QApplication::setOverrideCursor(Qt::WaitCursor);
  filterModel_->setFilterWildcard("[" + pattern + "]");
  QApplication::restoreOverrideCursor();
  
  slotClear();
}

/**
 * Очищаем поля ввода
 */
void StationFind::slotClear()
{
  var("clear");
  beginUpdateGui();
  ui_->station->clear();
  cur_.Clear();
  endUpdateGui();
}

void StationFind::setEnabled(bool en)
{
  // если false - то сохраняем состояние фокуса, чтобы потмо восстановить его
  if ( !en ) {
    isLastFocused_ = ui_->station->hasFocus();
  }
  
  ui_->station->setEnabled(en);
  ui_->coordEdit->setEnabled(en);
  ui_->savedStation->setEnabled(en);
  ui_->addBtn->setEnabled(en);
  ui_->removeBtn->setEnabled(en);

  // восстанавливаем фокус
  if ( en && isLastFocused_ ){
    ui_->station->setFocus();  
  }
}


//-----------------------------
//выбор станции


/**
 * загружаем станции
 * @param  types           [description]
 * @param  autoDeleteModel [description]
 * @return                 [description]
 */
bool StationFind::loadStation(const QList<int>& sttypes, bool autoDeleteModel)
{
  meteo::sprinf::MultiStatementRequest req;
  foreach ( int t, sttypes ) {
    req.add_type(t);
  }
  
  meteo::sprinf::Stations stations;
  if (!meteo::global::loadStations(req, &stations)) {
    stationsLoaded_ = false;
    lastError_ =  meteo::msglog::kServiceAnswerFailed.arg(meteo::global::serviceTitle(meteo::settings::proto::kSprinf));
    return false;
  }

  meteo::sprinf::StationTypes types;
  if ( ! meteo::global::loadStationTypes(&types, req) ) {
    lastError_ = meteo::msglog::kServiceAnswerFailed.arg(meteo::global::serviceTitle(meteo::settings::proto::kSprinf));
    stationsLoaded_ = false;
    return false;
  }
  
  if ( types.result() == false ) {
    lastError_ = pbtools::toQString(types.error_message());
    stationsLoaded_ = false;
    return false;
  }
  
  QMap<QString,int> map;
  for ( int i=0,isz=types.station_size(); i<isz; ++i ) {
    map[pbtools::toQString(types.station(i).name())] = types.station(i).type();
  }
  
  stTypeMenu_.clear();
  foreach ( const QString& name, map.keys() ) {
    QAction* act = stTypeMenu_.addAction(name);
    act->setCheckable(true);
    act->setChecked(true);
    act->setData(map[name]);
    connect( act, SIGNAL(toggled(bool)), SLOT(slotFilterChecked(bool)) );
  }
  setStations(stations, map, autoDeleteModel);
  stationsLoaded_ = true;
  
  return true;
}
 

/**
 * заполняем станции из базы в модель автозаполнения
 * @param list            [description]
 * @param autoDeleteModel [description]
 */
void StationFind::setStations(const meteo::sprinf::Stations& list, const QMap<QString,int>& types,
			      bool autoDeleteModel)
{
  QObject* parent = autoDeleteModel ? this : nullptr;
  QStandardItemModel* model = new QStandardItemModel(list.station_size(), 4, parent);

  for ( int i = 0,isz = list.station_size(); i < isz; ++i ) {
    const ::meteo::sprinf::Station& st = list.station(i);

    QString index;
    if (st.index() > 0) {
      index = QString("%1").arg(st.index(), 5, 10, QChar('0'));
    }
    QString cccc  = pbtools::toQString(st.cccc());

    if ( cccc.isEmpty() && st.index() <= 0 &&
	 st.name().rus().empty() && st.name().international().empty()) { continue; }
    
    QString ru = pbtools::toQString(st.name().rus());
    QString en = pbtools::toQString(st.name().international());
    
    meteo::GeoPoint gp = meteo::GeoPoint(st.position().lat_radian(), st.position().lon_radian(), st.position().height_meters());

    QString station = cccc.isEmpty() ? index : cccc;
    
    QStandardItem* item1 = new QStandardItem;
    item1->setData(station, Qt::DisplayRole);
    item1->setData(station + " " + ru + " " + en, kCompleterRole);
    item1->setData(stationText(station, ru, en), kStationRole);
    item1->setData(QString::number(st.type()), StationFind::kStationTypeRole);
    item1->setData(QVariant::fromValue(gp), StationFind::kCoordinateRole);    
    
    QStandardItem* item2 = new QStandardItem;
    item2->setData(ru, Qt::DisplayRole);

    QStandardItem* item3 = new QStandardItem;
    item3->setData(en, Qt::DisplayRole);

    // QStandardItem* item4 = new QStandardItem;
    // item4->setData(gp.toString(false, "%1, %2"), Qt::DisplayRole);

    QStandardItem* item5 = new QStandardItem;
    item5->setData(types.key(st.type()), Qt::DisplayRole);
    
    model->appendRow(QList<QStandardItem*>() << item1 << item2 << item3 /*<< item4*/ << item5);
  }

  filterModel_->sort(kStationColumn);
  filterModel_->setSourceModel(model);

  model_ = model;

  if (nullptr != completer_) {
    StationFindPopup* popup = qobject_cast<StationFindPopup*>(completer_->popup());
    if (nullptr != popup) {
      popup->resizeWithText();
    }
  }
}

//текст для отображения после выбора
QString StationFind::stationText(const QString& index, const QString& ru, const QString en)
{
  return index + " ("  + (ru.isEmpty() ? en : ru) + ")";
}


/**
 * заполняем поля в соответствии с выбранным пунктом из автокомлетера
 * @param idx [description]
 * @param fillall - true если надо заполнить все поля из комплетера, если false то заполняются все, 
 * кроме того, в котором еще вводится информация
 */
void StationFind::slotSetupGui(const QModelIndex& index)
{
  if ( !index.isValid() ) { return; }
  
  beginUpdateGui();

  QModelIndex sibling = index.sibling(index.row(), StationFind::kStationColumn);
  
  if (sibling.isValid()) {   
    cur_.set_type(sibling.data(StationFind::kStationTypeRole).toInt());
    cur_.set_index(pbtools::toString(sibling.data(Qt::DisplayRole).toString()));
    
    meteo::GeoPoint gp = sibling.data(kCoordinateRole).value<meteo::GeoPoint>();
    cur_.set_fi(gp.lat());
    cur_.set_la(gp.lon());
    cur_.set_height(gp.alt());

    ui_->station->setText(sibling.data(kCompleterRole).toString());
    ui_->coordEdit->setCoord(gp);
  }
  
  QModelIndex siblingRu = index.sibling(index.row(), kRuNameColumn);
  if (siblingRu.isValid()) {
    cur_.set_name(pbtools::toString(siblingRu.data(Qt::DisplayRole).toString()));
  }
  QModelIndex siblingEn = index.sibling(index.row(), kEnNameColumn);
  if (siblingEn.isValid()) {
    cur_.set_eng_name(pbtools::toString(siblingEn.data(Qt::DisplayRole).toString()));
  }

    
  // вызываем обновление изменений и расчетов
  slotEmitFizzBuzzChaged();
    
  endUpdateGui();

  // someChanged();
}


/**
 * фунуция отработки изменения количества станций в комплетере
 * и автозаполнения первым попавшимся пунктом
 */
void StationFind::slotResizeCompletionPopup()
{
  if (nullptr == completer_ || nullptr == completer_->popup()) {
    return;
  }

  // StationFindPopup* popup = qobject_cast<StationFindPopup*>(completer_->popup());
  // if (nullptr != popup) {
  //   popup->resizeWithText();
  // }

  beginUpdateGui();
  
  ui_->coordEdit->setCoord(meteo::GeoPoint(0,0));
   
  if ( true == needToFillByFirstCompletion ) {
    needToFillByFirstCompletion = false;
    ui_->station->setText(pbtools::toQString(cur_.index()));
  } else {
    var("clear");
    cur_.Clear();
  }

  endUpdateGui();
}

/**
 * если выбрана другая станция во всплывающем окне
 * заполняем выбранным айтемом
 */
void StationFind::slotPopupIndexChanged()
{
  if (nullptr == completer_ || nullptr == completer_->popup()) {
    return;
  }
  
  needToFillByFirstCompletion = true;

  slotSetupGui(completer_->popup()->currentIndex());
}

/**
 * обновляем в соответствии с первым попаданием
 */
void StationFind::slotUpdateByFirstCompletion()
{
  if ( nullptr == model_ || nullptr == completer_) {
    return;
  }

  needToFillByFirstCompletion = true;
  
  if ( true == ui_->station->text().isEmpty() ) {
    return;
  }

  // 
  // заполняем поля в соответствии с индексом
  // 
  slotSetupGui(completer_->currentIndex());
}


/**
 * проверяем введены ли значения и закрашиваем
 */
void StationFind::checkInputed()
{
  meteo::GeoPoint gp = ui_->coordEdit->coord();
    
  if ( ui_->station->text().isEmpty()) {
    if( MnMath::isEqual(gp.fi(), float(0.0)) && MnMath::isEqual(gp.la(), float(0.0)) ) {
      ui_->station->setProperty("requiredError",  true);
      ui_->coordEdit->setPropertyError("requiredError",  true);
    } else {
      ui_->station->setProperty("requiredError",  false);
      ui_->coordEdit->setPropertyError("requiredError",  false);
    }
  } else {
    ui_->station->setProperty("requiredError",  false);
    ui_->coordEdit->setPropertyError("requiredError",  false);
  }
  
  // устанавливаем красные поля для необходимых данных
  this->setStyleSheet("*[requiredError='true'] \
    {\
      border-color:     red;      \
      border-width:     1px;      \
      border-style:     solid;    \
      border-radius:    3px;      \
      padding:          4px;      \
      background-color: #ffc0c0;  \
    } ");
  return;
  
}


/**
 * ищем ближайшую станцию
 * @param  station [description]
 * @param  point   [description]
 * @param  radius  [description]
 * @return         [description]
 */
bool StationFind::findNearestStation(meteo::sprinf::Station* station, const meteo::GeoPoint& point, int radius) const
{
  meteo::sprinf::CircleRegionRequest req;
  req.mutable_circle()->set_radius_meter(radius);
  req.mutable_circle()->mutable_center()->set_lat_radian(point.lat());
  req.mutable_circle()->mutable_center()->set_lon_radian(point.lon());

  meteo::rpc::Channel* ch = meteo::global::serviceChannel(meteo::settings::proto::kSprinf);
  if ( nullptr == ch ) {
    error_log << meteo::msglog::kServiceConnectFailedSimple.arg(meteo::global::serviceTitle(meteo::settings::proto::kSprinf));
    return false;
  }

  QList<int> types;
  QList<QAction*> list = stTypeMenu_.actions();
  for ( int i=0,isz=list.size(); i<isz; ++i ) {
    if ( list.at(i)->isChecked() ) {
      types << list.at(i)->data().toInt();
    }
  }

  bool res = false;
  foreach ( int type, types ) {
    req.set_station_type(type);

    meteo::sprinf::StationsWithDistance* resp = nullptr;
    resp = ch->remoteCall(&meteo::sprinf::SprinfService::GetStationsByPosition, req, 30000);

    if ( nullptr == resp ) { break; }

    if ( resp->result() == false ) {
      delete resp;
      break;
    }

    double min = std::numeric_limits<double>::max();
    for ( int i=0,isz=resp->station_size(); i<isz; ++i ) {
      const meteo::sprinf::StationWithDistance& st = resp->station(i);
      if ( st.distance_to() < min ) {
        res = true;
        min = st.distance_to();
        if ( nullptr != station ) { station->CopyFrom(st.main()); }
      }
    }

    delete resp;
  }

  ch->disconnect();
  delete ch;

  return res;
}

// QModelIndex StationWidget::findItem(const QString& indexOrICAO, int stationType, int dataType) const
// {
//   if ( nullptr == model_ ) { return QModelIndex(); }

//   bool checkStType = stationType != -1;
//   bool checkDataType = dataType != -1;

//   QString stType = QString::number(stationType);

//   for ( int i=0,isz=model_->rowCount(); i<isz; ++i ) {
//     QModelIndex idx = model_->index(i,kNameAndCoordColumn);
//     if ( idx.data(kIndexOrICAORole).toString() != indexOrICAO ) {
//       continue;
//     }
//     if ( checkStType && idx.data(StationWidget::kStationTypeRole).toString() != stType ) {
//       continue;
//     }
//     if ( checkDataType && idx.data(StationWidget::kDataTypeRole).toInt() != dataType ) {
//       continue;
//     }
//     return idx;
//   }

//   return QModelIndex();
// }

// QModelIndex StationWidget::findItem(const QString& name, StationWidget::CompleterRole role) const
// {
//   if ( 0 == model_ ) { return QModelIndex(); }

//   for ( int i=0,isz=model_->rowCount(); i<isz; ++i ) {
//     if ( model_->index(i,kNameAndCoordColumn).data(role).toString() == name ) {
//       return model_->index(i,kNameAndCoordColumn);
//     }
//   }

//   return QModelIndex();
// }

QModelIndex StationFind::findItem(const meteo::GeoPoint& coord, double /*delta = .0000000001*/) const
{
  if ( nullptr == model_ ) { return QModelIndex(); }

  for ( int i=0,isz=model_->rowCount(); i<isz; ++i ) {
    QModelIndex idx = model_->index(i, kStationColumn);
    if ( idx.data(kCoordinateRole).value<meteo::GeoPoint>().compareLatLon(coord) ) {
      return idx;
    }
  }

  return QModelIndex();
}

/**
 * устанавливаем координаты станции
 * ищем по ближайшим координатам
 * заполняем остальные поля
 * @param coord [description]
 */
void StationFind::setCoord(const meteo::GeoPoint& coord)
{
  qApp->setOverrideCursor(Qt::WaitCursor);

  var("clear");
  cur_.Clear();
  
  meteo::sprinf::Station station;
  if ( ui_->searchCheck->isEnabled()
       && ui_->searchCheck->isChecked()
       && findNearestStation(&station, coord, 150000) ) {

    cur_.set_type(station.type());
    
    meteo::GeoPoint p;
    p.setLat(station.position().lat_radian());
    p.setLon(station.position().lon_radian());
    p.setAlt(station.position().height_meters());
    cur_.set_fi(p.lat());
    cur_.set_la(p.lon());
    cur_.set_height(p.alt());

    QString stationText;
    
    if ( true  == station.has_cccc() &&
	 false == station.cccc().empty() ) {
      cur_.set_index(station.cccc());
      stationText = pbtools::toQString(station.cccc());
    }
    else if (true == station.has_index())  {
      stationText = QString("%1").arg(station.index(), 5, 10, QChar('0'));
      cur_.set_index(stationText.toStdString());
    }
    
    if (true == station.has_name()) {
      if (true == station.name().has_rus()) {
	stationText += " " + pbtools::toQString(station.name().rus());
	cur_.set_name(station.name().rus());
      }
    }

    if (true == station.name().has_international()) {
      stationText += " " +  pbtools::toQString(station.name().international());
      cur_.set_eng_name(station.name().international());
    }

    ui_->station->setText(stationText);
    needToFillByFirstCompletion = true;
    
  } else {
    ui_->coordEdit->setCoord(coord);
    cur_.Clear();
    cur_.set_fi(coord.lat());
    cur_.set_la(coord.lon());
    cur_.set_height(coord.alt());
  }

  //  slotEmitChaged();
  qApp->restoreOverrideCursor();
  return;
}

/**
 * обновляем поля по координатам
 */
void StationFind::slotUpdateByCoordinate()
{
  if ( nullptr == model_ ) { return; }

  meteo::GeoPoint coord = ui_->coordEdit->coord();

  beginUpdateGui();

  needToFillByFirstCompletion = true;
  
  QModelIndex idx = findItem(coord);
  if ( idx.isValid() ) {
    ui_->station->setText(idx.data(kStationTypeRole).toString());
  }
  else {
    slotClear();
    ui_->coordEdit->setCoord(coord);
  }

  endUpdateGui();
}


//-----------------------------
//список сохраненных

//! Сохранение в файл
void StationFind::saveStations()
{
  if ( false == TProtoText::toFile( stationList_ , meteo::global::kCustomStationsListFilename) ) {
    error_log << QObject::tr("Не удалось сохранить список станций в файл %1.")
                 .arg(meteo::global::kCustomStationsListFilename);
  }
}

/**
 * загружаем коордианты станций в поле из списка сохранений
 * @param p [description]
 */
void StationFind::setFromProto(const meteo::surf::Point &p)
{
  beginUpdateGui();
  cur_.CopyFrom(p);

  QString station;
  if (p.has_index()) {
    station += pbtools::toQString(p.index());
  }

  if (p.has_name()) {
    station += " " + pbtools::toQString(p.name());
  }

  if(p.has_eng_name()) {
    station += " " + pbtools::toQString(p.eng_name());
  }
  
  ui_->station->insert(station);
  needToFillByFirstCompletion = true;
  
  if (p.has_fi() && p.has_la()) {
    double alt=0.;
    if(p.has_height()){
      alt = p.height();
    }
    ui_->coordEdit->setCoord(meteo::GeoPoint(p.fi(),p.la(),alt));
  }

  endUpdateGui();
}

QString StationFind::toString(const meteo::surf::Point& p)
{
  QString itemName;

  if ( p.has_index() ) {
    itemName += QString::fromStdString(p.index());
  }
  if ( p.has_name() ) {
    itemName += "  " + QString::fromUtf8(p.name().c_str());
  }
  else if ( p.has_eng_name() ) {
    itemName += "  " + QString::fromUtf8(p.eng_name().c_str());
  }
  double alt=0.;
  if ( p.has_height() ) {
    alt = p.height();
  }
  if ( p.has_fi() && p.has_la() ) {
    itemName += " " + meteo::GeoPoint(p.fi(),p.la(),alt).toString(false,"(%1, %2)");
  }
  return itemName;
}

meteo::sprinf::Station StationFind::toStation() const
{
  meteo::sprinf::Station st;
  QString idx = pbtools::toQString(cur_.index());
  if ( QRegExp(tr("[A-ZА-Яa-zа-я]{4}")).exactMatch(idx) ) {
    st.set_cccc(pbtools::toString(idx));
  }
  else {
    st.set_index(idx.toInt());
  }
  if ( !cur_.name().empty() ) {
    st.mutable_name()->set_rus(cur_.name());
  }
  if ( !cur_.eng_name().empty() ) {
    st.mutable_name()->set_international(cur_.eng_name());
  }
  st.mutable_position()->set_height_meters(cur_.height());
  st.mutable_position()->set_lat_radian(cur_.fi());
  st.mutable_position()->set_lon_radian(cur_.la());
  st.set_type(cur_.type());

  return st;
}

/**
 * загружаем сохранненные станции из файлa
 */
void StationFind::loadSavedStations()
{
  stationList_.Clear();
  ui_->savedStation->clear();

  if ( !QFile::exists(meteo::global::kCustomStationsListFilename) ) {
    return;
  }

  if ( false == TProtoText::fromFile( meteo::global::kCustomStationsListFilename, &stationList_ ) ) {
    error_log << QObject::tr("Не удалось прочитать список станций из файла %1.")
                 .arg(meteo::global::kCustomStationsListFilename);
    return;
  }

  fillStationsCombo();
}


void StationFind::slotAddStation()
{
  for ( int i = 0, sz = stationList_.point_size(); i < sz; ++i  ) {
    const meteo::surf::Point& p = stationList_.point(i);
    if( MnMath::isEqual(cur_.fi(), p.fi()) && MnMath::isEqual(cur_.la(), p.la())) {
      return;
    }
  }
  meteo::surf::Point* p = stationList_.add_point();
  if (nullptr != p) {
    p->CopyFrom(cur_);
  }

  fillStationsCombo();
  emit add();
}

void StationFind::slotDelStation()
{
  meteo::surf::Polygon st_vs =  stationList_;
  stationList_.Clear();
  for ( int i = 0, sz = st_vs.point_size(); i < sz; ++i  ) {
    const meteo::surf::Point& p = st_vs.point(i);
    if ( ui_->savedStation->currentText() == toString(p) ) {
      continue;
    }
    stationList_.add_point()->CopyFrom(p);
  }
  
  fillStationsCombo();
  emit remove();
}

/**
 * заполняем выпадающий список со станциями, которые считываем из файла
 */
void StationFind::fillStationsCombo()
{
  ui_->savedStation->clear();

  for ( int i = 0, sz = stationList_.point_size(); i < sz; ++i  ) {
    const meteo::surf::Point& p = stationList_.point(i);
    ui_->savedStation->addItem(toString(p),QVariant::fromValue(p));
  }

  if ( stationList_.point_size() > 0 ) {
    ui_->savedStation->setCurrentIndex(stationList_.point_size()-1);
    slotChangeCurStation(stationList_.point_size()-1);
  }

  slotUpdateComboVisible();
}

/**
 * выбираем другую станцию из списка
 * @param index [description]
 */
void StationFind::slotChangeCurStation(int index)
{
  // заполняем поля из прото
  setFromProto(ui_->savedStation->itemData(index, Qt::UserRole).value<meteo::surf::Point>());
  slotUpdateByFirstCompletion();

  if ( stationList_.point_size() > 1 ) {
    stationList_.mutable_point()->SwapElements(index, stationList_.point_size() - 1);
  }

  saveStations();
  
  // испускаем сигнал о том, что изменили станцию
  // его ловит forecastwidget
  slotEmitFizzBuzzChaged();
  emit curChanged();
}


/**
 * показываем/скрываем кнопки с сохраненными станциями
 */
void StationFind::slotUpdateComboVisible()
{
  if ( !ui_->addBtn->isVisible() ) { return; }

  ui_->savedStation->setHidden(0 == ui_->savedStation->count());
}

//------

QString StationFind::stationIndex() const
{
  return pbtools::toQString(cur_.index());
}

// void StationFind::setStation(const QString &station)
// {
//   ui_->station->setText(station);
// }

QString StationFind::ruName() const
{
  return pbtools::toQString(cur_.name());
}

QString StationFind::enName() const
{
  return pbtools::toQString(cur_.eng_name());
}

meteo::GeoPoint StationFind::coord() const
{
  return ui_->coordEdit->coord();
}

GeoPointEditor* StationFind::geopointEditor() const
{
  return ui_->coordEdit;
}

//----------------------

StationFindPopup::StationFindPopup()
{
  setRootIsDecorated(false);
  header()->setVisible(false);
  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setSelectionBehavior(QAbstractItemView::SelectRows);
  for (int i = 0, sz = columns_; i < sz; ++i) {
    resizeColumnToContents(i);
  }  
}


void StationFindPopup::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
  Q_UNUSED(selected);
  Q_UNUSED(deselected);
  emit signalSelectionChanged();
  QTreeView::selectionChanged(selected, deselected);
}


void StationFindPopup::resizeWithText()
{
  for (int i = 0, sz = columns_; i < sz; ++i) {
    resizeColumnToContents(i);
  }
  
  setFixedWidth(viewportSizeHint().width() + 15);
  // debug_log << width() << height() << viewportSizeHint();// << viewport().size().width();
}
