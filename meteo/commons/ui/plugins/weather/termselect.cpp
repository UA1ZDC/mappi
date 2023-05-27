#include "termselect.h"

#include <qstringlist.h>
#include <qdir.h>
#include <qfile.h>
#include <qbytearray.h>

#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/paths.h>

#include <meteo/commons/global/global.h>
#include <commons/textproto/tprototext.h>
#include <meteo/commons/global/gradientparams.h>
#include <meteo/commons/ui/map/weather.h>
#include <meteo/commons/ui/mainwindow/widgethandler.h>
#include <meteo/commons/ui/mainwindow/mainwindow.h>
#include <meteo/commons/ui/custom/filterheader.h>
#include <sql/psql/psqlquery.h>
#include <meteo/commons/proto/sprinf.pb.h>

#include <meteo/commons/global/weatherloader.h>
#include <commons/meteo_data/tmeteodescr.h>

#include "weatheritem.h"
#include "ui_termselect.h"

namespace {
const int kWeatherInfoType = Qt::UserRole + 1;
}

namespace meteo {
namespace map {
namespace weather {

const QString kDateFormat("yyyy_MM_dd_hh_mm_ss");

TermSelect::TermSelect( const QMap< QString, proto::Map >& types, MapWindow* window )
  : MapWidget(window),
  ui_( new Ui::TermSelect ),
  types_(types)
{
  ui_->setupUi(this);
  FilterHeader* hdr = new FilterHeader(ui_->mapstree);
  ui_->mapstree->setHeader(hdr);
  hdr->setSectionsClickable(true);
  ui_->mapstree->setRootIsDecorated(true);

  loadTypesInList();
  ui_->deweatherterm->setDateTime( QDateTime::currentDateTimeUtc() );

  QObject::connect( ui_->mapstree, SIGNAL( itemSelectionChanged() ), this, SLOT( slotItemSelectedChanged() ) );
  QObject::connect( ui_->cmbmaptype, SIGNAL( currentIndexChanged(int) ), this, SLOT( slotCurrentMapIndexChanged(int) ) );
  QObject::connect( ui_->deweatherterm, SIGNAL( dateChanged( const QDate& ) ), this, SLOT( slotDateChanged( const QDate& ) ) );
  QObject::connect( ui_->cmbspline, SIGNAL( currentIndexChanged( int ) ), this, SLOT( slotSpinSplineKoefChanged( int ) ) );
  QObject::connect( ui_->hourCombo, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(slotHourChange(const QString &)) );

  QObject::connect( ui_->okbtn, SIGNAL( clicked() ), this, SLOT( slotOk() ) );
  QObject::connect( ui_->rmbtn, SIGNAL( clicked() ), this, SLOT( slotRmMap() ) );
  QObject::connect( ui_->nobtn, SIGNAL( clicked() ), this, SLOT( slotNo() ) );
  QObject::connect( ui_->mapstree, SIGNAL(customContextMenuRequested(const QPoint& ) ), this, SLOT(slotContextMenu( const QPoint& ) ) );
  QObject::connect( ui_->mapstree, SIGNAL( itemDoubleClicked(QTreeWidgetItem *, int ) ), this, SLOT( slotDblCl(QTreeWidgetItem *, int) ) );

  menu_ = new QMenu(this);
  QAction* add = new QAction(QIcon(":/meteo/icons/tools/plus.png"),"Добавить карту", menu_);
  QObject::connect( add, SIGNAL(triggered()), SLOT(slotAddMap()));
  add->setIconVisibleInMenu(true);
  menu_->addAction(add);
  QAction* del = new QAction(QIcon(":/meteo/icons/tools/minus.png"),"Удалить карту", menu_);
  QObject::connect( del, SIGNAL(triggered()), SLOT(slotRmMap()));
  del->setIconVisibleInMenu(true);
  menu_->addAction(del);
  ui_->mapstree->setContextMenuPolicy(Qt::CustomContextMenu);
  if( nullptr != mapdocument() && nullptr != mapdocument()->eventHandler() ){
    mapdocument()->eventHandler()->installEventFilter(this);
  }
}

TermSelect::~TermSelect()
{
  delete menu_; menu_ = nullptr;
  delete ui_; ui_ = nullptr;
}

void TermSelect::setCurrentMapType( const QString& type )
{
  QMapIterator<QString,proto::Map> it(types_);
  while ( true == it.hasNext() ) {
    it.next();
    const proto::Map& map = it.value();
    if ( QString::fromStdString( map.title() ) == type ) {
      setCurrentMapType(map);
      ui_->hourCombo->setCurrentIndex(1);
      return;
    }
  }
  error_log << QObject::tr("Неизвестный тип карты %1").arg(type);
}

void TermSelect::setCurrentMapType( const proto::Map& map )
{
  currentmap_ = map;
  QString title = QString::fromStdString( map.title() );
  if ( false == types_.contains( title ) ) {
    types_.insert( title, map );
  }
  int indx = ui_->cmbmaptype->findText( QString::fromStdString( map.title() ) );
  if ( -1 == indx ) {
    warning_log << QObject::tr("Неизвестный тип карты") << map.title();
    indx = 0;
  }
  ui_->cmbmaptype->setCurrentIndex(indx);
  if ( true == map.has_spline_koef() ) {
    int spline_koef = map.spline_koef();
    if ( 0 < spline_koef ) {
      ui_->cmbspline->setCurrentIndex( spline_koef - 1 );
    }
    else {
      if ( 0 > spline_koef ) {
        spline_koef = 0;
      }
      ui_->cmbspline->setCurrentIndex( spline_koef );
    }
  }
  setWindowTitle( QObject::tr("%1").arg(title) );
  loadTermsForCurrentMap();
}

void TermSelect::keyReleaseEvent( QKeyEvent* e )
{
  if ( Qt::Key_Escape == e->key() ) {
    QWidget::close();
    e->accept();
    return;
  }

  QWidget::keyReleaseEvent(e);
}

bool TermSelect::eventFilter( QObject* watched, QEvent* ev )
{
  if( nullptr == mapdocument() ){
    return MapWidget::eventFilter(watched, ev);
  }
  if ( watched == mapdocument()->eventHandler() ) {
    if ( meteo::map::LayerEvent::LayerChanged == ev->type()) {
      LayerEvent* lev = static_cast<LayerEvent*>(ev);
      if ( lev->changeType() != LayerEvent::ObjectChanged) {
  updateItems();
      }
    }
  }
  return MapWidget::eventFilter(watched, ev);
}

void TermSelect::loadTypesInList()
{
  ui_->cmbmaptype->clear();
  QStringList titles;
  QStringList ats;
  QMap<QString, proto::Map>::iterator it = types_.begin();
  QMap<QString, proto::Map>::iterator end = types_.end();
  for ( ; it != end; ++it ) {
    proto::Map& map = it.value();
    if ( true == QString::fromStdString(map.title()).contains(QObject::tr("АТ-")) ||
         true == QString::fromStdString(map.title()).contains(QObject::tr("ОТ"))) {
      ats.append(QString::fromStdString(map.title()));
    }
    else {
      titles.append(QString::fromStdString(map.title()));
    }
  }
  std::sort(ats.begin(), ats.end(), [=]( const QString& a, const QString& b) {
    int aNum, bNum;
    if ( true == a.contains("ОТ")) {
      aNum = 15000;
    }
    else {
      auto copy = a;
      aNum = copy.remove(QObject::tr("АТ-")).toInt();
    }
    if ( true == b.contains(QObject::tr("ОТ"))) {
      bNum = 15000;
    }
    else {
      auto copy = b;
      bNum = copy.remove(QObject::tr("АТ-")).toInt();
    }
    if ( aNum > bNum ) {
      return true;
    }
    else {
      return false;
    }
  });
  for( auto title : titles ) {
    ui_->cmbmaptype->addItem( title );
  }
  for ( auto at : ats ) {
    ui_->cmbmaptype->addItem( at );
  }
}

void TermSelect::loadTermsForCurrentMap()
{
  weatherMaps_.clear();
  ui_->mapstree->clear();
  if(0 == currentmap_.data_size()) {
    error_log << QObject::tr("Шаблон погоды %1 не корректный. В нем не выбран ни один вид данных, который необходимо отобразаить в документе")
                 .arg(QString::fromStdString(currentmap_.title()));
    return;
  }

  QDate date = ui_->deweatherterm->dateTime().date();
  if(false == date.isValid()) {
    error_log << QObject::tr("Выберите корректную дату, за которую необходимо получить данные для карт погоды");
    return;
  }

  QApplication::setOverrideCursor(Qt::WaitCursor);

//  QDateTime dtbeg = QDateTime(date, QTime(0, 0, 0));
 // QDateTime dtend = QDateTime(date, QTime(23, 59, 59));

 // Weather weather;
//  proto::MapList allmaps = weather.getAvailableMaps(currentmap_, dtbeg, dtend);
  getAvailableDataForLayers();
  QApplication::restoreOverrideCursor();

  QTreeWidgetItem* item = nullptr;
  for(auto mapStr : weatherMaps_.keys()) {

    proto::Map map;
    map.ParseFromString(mapStr);

    for(proto::WeatherLayer& layer : weatherMaps_[mapStr]) {
      auto l = map.add_data();
      l->CopyFrom(layer);
    }

    QDateTime dt = QDateTime::fromString(QString::fromStdString(map.datetime()), Qt::ISODate);
    int hour     = map.hour();
    int model    = map.model();

    if(nullptr == item) {
      item = new WeatherItem(ui_->mapstree);
    }
    else {
      item = new WeatherItem(ui_->mapstree, item);
    }

    QSize qsz = item->sizeHint(0);
    qsz.setHeight(35);
    item->setSizeHint(0, qsz);

    std::string str;
    if(false == map.SerializeToString(&str)) {
      error_log << QObject::tr("Не удалось сериализовать информацию о данных карты \n\t = %1")
                   .arg(QString::fromStdString(map.Utf8DebugString()));
      continue;
    }

    QByteArray arr(str.data(), str.size());
    item->setData(WeatherItem::kTermClmn, kWeatherInfoType, QVariant(arr));
    item->setText(WeatherItem::kTermClmn, dt.addSecs(hour * 3600).toString("hh:mm:ss"));

    QString center = QString::number(map.center());
    QMap<int, QPair<QString, int>> centers = global::kMeteoCenters();
    if(centers.contains(map.center())) {
      center = centers[map.center()].first;
    }

    for(proto::WeatherLayer& layer : weatherMaps_[mapStr]) {
      std::string lstr;
      layer.SerializeToString(&lstr);

      QTreeWidgetItem* child = new QTreeWidgetItem(item);
      QByteArray larr(lstr.data(), lstr.size());

      child->setData(WeatherItem::kTermClmn, kWeatherInfoType, QVariant(larr));
      child->setFirstColumnSpanned(true);
      child->setText(0, layernameFromInfo(layer));

      QSize chsz = child->sizeHint(0);
      chsz.setHeight(30);
      child->setSizeHint(0, chsz);
    }

    item->setText(WeatherItem::kCenterClmn , center);
    item->setText(WeatherItem::kHourClmn   , QString::number(hour));
    item->setText(WeatherItem::kModelClmn  , QString::number(model));
    item->setText(WeatherItem::kLevelClmn  , QString::number(map.level()));
//    item->setText(WeatherItem::kLvlTypeClmn, QString::number(map.type_level()));
  }
  if(0 != ui_->mapstree->topLevelItemCount()) {
    ui_->mapstree->topLevelItem(0)->setSelected(true);
  }
  slotHourChange(ui_->hourCombo->currentText());
  updateItems();

  QApplication::restoreOverrideCursor();
}

void TermSelect::getAvailableDataForLayers()
{
  for ( int i = 0, sz = currentmap_.data_size(); i < sz; ++i ) {
    getAvailableData(currentmap_.data(i));
  }
}

void TermSelect::getAvailableData(proto::WeatherLayer wl)
{
  QDate date = ui_->deweatherterm->dateTime().date();
  if(false == date.isValid()) {
    error_log << QObject::tr("Выберите корректную дату, за которую необходимо получить данные для карт погоды");
    return;
  }

//  QApplication::setOverrideCursor(Qt::WaitCursor);

  QDateTime dtbeg = QDateTime(date, QTime(0, 0, 0));
  QDateTime dtend = QDateTime(date, QTime(23, 59, 59));

  if ( proto::kPuanson == wl.mode() ) {
    auto& punches = meteo::map::WeatherLoader::instance()->punchlibrary();
    if ( false == punches.contains(QString::fromStdString(wl.template_name()))) {
      error_log << QObject::tr("Не найден шаблон пуансона %1").arg(QString::fromStdString(wl.template_name()));
      return;
    }
    auto punch = punches[QString::fromStdString(wl.template_name())];

    if ( proto::kSurface == wl.source() || proto::kAuto == wl.source() ) {
      getSrc(wl, dtbeg, dtend, punch);
    }
    if ( proto::kField == wl.source() || proto::kAuto == wl.source() ) {
      getField(wl, dtbeg, dtend, punch);
    }
  }
  else if ( proto::kRadar == wl.mode() ) {
    auto& clrs = WeatherLoader::instance()->radarlibrary();
    QString clrname = QString::fromStdString( wl.template_name() );
    if ( false == clrs.contains(clrname) ) {
      return;
    }
    else {
      getRadar(wl, dtbeg, dtend, clrs[clrname]);
    }
  }
  else if ( proto::kIsoline == wl.mode() || proto::kIsoGrad == wl.mode() || proto::kGradient == wl.mode() ) {
    auto& isos = meteo::map::WeatherLoader::instance()->isolibrary();
    auto isoname = QString::fromStdString(wl.template_name());
    if ( false == isos.contains(isoname)) {
      error_log << QObject::tr("Не найден шаблон изолиний %1").arg(QString::fromStdString(wl.template_name()));
      return;
    }
    else {
      getIso(wl, dtbeg, dtend, isos[isoname]);
    }
  }
  else if ( proto::kSigwx == wl.mode() ) {
    getSigwx(wl, dtbeg, dtend);
  }
}

void TermSelect::getSrc(proto::WeatherLayer wl, QDateTime dtbeg, QDateTime dtend, meteo::puanson::proto::Puanson punch)
{
  meteo::surf::DataRequest req;
  req.set_date_start(dtbeg.toString(Qt::ISODate).toStdString());
  req.set_date_end(dtend.toString(Qt::ISODate).toStdString());
  req.set_type_level(wl.type_level());
  req.set_level_p(wl.level());
  if ( true == wl.has_query_type() ) {
    req.set_query_type(wl.query_type());
  }
  for ( int i = 0, sz = wl.data_type_size(); i < sz; ++i ) {
    req.add_type(wl.data_type(i));
  }
  for(const meteo::puanson::proto::CellRule& rule : punch.rule()) {
    req.add_meteo_descrname(rule.id().name());
  }

  meteo::rpc::Channel* ch = meteo::global::serviceChannel( meteo::settings::proto::kSrcData );
  if ( nullptr == ch ) {
    error_log << QObject::tr("Не удалось установить соединение с сервисом данных");
    return;
  }

  auto reply = ch->remoteCall(&meteo::surf::SurfaceService::GetDataCount, req, 10000);
  delete ch;
  if ( nullptr == reply ) {
    error_log << QObject::tr("Ответ от сервиса данных не получен.");
    return;
  }

  if ( false == reply->result() ) {
    error_log << QObject::tr("Ответ от сервиса данных не получен.");
    delete reply;
    return;
  }

  for ( int i = 0, sz = reply->data_size(); i < sz; ++i ) {
    proto::Map map;
    auto data = reply->data(i);
    QDateTime dt = QDateTime::fromString(QString::fromStdString(data.date()), Qt::ISODate);
    if ( 0 != dt.time().minute() ) {
      continue;
    }
    map.set_datetime(data.date());
    map.set_center(global::kCenterWeather);
    map.set_hour(0);
    map.set_model(100 == wl.type_level() ? global::kModelAero : global::kModelSurface);
    map.set_level(wl.level());
    proto::WeatherLayer src;
    src.CopyFrom(wl);
    src.set_data_size(data.count());
    src.set_datetime(map.datetime());
    src.set_source(proto::kAuto);
    src.set_center(250);
    src.set_model(map.model());
    src.set_hour(map.hour());
    std::string mapStr;
    map.SerializeToString(&mapStr);
    weatherMaps_[mapStr].append(src);
  }
  delete reply;
}

void TermSelect::getField(proto::WeatherLayer wl, QDateTime dtbeg, QDateTime dtend, puanson::proto::Puanson punch)
{
  meteo::field::DataRequest req;
  req.set_date_start(dtbeg.toString(Qt::ISODate).toStdString());
  req.set_date_end(dtend.toString(Qt::ISODate).toStdString());
  req.add_level(wl.level());
  req.add_type_level(wl.type_level());
  for(const meteo::puanson::proto::CellRule& rule : punch.rule()) {
    req.add_meteo_descr(rule.id().descr());
  }

  auto ch = meteo::global::serviceChannel(meteo::settings::proto::kField);
  if ( nullptr == ch ) {
    error_log.msgBox() << QObject::tr("Не удалось установить соединение с сервисом полей");
    delete ch;
    return;
  }
  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
  meteo::field::DataDescResponse* reply = ch->remoteCall(&meteo::field::FieldService::GetAvailableData, req, 100000);
  delete ch;
  qApp->restoreOverrideCursor();
  if ( nullptr == reply ) {
    error_log << QObject::tr("Ответ от сервиса полей не получен.");
    return;
  }

  if ( false == reply->result() ) {
    error_log << QObject::tr("Ответ от сервиса полей не получен.");
    delete reply;
    return;
  }

  for ( int i = 0, sz = reply->descr_size(); i < sz; ++i ) {
    proto::Map map;
    auto descr = reply->descr(i);
    QDateTime dt = QDateTime::fromString(QString::fromStdString(descr.date()), Qt::ISODate );
    if ( global::kCenterWeather == descr.center() ) {
      continue;
    }
    if ( 0 != dt.time().minute() ) {
      continue;
    }
    map.set_datetime(descr.date());
    map.set_center(descr.center());
    map.set_hour(descr.hour()/3600);
    map.set_model(descr.model());
    map.set_level(wl.level());
    proto::WeatherLayer field;
    field.CopyFrom(wl);
    field.set_data_size(descr.count_point());
    field.set_datetime(map.datetime());
    field.set_source(proto::kField);
    field.set_center(map.center());
    field.set_model(map.model());
    field.set_hour(map.hour());
    field.set_net_type( descr.net_type() );
    std::string mapStr;
    map.SerializeToString(&mapStr);
    bool already = false;
    for ( auto l : weatherMaps_[mapStr] ) {
      if ( l.center() == field.center() && l.datetime() == field.datetime() ) {
        already = true;
        break;
      }
    }
    if ( false == already ) {
      weatherMaps_[mapStr].append(field);
    }
  }
  delete reply;
}

void TermSelect::getRadar(proto::WeatherLayer wl, QDateTime dtbeg, QDateTime dtend, proto::RadarColor color)
{
  if ( false == wl.has_h1() ) {
    return;
  }
  if ( false == wl.has_h2() ) {
    return;
  }

  meteo::surf::DataRequest req;
  req.set_date_start(dtbeg.toString(Qt::ISODate).toStdString());
  req.set_date_end(dtend.toString(Qt::ISODate).toStdString());
  req.add_meteo_descr(color.descr());
  req.set_level_h(wl.h1());
  req.set_level_h2(wl.h2());
  req.add_type(surf::kRadarMapType);

  meteo::rpc::Channel* ch = meteo::global::serviceChannel( meteo::settings::proto::kSrcData );
  if ( nullptr == ch ) {
    error_log << QObject::tr("Не удалось установить соединение с сервисом данных");
    return;
  }

  auto reply = ch->remoteCall(&meteo::surf::SurfaceService::GetAvailableRadar, req, 10000);
  delete ch;
  if ( nullptr == reply ) {
    error_log << QObject::tr("Ответ от сервиса данных не получен.");
    return;
  }

  if ( false == reply->result() ) {
    error_log << QObject::tr("Ответ от сервиса данных не получен.");
    delete reply;
    return;
  }

  for ( int i = 0, sz = reply->descr_size(); i < sz; ++i ) {
    proto::Map map;
    auto descr = reply->descr(i);
    map.set_datetime(descr.date());
    map.set_center(global::kCenterWeather);
    map.set_model(global::kModelSurface);
    map.set_hour(0);
    map.set_level(wl.level());
    proto::WeatherLayer radar;
    radar.CopyFrom(wl);
    radar.set_data_size(descr.count());
    radar.set_datetime(map.datetime());
    radar.set_source(proto::kAuto);
    radar.set_center(map.center());
    radar.set_model(map.model());
    radar.set_hour(map.hour());
    std::string mapStr;
    map.SerializeToString(&mapStr);
    weatherMaps_[mapStr].append(radar);
  }
  delete reply;
}

void TermSelect::getIso(proto::WeatherLayer wl, QDateTime dtbeg, QDateTime dtend, proto::FieldColor color)
{
  meteo::field::DataRequest req;
  req.set_date_start(dtbeg.toString(Qt::ISODate).toStdString());
  req.set_date_end(dtend.toString(Qt::ISODate).toStdString());
  req.add_level(wl.level());
  req.add_type_level(wl.type_level());
  req.add_meteo_descr(color.descr());
  if ( wl.has_net_type() ) {
    req.set_net_type(wl.net_type());
  }

  auto ch = meteo::global::serviceChannel(meteo::settings::proto::kField);
  if ( nullptr == ch ) {
    error_log.msgBox() << QObject::tr("Не удалось установить соединение с сервисом полей");
    delete ch;
    return;
  }
  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
  meteo::field::DataDescResponse* reply = ch->remoteCall(&meteo::field::FieldService::GetAvailableData, req, 100000);
  delete ch;
  qApp->restoreOverrideCursor();
  if ( nullptr == reply ) {
    error_log << QObject::tr("Ответ от сервиса полей не получен.");
    return;
  }

  if ( false == reply->result() ) {
    error_log << QObject::tr("Ответ от сервиса полей не получен.");
    delete reply;
    return;
  }

  for ( int i = 0, sz = reply->descr_size(); i < sz; ++i ) {
    proto::Map map;
    auto descr = reply->descr(i);
    QDateTime dt = QDateTime::fromString(QString::fromStdString(descr.date()), Qt::ISODate );
    if ( 0 != dt.time().minute() ) {
      continue;
    }
    map.set_datetime(descr.date());
    map.set_center(descr.center());
    map.set_hour(descr.hour()/3600);
    map.set_model(descr.model());
    map.set_level(wl.level());
    proto::WeatherLayer field;
    field.CopyFrom(wl);
    field.set_data_size(descr.count_point());
    field.set_datetime(map.datetime());
    field.set_source(proto::kField);
    field.set_center(map.center());
    field.set_model(map.model());
    field.set_hour(map.hour());
    field.set_net_type(descr.net_type());
    field.add_meteo_descr(descr.meteodescr());

    std::string mapStr;
    map.SerializeToString(&mapStr);
    weatherMaps_[mapStr].append(field);
  }
  delete reply;
}

void TermSelect::getSigwx(proto::WeatherLayer wl, QDateTime dtbeg, QDateTime dtend)
{
  Q_UNUSED(dtend);
  meteo::surf::DataRequest req;
  req.set_date_start(QString("%1T%2Z").arg(dtbeg.toString("yyyy-MM-dd"))
                         .arg(QString("00:00:00")).toStdString());
  req.set_date_end(QString("%1T%2Z").arg(dtbeg.toString("yyyy-MM-dd"))
                       .arg(QString("23:59:59")).toStdString());

  auto ch = meteo::global::serviceChannel(meteo::settings::proto::kSrcData);
  if ( nullptr == ch ) {
    error_log.msgBox() << QObject::tr("Не удалось установить соединение с сервисом полей");
    delete ch;
    return;
  }

  auto reply = ch->remoteCall(&meteo::surf::SurfaceService::GetAvailableSigwx, req, 100000);
  delete ch;
  if ( nullptr == reply ) {
    error_log << QObject::tr("Ответ от сервиса данных не получен.");
    return;
  }

  if ( false == reply->result() ) {
    error_log << QObject::tr("Ответ от сервиса данных не получен.");
    delete reply;
    return;
  }

  for ( int i = 0, sz = reply->sigwx_size(); i < sz; ++i ) {
    const surf::SigwxDesc& sig = reply->sigwx(i);
    QByteArray arr;
    global::protomsg2arr( sig, &arr );
    proto::WeatherLayer info = Weather::sigwx2layer(sig);
    if ( -1 == info.center() ) {
      continue;
    }
    if ( wl.h1() != info.h1() || wl.h2() != info.h2() ) {
      continue;
    }
    proto::Map map;
    map.set_datetime(info.datetime());
    map.set_center(info.center());
    map.set_hour(0);
    info.set_hour(0);
    info.set_model(map.model());
    map.set_level(0);
    std::string mapStr;
    map.SerializeToString(&mapStr);
    weatherMaps_[mapStr].append(info);
  }
  delete reply;
}

TermSelect::MapStatus TermSelect::hasMap( QTreeWidgetItem* item ) const
{
  if ( nullptr == mapdocument() ) {
    return kNoApplied;
  }
  if( nullptr == item ){
    return kNoApplied;
  }
  QList<proto::WeatherLayer> list;
  for ( int i = 0, sz = item->childCount(); i < sz; ++i ) {
    QByteArray arr = item->child(i)->data( WeatherItem::kTermClmn, kWeatherInfoType ).toByteArray();
    proto::WeatherLayer l;
    if ( false == global::arr2protomsg( arr, &l ) ) {
      warning_log << QObject::tr("Не удалось получить информацию о слое %1. Возможно некорректное функционирование программы.")
        .arg( item->child(i)->text(WeatherItem::kTermClmn) );
      continue;
    }
    list.append(l);
  }
  QList<meteo::map::Layer*> layers = mapdocument()->layers();
  int cnt = 0;
  for ( int i = 0, sz = list.size(); i < sz; ++i ) {
    const proto::WeatherLayer& layer = list[i];
    for ( int j = 0, jsz = layers.size(); j < jsz; ++j ) {
      Layer* l = layers[j];
      if ( true == l->isEqual(layer) ) {
        ++cnt;
        break;
      }
    }
  }
  MapStatus st = kNoApplied;
  if ( 0 == cnt ) {
    st = kNoApplied;
  }
  else if ( list.size() == cnt ) {
    st = kAppliedFull;
  }
  else {
    st = kAppliedPart;
  }
  return st;
}

bool TermSelect::hasLayer( QTreeWidgetItem* item ) const
{
  if ( nullptr == mapdocument() ) {
    return false;
  }
  if( nullptr == item ){
    return false;
  }
  QByteArray arr = item->data( WeatherItem::kTermClmn, kWeatherInfoType ).toByteArray();
  proto::WeatherLayer layer;
  global::arr2protomsg( arr, &layer );

  bool fl = false;
  QList<meteo::map::Layer*> layers = mapdocument()->layers();
  for ( int i = 0, sz = layers.size(); i < sz; ++i ) {
    Layer* l = layers[i];
    if ( true == l->isEqual(layer) ) {
      fl = true;
      break;
    }
  }
  return fl;
}

void TermSelect::updateItems()
{
  for( int i = 0, sz = ui_->mapstree->topLevelItemCount(); i < sz; ++i ){
    QTreeWidgetItem* item = ui_->mapstree->topLevelItem(i);
    if( nullptr == item ) {
      continue;
    }
    MapStatus st = hasMap(item);
    switch ( st ) {
      case kAppliedFull:
        for ( int j = 0, jsz = ui_->mapstree->columnCount(); j < jsz; ++j ){
          item->setBackgroundColor(j, Qt::yellow);
        }
        break;
      case kAppliedPart:
        for ( int j = 0, jsz = ui_->mapstree->columnCount(); j < jsz; ++j ){
          item->setBackgroundColor(j, Qt::gray);
        }
        break;
      case kNoApplied:
        for ( int j = 0, jsz = ui_->mapstree->columnCount(); j < jsz; ++j ){
          item->setBackgroundColor(j, Qt::white);
        }
        break;
    }
    for ( int j = 0, jsz = item->childCount(); j < jsz; ++j ) {
      QTreeWidgetItem* child = item->child(j);
      if (  false == hasLayer(child ) ) {
        for ( int k = 0, ksz = ui_->mapstree->columnCount(); k < ksz; ++k ){
          child->setBackgroundColor(k, Qt::white);
        }
      }
      else {
        for ( int k = 0, ksz = ui_->mapstree->columnCount(); k < ksz; ++k ){
          child->setBackgroundColor(k, Qt::yellow);
        }
      }
    }
  }
  slotItemSelectedChanged();
}

QString TermSelect::keyFromParameters( const QDateTime& dt, int center, int hour, int model, int level, int leveltype )
{
  return QString("%1::%2::%3::%4::%5::%6")
    .arg( dt.toString(kDateFormat) )
    .arg(center)
    .arg(hour)
    .arg(model)
    .arg(level)
    .arg(leveltype);
}

bool TermSelect::parametersFromKey( const QString& key, QDateTime* dt, int* center, int* hour, int* model, int* level, int* leveltype )
{
  if ( nullptr == dt
    || nullptr == center
    || nullptr == hour
    || nullptr == model
    || nullptr == level
    || nullptr == leveltype
      ) {
    error_log << QObject::tr("Нулевые указатели на параметры");
    return false;
  }
  QStringList list = key.split( "::", QString::KeepEmptyParts );
  if ( 6 != list.size() ) {
    error_log << QObject::tr("Не удалось получить значения параметров из ключа");
    return false;
  }

  *dt = QDateTime::fromString( list[0], kDateFormat );
  if ( false == dt->isValid() ) {
    error_log << QObject::tr("Не удалось получить дату из строки %1").arg(list[0]);
    return false;
  }

  *center = list[1].toInt();
  *hour = list[2].toInt();
  *model = list[3].toInt();
  *level = list[4].toInt();
  *leveltype = list[5].toInt();

  return true;
}

QString TermSelect::keyFromMap( const QDateTime& dt, int center, int hour, int model )
{
  return QString("%1::%2::%3::%4")
    .arg( dt.toString(kDateFormat) )
    .arg(center)
    .arg(hour)
    .arg(model);
}

bool TermSelect::mapFromKey( const QString& key, QDateTime* dt, int* center, int* hour, int* model )
{
  if ( nullptr == dt
    || nullptr == center
    || nullptr == hour
    || nullptr == model
      ) {
    error_log << QObject::tr("Нулевые указатели на параметры");
    return false;
  }
  QStringList list = key.split( "::", QString::KeepEmptyParts );
  if ( 4 != list.size() ) {
    error_log << QObject::tr("Не удалось получить значения параметров из ключа");
    return false;
  }

  *dt = QDateTime::fromString( list[0], kDateFormat );
  if ( false == dt->isValid() ) {
    error_log << QObject::tr("Не удалось получить дату из строки %1").arg(list[0]);
    return false;
  }

  *center = list[1].toInt();
  *hour = list[2].toInt();
  *model = list[3].toInt();

  return true;
}

QString TermSelect::keyFromFieldDescription( const meteo::field::DataDesc& descr, bool* ok )
{
  if ( nullptr != ok ) {
    *ok = false;
  }
  QString key;
  if ( false == descr.has_date()
    || false == descr.has_center()
    || false == descr.has_hour()
    || false == descr.has_model()
      ) {
    error_log << QObject::tr("Информация о поле не полная\n\t = %1")
      .arg( QString::fromStdString( descr.Utf8DebugString() ) );
    return key;
  }
  QString strdt = QString::fromStdString( descr.date() );
  QDateTime dt = QDateTime::fromString( strdt, Qt::ISODate );
  if ( false == dt.isValid() ) {
    error_log << QObject::tr("Не удалось получить дату из строки %1")
      .arg(strdt);
    return key;
  }
  int center = descr.center();
  int hour = descr.hour();
  int model = descr.model();
  key = keyFromMap( dt, center, hour, model );

  if ( nullptr != ok ) {
    *ok = true;
  }
  return key;
}

QString TermSelect::keyFromSurfaDescription( const meteo::surf::CountData& countdata, bool* ok )
{
  QString key;
  if ( nullptr != ok ) {
    *ok = false;
  }
  QString strdt = QString::fromStdString( countdata.date() );
  QDateTime dt = QDateTime::fromString( strdt, Qt::ISODate );
  if ( false == dt.isValid() ) {
    error_log << QObject::tr("Не удалось получить дату из строки %1")
      .arg(strdt);
    return key;
  }
  int center = global::kCenterWeather;
  int hour = 0;
  int model = global::kModelSurface;
  key = keyFromMap( dt, center, hour, model );
  if ( nullptr != ok ) {
    *ok = true;
  }
  return key;
}

void TermSelect::slotCurrentMapIndexChanged( int indx )
{
  Q_UNUSED(indx);
  QString title = ui_->cmbmaptype->currentText();
  QMapIterator<QString,proto::Map> it(types_);
  while ( true == it.hasNext() ) {
    it.next();
    const proto::Map& map = it.value();
    if ( QString::fromStdString( map.title() ) == title ) {
      currentmap_ = map;
      loadTermsForCurrentMap();
      QWidget::setWindowTitle(title);
      return;
    }
  }
  error_log << QObject::tr("Неизвестный тип карты %1").arg(title);
}

void TermSelect::slotDateChanged( const QDate& date )
{
  Q_UNUSED(date);
  loadTermsForCurrentMap();
}

void TermSelect::slotItemSelectedChanged()
{
  QList<QTreeWidgetItem*> list = ui_->mapstree->selectedItems();
  if ( 0 == list.size() ) {
    ui_->okbtn->setDisabled(true);
    ui_->rmbtn->setDisabled(true);
  }
  else {
    QTreeWidgetItem* item = list[0];
    if ( nullptr != item->parent() ) {
      item->parent()->setSelected(true);
      item->setSelected(false);
      item = item->parent();
      QByteArray arr  = item->data( WeatherItem::kTermClmn, kWeatherInfoType ).toByteArray();
      proto::WeatherLayer info;
      global::arr2protomsg( arr, &info );
//      debug_log << "LAYER =" << info.Utf8DebugString();
    }
    else {
      QByteArray arr  = item->data( WeatherItem::kTermClmn, kWeatherInfoType ).toByteArray();
      proto::Map info;
      global::arr2protomsg( arr, &info );
//      debug_log << "MAP =" << info.Utf8DebugString();
    }
    MapStatus st = hasMap(item);
    switch (st) {
      case kAppliedFull:
        ui_->okbtn->setDisabled(true);
        ui_->rmbtn->setDisabled(false);
        break;
      case kAppliedPart:
        ui_->okbtn->setDisabled(false);
        ui_->rmbtn->setDisabled(false);
        break;
      case kNoApplied:
        ui_->okbtn->setDisabled(false);
        ui_->rmbtn->setDisabled(true);
        break;
    }
  }
}

void TermSelect::slotDblCl(QTreeWidgetItem *item, int){
  MapStatus st = hasMap(item);
  switch ( st ) {
    case kAppliedFull:
    case kAppliedPart:
      slotRmMap();
      break;
    case kNoApplied:
      slotOk();
      break;
  }

}

void TermSelect::slotOk()
{
  if ( nullptr == mapdocument() ) {
    error_log.msgBox() << QObject::tr("Документ не создан");
    return;
  }
  QList<QTreeWidgetItem*> list = ui_->mapstree->selectedItems();
  if ( 0 == list.size() ) {
    error_log << QObject::tr("Не выбраны данные из списка доступных для построения карты погоды");
    QApplication::restoreOverrideCursor();
    return;
  }
  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

  QByteArray arr  = list[0]->data( WeatherItem::kTermClmn, kWeatherInfoType ).toByteArray();
  proto::Map info;
  if ( false == info.ParseFromString( std::string( arr.data(), arr.size() ) ) ) {
    error_log << QObject::tr("Не удалось получить информацию о выбранной карте");
    QApplication::restoreOverrideCursor();
    return;
  }
  info.set_spline_koef( ui_->cmbspline->currentText().toInt() );
  Weather weather;
  info.set_title(currentmap_.title());
  info.set_title(makeMapInfoTitle(info).toStdString());

  if ( false == weather.createMap( mapdocument(), info ) ) {
    error_log << QObject::tr("Карта не создана \n\t = %1 %2")
      .arg( QString::fromStdString( info.Utf8DebugString() ) );
  }
  updateItems();
  QApplication::restoreOverrideCursor();
//  QWidget::close();
}

QString TermSelect::makeMapInfoTitle(const proto::Map& minfo)
{
  QString pattern = QString::fromStdString(minfo.map_name_pattern());
  pattern.replace( "[template]", QString::fromStdString(minfo.title()) );
  QDateTime dt = PsqlQuery::datetimeFromString( QString::fromStdString( minfo.datetime() ) );
  if ( true == dt.isValid() ) {
    pattern = replacePatternByDate( pattern, dt );
  }
  QString center = QString::number(minfo.center());
  QMap<int, QPair<QString, int>> centers = global::kMeteoCenters();
  if(centers.contains(minfo.center())) {
    center = centers[minfo.center()].first;
  }


  pattern.replace( "[center]", center );
  QString hour;
  if ( minfo.has_hour() && 0 != minfo.hour() ) {
    hour = QObject::tr("прогноз на %1 ч.").arg(minfo.hour());
  }
  pattern.replace( "[hour]", hour );

  QString lvlstr;
  if ( 1 == minfo.type_level() ) {
    lvlstr = QObject::tr("у земли");
  }
  else if ( 100 == minfo.type_level() ) {
    lvlstr = QObject::tr("%1 мбар.").arg( minfo.level() );
  }
  else if ( 102 == minfo.type_level() ) {
    lvlstr = QObject::tr("Уровень моря.");
  }
  else if ( 160 ==  minfo.type_level() ) {
    lvlstr = QObject::tr("Глубина %1 м.").arg( minfo.level() );
  }
  else if ( 9 ==  minfo.type_level() ) {
    lvlstr = QObject::tr("Дно");
  }
  pattern.replace("[level]",lvlstr);
  return pattern;
}

void TermSelect::slotNo()
{
  QWidget::close();
}

void TermSelect::slotAddMap()
{
  slotOk();
}

void TermSelect::slotRmMap()
{
  if ( nullptr == mapdocument() ) {
    return;
  }

  QList<QTreeWidgetItem*> list = ui_->mapstree->selectedItems();
  if ( 0 == list.size() ) {
    error_log << QObject::tr("Не выбраны данные из списка доступных для построения карты погоды");
    QApplication::restoreOverrideCursor();
    return;
  }

  proto::Map info;
  QByteArray arr = list[0]->data( WeatherItem::kTermClmn, kWeatherInfoType ).toByteArray();
  if ( false == global::arr2protomsg( arr, &info ) ) {
    return;
  }
  for ( int i = 0, sz = info.data_size(); i < sz; ++i ) {
    const proto::WeatherLayer& layer = info.data(i);
    QList<meteo::map::Layer*> list = mapdocument()->layers();
    for ( int j = 0, jsz = list.size(); j < jsz; ++j ) {
      Layer* l = list[j];
      if ( true == l->isEqual(layer) ) {
        delete l;
        break;
      }
    }
  }
  mapdocument()->setMap(info);

  updateItems();
}

void TermSelect::slotContextMenu( const QPoint& pnt )
{
  QAction* add = menu_->actions()[0];
  QAction* del = menu_->actions()[1];
  QTreeWidgetItem* item = ui_->mapstree->currentItem();
  if( nullptr == item || nullptr == del ){
    return;
  }
  if ( nullptr != item->parent() ) {
    item = item->parent();
  }
  MapStatus st = hasMap(item);
  switch (st) {
    case kAppliedFull:
      add->setDisabled(true);
      del->setDisabled(false);
      break;
    case kAppliedPart:
      add->setDisabled(false);
      del->setDisabled(false);
      break;
    case kNoApplied:
      add->setDisabled(false);
      del->setDisabled(true);
      break;
  }
  menu_->exec( ui_->mapstree->mapToGlobal(pnt) );
}

void TermSelect::slotSpinSplineKoefChanged( int indx )
{
  currentmap_.set_spline_koef(indx+1);
}

void TermSelect::slotHourChange(const QString& hour)
{
  if( 0 == ui_->hourCombo->currentIndex() ){
    for( int i = 0; i < ui_->mapstree->topLevelItemCount(); i++ ){
      ui_->mapstree->topLevelItem(i)->setHidden(false);
    }
    return;
  }
  if( 1 == ui_->hourCombo->currentIndex() ){
    for( int i = 0; i < ui_->mapstree->topLevelItemCount(); i++ ){
      if( ui_->mapstree->topLevelItem(i)->text(WeatherItem::kHourClmn) == "0" ){
        ui_->mapstree->topLevelItem(i)->setHidden(false);
      }
      else{
        ui_->mapstree->topLevelItem(i)->setHidden(true);
      }
    }
    return;
  }
  for( int i = 0; i < ui_->mapstree->topLevelItemCount(); i++ ){
    if( ui_->mapstree->topLevelItem(i)->text(WeatherItem::kHourClmn) == hour ){
      ui_->mapstree->topLevelItem(i)->setHidden(false);
    }
    else{
      ui_->mapstree->topLevelItem(i)->setHidden(true);
    }
  }
}

}
}
}
