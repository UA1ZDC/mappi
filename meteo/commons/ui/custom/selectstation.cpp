#include "selectstation.h"
#include "ui_selectstation.h"

#include <qcompleter.h>
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


SelectStation::SelectStation(QWidget* parent, Qt::WindowFlags fl)
  : QWidget(parent, fl),
    ui_(new Ui::SelectStation),
    model_(0),
    indexCompleter_(0),
    ruCompleter_(0),
    enCompleter_(0)
{
  cacheOn_ = true;

  dataType_ = -1;
  stationType_ = -1;
  stationsLoaded_ = false;
  ui_->setupUi(this);

  filterModel_ = new QSortFilterProxyModel(this);
  filterModel_->setFilterRole(SelectStation::kStationTypeRole);
  filterModel_->setFilterKeyColumn(kIndexOrICAOColumn);

  indexCompleter_ = new QCompleter(this);
  indexCompleter_->setCompletionRole(kIndexOrICAORole);
  indexCompleter_->setModel(filterModel_);

  ruCompleter_ = new QCompleter(this);
  ruCompleter_->setCompletionColumn(kRuNameColumn);
  ruCompleter_->setCaseSensitivity(Qt::CaseInsensitive);
  ruCompleter_->setCompletionRole(kRuNameRole);
  ruCompleter_->setModel(filterModel_);

  enCompleter_ = new QCompleter(this);
  enCompleter_->setCompletionColumn(kEnNameColumn);
  enCompleter_->setCaseSensitivity(Qt::CaseInsensitive);
  enCompleter_->setCompletionRole(kEnNameRole);
  enCompleter_->setModel(filterModel_);

  ui_->indexEdit->setCompleter(indexCompleter_);
  ui_->ruNameEdit->setCompleter(ruCompleter_);
  ui_->enNameEdit->setCompleter(enCompleter_);

  connect( indexCompleter_, SIGNAL(activated(QModelIndex)), SLOT(slotSetupGui(QModelIndex)) );
  connect( ruCompleter_, SIGNAL(activated(QModelIndex)), SLOT(slotSetupGui(QModelIndex)) );
  connect( enCompleter_, SIGNAL(activated(QModelIndex)), SLOT(slotSetupGui(QModelIndex)) );

  connect( ui_->indexEdit,  SIGNAL(returnPressed()), SLOT(slotUpdateByFirstCompletion()) );
  connect( ui_->ruNameEdit, SIGNAL(returnPressed()), SLOT(slotUpdateByFirstCompletion()) );
  connect( ui_->enNameEdit, SIGNAL(returnPressed()), SLOT(slotUpdateByFirstCompletion()) );

  loadSavedStations();
  ui_->groupBox_2->hide();
}

SelectStation::~SelectStation()
{
  saveStations();

  delete ui_;
  ui_ = 0;

  delete indexCompleter_;
  indexCompleter_ = 0;

  delete ruCompleter_;
  ruCompleter_ = 0;

  delete enCompleter_;
  enCompleter_ = 0;
}


void SelectStation::saveStations()
{
  if ( false == TProtoText::toFile( stationList_ , meteo::global::kCustomStationsListFilename) ) {
    error_log << QObject::tr("Не удалось сохранить список станций в файл %1.")
                 .arg(meteo::global::kCustomStationsListFilename);
    return ;
  }
}

void SelectStation::setEnabled(bool en)
{
  ui_->indexEdit->setEnabled(en);
  ui_->ruNameEdit->setEnabled(en);
  ui_->enNameEdit->setEnabled(en);
}

meteo::sprinf::Station SelectStation::toStation() const
{
  meteo::sprinf::Station st;
  QString idx = ui_->indexEdit->text();
  if ( QRegExp(tr("[A-ZА-Яa-zа-я]{4}")).exactMatch(idx) ) {
    st.set_cccc(pbtools::toString(idx));
  }
  else {
    st.set_index(idx.toInt());
  }
  if ( !ui_->ruNameEdit->text().isEmpty() ) {
    st.mutable_name()->set_rus(pbtools::toString(ui_->ruNameEdit->text()));
  }
  if ( !ui_->enNameEdit->text().isEmpty() ) {
    st.mutable_name()->set_international(pbtools::toString(ui_->enNameEdit->text()));
  }
  st.mutable_position()->set_height_meters(coord().alt());
  st.mutable_position()->set_lat_radian(coord().lat());
  st.mutable_position()->set_lon_radian(coord().lon());
  st.set_data_type(dataType());

  return st;
}

void SelectStation::toProto(meteo::surf::Point *p)
{
  p->set_fi(coord().fi());
  p->set_la(coord().la());
  p->set_height(coord().alt());
  p->set_name(ruName().toUtf8().constData());
  p->set_eng_name(enName().toUtf8().constData());
  p->set_index(stationIndex().toStdString());
}

void SelectStation::setFromProto(const meteo::surf::Point &p)
{
  double alt=0.;
  if(p.has_height()){
    alt = p.height();
  }
  if(p.has_fi() && p.has_la()) {
    setCoord(meteo::GeoPoint(p.fi(),p.la(),alt));
  }
  if(p.has_index()){
    setStationIndex(QString::fromStdString(p.index()));
  }
  if(p.has_name()){
    setRuName(QString::fromUtf8(p.name().c_str()));
  }
  if(p.has_eng_name()){
    setEnName(QString::fromUtf8(p.eng_name().c_str()));
  }
}


void SelectStation::loadSavedStations()
{
  stationList_.Clear();

  if ( !QFile::exists(meteo::global::kCustomStationsListFilename) ) {
    return;
  }

  if ( false == TProtoText::fromFile( meteo::global::kCustomStationsListFilename, &stationList_ ) ) {
    error_log << QObject::tr("Не удалось прочитать список станций из файла %1.")
                 .arg(meteo::global::kCustomStationsListFilename);
    return ;
  }

  if ( 0 < stationList_.point_size() ) {
    setFromProto(stationList_.point(stationList_.point_size()-1));
  }
}

QString SelectStation::toString(const meteo::surf::Point& p)
{
  QString itemName;

  if(p.has_index()){
    itemName+=QString::fromStdString(p.index());
  }
  if(p.has_name()){
    itemName+=" " + QString::fromUtf8(p.name().c_str());
  }
  else {
    if(p.has_eng_name()){
      itemName+=" "+ QString::fromUtf8(p.eng_name().c_str());
    }
  }
  double alt=0.;
  if(p.has_height()){
    alt = p.height();
  }
  if(p.has_fi() && p.has_la()) {
    itemName+=" "+ meteo::GeoPoint(p.fi(),p.la(),alt).toString();
  }
  return itemName;
}

void SelectStation::setStations(const meteo::sprinf::Stations& list, bool autoDeleteModel)
{
  QObject* parent = autoDeleteModel ? this : 0;
  QStandardItemModel* model = new QStandardItemModel(list.station_size(), 4, parent);

  for ( int i=0,isz=list.station_size(); i<isz; ++i ) {
    const ::meteo::sprinf::Station& st = list.station(i);

    QString index = QString("%1").arg(st.index(), 5, 10, QChar('0'));
    QString cccc  = pbtools::toQString(st.cccc());

    if ( cccc.isEmpty() && st.index() <= 0 ) { continue; }

    QString ruName = pbtools::toQString(st.name().rus());
    QString enName = pbtools::toQString(st.name().international());
    meteo::GeoPoint gp = meteo::GeoPoint(st.position().lat_radian(), st.position().lon_radian(), st.position().height_meters());

    QStandardItem* item1 = new QStandardItem;
    item1->setData(cccc.isEmpty() ? index : cccc, Qt::DisplayRole);
    item1->setData(cccc.isEmpty() ? index : cccc, SelectStation::kIndexOrICAORole);
    item1->setData(st.data_type(), SelectStation::kDataTypeRole);
    item1->setData(QString::number(st.type()), SelectStation::kStationTypeRole);

    QStandardItem* item2 = new QStandardItem;
    item2->setData(ruName, Qt::DisplayRole);
    item2->setData(ruName, SelectStation::kRuNameRole);

    QStandardItem* item3 = new QStandardItem;
    item3->setData(enName, Qt::DisplayRole);
    item3->setData(enName, SelectStation::kEnNameRole);

    QStandardItem* item4 = new QStandardItem;
    item4->setData(QVariant::fromValue(gp), SelectStation::kCoordinateRole);

    model->appendRow(QList<QStandardItem*>() << item1 << item2 << item3 << item4);
  }

  setModel(model);
}

bool SelectStation::loadStationTypes(meteo::sprinf::StationTypes* types, const meteo::sprinf::MultiStatementRequest& req)
{
  if ( 0 == types ) { return false; }

  meteo::sprinf::TypesRequest request;
  for ( int i=0,isz=req.type_size(); i<isz; ++i ) {
    request.add_type(req.type(i));
  }
  meteo::sprinf::Stations stations;
  if (meteo::global::loadStations(req,&stations)){
    if ( ! meteo::global::loadStationTypes(types, req) ) {
      lastError_ = meteo::msglog::kServiceAnswerFailed.arg(meteo::global::serviceTitle(meteo::settings::proto::kSprinf));
      stationsLoaded_ = false;
      return false;
    }
    if ( types->result() == false ) {
      lastError_ = pbtools::toQString(types->error_message());
      stationsLoaded_ = false;
      return false;
    }
  }
  else {
    stationsLoaded_ = false;
    lastError_ =  meteo::msglog::kServiceAnswerFailed.arg(meteo::global::serviceTitle(meteo::settings::proto::kSprinf));
    return false;
  }
  return true;
}

void SelectStation::slotSetupGui(const QModelIndex& idx)
{
  slotClear();
  if ( !idx.isValid() ) { return; }
  beginUpdateGui();
  dataType_ = idx.sibling(idx.row(), kIndexOrICAOColumn).data(kDataTypeRole).toInt();
  stationType_ = idx.sibling(idx.row(), kIndexOrICAOColumn).data(kStationTypeRole).toString().toInt();
  ui_->indexEdit->setText(idx.sibling(idx.row(), kIndexOrICAOColumn).data(kIndexOrICAORole).toString());
  ui_->ruNameEdit->setText(idx.sibling(idx.row(), kRuNameColumn).data(kRuNameRole).toString());
  ui_->enNameEdit->setText(idx.sibling(idx.row(), kEnNameColumn).data(kEnNameRole).toString());
  setCoord(idx.sibling(idx.row(), kCoordinateColumn).data(kCoordinateRole).value<meteo::GeoPoint>());
  endUpdateGui();
  emit completed();
}


void SelectStation::setCoord(const meteo::GeoPoint& pnt){
  setLatDegree(pnt.latDeg());
  setLonDegree(pnt.lonDeg());
  setAltitudeMeter(pnt.alt());
  coord_ = pnt;
}

void SelectStation::setLatDegree(double lat)
{
  int idx = 0;
  if ( lat < 0 ) {
    idx = 1;
    lat = -lat;
  }
  ui_->latEdit->setDecDegree(lat);
  ui_->latText->setText(idx == 0 ? "с.ш." : "ю.ш.");
}

void SelectStation::setLonDegree(double lon)
{
  int idx = 0;
  if ( 0 > lon ) {
    idx = 1;
    lon = -lon;
  }
  ui_->lonEdit->setDecDegree(lon);
  ui_->lonText->setText(idx == 0 ? "в.д." : "з.д.");
}

void SelectStation::setAltitudeMeter(double alt)
{
 ui_->altEdit->setText(QString::number(alt) + " м");
}

void SelectStation::setModel(QAbstractItemModel* model)
{
  model_ = model;

  filterModel_->setSourceModel(model_);
}

QModelIndex SelectStation::findItem(const QString& indexOrICAO, int stationType, int dataType) const
{
  if ( 0 == model_ ) { return QModelIndex(); }

  bool checkStType = stationType != -1;
  bool checkDataType = dataType != -1;

  QString stType = QString::number(stationType);

  for ( int i=0,isz=model_->rowCount(); i<isz; ++i ) {
    QModelIndex idx = model_->index(i,kIndexOrICAOColumn);
    if ( idx.data(kIndexOrICAORole).toString() != indexOrICAO ) {
      continue;
    }
    if ( checkStType && idx.data(SelectStation::kStationTypeRole).toString() != stType ) {
      continue;
    }
    if ( checkDataType && idx.data(SelectStation::kDataTypeRole).toInt() != dataType ) {
      continue;
    }
    return idx;
  }

  return QModelIndex();
}

QModelIndex SelectStation::findItem(const QString& name, SelectStation::CompleterRole role) const
{
  if ( 0 == model_ ) { return QModelIndex(); }

  int column = ( kRuNameRole == role ) ? kRuNameColumn : kEnNameColumn;
  role = ( kRuNameRole == role ) ? kRuNameRole : kEnNameRole;

  for ( int i=0,isz=model_->rowCount(); i<isz; ++i ) {
    if ( model_->index(i,column).data(role).toString() == name ) {
      return model_->index(i,column);
    }
  }

  return QModelIndex();
}

QModelIndex SelectStation::findItem(const meteo::GeoPoint& coord, double ) const
{
  if ( 0 == model_ ) { return QModelIndex(); }

  for ( int i=0,isz=model_->rowCount(); i<isz; ++i ) {
    QModelIndex idx = model_->index(i,kCoordinateColumn);
    if ( idx.data(kCoordinateRole).value<meteo::GeoPoint>().compareLatLon(coord) ) {
      return idx;
    }
  }

  return QModelIndex();
}

void SelectStation::slotClear()
{
  beginUpdateGui();

  ui_->indexEdit->clear();
  ui_->ruNameEdit->clear();
  ui_->enNameEdit->clear();
  ui_->latEdit->setDecDegree(0);
  ui_->lonEdit->setDecDegree(0);
  ui_->altEdit->setText("0 м");

  dataType_ = -1;
  stationType_ = -1;

  endUpdateGui();
}

void SelectStation::beginUpdateGui()
{
  ui_->indexEdit->blockSignals(true);
  ui_->ruNameEdit->blockSignals(true);
  ui_->enNameEdit->blockSignals(true);
}

void SelectStation::endUpdateGui()
{
  ui_->indexEdit->blockSignals(false);
  ui_->ruNameEdit->blockSignals(false);
  ui_->enNameEdit->blockSignals(false);
}

QString SelectStation::stationIndex() const
{
  return ui_->indexEdit->text();
}

void SelectStation::setStationIndex(const QString &index)
{
  ui_->indexEdit->setText(index);
}

QString SelectStation::ruName() const
{
  return ui_->ruNameEdit->text();
}

void SelectStation::setRuName(const QString& name)
{
  ui_->ruNameEdit->setText(name);
}

QString SelectStation::enName() const
{
  return ui_->enNameEdit->text();
}

void SelectStation::setEnName(const QString& name)
{
  ui_->enNameEdit->setText(name);
}

meteo::GeoPoint SelectStation::coord() const
{
  return coord_;
}

int SelectStation::dataType() const
{
  return dataType_;
}

bool SelectStation::loadStation(const QList<int>& types, bool autoDeleteModel)
{
  meteo::sprinf::MultiStatementRequest req;
  foreach ( int t, types ) {
    req.add_type(t);
  }

  return loadStation(req, autoDeleteModel);
}

bool SelectStation::loadStation(const meteo::sprinf::MultiStatementRequest& req, bool autoDeleteModel)
{
  meteo::sprinf::Stations stations;
  if (meteo::global::loadStations(req,&stations)){

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
    setStations(stations,autoDeleteModel);
    stationsLoaded_ = true;
    return true;
  }
  else {
    stationsLoaded_ = false;
    lastError_ =  meteo::msglog::kServiceAnswerFailed.arg(meteo::global::serviceTitle(meteo::settings::proto::kSprinf));
    return false;
  }
}

void SelectStation::slotUpdateByFirstCompletion()
{
  if ( 0 == model_ ) { return; }

  QLineEdit* le = qobject_cast<QLineEdit*>(sender());

  QCompleter* comp = le->completer();
  if ( 0 == comp ) { return; }

  slotSetupGui(comp->currentIndex());
}

