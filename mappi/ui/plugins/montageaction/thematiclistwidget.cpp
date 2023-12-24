#include "thematiclistwidget.h"
#include "ui_thematiclistwidget.h"

#include <qmap.h>
#include <qobject.h>
#include <qbitmap.h>
#include <qthread.h>
#include <qaction.h>
#include <qmenu.h>
#include <QTimeZone>

#include <commons/textproto/tprototext.h>

#include <meteo/commons/ui/custom/filterheader.h>

#include <mappi/global/streamheader.h>
#include <mappi/proto/sessiondataservice.pb.h>
#include <mappi/services/fileservice/fileserviceclient/fileserviceclient.h>
#include <mappi/services/sessiondataservice/sessiondataservice.h>
#include <mappi/ui/plugins/maskslider/transparencyslider.h>
#include <mappi/ui/satlayer/satlayer.h>
#include <mappi/ui/satelliteimage/georastr.h>
#include <mappi/ui/satelliteimage/satelliteimage.h>
#include <mappi/ui/satelliteimage/satlegend.h>
#include <mappi/proto/schedule.pb.h>

Q_DECLARE_METATYPE( ::mappi::proto::SatelliteImage )

using ThematicData = ::mappi::proto::ThematicData;

namespace meteo {
namespace map {

static const QString kLayer = QObject::tr("Просмотр тематических обработок");

bool sortList(const ::mappi::proto::ThematicData& data1, const ::mappi::proto::ThematicData& data2)
{
  bool check = QDateTime::fromString(QString::fromStdString(data1.date_start()), Qt::ISODate) <
               QDateTime::fromString(QString::fromStdString(data2.date_start()), Qt::ISODate);
  return  check;
}

ThematicListWidget::ThematicListWidget(MapWindow *window) :
  MapWidget(window),
  ui_(new Ui::ThematicListWidget)
{
  qRegisterMetaType< ::mappi::FileServiceResponse >("::mappi::FileServiceResponse");

  ui_->setupUi(this);

  scene_ = new AreaScene(ui_->graphicsView);
  ui_->graphicsView->setScene(scene_);

  activeItemBg_ = QBrush(Qt::yellow);
  defaultItemBg_ = ui_->treeWidget->palette().base();

  setWindowTitle("Список тематических обработок");

  FilterHeader* header = new FilterHeader(ui_->treeWidget);
  ui_->treeWidget->setHeader(header);
  ui_->treeWidget->setColumnWidth(0, 20);
  header->setCortColumn(kDateColumn, Qt::AscendingOrder);
  header->setSectionsClickable(true);
  header->setSortIndicatorShown(true);

  ui_->treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);
  ui_->dateEdit->setDate(QDate::currentDate());

  //  init();

  connect(ui_->treeWidget, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(slotAddLayer()));
  // connect(ui_->comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(slotTypeChanged()));
  // connect(ui_->channelBox, SIGNAL(currentIndexChanged(int)), this, SLOT(slotTypeChanged()));
  connect(ui_->dateEdit, SIGNAL(dateChanged(QDate)), this, SLOT(slotTypeChanged()));
  connect(ui_->treeWidget, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(slotShowTableContextMenu()));
  connect(ui_->treeWidget, SIGNAL(clicked(QModelIndex)), this, SLOT(slotHideImage()));
  connect(ui_->treeWidget, SIGNAL(clicked(QModelIndex)), this, SLOT(slotShowMapInfo()));
  connect(ui_->treeWidget, SIGNAL(itemSelectionChanged()), this, SLOT(slotShowMapInfo()));

  map::Document* doc = mapdocument();
  if ( !doc->isStub() ) {
    doc->turnEvents();
    if ( doc->eventHandler() != 0 ) {
      doc->eventHandler()->installEventFilter(this);
    }
  }

}

ThematicListWidget::~ThematicListWidget()
{
  //delete ui_;
}

void ThematicListWidget::init()
{
  if ( false == inited_ ) {
    qApp->setOverrideCursor(Qt::WaitCursor);
    // if ( false == initTypes() ) {
    //   error_msg << tr("Ответ от сервиса данных не получен.");
    //   qApp->restoreOverrideCursor();
    //   return;
    // }

    if ( false == loadSessions() ) {
      error_msg << tr("Ответ от сервиса данных не получен.");
      qApp->restoreOverrideCursor();
      return;
    }

    inited_ = true;
    qApp->restoreOverrideCursor();
  }
}

// bool ThematicListWidget::initTypes()
// {
//   meteo::rpc::Channel* ch = meteo::global::serviceChannel( meteo::settings::proto::kDataControl );
//   if ( 0 == ch) {
//     error_log << meteo::msglog::kServiceConnectFailedSimple.arg(meteo::settings::proto::kDataControl);
//     return false;
//   }

//   Dummy req;
//   ::mappi::proto::ThematicList* resp;

//   resp = ch->remoteCall(&::mappi::proto::SessionDataService::GetAvailableThematicTypes, req, 10000, true);
//   delete ch;

//   if ( resp == nullptr) {
//     return false;
//   }

//   for ( int i = 0, sz = resp->themes_size(); i < sz; ++i) {
//     QString title = QString::fromStdString(resp->themes(i).title());
//     ::mappi::conf::ThemType type = resp->themes(i).type();
//     if ( ::mappi::conf::ThemType::kGrayScale == type ) {
//       grayscaleChannels();
//     }
//     ui_->comboBox->addItem(title);
//     ui_->comboBox->setItemData(i, static_cast<int>(type));
//   }
//   delete resp;

//   int themType = ui_->comboBox->currentData().toInt();
//   bool gracescale = ::mappi::conf::ThemType::kGrayScale == themType;
//   ui_->channelBox->setVisible(gracescale);

//   return true;
// }

bool ThematicListWidget::loadSessions()
{
  ui_->treeWidget->clear();

  meteo::rpc::Channel* ch = meteo::global::serviceChannel( meteo::settings::proto::kDataControl );
  if ( 0 == ch) {
    error_log << meteo::msglog::kServiceConnectFailedSimple.arg(meteo::settings::proto::kDataControl);
    return false;
  }

  ::mappi::proto::ThematicData theme;
  // theme.set_title(ui_->comboBox->currentText().toStdString());
  //  debug_log <<  ::mappi::proto::ThemType::kGrayScale << ui_->comboBox->currentData().toInt() << ui_->channelBox->currentText().toStdString();
  // if ( ::mappi::conf::ThemType::kGrayScale == ui_->comboBox->currentData().toInt() ) {
  //   theme.set_channel_alias(ui_->channelBox->currentText().toStdString());
  // }

  QDateTime dt = ui_->dateEdit->dateTime();
  dt.setTimeZone(QTimeZone::systemTimeZone());
  QTime time(0,0,0);
  dt.setTime(time);
  theme.set_date_start(dt.toUTC().toString(Qt::ISODate).toStdString());
  time.setHMS(23,59,59);
  dt.setTime(time);
  theme.set_date_end(dt.toUTC().toString(Qt::ISODate).toStdString());
  //theme.set_format("png");

  ::mappi::proto::ThematicList* resp;

  //var(theme.Utf8DebugString());
  
  resp = ch->remoteCall(&::mappi::proto::SessionDataService::GetAvailableThematic, theme, 10000, true);
  delete ch;

  if ( nullptr == resp ) {
    return false;
  }

  var(resp->Utf8DebugString());
  QVector<ThematicData> list;

  for ( int i = 0, sz = resp->themes_size(); i < sz; ++i ) {
    list.append(resp->themes(i));
  }
  std::sort(list.begin(), list.end(), sortList);

  for ( int i = 0, sz = list.size(); i < sz; ++i ) {
    QString nameString = QString::fromStdString(list.at(i).satellite_name());

    QDateTime dts = QDateTime::fromString(QString::fromStdString(list.at(i).date_start()), Qt::ISODate);
    QDateTime dte = QDateTime::fromString(QString::fromStdString(list.at(i).date_end()), Qt::ISODate);
    dts.setTimeSpec(Qt::UTC);
    dte.setTimeSpec(Qt::UTC);
    QTime time = dts.toLocalTime().time();
    QString timeString = time.toString(Qt::ISODate);

    int secs = dts.secsTo(dte);
    QString lengthString = QDateTime::fromTime_t(secs).toUTC().toString("mm:ss");

    QStringList strList = QStringList() << QString() << QString::fromStdString(list.at(i).instrument_name())
                                        << QString::fromStdString(list.at(i).name())
                                        << nameString << timeString << lengthString;

    ::mappi::proto::SatelliteImage params;
    params.set_satellite_name(nameString.toStdString());
    params.set_date_start(dts.toString(Qt::ISODate).toStdString());
    params.set_date_end(dte.toString(Qt::ISODate).toStdString());
    QString filename = QString::fromStdString(list.at(i).path());
    params.set_path(filename.toStdString());
    params.set_session_id(list.at(i).session_id());
    params.set_instrument_type(list.at(i).instrument_type());
    params.set_type(list.at(i).type());
    params.set_themname(list.at(i).name());
    params.set_format(list.at(i).format());
    QString title = mkTitle(params);

    QTreeWidgetItem* item = new QTreeWidgetItem(strList);
    item->setData(kIconColumn, kSessionParamRole, QVariant::fromValue(params));
    item->setData(kIconColumn, kTitleRole, title);

    foreach ( Layer* l, mapdocument()->layers() ) {
      SatLayer* layer = maplayer_cast<SatLayer*>(l);

      if ( nullptr == layer ) { continue; }

      QString s = mkTitle(layer->sessionParams());
      if ( title == s ) {
        item->setText(kLayerUuidColumn, l->uuid());
        break;
      }
    }

    ui_->treeWidget->insertTopLevelItem(i, item);
  }

  delete resp;

  return true;
}

void ThematicListWidget::slotTypeChanged()
{
  if ( false == inited_ ) {
    return;
  }

  // int themType = ui_->comboBox->currentData().toInt();
  // QString themName = ui_->comboBox->currentText();

  loadSessions();

  FilterHeader *filterHeader = qobject_cast<FilterHeader*>(ui_->treeWidget->header());
  if ( filterHeader != nullptr ) {
    filterHeader->reloadFilters();
  }
  ui_->treeWidget->clearSelection();

}

void ThematicListWidget::slotHideImage()
{
  if ( 0 != ui_->treeWidget->currentColumn()) { return; }

  QTreeWidgetItem* item = ui_->treeWidget->currentItem();
  if ( 0 == item ) {
    return;
  }

  SatLayer* layer = nullptr;

  QString title = item->data(kIconColumn, kTitleRole).toString();
  foreach ( Layer* l, mapdocument()->layers() ) {
    if ( title == l->name() ) {
      layer = maplayer_cast<SatLayer*>(l);
      break;
    }
  }

  if ( nullptr == layer ) { return; }

  bool v = layer->visible();
  layer->setVisisble(!v);
  item->setIcon(0, getEyeIcon(!v));
}

void ThematicListWidget::slotShowTableContextMenu()
{
  QMenu contextMenu(QObject::tr("Контекстное меню"), this);
  QAction add(QIcon(":/mappi/icons/add.png"), QObject::tr("Добавить слой"), this);
  QAction remove(QIcon(":/mappi/icons/delete.png"), QObject::tr("Удалить слой"), this);
  connect(&add, SIGNAL(triggered(bool)), this, SLOT(slotAddLayer()));
  connect(&remove, SIGNAL(triggered(bool)), this, SLOT(slotRemoveLayer()));
  contextMenu.addAction(&add);
  contextMenu.addAction(&remove);

  SatLayer* layer = nullptr;

  QTreeWidgetItem* item = ui_->treeWidget->currentItem();
  if ( 0 == item ) {
    return;
  }

  QString uuid = item->text(kLayerUuidColumn);
  foreach ( Layer* l, mapdocument()->layers() ) {
    if ( l->uuid() == uuid ) {
      layer = maplayer_cast<SatLayer*>(l);
      break;
    }
  }

  bool hasLayer = nullptr != layer;

  add.setDisabled(hasLayer);
  remove.setDisabled(!hasLayer);

  contextMenu.exec(QCursor::pos());
}

void ThematicListWidget::slotAddLayer()
{
  QTreeWidgetItem* item = ui_->treeWidget->currentItem();
  if ( 0 == item ) {
    return;
  }

  SatLayer* layer = nullptr;

  QString uuid = item->text(kLayerUuidColumn);
  foreach ( Layer* l, mapdocument()->layers() ) {
    if ( l->uuid() == uuid ) {
      layer = maplayer_cast<SatLayer*>(l);
      break;
    }
  }

  ::mappi::proto::SatelliteImage params = item->data(kIconColumn, kSessionParamRole).value< ::mappi::proto::SatelliteImage >();


  if ( 0 == layer ) {
    layer = new SatLayer(mapdocument(), mkTitle(params));
    layer->setSessionParams(params);
    mapdocument()->moveLayer(layer->uuid(), -1);

    QByteArray tleData = getTLE(params.session_id());
    if ( true == tleData.isEmpty() ) {
      error_msg << tr("Не удалось загрузить параметры сессии.");
      debug_log << params.Utf8DebugString();
      delete layer;
      return;
    }

    QDataStream dStr(tleData);
    MnSat::TLEParams tle;
    dStr >> tle;

    layer->setTLE(tle);

    var(params.instrument_type());
    ::mappi::conf::Instrument instr;
    if ( !::mappi::inter::Settings::instance()->instrument(params.instrument_type(), &instr) ) {
      error_msg << tr("Не удалось загрузить параметры инструмента.");
      debug_log << params.Utf8DebugString();

      delete layer;
      return;
    }

    qApp->setOverrideCursor(Qt::WaitCursor);

    PosGrid* grid = new PosGrid;

    QDateTime dts = QDateTime::fromString(QString::fromStdString(params.date_start()), Qt::ISODate);
    QDateTime dte = QDateTime::fromString(QString::fromStdString(params.date_end()), Qt::ISODate);

    grid->setSatellite(dts, dte, tle, instr.scan_angle()*meteo::DEG2RAD, instr.samples(), instr.velocity());

    // высоту сетки берём с запасом, т.к. на данном этапе она неизвестна
    if ( !grid->buildGridF2X(instr.samples(), instr.samples()*3) ) {
      error_log << tr("Не удалось построить сетку.");
      debug_log << dts << dte << instr.scan_angle()*meteo::DEG2RAD << instr.samples() << instr.velocity();
    }

    layer->setProjection(grid);

    item->setText(kLayerUuidColumn, layer->uuid());

    qApp->restoreOverrideCursor();
  }

  foreach ( Object* o, layer->objects() ) {
    SatelliteImage* i = mapobject_cast<SatelliteImage*>(o);
    if ( nullptr != i && i->thematicType() == params.type() ) {
      return;
    }
  }

  qApp->setOverrideCursor(Qt::WaitCursor);

  QString name = item->text(kProductColumn);
  if ( addImageObject(layer, name) ) {

    layer->setSessionParams(item->data(kIconColumn, kSessionParamRole).value< ::mappi::proto::SatelliteImage >());
    layer->setThematicTypeName(name);

    ui_->treeWidget->clearSelection();
    for (int idx = 0; idx < ui_->treeWidget->columnCount(); idx++) {
      item->setBackground(idx, activeItemBg_);
    }
    
    item->setIcon(kIconColumn, getEyeIcon(true));
    layer->repaint();
  }
  
  qApp->restoreOverrideCursor();
}

void ThematicListWidget::slotRemoveLayer()
{
  QTreeWidgetItem* item = ui_->treeWidget->currentItem();
  if ( 0 == item ) {
    return;
  }
  Document* doc = mapdocument();
  QString uuid = item->text(kLayerUuidColumn);
  foreach ( Layer* l, doc->layers() ) {
    if ( uuid == l->uuid() ) {
      doc->eventHandler()->notifyLayerChanges(l, LayerEvent::Deleted);
      ui_->treeWidget->clearSelection();
      delete l;
      return;
    }
  }
}

void ThematicListWidget::slotShowMapInfo()
{
  QTreeWidgetItem* item = ui_->treeWidget->currentItem();
  
  ::mappi::conf::Session session;
  session.set_satellite(ui_->treeWidget->currentItem()->data(kSatelliteColumn, Qt::DisplayRole).toString().toStdString());
  QDate date = ui_->dateEdit->date();
  QTime ts, te;
  ts = QTime::fromString(ui_->treeWidget->currentItem()->data(kDateColumn, Qt::DisplayRole).toString(), "hh:mm:ss");
  te = QTime::fromString(ui_->treeWidget->currentItem()->data(kDurationColumn, Qt::DisplayRole).toString(), "mm:ss");
  QDateTime dts, dte;
  dts.setDate(date);
  dts.setTime(ts);
  dts = dts.toUTC();
  dte = dts.addSecs(te.minute()*60 + te.second());
  session.set_aos(dts.toString(Qt::ISODate).toStdString());
  session.set_los(dte.toString(Qt::ISODate).toStdString());

  //var(session.Utf8DebugString());

  ::mappi::proto::SatelliteImage params = item->data(kIconColumn, kSessionParamRole).value< ::mappi::proto::SatelliteImage >();

  ::mappi::conf::InstrumentType instrType = params.instrument_type();
  ::mappi::conf::Instrument instrument;

  if (!::mappi::inter::Settings::instance()->instrument(instrType, &instrument)) {
    error_log << QObject::tr("Прибор не найден");
    return;
  }

  
  QByteArray tleData = getTLE(params.session_id());
  if ( true == tleData.isEmpty() ) {
    error_msg << tr("Не удалось загрузить параметры сессии.");
    debug_log << params.Utf8DebugString();
    return;
  }

  //var(instrument.Utf8DebugString());
  
  QDataStream dStr(tleData);
  MnSat::TLEParams tle;
  dStr >> tle;
  scene_->setSession(session, instrument, tle);
}

bool ThematicListWidget::eventFilter(QObject* obj, QEvent* event)
{
  Q_UNUSED( obj );

  if ( map::LayerEvent::LayerChanged != event->type() ) { return false; }

  map::Document* doc = mapdocument();
  if ( 0 == doc ) {
    return false;
  }

  map::LayerEvent* ev = static_cast<map::LayerEvent*>(event);
  if ( map::LayerEvent::Deleted == ev->changeType() ) {
    for ( int i = 0, isz = ui_->treeWidget->topLevelItemCount(); i < isz; ++i ) {
      QTreeWidgetItem* item = ui_->treeWidget->topLevelItem(i);
      if ( item->text(kLayerUuidColumn) == ev->layer() ) {
        item->setText(kLayerUuidColumn, "");
        item->setIcon(0, QIcon());

        for (int idx = 0; idx < ui_->treeWidget->columnCount(); idx++) {
          item->setBackground(idx, defaultItemBg_);
          // item->setBackground(kSatelliteColumn, defaultItemBg_);
          // item->setBackground(kDateColumn, defaultItemBg_);
          // item->setBackground(kDurationColumn, defaultItemBg_);
        }
      }
    }
    return false;
  }

  if ( map::LayerEvent::Visibility != ev->changeType() ) {
    return false;
  }

  SatLayer* layer = maplayer_cast<SatLayer*>(doc->layerByUuid(ev->layer()));
  if ( nullptr == layer ) {
    return false;
  }

  for ( int i = 0, isz = ui_->treeWidget->topLevelItemCount(); i < isz; ++i ) {
    QTreeWidgetItem* item = ui_->treeWidget->topLevelItem(i);
    if ( layer->uuid() == item->text(kLayerUuidColumn) ) {
      bool v = layer->visible();
      item->setIcon(0, getEyeIcon(v));
    }
  }

  return false;
}

void ThematicListWidget::resizeEvent(QResizeEvent *e)
{
  QWidget::resizeEvent(e);
  resizeMap(ui_->graphicsView->size());
}

void ThematicListWidget::showEvent(QShowEvent *e)
{
  resizeMap(ui_->graphicsView->size());
  QWidget::showEvent(e);
}

bool ThematicListWidget::addImageObject(SatLayer* layer, const QString& name)
{
  if ( nullptr == layer ) { return false; }

  const ::mappi::proto::SatelliteImage& params = layer->sessionParams();

  QString origFile = QString::fromStdString(params.path());
  QString fileName = origFile; // getFile(origFile); TODO пока так)
  //var(fileName);
  if ( fileName.isEmpty() ) {
    error_msg << tr("Не удалось загрузить файл с данными.").arg(fileName);
    debug_log << origFile;
    return false;
  }

  TColorGradList grad = ::mappi::inter::Settings::instance()->palette(params.themname());
  layer->setPalette(grad);

  GeoRastr* o = new GeoRastr(layer);
  o->setProtoData(params);
  if ( !o->load(fileName) ) {
    error_log << QObject::tr("Ошибка загрузки файла '%1'").arg(fileName);
    delete o;
    return false;
  }

  SatLegend* sl = new SatLegend(layer->document());
  Legend* l = layer->document()->setLegend(sl);
  delete l;

  sl->setLabel(name);
  sl->setOrient(meteo::kHorizontal);
  sl->setPositionOnDocument(meteo::kBottomCenter);
  sl->setVisible(true);


  return true;
}

QString ThematicListWidget::mkTitle(const ::mappi::proto::SatelliteImage& params) const
{
  QDateTime dts = QDateTime::fromString(QString::fromStdString(params.date_start()), Qt::ISODate);
  dts.setTimeSpec(Qt::UTC);

  QDateTime dte = QDateTime::fromString(QString::fromStdString(params.date_end()), Qt::ISODate);
  dte.setTimeSpec(Qt::UTC);

  int secs = dts.secsTo(dte);
  QString duration = QDateTime::fromTime_t(secs).toUTC().toString("mm:ss");

  QString name = QString::fromStdString(params.satellite_name());

  return QString("%1  %2,  %3").arg(dts.toLocalTime().toString("dd.MM hh:mm:ss"), name, duration);
}

// ::mappi::proto::SatelliteImage ThematicListWidget::loadSessionParam(int sessionId, const QString& thematicName, const QString& channelName) const
// {
//   meteo::rpc::Channel* ch = meteo::global::serviceChannel( meteo::settings::proto::kDataControl );
//   if ( 0 == ch) {
//     error_log << meteo::msglog::kServiceConnectFailedSimple.arg(meteo::settings::proto::kDataControl);
//     return ::mappi::proto::SatelliteImage();
//   }

//   ::mappi::proto::ThematicData req;
//   if ( sessionId > 0 ) {
//     req.set_session_id(sessionId);
//   }
//   if ( !thematicName.isEmpty() ) {
//     req.set_name(thematicName.toStdString());
//   }
//   if ( !channelName.isEmpty() ) {
//     req.set_channel_alias(channelName.toStdString());
//   }

//   ::mappi::proto::ThematicList* resp;
//   resp = ch->remoteCall(&::mappi::proto::SessionDataService::GetAvailableThematic, req, 10000, true);
//   delete ch;

//   if ( nullptr == resp ) {
//     return ::mappi::proto::SatelliteImage();
//   }

//   ::mappi::proto::SatelliteImage param;

//   if ( resp->themes_size() > 0 ) {
//     auto t = resp->themes(0);

//     param.set_satellite_name(t.satellite_name());
//     param.set_date_start(t.date_start());
//     param.set_date_end(t.date_end());
//     param.set_path(t.path());
//     param.set_session_id(t.session_id());
//     param.set_instrument_type(t.instrument_type());
//     param.set_type(t.type());
//   }

//   delete resp;
//   return param;
// }

QPixmap ThematicListWidget::getEyeIcon(bool active)
{
  QPixmap eye = QPixmap(":mappi/icons/eye.png");
  if(false == active) {
    QPixmap pix(eye.size());
    pix.fill(Qt::gray);
    pix.setMask(eye.createMaskFromColor(Qt::transparent));
    return pix;
  }
  return eye;
}

void ThematicListWidget::resizeMap(const QSize &size)
{
  if (nullptr != ui_->graphicsView->scene()) {
    // На 4 меньше, чтобы вся рамка влезла...
    QSize sz(size.width() - 4, size.height() - 4);
    scene_->resize(sz);
  }
}

QString ThematicListWidget::getFile(const QString& fileName, bool* ok)
{
  if ( 0 != ok ) { *ok = false; }

  ::mappi::FileServiceClient* client = new ::mappi::FileServiceClient();
  client->setFileName(fileName);
  //  client->setRewrite(true);

  QThread* th = new QThread;
  client->moveToThread(th);

  connect( th, &QThread::started, client, &::mappi::FileServiceClient::slotRun );

  //  connect( client, &::mappi::FileServiceClient::fileResponseFinish, th, &QThread::quit );
  //  connect( client, &::mappi::FileServiceClient::fileNotFound, th, &QThread::quit );
  //  connect( client, &::mappi::FileServiceClient::connectionFailure, th, &QThread::quit );
  //  connect( client, &::mappi::FileServiceClient::requestFailure, th, &QThread::quit );
  //  connect( client, &::mappi::FileServiceClient::responseTimeout, th, &QThread::quit );


  connect( client, &::mappi::FileServiceClient::fileResponseFinish, [=](){ th->quit(); } );
  connect( client, &::mappi::FileServiceClient::fileNotFound, [=](){ th->quit(); } );
  connect( client, &::mappi::FileServiceClient::connectionFailure, [=](){ th->quit(); } );
  connect( client, &::mappi::FileServiceClient::requestFailure, [=](){ th->quit(); } );
  connect( client, &::mappi::FileServiceClient::responseTimeout, [=](){ th->quit(); } );

  th->start();
  th->wait();

  th->deleteLater();
  client->deleteLater();

  if ( !client->isSuccessful() ) {
    return QString();
  }

  if ( 0 != ok ) { *ok = true; }

  return client->getClientFilename();
}

QByteArray ThematicListWidget::getTLE(int sessionID) const
{
  ::mappi::proto::SessionData req;
  ::mappi::proto::SessionList* resp;
  req.set_id(sessionID);
  meteo::rpc::Channel* ch = meteo::global::serviceChannel( meteo::settings::proto::kDataControl );
  if ( 0 == ch) {
    error_log << meteo::msglog::kServiceConnectFailedSimple.arg(meteo::settings::proto::kDataControl);
    return QByteArray();
  }
  resp = ch->remoteCall(&::mappi::proto::SessionDataService::GetSession, req, 10000, true);
  delete ch;

  QByteArray arr;
  if ( resp->sessions_size() > 0 && resp->sessions(0).has_tle() ) {
    arr = QByteArray(resp->sessions(0).tle().data());
    arr = QByteArray::fromBase64(arr);
  }
  delete resp;
  return arr;
}

} // map
} // meteo
