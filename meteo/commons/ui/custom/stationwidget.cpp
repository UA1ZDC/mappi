#include "stationwidget.h"
#include "ui_stationwidget.h"

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
#include <meteo/commons/ui/map/view/actions/traceaction.h>

Q_DECLARE_METATYPE( meteo::GeoPoint )
Q_DECLARE_METATYPE( meteo::surf::Point )

#define SET_NAME(name) QDir::homePath() + "/.meteo/" + name + ".station.ini"

static bool kStationDefined = false;

StationWidget::StationWidget(QWidget* parent, Qt::WindowFlags fl)
  : QWidget(parent, fl),
    ui_(new Ui::StationWidget),
    model_(nullptr),
    completer_(nullptr)
{
  stationsLoaded_ = false;

  ui_->setupUi(this);

  // таймер для задержки сигналов для избежания дребезга
  timer_ = new QTimer(this);
  timer_->setSingleShot(true);
  QObject::connect( timer_,  SIGNAL(timeout()), this, SLOT(slotEmitChaged()) );
  timerSort_ = new QTimer(this);
  timerSort_->setSingleShot(true);
  QObject::connect( timerSort_,  SIGNAL(timeout()), this, SLOT(changeSorting()) );

  // фильтр поиска
  filterModel_ = new StationFilterProxyModel(this);
  filterModel_->setFilterRole(StationWidget::kStationTypeRole);
  filterModel_->setFilterKeyColumn(kStationColumn);

  completer_ = new QCompleter(this);
  completer_->setCaseSensitivity(Qt::CaseInsensitive);
  completer_->setCompletionColumn(kStationColumn);
  completer_->setCompletionRole(StationWidget::kCompleterRole);
  completer_->setFilterMode(Qt::MatchContains);
  completer_->setModel(filterModel_);
  StationWidgetPopup* pop = new StationWidgetPopup;
  completer_->setPopup(pop);
  ui_->station->setCompleter(completer_);

  QObject::connect( completer_, SIGNAL(activated(QModelIndex)), SLOT(slotSetupGui(QModelIndex)));
  QObject::connect( ui_->station, SIGNAL(returnPressed()), SLOT(slotUpdateByFirstCompletion()));
  QObject::connect( ui_->station, SIGNAL(textChanged(QString)), this, SLOT(slotResizeCompletionPopup()));

  QObject::connect( ui_->coordEdit, SIGNAL(coordChanged(meteo::GeoPoint)), this, SLOT(slotUpdateByCoordinate(meteo::GeoPoint)));

  QObject::connect( ui_->addBtn, SIGNAL(clicked()), SLOT(slotAddStation()));
  QObject::connect( ui_->removeBtn, SIGNAL(clicked()), SLOT(slotDelStation()));
  QObject::connect( ui_->savedStation, SIGNAL(activated(int)), SLOT(slotChangeCurStation(int)));

  QObject::connect( ui_->filterBtn, SIGNAL(clicked(bool)), SLOT(slotShowFilterMenu()));

  QObject::connect( this, SIGNAL(add()), SLOT(slotUpdateComboVisible()));
  QObject::connect( this, SIGNAL(remove()), SLOT(slotUpdateComboVisible()));

  ui_->savedStation->view()->setTextElideMode(Qt::ElideRight);
  //ui_->coordEdit->setMapButtonVisible(true);
  //ui_->searchCheck->setEnabled(ui_->coordEdit->isMapButtonPressed());

  QObject::connect(pop, SIGNAL(signalSelectionChanged()),
       this, SLOT(slotPopupIndexChanged()), Qt::DirectConnection);

  QObject::connect(ui_->coordEdit, &GeoPointEditor::signalOnMap,
                   this, &StationWidget::slotOnMap);

  loadSavedStations();

  //setHorisontal(false);
}

StationWidget::~StationWidget()
{
  saveStations();

  delete ui_;
  ui_ = nullptr;

  delete completer_;
  completer_ = nullptr;

  delete timer_;
  timer_ = nullptr;

  delete timerSort_;
  timerSort_ = nullptr;

  delete settings_;
  settings_ = nullptr;
}

void StationWidget::setObjectName(const QString& name)
{
  delete settings_;
  settings_ = new QSettings(SET_NAME(name), QSettings::IniFormat);

  QObject::setObjectName(name);
}

void StationWidget::setSizePolicy(QSizePolicy pol)
{
  QWidget::setSizePolicy(pol);
}

void StationWidget::setSizePolicy(QSizePolicy::Policy hor, QSizePolicy::Policy ver)
{
  QWidget::setSizePolicy(hor, ver);
}


void StationWidget::slotOnMap(bool isEnabled)
{
  emit signalOnMap(isEnabled);
}

//-----------------------------

//TODO не поняла с этим и curChanged и надо ли красным

void StationWidget::slotEmitChaged()
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
void StationWidget::slotEmitFizzBuzzChaged()
{
  checkInputed();

  // если установлена задержка на сигнал - то задерживаем его отправку по таймеру
//  if ( true == isNeedDelay_ ) {
//    timer_->start(1500);
//  } else {
//    slotEmitChaged();
//  }
  slotEmitChaged();
}

//! блокировка сигналов gui
void StationWidget::beginUpdateGui()
{
  if ( is_signal_blocked_ == false ) {
    ui_->station->blockSignals(true);
    ui_->coordEdit->blockSignals(true);
    ui_->coordEdit->blockAllSignals(true);
    is_signal_blocked_ = true;
  }
}

//! разблокировка сигналов gui
void StationWidget::endUpdateGui()
{
  if ( is_signal_blocked_ ) {
    ui_->station->blockSignals(false);
    ui_->coordEdit->blockSignals(false);
    ui_->coordEdit->blockAllSignals(false);
    is_signal_blocked_ = false;
  }
}

void StationWidget::slotShowFilterMenu()
{
  stTypeMenu_.exec(mapToGlobal(ui_->filterBtn->geometry().bottomLeft()));
}

void StationWidget::slotFilterChecked(bool checked)
{
  Q_UNUSED( checked );

  if (nullptr == settings_) {
    settings_ = new QSettings(SET_NAME(objectName()), QSettings::IniFormat);
  }

  QAction* act = qobject_cast<QAction*>(sender());
  if ( nullptr == act ) { return; }

  QApplication::setOverrideCursor(Qt::WaitCursor);
  QStringList pattern;
  QList<QAction*> list = stTypeMenu_.actions();
  for ( int i = 0, isz = list.size(); i < isz; ++i ) {
    if ( list.at(i)->isChecked() ) {
      pattern.append(QString::number(list.at(i)->data().toInt()));
    }
    settings_->setValue("filter." + QString::number(list.at(i)->data().toInt()), list.at(i)->isChecked());
  }
  settings_->sync();

  filterModel_->setFilterRegExp(QString("\\b(") + pattern.join("|") + ")\\b");
  QRegExp rx = filterModel_->filterRegExp();

  QApplication::restoreOverrideCursor();

  slotClear();
}


/**
 * Очищаем поля ввода
 */
void StationWidget::slotClear()
{
  //  var("clear");
  beginUpdateGui();
  ui_->station->clear();
  cur_.Clear();
  endUpdateGui();
}

void StationWidget::setEnabled(bool en)
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
 * Загружаем станции
 * @param  sttypes         Загружаемые типы станций
 * @param  autoDeleteModel Автоудаление
 * @param  show_types      Отображаемые типы станций в меню (по ним поиск). Если пустой, то по всем, что в sstypes
 * @return
 */
bool StationWidget::loadStation(const QList<meteo::sprinf::MeteostationType>& sttypes, bool autoDeleteModel,
        const QList<meteo::sprinf::MeteostationType>& show_types)
{
  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

  if (nullptr == settings_) {
    settings_ = new QSettings(SET_NAME(objectName()), QSettings::IniFormat);
  }

  types_ = sttypes;

  //типы станций
  meteo::sprinf::MultiStatementRequest req;
  meteo::sprinf::StationTypes types;
  if ( ! meteo::global::loadStationTypes(&types, req) ) {
    lastError_ = meteo::msglog::kServiceAnswerFailed.arg(meteo::global::serviceTitle(meteo::settings::proto::kSprinf));
    stationsLoaded_ = false;
    QApplication::restoreOverrideCursor();
    return false;
  }

  if ( types.result() == false ) {
    lastError_ = pbtools::toQString(types.error_message());
    stationsLoaded_ = false;
    QApplication::restoreOverrideCursor();
    return false;
  }

  QMap<int, QString> map;
  for ( int i=0,isz=types.station_size(); i<isz; ++i ) {
    map[types.station(i).type()] = pbtools::toQString(types.station(i).name());
  }

  //меню
  QStringList pattern;
  stTypeMenu_.clear();
  foreach ( int type, map.keys() ) {
    if (show_types.contains(meteo::sprinf::MeteostationType(type)) ||
  (show_types.isEmpty() && types_.contains(meteo::sprinf::MeteostationType(type)) )) {
      QAction* act = stTypeMenu_.addAction(map[type]);
      act->setCheckable(true);
      bool checked = true;
      if (settings_->contains("filter." + QString::number(type))) {
  checked = settings_->value("filter." + QString::number(type)).toBool();
      }
      act->setChecked(checked);
      if (checked) {
  pattern.append(QString::number(type));
      }

      act->setData(type);
      connect( act, SIGNAL(toggled(bool)), SLOT(slotFilterChecked(bool)) );
    }
  }

  filterModel_->setFilterRegExp(QString("\\b(") + pattern.join("|") + ")\\b");

  //станции
  if (!stationsLoaded_) {
    meteo::sprinf::MultiStatementRequest reqs;
    meteo::sprinf::Stations stations;
    if (!meteo::global::loadStations(reqs, &stations)) {
      stationsLoaded_ = false;
      lastError_ =  meteo::msglog::kServiceAnswerFailed.arg(meteo::global::serviceTitle(meteo::settings::proto::kSprinf));
      QApplication::restoreOverrideCursor();
      return false;
    }
    setStations(stations, map, autoDeleteModel);
    stationsLoaded_ = true;
  }

  QApplication::restoreOverrideCursor();
  return true;
}


/**
 * заполняем станции из базы в модель автозаполнения
 * @param list            [description]
 * @param autoDeleteModel [description]
 */
void StationWidget::setStations(const meteo::sprinf::Stations& list, const QMap<int, QString>& types,
            bool autoDeleteModel)
{
  QObject* parent = autoDeleteModel ? this : nullptr;
  QStandardItemModel* model = new QStandardItemModel(list.station_size(), 4, parent);
  for ( int i = 0,isz = list.station_size(); i < isz; ++i ) {
    const ::meteo::sprinf::Station& st = list.station(i);
    if (!types_.contains(meteo::sprinf::MeteostationType(st.type()))) {
      continue;
    }

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
    item1->setData(QString::number(st.type()), StationWidget::kStationTypeRole);
    item1->setData(QVariant::fromValue(gp), StationWidget::kCoordinateRole);

    QStandardItem* item2 = new QStandardItem;
    item2->setData(ru, Qt::DisplayRole);

    QStandardItem* item3 = new QStandardItem;
    item3->setData(en, Qt::DisplayRole);

    // QStandardItem* item4 = new QStandardItem;
    // item4->setData(gp.toString(false, "%1, %2"), Qt::DisplayRole);

    QStandardItem* item5 = new QStandardItem;
    item5->setData(types.value(st.type()), Qt::DisplayRole);

    model->appendRow(QList<QStandardItem*>() << item1 << item2 << item3 /*<< item4*/ << item5);
  }

  //  filterModel_->sort(kStationColumn);
  filterModel_->setSourceModel(model);

  model_ = model;

  if (nullptr != completer_) {
    StationWidgetPopup* popup = qobject_cast<StationWidgetPopup*>(completer_->popup());
    if (nullptr != popup) {
      popup->resizeWithText();
    }
  }
}

//текст для отображения после выбора
QString StationWidget::stationText(const QString& index, const QString& ru, const QString en)
{
  return index + " ("  + (ru.isEmpty() ? en : ru) + ")";
}


/**
 * заполняем поля в соответствии с выбранным пунктом из автокомлетера
 * @param idx [description]
 * @param fillall - true если надо заполнить все поля из комплетера, если false то заполняются все,
 * кроме того, в котором еще вводится информация
 */
void StationWidget::slotSetupGui(const QModelIndex& index)
{
  if ( !index.isValid() ) { return; }

  beginUpdateGui();

  QModelIndex sibling = index.sibling(index.row(), StationWidget::kStationColumn);

  if (sibling.isValid()) {
    cur_.set_type(sibling.data(StationWidget::kStationTypeRole).toInt());
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
void StationWidget::slotResizeCompletionPopup()
{
  if (nullptr == completer_ || nullptr == completer_->popup()) {
    return;
  }

  timerSort_->stop();

  // StationWidgetPopup* popup = qobject_cast<StationWidgetPopup*>(completer_->popup());
  // if (nullptr != popup) {
  //   popup->resizeWithText();
  // }

  beginUpdateGui();


  //debug_log << ui_->station->text() << pbtools::toQString(cur_.index()).remove(-1, 1) << kStationDefined;

  if ( true == kStationDefined ) {
    kStationDefined = false;
    // if (!ui_->station->text().isEmpty() &&
    // 	ui_->station->text() != pbtools::toQString(cur_.index()).remove(-1, 1)) {
      //debug_log << "not_eq";
      //ui_->station->setText(pbtools::toQString(cur_.index()));
      //}
      //ui_->coordEdit->setCoord(meteo::GeoPoint(0,0));

    //    var("clear");
    cur_.Clear();
    meteo::GeoPoint gp = ui_->coordEdit->coord();
    cur_.set_fi(gp.lat());
    cur_.set_la(gp.lon());
    cur_.set_height(gp.alt());
    cur_.set_type(0);
    sortedCol_ = kUndefColumn;
  }

  cur_.set_index(pbtools::toString(ui_->station->text()));

  filterModel_->setText((ui_->station->text()));

  timerSort_->start(100);

  endUpdateGui();
}


bool StationWidget::isIcaoName(const QString& text, int /*stationType*/)
{
  if ( nullptr == completer_ ||  nullptr == completer_->popup()) { return false; }

  QAbstractItemModel* model = completer_->popup()->model();

  for ( int i=0, isz = model->rowCount(); i<isz; ++i ) {
    QModelIndex cur = model->index(i, kStationColumn);

    if ( cur.data(Qt::DisplayRole).toString().startsWith(text.toUpper()) ) {
      return true;
    }
  }

  return false;
}

void StationWidget::changeSorting()
{
  if (!ui_->station->text().isEmpty()) {
    if (sortedCol_ == kUndefColumn || ui_->station->text().length() < 5) {

      if (ui_->station->text()[0].isNumber()) { //цифра
  sortedCol_ = kStationColumn;
      } else if (ui_->station->text()[0].unicode() <= 127) { //латиница
  if (isIcaoName(ui_->station->text(), meteo::sprinf::kStationAirport)) {
    sortedCol_ = kStationColumn;
  } else {
    sortedCol_ = kEnNameColumn;
  }
      } else {
  if (isIcaoName(ui_->station->text(), meteo::sprinf::kStationAerodrome)) { //кириллица
    sortedCol_ = kStationColumn;
  } else {
    sortedCol_ = kRuNameColumn;
  }
      }
    }

    if (sortedCol_ != filterModel_->sortColumn()) {
      filterModel_->sort(sortedCol_);
    } else {
      filterModel_->invalidate();
    }
  } else {
    sortedCol_ = kUndefColumn;
  }
}

/**
 * если выбрана другая станция во всплывающем окне
 * заполняем выбранным айтемом
 */
void StationWidget::slotPopupIndexChanged()
{
  if (nullptr == completer_ || nullptr == completer_->popup()) {
    return;
  }

  kStationDefined = true;

  slotSetupGui(completer_->popup()->currentIndex());
}

/**
 * обновляем в соответствии с первым попаданием
 */
void StationWidget::slotUpdateByFirstCompletion()
{
  //  trc;
  if ( nullptr == model_ || nullptr == completer_) {
    return;
  }

  kStationDefined = true;

  if ( true == ui_->station->text().isEmpty() ) {
    return;
  }


  //
  // заполняем поля в соответствии с индексом
  //
  if (nullptr != completer_ && nullptr != completer_->popup() &&
      completer_->popup()->isVisible()) {
    slotSetupGui(completer_->currentIndex());
  }
}


/**
 * проверяем введены ли значения и закрашиваем
 */
void StationWidget::checkInputed()
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
   setStyleSheet("*[requiredError='true']	\
    {\
       border-color:     red;      \
       border-width:     1px;      \
       border-style:     solid;    \
       border-radius:    3px;      \
       padding:          4px;      \
       background-color: #ffffff;  \
     } ");

   //   background-color: #ffc0c0;
  return;

}


/**
 * ищем ближайшую станцию
 * @param  station [description]
 * @param  point   [description]
 * @param  radius  [description]
 * @return         [description]
 */
bool StationWidget::findNearestStation(meteo::sprinf::Station* station, const meteo::GeoPoint& point, int radius) const
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

bool StationWidget::isStationValid() const
{
  if (cur_.has_type()) {
    return true;
  }

  return false;
}

QModelIndex StationWidget::findItem(const meteo::GeoPoint& coord, double /*delta = .0000000001*/) const
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
void StationWidget::setCoord(const meteo::GeoPoint& coord)
{
  qApp->setOverrideCursor(Qt::WaitCursor);

  // var("clear");
  // var(completer_->currentIndex().isValid());
  cur_.Clear();

  meteo::sprinf::Station station;

  if ( true == ui_->coordEdit->isSearchEnabled() &&
       true == ui_->coordEdit->isSearchChecked() &&
       true == findNearestStation(&station, coord, 150000) ) {
    //ищем станцию в модели

    if (!station.has_type() || !station.has_station() ||
  !findStation(pbtools::toQString(station.station()), station.type()) ) {
      ui_->coordEdit->setCoord(coord);
      cur_.set_fi(coord.lat());
      cur_.set_la(coord.lon());
      cur_.set_height(coord.alt());
      cur_.set_type(0);
    }

  } else {
    emit changed();
    emit curChanged();
    ui_->coordEdit->setCoord(coord);
    cur_.set_fi(coord.lat());
    cur_.set_la(coord.lon());
    cur_.set_height(coord.alt());
    cur_.set_type(0);
  }
  qApp->restoreOverrideCursor();

  return;
}

void StationWidget::setStation(const QString &index, const meteo::GeoPoint& coord,
             const QString& name /*= ""*/)
{
  cur_.Clear();
  cur_.set_index(pbtools::toString(index));
  cur_.set_name(name.toStdString());
  cur_.set_fi(coord.lat());
  cur_.set_la(coord.lon());
  cur_.set_height(coord.alt());
  cur_.set_type(0);

  beginUpdateGui();
  kStationDefined = true;
  ui_->station->setText(index + " " + name);
  ui_->coordEdit->setCoord(coord);
  endUpdateGui();

  slotEmitFizzBuzzChaged();
  emit curChanged();
}

//! поиск по индексу/икао и типу
bool StationWidget::findStation(const QString &station, int stationType)
{
  if ( nullptr == model_ ) { return false; }

  QModelIndex index;

  for ( int i=0,isz=model_->rowCount(); i<isz; ++i ) {
     QModelIndex cur = model_->index(i, kStationColumn);

    if ( cur.data(Qt::DisplayRole).toString() != station ) {
      continue;
    }
    if ( cur.data(StationWidget::kStationTypeRole).toInt() != stationType ) {
      continue;
    }

    index = cur;

    break;
  }

  if (!index.isValid() ) {
    return false;
  }

  kStationDefined = true;
  slotSetupGui(index);

  return true;
}

/**
 * обновляем поля по координатам
 */
void StationWidget::slotUpdateByCoordinate(const meteo::GeoPoint& coord)
{
  if ( nullptr == model_ ) {
    return;
  }

  beginUpdateGui();

  kStationDefined = true;

  if (ui_->coordEdit->isMapButtonPressed()) {
    beginUpdateGui();
    ui_->station->clear();
    endUpdateGui();
  }

  if ( true == ui_->coordEdit->isMapButtonPressed() &&
       true == ui_->coordEdit->isSearchChecked() ){
    setCoord(coord);
  }
  else {
    if (! (MnMath::isEqual(coord.fi(), float(0.0)) && MnMath::isEqual(coord.la(), float(0.0)))) {
      QModelIndex idx = findItem(coord);
      //var(idx.isValid());
      if ( idx.isValid() ) {
        slotSetupGui(idx);
      } else {
        setStation("", coord);
      }
    }
  }

  endUpdateGui();
}

void StationWidget::setSearchVisible(bool visible)
{
  ui_->coordEdit->setSearchVisible(visible);
}

bool StationWidget::isSearchVisible() const
{
  return ui_->coordEdit->isSearchVisible();
}

void StationWidget::setMapButtonVisible(bool visible)
{
  ui_->coordEdit->setMapButtonVisible(visible);
 }

bool StationWidget::isMapButtonVisible() const
{
  return ui_->coordEdit->isMapButtonVisible();
}

bool StationWidget::isFilterVisible() const
{
  return ui_->filterBtn->isVisible();
}

void StationWidget::setFilterVisible(bool visible)
{
  ui_->filterBtn->setVisible(visible);
}


bool StationWidget::isSavedStationsVisible() const
{
  return ui_->savedStation->isVisible();
}

void StationWidget::setSavedStationsVisible(bool visible)
{
  ui_->savedStation->setVisible(visible);
  ui_->addBtn->setVisible(visible);
  ui_->removeBtn->setVisible(visible);
  if (!isHor_) {
    ui_->line->setVisible(false);
  } else {
    ui_->line->setVisible(visible);
  }
}

bool StationWidget::isCoordVisible() const
{
  return ui_->coordEdit->isVisible();
}

void StationWidget::setCoordVisible(bool visible)
{
  setMapButtonVisible(visible);
  ui_->coordEdit->setVisible(visible);
}

bool StationWidget::isHorisontal() const
{
  return isHor_;
}

void StationWidget::setHorisontal(bool sethor)
{
  // if ((sethor && isHor_) || (!sethor && !isHor_)) {
  //   return;
  // }

  isHor_ = sethor;

  if (sethor) {
    delete layout();

    ui_->savedStation->setMaximumSize(QSize(200, QWIDGETSIZE_MAX));
    //setSavedStationsVisible(true);
    ui_->filterBtn->show();

    QHBoxLayout *hl_ = new QHBoxLayout(this);
    hl_->setObjectName(QLatin1String("horizontalLayout"));
    hl_->setMargin(0);
    hl_->addWidget(ui_->filterBtn);
    hl_->addWidget(ui_->station);
    hl_->addWidget(ui_->coordEdit);
   // hl_->addWidget(ui_->searchCheck);
    ui_->hSpacer = new QSpacerItem(40, 20, QSizePolicy::Preferred, QSizePolicy::Minimum);
    hl_->addItem(ui_->hSpacer);
    hl_->addWidget(ui_->line);
    hl_->addWidget(ui_->savedStation);
    hl_->addWidget(ui_->addBtn);
    hl_->addWidget(ui_->removeBtn);

  } else {
    delete layout();

    ui_->savedStation->setMaximumSize(QSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX));
    ui_->filterBtn->hide();
    ui_->line->hide();
    //setMapButtonVisible(false);
    //setSavedStationsVisible(false);

    QVBoxLayout *lay = new QVBoxLayout(this);
    lay->setMargin(0);
    QHBoxLayout *hl1 = new QHBoxLayout();
    hl1->setMargin(0);
    hl1->addWidget(ui_->filterBtn);
    hl1->addWidget(ui_->station);
    QVBoxLayout *vl_ = new QVBoxLayout();
    vl_->setMargin(0);

    vl_->addWidget(ui_->coordEdit);
   // vl_->addWidget(ui_->searchCheck);
    vl_->addWidget(ui_->savedStation);
    vl_->addStretch();
    QHBoxLayout *hl_ = new QHBoxLayout();
    hl_->setMargin(0);
    hl_->addWidget(ui_->addBtn);
    hl_->addWidget(ui_->removeBtn);
    hl_->addWidget(ui_->savedStation);

    lay->addLayout(hl1);
    lay->addLayout(vl_);
    lay->addLayout(hl_);
  }


}


//-----------------------------
//список сохраненных

//! Сохранение в файл
void StationWidget::saveStations()
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
void StationWidget::setFromProto(const meteo::surf::Point &p)
{
  //  var(p.Utf8DebugString());

  if (!p.has_type()  ||
      !p.has_index() || p.index().empty() ||
      !findStation(pbtools::toQString(p.index()), p.type())) {

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

    meteo::GeoPoint gp;
    if (p.has_fi() && p.has_la()) {
      double alt=0.;
      if(p.has_height()){
        alt = p.height();
      }
      gp = meteo::GeoPoint(p.fi(),p.la(),alt);
    }

    beginUpdateGui();
    kStationDefined = true;
    ui_->station->setText(station);
    ui_->coordEdit->setCoord(gp);
    endUpdateGui();

    slotEmitFizzBuzzChaged();
    emit curChanged();
  }
}

QString StationWidget::toString(const meteo::surf::Point& p)
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

meteo::sprinf::Station StationWidget::toStation() const
{
  meteo::sprinf::Station st;
  st.set_station(cur_.index());

  QString idx = pbtools::toQString(cur_.index());
  if ( QRegExp(tr("[A-ZА-Яa-zа-я0-9]{4}")).exactMatch(idx) ) {
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
void StationWidget::loadSavedStations()
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


void StationWidget::slotAddStation()
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

void StationWidget::slotDelStation()
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
void StationWidget::fillStationsCombo()
{
  ui_->savedStation->clear();

  for ( int i = 0, sz = stationList_.point_size(); i < sz; ++i  ) {
    const meteo::surf::Point& p = stationList_.point(i);
    ui_->savedStation->addItem(toString(p),QVariant::fromValue(p));
  }

  if ( stationList_.point_size() > 0) {
    ui_->savedStation->setCurrentIndex(stationList_.point_size()-1);
    slotChangeCurStation(stationList_.point_size()-1);
  }

  slotUpdateComboVisible();
}

/**
 * выбираем другую станцию из списка
 * @param index [description]
 */
void StationWidget::slotChangeCurStation(int index)
{
  // заполняем поля из прото
  setFromProto(ui_->savedStation->itemData(index, Qt::UserRole).value<meteo::surf::Point>());
  //  slotUpdateByFirstCompletion();

  if ( stationList_.point_size() > 1 ) {
    stationList_.mutable_point()->SwapElements(index, stationList_.point_size() - 1);
  }

  saveStations();
}


/**
 * показываем/скрываем кнопки с сохраненными станциями
 */
void StationWidget::slotUpdateComboVisible()
{
  if ( !ui_->addBtn->isVisible() ) { return; }

  ui_->savedStation->setHidden(0 == ui_->savedStation->count());
}

//------

QString StationWidget::placeName()
{
  QString name = "";
  QString stName, stCoord, stIndex;

  stName = ruName();
  if (stName.isEmpty()) {
    stName = enName();
  }

  stIndex =  stationIndex();

  meteo::GeoPoint gp = ui_->coordEdit->coord();
  if(gp.isValid()){
    stCoord = gp.toString(true, "%1 %2 %3");
  }

  name = tr(" %1 (%2, %3)").arg(stName)
                                     .arg(stIndex)
                                     .arg(stCoord);
  return name;
 }


QString StationWidget::stationIndex() const
{
  if (cur_.index().empty()) return QString();

  QString index = pbtools::toQString(cur_.index());
  if (QRegExp(tr("\\d{5}")).exactMatch(index)) { //0 в начале может быть: у местных CCCC, в координатах, идущих вместо названия
    index.remove(QRegExp("^0"));
  }

  return index;
}


QString StationWidget::ruName() const
{
  if (cur_.name().empty()) return QString();

  return pbtools::toQString(cur_.name());
}

QString StationWidget::enName() const
{
  if (cur_.eng_name().empty()) return QString();

  return pbtools::toQString(cur_.eng_name());
}

meteo::GeoPoint StationWidget::coord() const
{
  return ui_->coordEdit->coord();
}

bool StationWidget::isMapButtonPressed() const
{
  return ui_->coordEdit->isMapButtonPressed();
}


//----------------------

StationWidgetPopup::StationWidgetPopup()
{
  setRootIsDecorated(false);
  header()->setVisible(false);
  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setSelectionBehavior(QAbstractItemView::SelectRows);
}


void StationWidgetPopup::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
  Q_UNUSED(selected);
  Q_UNUSED(deselected);
  emit signalSelectionChanged();
  QTreeView::selectionChanged(selected, deselected);
}


void StationWidgetPopup::resizeWithText()
{
  int colWidth = 0;
  for (int i = 0, sz = columns_; i < sz; ++i) {
    resizeColumnToContents(i);
    colWidth += columnWidth(i);
  }

  setFixedWidth(colWidth + 15);
}

bool StationWidget::isAltitudeVisible()
{
  return ui_->coordEdit->isAltitudeVisible();
}

void StationWidget::setIsAltitudeVisible(bool isVisible)
{
  ui_->coordEdit->setAltitudeVisible(isVisible);
}

meteo::map::MapScene* StationWidget::getAssociatedScene()
{
  return ui_->coordEdit->getAssociatedScene();
}

meteo::map::TraceAction* StationWidget::getAssociatedAction()
{
  return ui_->coordEdit->getAssociatedAction();
}


StationFilterProxyModel::StationFilterProxyModel(QObject *parent /*= 0*/):
  QSortFilterProxyModel(parent)
{
}

bool StationFilterProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
  QString leftData = sourceModel()->data(left).toString().toUpper();
  QString rightData = sourceModel()->data(right).toString().toUpper();

  if (curText_.isEmpty()) {
    return leftData < rightData;
  }

  uint lidx = leftData.indexOf(curText_);
  uint ridx = rightData.indexOf(curText_);

  if (lidx != ridx) {
    return lidx < ridx;
  }

  uint lcnt = leftData.count(curText_);
  uint rcnt = rightData.count(curText_);
  if ( lcnt != rcnt) {
    return lcnt < rcnt;
  }

  return leftData < rightData;
}
