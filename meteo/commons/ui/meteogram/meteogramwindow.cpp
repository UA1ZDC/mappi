#include "meteogramwindow.h"

#include <qsettings.h>

#include <cross-commons/app/paths.h>
#include <cross-commons/debug/tlog.h>

#include <commons/meteo_data/meteo_data.h>
#include <commons/geobasis/generalproj.h>
#include <commons/meteo_data/tmeteodescr.h>
#include <commons/textproto/pbtools.h>

#include <meteo/commons/global/global.h>
#include <meteo/commons/proto/field.pb.h>
#include <meteo/commons/proto/map_document.pb.h>
#include <meteo/commons/proto/meteo.pb.h>
#include <meteo/commons/proto/meteomenu.pb.h>
#include <meteo/commons/proto/surface_service.pb.h>
#include <meteo/commons/ui/custom/stationwidget.h>
#include <meteo/commons/ui/map/graph/funcs.h>
#include <meteo/commons/ui/map/graph/layergraph.h>
#include <meteo/commons/ui/mainwindow/mdisubwindow.h>
#include <meteo/commons/ui/mainwindow/widgethandler.h>
#include <meteo/commons/ui/map/geopolygon.h>
#include <meteo/commons/ui/map/layergrid.h>
#include <meteo/commons/ui/map/view/mapwindow.h>
#include <meteo/commons/global/weatherloader.h>
#include <meteo/commons/ui/map/view/actions/traceaction.h>
#include <meteo/commons/ui/map/view/actions/scaleaction.h>
#include <meteo/commons/ui/graph/coordaction/graphcoordaction.h>
#include <meteo/commons/ui/graph/valueaction/graphvalueaction.h>
#include <meteo/commons/ui/map/graph/ramkagraph.h>
//#include <meteogramplugin.h>


struct FieldData
{
  QDateTime dt;
  descr_t descr;
  int     hour;
  float   value;
  QString text;
};

uint qHash(const FieldData& key) { return qHash(key.dt.toString() + QString::number(key.descr)); }

uint qHash(const QDateTime& key) { return qHash(key.toString(Qt::ISODate)); }

bool operator==(const FieldData& fd1, const FieldData& fd2) { return qHash(fd1) == qHash(fd2); }

namespace meteo {

bool lessThan(const TMeteoData& m1, const TMeteoData& m2)
{
  QDateTime d1 = TMeteoDescriptor::instance()->dateTime(m1);
  QDateTime d2 = TMeteoDescriptor::instance()->dateTime(m2);
  return d1.secsTo(d2) > 0;
}

static const QString kTitle = QObject::tr("Метеограммы");
static const QString kName = QObject::tr("meteogramm_action");
static const QString kNameStationWgt = QObject::tr("meteogramm");

MeteogramWindow::MeteogramWindow(meteo::map::MapWindow* parent, map::Document* doc, bool stw_load)
  : PrepareWidget(parent),
    isContainsMap_(false),
    document_(doc),
    mapwindow_(parent),
    station_widget_(nullptr),
    lastSelectedCenter_(-1)
{
  qApp->setOverrideCursor(Qt::WaitCursor);
  setMode(PrepareWidget::kMeteogram);
  setWindowFlags(windowFlags() | Qt::Dialog);
  setWindowTitle(kTitle);

  setMargins(5,5,5,5);

  connect( this, SIGNAL(run()), SLOT(slotRun()) );

  connect( this, SIGNAL(dateTimeChanged(QDateTime,QDateTime,bool)), SLOT(slotUpdateCenters()) );
  connect( this, SIGNAL(paramChanged()), SLOT(slotUpdateCenters()) );
  connect( analyseRadio(), SIGNAL(toggled(bool)), SLOT(slotUpdateCenters()) );

  connect( centerCombo(), SIGNAL(activated(int)), SLOT(slotCenterChanged(int)) );

  QDateTime utc = QDateTime::currentDateTimeUtc();
  utc.setTime(QTime(utc.time().hour() + 1,0,0));
  setBeginDateTime(utc.addDays(-2));
  setEndDateTime(utc);

  if ( nullptr != mapwindow_ ) {
    //mapwindow_->setCursor(Qt::WaitCursor);
   // WidgetHandler::instance()->showStationBar(true);
    setStationGroupVisible(false);
    qApp->restoreOverrideCursor();
    //mapwindow_->unsetCursor();
  }
  else {
    setStationGroupVisible(true);
  }
 if(stw_load) addStWidget();

  centerCombo()->clear();
  centerCombo()->addItem(tr("<нет данных>"), -1);

  slotLoadState();

 // connect( checkP(), SIGNAL(toggled(bool)), this, SLOT(slotSaveState()) );
//  connect( checkPQNH(), SIGNAL(toggled(bool)), this, SLOT(slotSaveState()) );
//  connect( checkU(), SIGNAL(toggled(bool)), this, SLOT(slotSaveState()) );
 // connect( checkT(), SIGNAL(toggled(bool)), this, SLOT(slotSaveState()) );
//  connect( checkTd(), SIGNAL(toggled(bool)), this, SLOT(slotSaveState()) );
//  connect( checkWind(), SIGNAL(toggled(bool)), this, SLOT(slotSaveState()) );

  resize(minimumSizeHint());
 // setButtonsState(true, true,  true,  true,  true);

  map::WeatherLoader::instance()->loadPunchLibrary();
  qApp->restoreOverrideCursor();
  //unsetCursor();
}

void MeteogramWindow::closeEvent(QCloseEvent *event)
{
  slotSaveState();
  event->accept();
}


MeteogramWindow::~MeteogramWindow()
{
}

bool MeteogramWindow::addStWidget(){
  QList<meteo::sprinf::MeteostationType> stations_to_load;
  stations_to_load << meteo::sprinf::MeteostationType::kStationSynop
       << meteo::sprinf::MeteostationType::kStationAirport
       << meteo::sprinf::MeteostationType::kStationAerodrome
       << meteo::sprinf::MeteostationType::kStationAero
       << meteo::sprinf::MeteostationType::kStationHydro
       << meteo::sprinf::MeteostationType::kStationOcean
       << meteo::sprinf::MeteostationType::kStationRadarmap
       << meteo::sprinf::MeteostationType::kStationGeophysics;

  QToolBar *tb = new QToolBar(this->mapwindow_);
  this->mapwindow_->addToolBar(tb);
  this->station_widget_ = new StationWidget(tb);
  tb->addWidget(this->station_widget_);
  this->station_widget_->loadStation(stations_to_load);

  setStationWidget(station_widget_);
  return true;
}


map::LayerGraph* MeteogramWindow::addLayerT(const QList<TMeteoData>& mdList)
{
  if ( nullptr == document_ ) { return nullptr; }

  map::LayerGraph* layer = new map::LayerGraph(document_, QObject::tr("Температура"));

  AxisGraph* y = layer->yAxis();
  y->setPosition(AxisGraph::kLeft);
  y->setLabel(tr("T,°C"));

  AxisGraph* x = layer->xAxis();
  x->setPosition(AxisGraph::kBottom);
  x->setTickLabelType(AxisGraph::kltDateTime);

  layer->setData(mdList, "", "T");

  return layer;
}

map::LayerGraph*MeteogramWindow::addLayerTd(const QList<TMeteoData>& mdList)
{
  if ( nullptr == document_ ) { return nullptr; }

  map::LayerGraph* layer = new map::LayerGraph(document_, QObject::tr("Температура точки росы"));

  AxisGraph* y = layer->yAxis();
  y->setPosition(AxisGraph::kLeft);
  y->setLabel(tr("Тd,°C"));

  AxisGraph* x = layer->xAxis();
  x->setPosition(AxisGraph::kBottom);
  x->setTickLabelType(AxisGraph::kltDateTime);

  layer->setData(mdList, "", "Td");

  return layer;
}

map::LayerGraph*MeteogramWindow::addLayerP(const QList<TMeteoData>& mdList)
{
  if ( nullptr == document_ ) { return nullptr; }

  map::LayerGraph* layer = new map::LayerGraph(document_, QObject::tr("Давление"));

  AxisGraph* y = layer->yAxis();
  y->setPosition(AxisGraph::kLeft);
  y->setLabel(tr("P,гПа"));

  AxisGraph* x = layer->xAxis();
  x->setPosition(AxisGraph::kBottom);
  x->setTickLabelType(AxisGraph::kltDateTime);

  layer->setData(mdList, "", "P");

  return layer;
}

map::LayerGraph*MeteogramWindow::addLayerPQNH(const QList<TMeteoData>& mdList)
{
  if ( nullptr == document_ ) { return nullptr; }

  map::LayerGraph* layer = new map::LayerGraph(document_, QObject::tr("Давление (QNH)"));

  AxisGraph* y = layer->yAxis();
  y->setPosition(AxisGraph::kLeft);
  y->setLabel(tr("QNH,гПа"));

  AxisGraph* x = layer->xAxis();
  x->setPosition(AxisGraph::kBottom);
  x->setTickLabelType(AxisGraph::kltDateTime);

  layer->setData(mdList, "", "PQNH");

  return layer;
}

map::LayerGraph*MeteogramWindow::addLayerU(const QList<TMeteoData>& mdList)
{
  if ( nullptr == document_ ) { return nullptr; }

  map::LayerGraph* layer = new map::LayerGraph(document_, QObject::tr("Влажность"));

  AxisGraph* y = layer->yAxis();
  y->setPosition(AxisGraph::kRight);
  y->setLabel(tr("U,%"));

  AxisGraph* x = layer->xAxis();
  x->setPosition(AxisGraph::kBottom);
  x->setTickLabelType(AxisGraph::kltDateTime);

  layer->setData(mdList, "", "U");

  return layer;
}

map::LayerGraph* MeteogramWindow::addLayerWind(const QList<TMeteoData>& mdList)
{
  if ( nullptr == document_ ) { return nullptr; }

  map::LayerGraph* layer = new map::LayerGraph(document_, QObject::tr("Ветер"));

  AxisGraph* y = layer->yAxis();
  y->setPosition(AxisGraph::kRight);
  y->setTickLabelFormat("f,1");
  y->setLabel(tr("Высота,м"));

  AxisGraph* x = layer->xAxis();
  x->setPosition(AxisGraph::kBottom);
  x->setTickLabelType(AxisGraph::kltDateTime);

  ::meteo::puanson::proto::Puanson puan = map::WeatherLoader::instance()->punchlibraryspecial().value("ddff");
  layer->setPuanson(puan);
  layer->setData(mdList, "", "h0");

  y->setRange(-10, 90);

  meteo::Property prop = y->property();
  prop.set_visible(false);
  y->setProperty(prop);

  return layer;
}

void MeteogramWindow::getLoadSrcDataRequest(surf::DataRequest *req ,const QString& index) const
{
  QDateTime beg = beginDateTime();
  QDateTime end = endDateTime();

  req->set_date_start(pbtools::toString(beg.toString(Qt::ISODate)));
  req->set_date_end(pbtools::toString(end.toString(Qt::ISODate)));

  if ( checkT()->isChecked() ) {
    req->add_meteo_descrname("T");
  }
  if ( checkTd()->isChecked() ) {
    req->add_meteo_descrname("Td");
  }
  if ( checkP()->isChecked() ) {
    req->add_meteo_descrname("P");
    req->add_meteo_descrname("PQNH");
  }
  if ( checkU()->isChecked() ) {
    req->add_meteo_descrname("U");
  }
  if ( checkWind()->isChecked() ) {
    req->add_meteo_descrname("dd");
    req->add_meteo_descrname("ff");
      //req.add_meteo_descrname("wind");
  }

  req->add_station(index.toStdString());
  req->set_type_level(1);
  req->set_level_p(0);
}

QList<TMeteoData> MeteogramWindow::loadSrcData(bool remote,const QString &sindex) const
{
  surf::DataRequest req;
  this->getLoadSrcDataRequest(&req,sindex);
  req.set_use_parent(remote);
  rpc::Channel* channel = global::serviceChannel(settings::proto::kSrcData);
  if ( nullptr == channel ) {
    error_msg << tr("Ошибка: отсутствует подключение к сервису данных.");
    return QList<TMeteoData>();
  }

  //time_log << tr("Запрос данных (srcdata.service).");

//  var(req.Utf8DebugString());

  surf::DataReply* resp = channel->remoteCall(&::meteo::surf::SurfaceService::GetMeteoDataOnStation, req, 90000);
  channel->disconnect();
  delete channel;

  if ( nullptr == resp ) {
    time_log << tr("--> ответ не получен");
    error_msg << tr("Ошибка: превышено время ожидания ответа от сервиса.");
    return QList<TMeteoData>();
  }

  //time_log << tr("--> получено наборов данных %1").arg(resp->meteodata_size());

  QList<TMeteoData> mdList;
  for ( int i=0,isz=resp->meteodata_size(); i<isz; ++i ) {
    TMeteoData md;
    md << pbtools::fromBytes(resp->meteodata(i));
    md.add("h0", TMeteoParam("", 0, control::RIGHT));
    mdList << md;
  }

  delete resp;
  return mdList;
}

QList<TMeteoData> MeteogramWindow::loadFieldData() const
{
  QDateTime beg = beginDateTime();
  QDateTime end = endDateTime();

  field::DataRequest req;
  bool is_empty = true;
  if ( checkT()->isChecked() ) {
    req.add_meteo_descr(TMeteoDescriptor::instance()->descriptor("T"));
    is_empty = false;
  }
  if ( checkTd()->isChecked() ) {
    req.add_meteo_descr(TMeteoDescriptor::instance()->descriptor("Td"));
    is_empty = false;
  }
  if ( checkP()->isChecked() ) {
    req.add_meteo_descr(TMeteoDescriptor::instance()->descriptor("P"));
    is_empty = false;
  }
  if ( checkU()->isChecked() ) {
    req.add_meteo_descr(TMeteoDescriptor::instance()->descriptor("U"));
    is_empty = false;
  }
  if ( checkWind()->isChecked() ) {
    req.add_meteo_descr(TMeteoDescriptor::instance()->descriptor("dd"));
    req.add_meteo_descr(TMeteoDescriptor::instance()->descriptor("ff"));
    is_empty = false;
  }
  if(is_empty) {
      return QList<TMeteoData>();
    }
  qApp->setOverrideCursor(Qt::WaitCursor);

  req.set_forecast_start(pbtools::toString(beg.toString(Qt::ISODate)));
  req.set_forecast_end(pbtools::toString(end.toString(Qt::ISODate)));

  req.add_level(0);
  req.add_level(2);
  req.add_level(10);
  req.add_type_level(1);
  req.add_type_level(101);
  req.add_type_level(103);
  req.add_type_level(200);

  req.set_need_field_descr(true);
  int center = centerCombo()->itemData(centerCombo()->currentIndex()).toInt();
  if ( center > 0 ) {
    req.add_center(center);
  }

  surf::Point* p = req.add_coords();
  p->set_fi(stationEdit()->coord().fi());
  p->set_la(stationEdit()->coord().la());

  rpc::Channel* channel = global::serviceChannel(settings::proto::kField);
  if ( nullptr == channel ) {
    error_msg << tr("Ошибка: отсутствует подключение к сервису данных.");
    qApp->restoreOverrideCursor();
    return QList<TMeteoData>();
  }

 // time_log << tr("Запрос данных (field.service).");

//  var(req.Utf8DebugString());
  // debug_log<<req.Utf8DebugString();
  field::ValueDataReply* resp = channel->remoteCall(&field::FieldService::GetForecastValues, req, 180000);

  channel->disconnect();
  delete channel;

  if ( nullptr == resp ) {
    time_log << tr("--> ответ не получен");
    error_msg << tr("Ошибка: превышено время ожидания ответа от сервиса.");
    qApp->restoreOverrideCursor();
    return QList<TMeteoData>();
  }

  //time_log << tr("--> получено наборов данных %1").arg(resp->data_size());

  QList<TMeteoData> factList = toMeteoData(*resp);
  delete resp;
  qApp->restoreOverrideCursor();

  return factList;
}

void MeteogramWindow::updateCenters(const field::DataRequest& request)
{

  QComboBox* centers = centerCombo();
  centers->clear();
  qApp->setOverrideCursor(Qt::WaitCursor);

  rpc::Channel* channel = global::serviceChannel(settings::proto::kField);
  if ( nullptr == channel ) {
    qApp->restoreOverrideCursor();
    centers->insertItem(0, tr("<ошибка при обновлении списка>"), -1);
    return;
  }

  //time_log << tr("Запрос доступных центров.");

//  var(request.Utf8DebugString());

  field::CentersResponse* resp = nullptr;
  resp = channel->remoteCall(&field::FieldService::GetAvailableCentersForecast, request, 90000);

  /*field::CentersResponse* resp = 0;
  resp = channel->remoteCall(&field::FieldService::GetAvailableCenters, request, 90000);
   */
  channel->disconnect();
  delete channel;

  if ( nullptr == resp /*|| 0 == forecastResp*/ ) {
    time_log << tr("--> ответ не получен");
    centers->insertItem(0, tr("<ошибка при обновлении списка>"), -1);
    qApp->restoreOverrideCursor();
    return;
  }

  //resp->MergeFrom(*forecastResp);

  QMap<int,int> sortMap;
  for ( int i=0,isz=resp->info_size(); i<isz; ++i ) {
    sortMap.insert(resp->info(i).priority(), i);
  }

  QMapIterator<int,int> it(sortMap);
  while ( it.hasNext() ) {
    it.next();

    centers->addItem(pbtools::toQString(resp->info(it.value()).name()), resp->info(it.value()).number());
  }

  if ( centers->count() == 0 ) {
    centers->addItem(tr("<нет данных>"), -1);
  }
  else {
    centers->insertItem(0, tr("<автоматический выбор>"), -1);
    centers->setCurrentIndex(0);
  }

  for ( int i=0,isz=centers->count(); i<isz; ++i ) {
    if ( centers->itemData(i).toInt() == lastSelectedCenter_ ) {
      centers->setCurrentIndex(i);
      break;
    }
  }
  qApp->restoreOverrideCursor();
}

int MeteogramWindow::calcMin(float min, float max) const
{
  float delta = qAbs(min - max);
  return std::floor(min - delta*0.05);
}

int MeteogramWindow::calcMax(float min, float max) const
{
  float delta = qAbs(min - max);
  return std::ceil(max + delta*0.05);
}

void MeteogramWindow::merge(QList<TMeteoData>* list, const QList<TMeteoData>& from) const
{
  assert_log( nullptr != list );

  if ( nullptr == list ) { return; }

  QHash<QDateTime, int> hash;
  for ( int i=0,isz=list->size(); i<isz; ++i ) {
    hash.insert(TMeteoDescriptor::instance()->dateTime(list->at(i)), i);
  }

  QList<descr_t> descrs;
  descrs << TMeteoDescriptor::instance()->descriptor("T");
  descrs << TMeteoDescriptor::instance()->descriptor("Td");
  descrs << TMeteoDescriptor::instance()->descriptor("U");
  descrs << TMeteoDescriptor::instance()->descriptor("P");
  descrs << TMeteoDescriptor::instance()->descriptor("dd");
  descrs << TMeteoDescriptor::instance()->descriptor("ff");

  QDateTime dt;
  for ( int i=0,isz=from.size(); i<isz; ++i ) {
    dt = TMeteoDescriptor::instance()->dateTime(from[i]);
    if ( !hash.contains(dt) ) {
      list->append(from[i]);
    }
    else {
      int j = hash.value(dt);
      (*list)[j] = merge(list->at(j), from[i], descrs);
    }
  }
}

TMeteoData MeteogramWindow::merge(const TMeteoData& to, const TMeteoData& from, const QList<descr_t>& descrs) const
{
  TMeteoData md = to;
  foreach ( descr_t d, descrs ) {
    if ( !md.hasParam(d) && from.hasParam(d) ) {
      md.add(d, from.getParam(d));
    }
  }
  return md;
}

QList<TMeteoData> MeteogramWindow::toMeteoData(const field::ValueDataReply& resp) const
{
  // исключаем дубликаты параметров за один срок
  QSet<FieldData> filtered;
  for ( int i=0,isz=resp.data_size(); i<isz; ++i ) {
    const field::OnePointData& d = resp.data(i);
    QString dts = pbtools::toQString(d.fdesc().date()).replace("T", " ");
    QDateTime dt = QDateTime::fromString(dts, Qt::ISODate);
    dt = dt.addSecs(d.fdesc().hour());

    FieldData data;
    data.descr = d.fdesc().meteodescr();
    data.dt = dt;
    data.hour = d.fdesc().hour();
    data.text = pbtools::toQString(d.text());
    data.value = d.value();

    QSet<FieldData>::const_iterator it = filtered.constFind(data);
    if ( it == filtered.constEnd() || d.fdesc().hour() < it->hour  ) {
      filtered.insert(data);
    }
  }

  QHash<QString,TMeteoData> mdHash;

  foreach ( const FieldData& data, filtered ) {
    QString dts = data.dt.toString();

    if ( !mdHash.contains(dts) ) {
      TMeteoData md;
      md.setDateTime(data.dt);
      md.add("h0", TMeteoParam("", 0, control::RIGHT)); // special case: для наноски пуансонов
      mdHash.insert(dts, md);
    }

    TMeteoParam param(data.text, data.value, control::RIGHT);
    mdHash[dts].add(data.descr, param);
  }

  return mdHash.values();
}

void MeteogramWindow::buildGraph(QList<TMeteoData> &mdList,
                                 bool isBuildT,
                                 bool isBuildDewPoint,
                                 bool isBuildP,
                                 bool isBuildPQNH,
                                 bool isBuildU,
                                 bool isBuildWind
                                 )
{

  qSort(mdList.begin(), mdList.end(), lessThan);

  foreach ( map::Layer* l, document_->layers() ) {
    if ( document_->gridLayer() != l ) {
      delete l;
    }
  }

  float minT = std::numeric_limits<float>::max();
  float maxT = std::numeric_limits<float>::min();


  layers_.clear();
  QList<map::LayerGraph*> temperLayers;

  // температура
  if ( mdList.size() != 0 && isBuildT ) {
    map::LayerGraph* l = addLayerT(mdList);

    meteo::Property prop;
    prop.mutable_pen()->set_width(2);
    prop.mutable_pen()->set_color(qRgba(220,0,0,255));
    l->setGraphProperty(prop);

    minT = qMin(minT, l->yAxis()->lower());
    maxT = qMax(maxT, l->yAxis()->upper());
    temperLayers << l;

    layers_ << l;
  }

  // температура точки росы
  if ( mdList.size() != 0 && isBuildDewPoint ) {
    map::LayerGraph* l = addLayerTd(mdList);

    meteo::Property prop;
    prop.mutable_pen()->set_width(2);
    prop.mutable_pen()->set_color(qRgba(220,0,0,255));
    prop.mutable_pen()->set_style(kDashLine);
    l->setGraphProperty(prop);

    minT = qMin(minT, l->yAxis()->lower());
    maxT = qMax(maxT, l->yAxis()->upper());
    temperLayers << l;

    layers_ << l;
  }

  // настройка шкалы температуры
  float deltaT = qAbs(minT - maxT);
  minT = std::floor(minT - deltaT*0.05);
  maxT = std::ceil(maxT + deltaT*0.05);
  for ( int i=0,isz=temperLayers.size(); i<isz; ++i ) {
    temperLayers[i]->yAxis()->setRange(minT,maxT);
    temperLayers[i]->update();

    QVector<float> ticks;
    for ( float i=minT; i<=maxT; i+=1 ) {
      ticks << i;
    }
    temperLayers[i]->yAxis()->setTickVector(ticks, 20);
    temperLayers[i]->yAxis()->setTickLabelFormat("f,1", 20);

    ticks.clear();
    for ( int i= minT*10; i<=maxT*10; i+=5 ) {
      ticks << i/10;
    }
    temperLayers[i]->yAxis()->setTickVector(ticks, 16);
    temperLayers[i]->yAxis()->setTickLabelFormat("f,1", 16);
  }

  // давление
  if ( mdList.size() != 0 && isBuildP ) {
    map::LayerGraph* l = addLayerP(mdList);

    meteo::Property prop;
    prop.mutable_pen()->set_width(2);
    prop.mutable_pen()->set_color(qRgba(20,20,150,255));
    l->setGraphProperty(prop);

    int min = calcMin(l->yAxis()->lower(), l->yAxis()->upper());
    int max = calcMax(l->yAxis()->lower(), l->yAxis()->upper());
    l->yAxis()->setRange(min,max);

    QVector<float> ticks;
    for ( float i=min; i<=max; i+=1 ) {
      ticks << i;
    }
    l->yAxis()->setTickVector(ticks, 20);
    l->yAxis()->setTickLabelFormat(tr("f,1"), 20);

    ticks.clear();

    for ( int i= min*10; i<=max*10; i+=5 ) {
      ticks << i/10;
    }

    l->yAxis()->setTickVector(ticks, 16);
    l->yAxis()->setTickLabelFormat(tr("f,1"), 16);

    layers_ << l;
  }

  // давление QNH
  if ( mdList.size() != 0 && isBuildPQNH ) {
    map::LayerGraph* l = addLayerPQNH(mdList);

    meteo::Property prop;
    prop.mutable_pen()->set_width(2);
    prop.mutable_pen()->set_color(qRgba(20,20,150,255));
    l->setGraphProperty(prop);

    int min = calcMin(l->yAxis()->lower(), l->yAxis()->upper());
    int max = calcMax(l->yAxis()->lower(), l->yAxis()->upper());
    l->yAxis()->setRange(min,max);

    QVector<float> ticks;
    for ( float i=min; i<=max; i+=1 ) {
      ticks << i;
    }
    l->yAxis()->setTickVector(ticks, 20);
    l->yAxis()->setTickLabelFormat(tr("f,1"), 20);

    ticks.clear();
    for ( int i= min*10; i<=max*10; i+=5 ) {
      ticks << i/10;
    }

    l->yAxis()->setTickVector(ticks, 16);
    l->yAxis()->setTickLabelFormat(tr("f,1"), 16);

    layers_ << l;
  }

  // влажность
  if ( mdList.size() != 0 && isBuildU ) {
    map::LayerGraph* l = addLayerU(mdList);

    meteo::Property prop;
    prop.mutable_pen()->set_width(2);
    prop.mutable_pen()->set_color(qRgba(200,200,0,255));
    l->setGraphProperty(prop);

    int min = calcMin(l->yAxis()->lower(), l->yAxis()->upper());
    int max = calcMax(l->yAxis()->lower(), l->yAxis()->upper());
    l->yAxis()->setRange(min,max);

    QVector<float> ticks;
    for ( int i=min; i<=max; ++i ) {
      ticks << i;
    }
    l->yAxis()->setTickVector(ticks, 20);
    l->yAxis()->setTickLabelFormat("f,0", 20);

    ticks.clear();
    for ( int i= min*10; i<=max*10; i+=5 ) {
      ticks << i/10;
    }

    l->yAxis()->setTickVector(ticks, 16);
    l->yAxis()->setTickLabelFormat("f,1", 16);

    layers_ << l;
  }

  // ветер
  if ( mdList.size() != 0 && isBuildWind ) {
    map::LayerGraph* l = addLayerWind(mdList);

    meteo::Property prop;
    prop.mutable_pen()->set_color(qRgba(0,0,0,0));
    l->setGraphProperty(prop);

    layers_ << l;
  }

  // настройка шкалы времени
  for ( int i=0,isz=layers_.size(); i<isz; ++i ) {
    meteo::Property prop;
    prop.mutable_pen()->set_width(1);
    prop.mutable_pen()->set_color(qRgba(0,0,0,255));
    layers_[i]->xAxis()->setProperty(prop);
    layers_[i]->xAxis()->setSubTickCount(2);

    int min = std::floor(layers_[i]->xAxis()->lower());
    int max = std::ceil(layers_[i]->xAxis()->upper());

    QDateTime begDt = QDateTime::fromTime_t(min);
    QDateTime endDt = QDateTime::fromTime_t(max);
    QList< QPair<int,int> > params;
    if ( begDt.daysTo(endDt) >= 3 ) {
      params << QPair<int,int>(14400, 20);
      params << QPair<int,int>(7200, 18);
      params << QPair<int,int>(3600, 16);
      params << QPair<int,int>(1800, 15);
      params << QPair<int,int>(900, 14);
    }
    else {
      params << QPair<int,int>(3600, 20);
      params << QPair<int,int>(1800, 16);
      params << QPair<int,int>(900, 15);
    }

    for ( int j=0,isz=params.size(); j<isz; ++j ) {
      QVector<float> ticks;
      for ( int k=min; k<=max; k+=params[j].first ) {
        ticks << k;
      }
      layers_[i]->xAxis()->setTickLabelFormat("dd MMM\n hh:mm", params[j].second);
      layers_[i]->xAxis()->setTickVector(ticks, params[j].second);
      layers_[i]->update();
    }
  }

  // скрываем слои для которых отсутствуют данные
  for ( int i=0,isz=layers_.size(); i<isz; ++i ) {
    layers_[i]->setVisisble(!layers_[i]->graphCoords().isEmpty());
  }

  isContainsMap_ = true;
}



void MeteogramWindow::setButtonsState(bool isBuildT, bool isBuildDewPoint, bool isBuildP, bool isBuildU, bool isBuildPQNH, bool isBuildWind){
  checkT()->setChecked(isBuildT);
  checkTd()->setChecked(isBuildDewPoint);
  checkP()->setChecked(isBuildP);
  checkU()->setChecked(isBuildU);
  checkPQNH()->setChecked(isBuildPQNH);
  checkWind()->setChecked(isBuildWind);
}

void MeteogramWindow::generateDoc(QList<TMeteoData> &mdList)
{
  buildGraph(mdList,
             checkT()->isChecked(),
             checkTd()->isChecked(),
             checkP()->isChecked(),
             checkPQNH()->isChecked(),
             checkU()->isChecked(),
             checkWind()->isChecked());

  QDateTime dtStart, dtEnd;

  if ( !layers_.isEmpty() ) {
    int beg = std::floor(layers_.first()->xAxis()->lower());
    int end = std::ceil(layers_.first()->xAxis()->upper());
    dtStart = QDateTime::fromTime_t(beg);
    dtEnd = QDateTime::fromTime_t(end);
 }
  QString title = "Пустой документ" ;
  if(nullptr != stationEdit()){
    title = stationEdit()->stationIndex().isNull()? "Пустой документ" :
                                                         setupTitle(stationEdit()->ruName(),
                                                                    stationEdit()->stationIndex(),
                                                                                      dtStart, dtEnd,
                                                                                      stationEdit()->coord());
   }

  if(nullptr != document_){
      document_->setName(title);
      QList<map::Incut*> list = document_->incuts();
      if( 0 != list.size() ) {
        setIncut(list[0],dtStart, dtEnd);
      }

    }
  emit docGenerated(title);


}

QString MeteogramWindow::setupTitle(const QString &station, const QString &stationIndex,
                                    const QDateTime &dtStart, const QDateTime &dtEnd, meteo::GeoPoint stationCoord){
  QString finalTitle = kTitle;
  if ( !dtStart.isNull() && !dtEnd.isNull())
      finalTitle += tr(" - с %1 по %2").arg(dtStart.toString("hh:mm dd.MM.yy"), dtEnd.toString("hh:mm dd.MM.yy"));

  finalTitle += tr(" %1 (%2, %3)").arg(station)
                                 .arg(stationIndex)
                                 .arg(stationCoord.toString(true, "%1 %2 %3"));
  return finalTitle;
}
bool MeteogramWindow::setIncut(meteo::map::Incut *incut,const QDateTime &dtStart, const QDateTime &dtEnd)
{
  if(!incut) return false;
  incut->setMapName(kTitle);
  incut->setLabel("begdt",tr("C %1 по %2").arg(dtStart.toString("hh:mm dd.MM.yy"), dtEnd.toString("hh:mm dd.MM.yy")));
  if ( !layers_.isEmpty()&& nullptr != stationEdit()) {
    if ( isByStation() ) {
        incut->setLabel("station", tr("Станция: %1 (%2)").arg(stationEdit()->ruName()).arg(stationEdit()->stationIndex()));
     }
      else {
        incut->setLabel("station", QString());
      }
      incut->setLabel("coords", tr("Координаты: %1").arg(stationEdit()->coord().toString(false, "%1%2")));
      int c = centerCombo()->itemData(centerCombo()->currentIndex()).toInt();
      incut->setLabel("center", -1 == c ? QString() : tr("Центр: %1").arg(centerCombo()->currentText()));
   }

  incut->setWMOHeader(false);
  incut->setScale(false);
  return true;
}


void MeteogramWindow::askRemote(){
  if(nullptr == stationEdit()) return;
  QMessageBox::StandardButton reply;
  reply = QMessageBox::question(this, QObject::tr("Требуется подтверждение"), "Отсутствуют данные локально. Запросить данные с удаленного сервера?",
                                QMessageBox::Yes|QMessageBox::No);
  if (reply == QMessageBox::Yes) {
    meteo::ui::proto::MeteogramWindowState args;
    args.set_isbuildt(checkT()->isChecked());
    args.set_isbuilddewpoint(checkTd()->isChecked());
    args.set_isbuildp(checkP()->isChecked());
    args.set_isbuildu(checkU()->isChecked());
    args.set_isbuildpqnh(checkPQNH()->isChecked());
    args.set_isbuildwind(checkWind()->isChecked());
    args.set_isbystation(isByStation());
    args.set_begindatetime(beginDateTime().toString(Qt::ISODate).toStdString());
    args.set_enddatetime(endDateTime().toString(Qt::ISODate).toStdString());

    args.set_stationindex(stationEdit()->stationIndex().toStdString());
    args.set_stationtype(stationEdit()->stationType());

    meteo::surf::DataRequest req;
    this->getLoadSrcDataRequest(&req,stationEdit()->stationIndex());
    req.set_use_parent(true);
    QString message = QString("станция %1 за срок %2 - %3")
        .arg(stationEdit()->stationIndex())
        .arg(beginDateTime().toString())
        .arg(endDateTime().toString());
    req.set_notify_message(message.toStdString());
    req.set_notify_action("Построить метеограмму");

    rpc::Channel* channel = global::serviceChannel(settings::proto::kSrcData);
    if ( nullptr == channel ) {
      error_msg << tr("Ошибка: отсутствует подключение к сервису данных.");
      return ;
    }

  //  time_log << tr("Запрос данных (srcdata.service).");

    surf::DataReply* resp = channel->remoteCall(&::meteo::surf::SurfaceService::GetMeteoDataOnStation, req, 90000);
    channel->disconnect();
    delete channel;

    if ( nullptr == resp ) {
      time_log << tr("--> ответ не получен");
      error_msg << tr("Ошибка: превышено время ожидания ответа от сервиса.");
      return;
    }
   // QString requestID = QString::fromStdString(resp->request_id());
    delete resp;

    //FIXME MeteogramPlugin::instance()->addCallback(requestID, args);
  }
}

void MeteogramWindow::slotRun()
{
  if ( nullptr == document_ ) { return; }
  if(nullptr == stationEdit()) return;
  if ( isByStation() && stationEdit()->stationIndex().isEmpty() ) {
    info_msg << tr("Для построения Метеограммы необходимо выбрать станцию.");
    return;
  }
  if ( !checkT()->isChecked()
       && !checkTd()->isChecked()
       && !checkP()->isChecked()
       && !checkU()->isChecked()
       &&!checkWind()->isChecked()) {
      info_msg << tr("Для построения Метеограммы необходимо выбрать параметр.");
  }

  qApp->setOverrideCursor(Qt::WaitCursor);
  setEnabled(false);

  QList<TMeteoData> mdList;
  if ( isByStation() ) {
    mdList = loadSrcData(false,stationEdit()->stationIndex());
  }
  else {
    mdList = loadFieldData();
  }

  if (isByStation() && mdList.isEmpty()) {
    askRemote();
  } else {
    if (!mdList.isEmpty()) {
      generateDoc(mdList);
    }
  }
  setEnabled(true);
  qApp->restoreOverrideCursor();

}

void MeteogramWindow::slotUpdateCenters()
{
  if ( isByStation() ) { return; }
  field::DataRequest req;
  req.set_date_start(pbtools::toString(beginDateTime().toString(Qt::ISODate)));
  req.set_date_end(pbtools::toString(endDateTime().toString(Qt::ISODate)));
  bool is_empty = true;
  if ( checkT()->isChecked() ) {
    req.add_meteo_descr(TMeteoDescriptor::instance()->descriptor("T"));
    is_empty = false;
  }
  if ( checkTd()->isChecked() ) {
    req.add_meteo_descr(TMeteoDescriptor::instance()->descriptor("Td"));
    is_empty = false;
  }
  if ( checkU()->isChecked() ) {
    req.add_meteo_descr(TMeteoDescriptor::instance()->descriptor("U"));
    is_empty = false;
  }
  if ( checkP()->isChecked() ) {
    req.add_meteo_descr(TMeteoDescriptor::instance()->descriptor("P"));
    is_empty = false;
  }
  if ( checkPQNH()->isChecked() ) {
    req.add_meteo_descr(TMeteoDescriptor::instance()->descriptor("PQNH"));
    is_empty = false;
  }
  if ( checkWind()->isChecked() ) {
    req.add_meteo_descr(TMeteoDescriptor::instance()->descriptor("dd"));
    req.add_meteo_descr(TMeteoDescriptor::instance()->descriptor("ff"));
    is_empty = false;
  }

  if ( is_empty ) { return; }
  req.add_level(0);
  req.add_type_level(1);
  qApp->setOverrideCursor(Qt::WaitCursor);

  setEnabled(false);
  updateCenters(req);
  setEnabled(true);
  qApp->restoreOverrideCursor();
}

void MeteogramWindow::slotLoadState()
{
  QSettings s(MnCommon::userSettingsPath() + "meteogram.conf", QSettings::IniFormat);
  if ( s.contains("check_T") ) {
    checkT()->setChecked(s.value("check_T").toBool());
  }
  if ( s.contains("check_P") ) {
    checkP()->setChecked(s.value("check_P").toBool());
  }
  if ( s.contains("check_PQNH") ) {
    checkPQNH()->setChecked(s.value("check_PQNH").toBool());
  }
  if ( s.contains("check_Td") ) {
    checkTd()->setChecked(s.value("check_Td").toBool());
  }
  if ( s.contains("check_U") ) {
    checkU()->setChecked(s.value("check_U").toBool());
  }
  if ( s.contains("check_wind") ) {
    checkWind()->setChecked(s.value("check_wind").toBool());
  }
}

void MeteogramWindow::slotSaveState() const
{
  QSettings s(MnCommon::userSettingsPath() + "meteogram.conf", QSettings::IniFormat);
  s.setValue("check_T", checkT()->isChecked());
  s.setValue("check_P", checkP()->isChecked());
  s.setValue("check_PQNH", checkPQNH()->isChecked());
  s.setValue("check_Td", checkTd()->isChecked());
  s.setValue("check_U", checkU()->isChecked());
  s.setValue("check_wind", checkWind()->isChecked());
}

void MeteogramWindow::slotCenterChanged(int idx)
{
  lastSelectedCenter_ = centerCombo()->itemData(idx).toInt();
}


map::MapWindow* MeteogramWindow::createWindow(app::MainWindow *mwindow) {
  map::proto::Document blank;
  blank.set_doctype( meteo::map::proto::kMeteogram );
  blank.set_projection(kGeneral);
  GeoPointPb* gp = blank.mutable_map_center();
  gp->set_lat_radian(0);
  gp->set_lon_radian(0);
  gp->set_type( kLA_GENERAL );
  gp = blank.mutable_doc_center();
  gp->set_lat_radian(500);
  gp->set_lon_radian(-250);
  gp->set_type( kLA_GENERAL );
  blank.set_punchorient( map::proto::kNoOrient );
  blank.set_scale(20);
  blank.set_smooth_iso(true);

  map::MapWindow* mapWindow = new map::MapWindow(mwindow, blank, "", MnCommon::etcPath("meteo") + "/tasks.menu.conf");
  mapWindow->addPluginGroup("meteo.map.common");
  mapWindow->setWindowTitle(tr("Метеограммы"));

  if (nullptr != mwindow) {
    mwindow->toMdi(mapWindow);
  }

  delete mapWindow->mapscene()->takeAction("mousecoordaction");
  delete mapWindow->mapscene()->takeAction("ruleaction");
  delete mapWindow->mapscene()->takeAction("focusaction");

  map::ScaleAction* scaleact = qobject_cast<map::ScaleAction*>(mapWindow->mapscene()->getAction("scaleaction"));
  if (nullptr != scaleact) {
    scaleact->setMinScale(20);
    scaleact->setMaxScale(14);
    scaleact->setScaleSeparator(120.*20);
  }

  map::GraphCoordAction* grCoordAct = new map::GraphCoordAction(mapWindow->mapscene());
  mapWindow->mapscene()->addAction(grCoordAct, kBottomLeft, QPoint(0,0));

  map::GraphValueAction* grValAct = new map::GraphValueAction(mapWindow->mapscene());
  mapWindow->mapscene()->addAction(grValAct, kBottomLeft, QPoint(0,0));
  return mapWindow;
}

map::Document* MeteogramWindow::createDocument(map::MapWindow *mapWindow){

  map::Document* document = mapWindow->document();
  GeneralProj* proj = projection_cast<GeneralProj*>(document->projection());
  if (nullptr != proj) {
    proj->setFuncTransform(graphLinearF2X_one, graphLinearX2F_one);
    proj->setFuncTransform(graphLinearF2X_onef, graphLinearX2F_onef);
    proj->setRamka(0,0,500,500);
    proj->setStart(GeoPoint(0,0,0,LA_GENERAL));
    proj->setEnd(GeoPoint(500,500,0,LA_GENERAL));
    proj->setXfactor(16./9.);
  }

  RamkaGraph* ramka = new RamkaGraph(document);
  document->setRamka(ramka);
  return document;
}

} // meteo
