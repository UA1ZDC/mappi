#include <QtGui>

#include "plottingwidget.h"
#include "ui_plotting.h"
#include <meteo/commons/ui/mainwindow/widgethandler.h>
#include <meteo/commons/ui/custom/colorwidget.h>
#include <meteo/commons/ui/map/view/menu.h>
#include <meteo/commons/ui/map/view/widgets/layerswidget.h>
#include <cross-commons/debug/tlog.h>
#include <cross-commons/debug/tmap.h>

#include <commons/geobasis/geopoint.h>
#include <meteo/commons/global/dateformat.h>
#include <meteo/commons/global/global.h>
#include <meteo/commons/global/log.h>
#include <meteo/commons/ui/map/object.h>
#include <meteo/commons/ui/map/geopolygon.h>
#include <meteo/commons/ui/map/geotext.h>
#include <meteo/commons/ui/map/map.h>
#include <meteo/commons/ui/map/view/mapview.h>
#include <meteo/commons/ui/map/field.h>
#include <meteo/commons/proto/meteo.pb.h>
#include <commons/meteo_data/tmeteodescr.h>
#include <commons/textproto/tprototext.h>
#include <meteo/commons/proto/weather.pb.h>
#include <meteo/commons/ui/map/customevent.h>
#include <meteo/commons/ui/map/layeriso.h>
#include <meteo/commons/global/gradientparams.h>
#include <meteo/commons/ui/map/isoline.h>
#include <meteo/commons/ui/map/geogradient.h>
#include <meteo/commons/ui/map/layermenu.h>
#include <meteo/commons/ui/mainwindow/mainwindowplugin.h>
#include <meteo/commons/ui/custom/customtreewidgetitem.h>
#include <meteo/commons/ui/custom/gradienteditor.h>
#include <meteo/commons/ui/map/weather.h>
#include <meteo/commons/ui/mainwindow/mainwindow.h>

namespace
{
  int syncallTimeout() { return 100000; }
}

namespace meteo {
namespace map {
enum {
  Button          = 0,
  Center          = 1,
  Hour            = 2,
  Level           = 3,
  TypeLevel_name  = 4,
  CountPoint      = 5,
  Model           = 6,
  Id              = 7,
  Date            = 8,
  MeteoDescriptor = 9,
  NetTypeName     = 10,
  TypeLevel       = 11,
  NetType         = 12
};

PlottingWidget::PlottingWidget( MapWindow* parent )
 : MapWidget(parent),
ui_(new Ui::Plotting),
isoSettingsWidget_(nullptr),
currentItem_(-1),
currentLevel_(-1),
currentTypeLevel_(-1),
currentDataDescr_(-1),
currentHour_(0),
  allparams_( meteo::global::kIsoParamPath() )
{
  ui_->setupUi(this);
  ui_->dateEdit->setDisplayFormat(meteo::dtHumanFormatDateOnly);

  levelTypes_ = meteo::global::kLevelTypes();

  isoSettingsWidget_ = new IsoSettingsWidget( WidgetHandler::instance()->mainwindow() );
  QHBoxLayout* hbl = new QHBoxLayout(ui_->isoSettings);
  hbl->setMargin(0);
  hbl->addWidget(isoSettingsWidget_);
  isoSettingsWidget_->setDefault();
  isoSettingsWidget_->setDisabled(true);
  ui_->fillBtn->setDisabled(true);
  ui_->gradAlpha->setDisabled(true);

  ui_->addBtn->setDisabled(true);
  ui_->delBtn->setDisabled(true);

  ui_->fillBtn->setIcon(QIcon(":/meteo/icons/tools/fill.xpm"));

  QList<QRadioButton*> radio;
  foreach( QObject* o, ui_->levelsGrBox->children() ){
    QRadioButton* rb = static_cast<QRadioButton*>(o);
    if( nullptr != rb && rb->metaObject()->className() == QString("QRadioButton") ){
      radio.append(rb);
      QObject::connect( rb, SIGNAL(clicked()), SLOT(slotRadioButtonClicked()) );
    }
  }
  ui_->radioButton->setChecked(true);
  ui_->treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);

  ui_->splineBox->setCurrentIndex(1);
  QStringList lst;
  lst << ""<< "Центр"  << "Срок,ч" << "Уровень" << "Тип уровня"<< "Кол-во"
      << "Модель" << "N" << "Время" << "Дескриптор" << "Тип сетки,°";

  levels_ << 5 << 7 << 10 << 20 << 30 << 50 << 70 << 100 << 150
          << 200 << 250 << 300 << 400 << 500 << 700 << 850 << 925 << 1000;

  ui_->treeWidget->setHeaderLabels(lst);
  ui_->treeWidget->setRootIsDecorated(false);
  ui_->treeWidget->header()->setMinimumSectionSize(30);
  setupComboBox();

  ui_->treeWidget->setColumnHidden(Date, true);
  ui_->treeWidget->setColumnHidden(MeteoDescriptor, true);
  ui_->treeWidget->setColumnHidden(Id, true);
  ui_->treeWidget->setColumnHidden(Button, true);

  QObject::connect(ui_->treeWidget->header(), SIGNAL(sectionClicked(int)), this, SLOT(slotSorting(int)));
  QObject::connect(ui_->closeBtn, SIGNAL(clicked()), SLOT(close()));

  if( nullptr != mapdocument() ){
    if( nullptr != mapdocument()->eventHandler() ){
      mapdocument()->eventHandler()->installEventFilter(this);
    }
  }
  ui_->treeWidget->setColumnWidth(Level, 0);
  ui_->treeWidget->setColumnWidth(TypeLevel, 0);

  ui_->dateEdit->setDate(QDate::currentDate());
  QTime time;
  time.setHMS(QDateTime::currentDateTimeUtc().time().hour(), 0, 0);
  ui_->timeEdit->setTime(time);

  QObject::connect(ui_->setFill, SIGNAL(stateChanged(int)), SLOT(repaintSelf()));
  QObject::connect(ui_->setLine, SIGNAL(stateChanged(int)), SLOT(repaintSelf()));
  QObject::connect(ui_->fillBtn, SIGNAL(clicked()),SLOT(slotColorFillDialog()));
  QObject::connect(ui_->addBtn, SIGNAL(clicked()), SLOT(slotAddLayer()));
  QObject::connect(ui_->delBtn, SIGNAL(clicked()), SLOT(slotRemoveLayer()));
  QObject::connect(ui_->dateEdit, SIGNAL(dateTimeChanged(const QDateTime&)), SLOT(updateFields()));
  QObject::connect(ui_->timeEdit, SIGNAL(dateTimeChanged(const QDateTime&)), SLOT(updateFields()));
  QObject::connect(ui_->updateBtn, SIGNAL(clicked()), SLOT(updateFields()));
  QObject::connect(ui_->cB_type_data, SIGNAL(currentIndexChanged(const QString&)), SLOT(typeDataChange(const QString&)));
  QObject::connect(isoSettingsWidget_, SIGNAL(colorMinChanged(QColor)), SLOT(slotColorMinDialog(QColor)));
  QObject::connect(isoSettingsWidget_, SIGNAL(colorMaxChanged(QColor)), SLOT(slotColorMaxDialog(QColor)));
  QObject::connect(isoSettingsWidget_, SIGNAL(styleChange(Qt::PenStyle)), SLOT(slotStyleChanged(Qt::PenStyle)));
  QObject::connect(isoSettingsWidget_, SIGNAL(widthChange(int)), SLOT(slotWidthChanged(int)));
  QObject::connect(isoSettingsWidget_, SIGNAL(stepChange(float)), SLOT(slotStepChanged(float)));
  QObject::connect(isoSettingsWidget_, SIGNAL(minChange(float)), SLOT(slotMinChanged(float)));
  QObject::connect(isoSettingsWidget_, SIGNAL(maxChange(float)), SLOT(slotMaxChanged(float)));
  QObject::connect(isoSettingsWidget_, SIGNAL(settingsChange()), SLOT(slotSettingsChanged()));
  QObject::connect(ui_->gradAlpha, SIGNAL(currentIndexChanged(int)), SLOT( slotGradAlphaChanged()));
  QObject::connect( ui_->treeWidget, SIGNAL(customContextMenuRequested(QPoint)), SLOT(slotContextMenu(QPoint)));
  QObject::connect( ui_->treeWidget, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)), SLOT(slotItemSelectionChanged(QTreeWidgetItem*, QTreeWidgetItem*)));
  QObject::connect( ui_->treeWidget, SIGNAL(itemSelectionChanged()), SLOT(slotChangeSelection()));
  QObject::connect( ui_->settingsBtn, SIGNAL(clicked()), SLOT(slotSettingsClicked()));
  QObject::connect(ui_->hourCombo, SIGNAL(currentIndexChanged(QString)), SLOT(slotHourChanged(QString)));
  QObject::connect( ui_->treeWidget, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int )), SLOT(slotDoubleClicked(QTreeWidgetItem *, int )));

}

PlottingWidget::~PlottingWidget()
{
  delete ui_; ui_ = nullptr;
}

void PlottingWidget::closeEvent(QCloseEvent *e )
{
 /* Document* doc = mapdocument();
  if (doc != 0 && doc->eventHandler() != 0) {
    CustomEvent* ev = new CustomEvent(CustomEvent::ClearGeoPoints);
    doc->eventHandler()->postEvent(ev);
  }*/
  MapWidget::closeEvent(e);
}

void PlottingWidget::setupCheckBox()
{
  int lvlcount = isoparams_.level_size();
  if ( 0 == lvlcount ) {
    return;
  }

  if ( 1 == lvlcount && 100 == isoparams_.level(0).type_level() ) {
    currentTypeLevel_ = isoparams_.level(0).type_level();
    currentLevel_ = -1;
  }
  else {
    slotRadioButtonClicked();
  }
}

Layer* PlottingWidget::layerByItem(QTreeWidgetItem* item)
{
  if( nullptr == mapdocument() ) {
    return nullptr;
  }
  if( nullptr == item ){
    return nullptr;
  }
  field::DataDesc desc;
  global::arr2protomsg( item->data( Date, Qt::UserRole ).toByteArray(), &desc );
  QList<Layer*> list = mapdocument()->layers();
  for ( int i = 0, sz = list.size(); i < sz; ++i ) {
    Layer* l = list[i];
    if( true == l->isEqual(desc) ){
      return l;    }
  }
  return nullptr;
}

proto::WeatherLayer PlottingWidget::infoByItem( QTreeWidgetItem* item )
{
  Layer* l = layerByItem(item);
  if ( nullptr != l ) {
    return l->info();
  }
  proto::WeatherLayer info;
  if( nullptr == item ){
    return info;
  }
  field::DataDesc desc;
  global::arr2protomsg( item->data( Date, Qt::UserRole ).toByteArray(), &desc );
  if ( true == ui_->setLine->isChecked() && true == ui_->setFill->isChecked() ) {
    info.set_mode( proto::kIsoGrad );
  }
  else if ( true == ui_->setLine->isChecked() ) {
    info.set_mode( proto::kIsoline );
  }
  else if ( true == ui_->setFill->isChecked() ) {
    info.set_mode( proto::kGradient );
  }
  else {
    info.set_mode( proto::kEmptyLayer );
  }
  info.set_type( kLayerIso );
  info.set_source( proto::kField );
  info.set_center( desc.center() );
  info.set_model( desc.model() );
  info.set_level( desc.level() );
  info.set_type_level( desc.level_type() );
  info.set_template_name( QString::number( desc.meteodescr() ).toStdString() );
  info.set_datetime( desc.date() );
  info.set_hour( desc.hour()/3600 );
  info.set_data_size( desc.count_point() );
  info.set_center_name( global::kMeteoCenters()[info.center()].first.toStdString() );
  info.set_net_type(desc.net_type());
  info.add_meteo_descr(desc.meteodescr());
  return info;
}

void PlottingWidget::setupComboBox()
{
  QString text = ui_->cB_type_data->currentText();
  ui_->cB_type_data->clear();
  meteo::map::GradientParams settings( meteo::global::kIsoParamPath() );
  const proto::FieldColors& clrs = settings.protoParams();
  for(int i = 0, sz = clrs.color_size(); i < sz; ++i ){
    const proto::FieldColor& iso = clrs.color(i);
    int descr = iso.descr();
    QString title = QString::fromStdString(iso.name());
    ui_->cB_type_data->addItem(title,descr);
    // desc_.insert(descr, title);
  }
  int idx = ui_->cB_type_data->findText(text);
  if( -1 != idx ){
    ui_->cB_type_data->setCurrentIndex(idx);
  }
  if( 0 == ui_->cB_type_data->count() ){
    hideItems();
  }
  currentTypeData_ = ui_->cB_type_data->itemText( ui_->cB_type_data->currentIndex());
  currentDataDescr_ = ui_->cB_type_data->itemData( ui_->cB_type_data->currentIndex()).toInt();
}

void PlottingWidget::repaintLines(LayerIso* liso)
{
  if (nullptr == liso) return;

  if (ui_->setLine->checkState() == Qt::Checked) {
    if (liso->hasIsoline()) {
      liso->setFieldColor(isoparams_);
      std::unordered_set<IsoLine*> line = liso->objectsByType<IsoLine*>(true);
      for ( auto l: line ) {
        l->setVisible(true);
      }
    }
    else {
      int iso_count = liso->addIsoLines(isoparams_);
      if(0 < iso_count){
        liso->addExtremums();
      }
    }
  }
  else {
    //hide
    if (liso->hasIsoline()) {
      std::unordered_set<IsoLine*> line = liso->objectsByType<IsoLine*>(true);
      for ( auto l: line ) {
        l->setVisible(false);
      }
    }
  }
}

void PlottingWidget::repaintGradient(LayerIso* liso)
{
  if (ui_->setFill->checkState() == Qt::Checked) {
    if ( true == liso->hasGradient() ) {
      std::unordered_set<GeoGradient*> grad = liso->objectsByType<GeoGradient*>();
      for ( auto g: grad ) {
        g->setAlphaPercent(ui_->gradAlpha->currentText().toInt());
      }
      liso->showGradient();
    }
    else {
      //чтоб установить палитру для линии, если нет, или есть только общая
      TColorGradList gradlist = allparams_.gradParams( currentLevel_, currentTypeLevel_, isoparams_ );
      allparams_.setGradColor( currentLevel_, currentTypeLevel_, gradlist, &isoparams_ );
      liso->setFieldColor(isoparams_);
      liso->addGradient();
      //liso->addGradient(isoparams_);
      std::unordered_set<GeoGradient*> grad = liso->objectsByType<GeoGradient*>();
      for ( auto g: grad ) {
        g->setAlphaPercent(ui_->gradAlpha->currentText().toInt());
      }
    }
  }
  else {
    if ( true == liso->hasGradient()) {
      liso->hideGradient();
    }
  }
}

void PlottingWidget::repaintSelf(bool deletelines /* = false*/, bool deletegrad /*= false*/)
{
  if( nullptr == mapdocument() ) {
    return;
  }
  QTreeWidgetItem* item = ui_->treeWidget->currentItem();
  if( nullptr == item ){
    return;
  }
  localOpacity_.insert(item->text(Id), ui_->gradAlpha->currentIndex());

  Layer* l = layerByItem(item);
  if( nullptr == l ){
    return;
  }

  LayerIso* liso = maplayer_cast<LayerIso*>(l);
  if( nullptr != liso ) {
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    if (deletelines) {
      liso->removeIsolines();
    }
    if (deletegrad) {
      liso->removeGradient();
    }
    repaintLines(liso);
    repaintGradient(liso);
    QApplication::restoreOverrideCursor();
    if (!deletelines) {
      liso->repaint();
    }
  }
  mapdocument()->eventHandler()->notifyDocumentChanges(DocumentEvent::Changed);
}

void PlottingWidget::addLayer(QTreeWidgetItem* item)
{
  if ( nullptr == mapdocument() ) {
    return;
  }
  if ( true == hasLayer(item) ){
    return;
  }
  qApp->setOverrideCursor(QCursor(Qt::WaitCursor));

  proto::WeatherLayer info = infoByItem(item);
  Weather weather;
  Layer* layer = nullptr;
  QString id = item->text(Id); //mongo DB ID is QString

  if ( true == localparms_.contains(id) ) {
    layer = weather.buildIsoLayer( mapdocument(), info, ui_->splineBox->currentText().toInt(), localparms_[id] );
  }
  else {
    layer = weather.buildIsoLayer( mapdocument(), info, ui_->splineBox->currentText().toInt() );
  }

  if ( nullptr == layer ) {
    error_log << QObject::tr("Не удалось создать слой");
    qApp->restoreOverrideCursor();
    return;
  }
  std::unordered_set<GeoGradient*> grad = layer->objectsByType<GeoGradient*>();
  for ( auto g: grad ) {
    g->setAlphaPercent(ui_->gradAlpha->currentText().toInt());
  }
  updateItems();
  qApp->restoreOverrideCursor();
}


void PlottingWidget::removeLayer(QTreeWidgetItem* item)
{
  if( nullptr == mapdocument() ) {
    return;
  }
  Layer* l = layerByItem(item);
  if( nullptr == l ){
    return;
  }
  delete l;
}

void PlottingWidget::updateCurrentSettings()
{
  QTreeWidgetItem* item = ui_->treeWidget->currentItem();
  if( nullptr == item || nullptr == isoSettingsWidget_){
    return;
  }
  QString id = item->text(Id);

  proto::LevelColor level = GradientParams::levelProto(currentLevel_, currentTypeLevel_, isoparams_);
  level.mutable_color()->set_min_color( isoSettingsWidget_->colorMin().rgba() );
  level.mutable_color()->set_max_color( isoSettingsWidget_->colorMax().rgba() );
  level.set_step_iso(isoSettingsWidget_->step());
  level.mutable_color()->set_min_value(isoSettingsWidget_->min());
  level.mutable_color()->set_max_value(isoSettingsWidget_->max());
  GradientParams::setLevelProto(currentLevel_, currentTypeLevel_, level, &isoparams_);
  QPen pen = pen2qpen( isoparams_.pen() );
  pen.setStyle( isoSettingsWidget_->lineStyle() );
  pen.setWidth( isoSettingsWidget_->lineWidth() );
  isoparams_.mutable_pen()->CopyFrom( qpen2pen(pen) );
  localparms_.insert(id, isoparams_);
}

bool PlottingWidget::hasLayer(QTreeWidgetItem* item)
{
  if( nullptr == mapdocument() ) {
    return false;
  }
  if( nullptr == item ){
    return false;
  }
  field::DataDesc desc;
  global::arr2protomsg( item->data( Date, Qt::UserRole ).toByteArray(), &desc );

  QList<Layer*> list = mapdocument()->layers();
  for ( int i = 0, sz = list.size(); i < sz; ++i ) {
    Layer* l = list[i];
    if( true == l->isEqual(desc) ) {
      return true;
    }
  }
  return false;
}

bool PlottingWidget::hasLayer(Layer* layer)
{
  if ( nullptr == mapdocument() ){
    return false;
  }
  if( true == mapdocument()->layers().contains(layer)  ){
    return true;
  }
  return false;
}

void PlottingWidget::setCurrentIsoDescriptor( int32_t descr )
{
  if (descr != currentDataDescr_) {
    currentItem_ = -1;
  }

  int indx = ui_->cB_type_data->findData(descr);
  if ( 0 > indx ) {
    warning_log << msglog::kGuiIncorrectElement.arg(QObject::tr("дескриптор %1").arg(descr));
    return;
  }
  ui_->cB_type_data->setCurrentIndex(indx);
  currentTypeData_ = ui_->cB_type_data->itemText( ui_->cB_type_data->currentIndex());
  currentDataDescr_ = descr;
  setupCheckBox();
}

QString PlottingWidget::currentIsoName()
{
  return ui_->cB_type_data->currentText();
}

void PlottingWidget::setCurrentIsoDescriptor( const QString& name )
{
  int indx = ui_->cB_type_data->findText(name);
  if ( 0 > indx ) {
    warning_log << msglog::kGuiIncorrectElement.arg(QObject::tr("вид изолинии %1").arg(name));
    return;
  }
  ui_->cB_type_data->setCurrentIndex(indx);
  currentTypeData_ = ui_->cB_type_data->itemText( ui_->cB_type_data->currentIndex());
  currentDataDescr_ = ui_->cB_type_data->itemData(indx).toInt();
  setupCheckBox();
}

//! Установка даты/времени последних доступных данных (без обновления списка полей)
void PlottingWidget::setLastDataDate(const QString& isoname)
{
  updateCurrentLevel();
  int indx = ui_->cB_type_data->findText(isoname);
  if ( 0 > indx ) {
    return;
  }

  uint32_t descr = ui_->cB_type_data->itemData(indx).toUInt();
  QDateTime end = QDateTime::currentDateTimeUtc();
  QDateTime last = end.addDays(-1);

  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
  auto reply = std::unique_ptr<meteo::field::DataDescResponse>(sendRequest(last, end, descr));
  if( nullptr == reply || 0 == reply->descr_size()){
    QApplication::restoreOverrideCursor();
    return;
  }

  for( int i = 0; i < reply->descr_size(); i++){
    const meteo::field::DataDesc& d = reply->descr(i);
    QDateTime dt = QDateTime::fromString(QString::fromStdString (d.date()), Qt::ISODate);
    if (last < dt) {
      last = dt;
    }
  }

  ui_->dateEdit->blockSignals(true);
  ui_->timeEdit->blockSignals(true);

  ui_->dateEdit->setDate(last.date());
  ui_->timeEdit->setTime(QTime(last.time().hour(), 0, 0));

  ui_->dateEdit->blockSignals(false);
  ui_->timeEdit->blockSignals(false);

  QApplication::restoreOverrideCursor();
}

field::DataDescResponse* PlottingWidget::sendRequest(const QDateTime& start, const QDateTime& end, uint32_t descr)
{
  if( true == currentTypeData_.isEmpty() ){
    return nullptr;
  }


  meteo::field::DataRequest request;
  request.add_meteo_descr(descr);
  request.set_date_start(QString("%1").arg(start.addSecs(0).toString("yyyy-MM-dd hh:mm:ss")).toStdString());
  request.set_date_end(QString("%1").arg(end.addSecs(0).toString("yyyy-MM-dd hh:mm:ss")).toStdString());
  //надоело размышлять за какой срок выбрать прогноз. Теперь будет выбор вместе с прогнозами на эту дату.
  //request.set_forecast_start(QString("%1").arg(start.addSecs(0).toString("yyyy-MM-dd hh:mm:ss")).toStdString());
  //request.set_forecast_end(QString("%1").arg(end.addSecs(0).toString("yyyy-MM-dd hh:mm:ss")).toStdString());

  request.clear_type_level();
  request.clear_level();

  if( -1 != currentTypeLevel_ ){
    request.add_type_level(currentTypeLevel_);
  }
  if( -1 != currentLevel_ ){
    request.add_level(currentLevel_);
  }
  if( -1 != currentHour_ ){
    request.add_hour(currentHour_);
  }

  meteo::field::DataDescResponse* reply = nullptr;
  meteo::rpc::Channel* ctrl_field_ =  meteo::global::serviceChannel(meteo::settings::proto::kField);
  if(nullptr == ctrl_field_) {
    error_log.msgBox() << msglog::kNoConnect.arg(meteo::global::serviceTitle(meteo::settings::proto::kField));
  }
  else {
    reply = ctrl_field_->remoteCall( &meteo::field::FieldService::GetAvailableData, request, ::syncallTimeout());
    delete ctrl_field_;
  }
  if ( nullptr == reply ) {
    error_log << msglog::kServiceAnswerFailed.arg(meteo::global::serviceTitle(settings::proto::kField));
  }
  return reply;
}

void PlottingWidget::hideItems()
{
  QTreeWidgetItemIterator it( ui_->treeWidget );
  while( *it ){
    (*it)->setHidden(true);
    ++it;
  }
}

void PlottingWidget::updateItems()
{
  if( nullptr == mapdocument() ){
    return;
  }
  if( nullptr == ui_){
    return;
  }
  if ( true == hasLayer( ui_->treeWidget->currentItem() ) ){
    ui_->addBtn->setDisabled(true);
    ui_->delBtn->setDisabled(false);
  }
  else{
    ui_->addBtn->setDisabled(false);
    ui_->delBtn->setDisabled(true);
  }
  QTreeWidgetItem* item = nullptr;
  for( int j = 0; j < ui_->treeWidget->topLevelItemCount(); j++ ){
    item = ui_->treeWidget->topLevelItem(j);
    if( nullptr == item ){
      continue;
    }
    if( hasLayer(item) ){
      for( int i = 0; i <= ui_->treeWidget->columnCount(); i++ ){
        item->setBackgroundColor(i, Qt::yellow);
      }
    }
    else{
      for( int i = 0; i <= ui_->treeWidget->columnCount(); i++ ){
        item->setBackgroundColor(i, Qt::white);
      }
    }
  }
}

bool PlottingWidget::eventFilter(QObject* watched, QEvent* ev)
{
  if( nullptr == mapdocument() ){
    return MapWidget::eventFilter(watched, ev);
  }
  if ( watched !=  mapdocument()->eventHandler()) {
    return MapWidget::eventFilter(watched, ev);
  }
  else if ( meteo::map::LayerEvent::LayerChanged == ev->type() ) {
    updateItems();
    QTreeWidgetItem* item = ui_->treeWidget->currentItem();
    if ( nullptr != item ) {
      QString id = item->text(Id);
      LayerIso* iso = maplayer_cast<LayerIso*>( layerByItem(item) );
      if ( nullptr != iso && nullptr != isoSettingsWidget_ ) {
        isoSettingsWidget_->update(iso);
        localparms_[id] = iso->fieldColor();
      }
    }
  }
  return MapWidget::eventFilter(watched, ev);
}

void PlottingWidget::typeDataChange(const QString &at ){
  currentTypeData_ = at;
  currentDataDescr_ = ui_->cB_type_data->itemData( ui_->cB_type_data->currentIndex()).toInt();
  setupCheckBox();
  updateFields();
}

void PlottingWidget::updateFields()
{
  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
  updateCurrentLevel();
  QDateTime start = QDateTime(ui_->dateEdit->date(), ui_->timeEdit->time());
  auto reply = std::unique_ptr<meteo::field::DataDescResponse>(sendRequest(start, start.addSecs(59*60+59), currentDataDescr_));
  if( nullptr == reply ){
    QApplication::restoreOverrideCursor();
    return;
  }
  if ( start != QDateTime( ui_->dateEdit->date(), ui_->timeEdit->time() ) ){
    QApplication::restoreOverrideCursor();
    return;
  }
  ui_->treeWidget->clear();
  for( int i = 0; i < reply->descr_size(); i++){
    const meteo::field::DataDesc& d = reply->descr(i);
    QString date_str = QString::fromStdString (d.date());
    QDateTime dt = QDateTime::fromString(date_str, Qt::ISODate);
    CustomTreeWidgetItem* item = new CustomTreeWidgetItem(ui_->treeWidget);
    QByteArray arr;
    global::protomsg2arr( d, &arr );
    item->setData( Date, Qt::UserRole, arr );
    item->setText(Date, dt.toString("yyyy-MM-dd hh:mm"));
    item->setText(CountPoint, QString::number(d.count_point()));
    item->setText(MeteoDescriptor, QString::number(d.meteodescr()));
    item->setText(Center, QString::number(d.center()));
    item->setData(Center, Qt::UserRole, d.center());
    item->setText(Level, QString::number(d.level()));
    item->setText(Hour, QString::number(d.hour()/3600));
    item->setText(Model, QString::number(d.model()));
    item->setText(Id, QString::fromStdString(d.id()));
    //item->setText(TypeLevel, QString::number(d.level_type()));
    item->setText(Center, QString::fromStdString(d.center_name()));
    item->setText(TypeLevel_name, levelTypes_.value(d.level_type()));
    //item->setText(NetType, QString::number(d.net_type()));
    item->setText(NetTypeName, obanal::netTypetoStr(::NetType(d.net_type())));
  }

  for (int i = 0, sz = ui_->treeWidget->columnCount(); i < sz; ++i)
  {
    if( i != Center && i != TypeLevel){
      ui_->treeWidget->resizeColumnToContents(i);
    }
  }

 // qApp->processEvents();



  updateItems();
  isoSettingsWidget_->setColorMin(Qt::white);
  isoSettingsWidget_->setColorMax(Qt::white);
  isoSettingsWidget_->setDefault();
  isoSettingsWidget_->setDisabled(true);
  ui_->fillBtn->setDisabled(true);
  ui_->gradAlpha->setDisabled(true);
  if (-1 != currentItem_) {
    QTreeWidgetItem* item = ui_->treeWidget->topLevelItem(currentItem_);
    if (nullptr != item) {
      ui_->treeWidget->setCurrentItem(item);
    }
  }
  QApplication::restoreOverrideCursor();
}

void PlottingWidget::updateCurrentLevel()
{
  QRadioButton* rb = static_cast<QRadioButton*>(ui_->buttonGroup->checkedButton());
  if(nullptr == rb ){
    return;
  }
  QString t = rb->text();
  t.replace("&","");
  if(QObject::tr("Все") == t){
    currentLevel_ = -1;
    currentTypeLevel_ = -1;
  }
  else if(QObject::tr("Приземный") == t){
    currentLevel_ = 0;
    currentTypeLevel_ = 1;
  }
  else if(QObject::tr("Тропопауза") == t){
    currentLevel_ = 0;
    currentTypeLevel_ = 7;
  }
  else if(QObject::tr("Макс.ветер") == t){
    currentLevel_ = 0;
    currentTypeLevel_ = 6;
  }
  else if(QObject::tr("Уровень моря") == t){
    currentLevel_ = 0;
    currentTypeLevel_ = 101;
  }
  else if(QObject::tr("OT500/1000") == t){
    currentLevel_ = 15000;
    currentTypeLevel_ = 15000;
  }
  else if(QObject::tr("<5") == t){
    currentLevel_ = 2;
    currentTypeLevel_ = 100;
  }
  else{
    currentLevel_ = t.toInt();
    currentTypeLevel_ = 100;
  }
}

void PlottingWidget::slotSettingsClicked()
{
  app::MainWindowPlugin* plug = WidgetHandler::instance()->mainwindow()->plugin("settingsplugin");
  if ( nullptr != plug ) {
    QWidget* wgt = plug->createWidget(WidgetHandler::instance()->mainwindow(), QString::number(currentDataDescr_) );
    if ( nullptr != wgt ) {
      wgt->setWindowFlags(Qt::Dialog);
      wgt->show();
      connect(wgt, SIGNAL(updateSettings()), SLOT(updateParams()));
    }
  }
}

void PlottingWidget::updateParams()
{
  allparams_.loadParamsFromDir();

  QTreeWidgetItem * item = ui_->treeWidget->currentItem();
  if (nullptr != item) {
    QString id = item->text(Id);
    int descr = item->text(MeteoDescriptor).toInt();
    if ( !localparms_.contains(id) ) {
      isoparams_.CopyFrom(allparams_.protoParams(descr));
    }
    isoSettingsWidget_->setSettings(currentLevel_, currentTypeLevel_, isoparams_);
  }
}

void PlottingWidget::slotRadioButtonClicked()
{
  updateFields();
}

void PlottingWidget::slotColorMinDialog(const QColor& color)
{
  Q_UNUSED(color);
  updateCurrentSettings();
  repaintSelf();
}

void PlottingWidget::slotColorMaxDialog(const QColor& color)
{
  Q_UNUSED(color);
  updateCurrentSettings();
  repaintSelf();
}

void PlottingWidget::slotStyleChanged(Qt::PenStyle style)
{
  updateCurrentSettings();
  Q_UNUSED(style)
  repaintSelf();
}

void PlottingWidget::slotWidthChanged(int width)
{
  updateCurrentSettings();
  Q_UNUSED(width)
  repaintSelf();
}

void PlottingWidget::slotStepChanged(float step)
{
  updateCurrentSettings();
  Q_UNUSED(step);
  repaintSelf(true);
}

void PlottingWidget::slotMinChanged(float min)
{
  Q_UNUSED(min);
  updateCurrentSettings();
  repaintSelf(true);
}

void PlottingWidget::slotMaxChanged(float max)
{
  Q_UNUSED(max);
  updateCurrentSettings();
  repaintSelf(true);
}

void PlottingWidget::slotGradAlphaChanged()
{
  updateCurrentSettings();
  repaintSelf();
}


void PlottingWidget::slotSettingsChanged()
{
  updateCurrentSettings();
  repaintSelf();
}

void PlottingWidget::slotSorting(int col)
{
  Qt::SortOrder order = ui_->treeWidget->header()->sortIndicatorOrder();
  ui_->treeWidget->sortByColumn(col, order);
}

void PlottingWidget::slotSwitchLayer(bool on)
{
  ItemWidget* w = static_cast<ItemWidget*>(sender());
  if( nullptr == w ){
    return;
  }
  QTreeWidgetItem* item = itemsWidget_.key(w);
  if( nullptr == item ){
    return;
  }
  ui_->treeWidget->setCurrentItem(item);
  if( true == on ){
    addLayer(item);
  }else{
    removeLayer(item);
  }
}

void PlottingWidget::slotHourChanged(const QString& atext)
{
  if(QObject::tr("Фактический") == atext){
    currentHour_ = 0;
  } else if(QObject::tr("Все") == atext){
    currentHour_ = -1;
  } else {
    currentHour_ = atext.toInt()*3600;
  }

  updateFields();


}

void PlottingWidget::slotColorMinChanged()
{
  QColorDialog* dlg = new QColorDialog(this);
  if( dlg->exec() == QDialog::Accepted  ){
    if( nullptr != isoSettingsWidget_ ){
      isoSettingsWidget_->setColorMin(dlg->currentColor());
    }
  }
}

void PlottingWidget::slotColorMaxChanged()
{
  QColorDialog* dlg = new QColorDialog(this);
  if( dlg->exec() == QDialog::Accepted  ){
    if( nullptr != isoSettingsWidget_ ){
      isoSettingsWidget_->setColorMax(dlg->currentColor());
    }
  }
}

void PlottingWidget::slotStyleChanged()
{
  QAction* act = static_cast<QAction*>(sender());
  if( nullptr == act ){
    return;
  }
  if( nullptr != isoSettingsWidget_ ){
    isoSettingsWidget_->setStyle((Qt::PenStyle)act->data().toInt());
  }
}

void PlottingWidget::slotWidthChanged()
{
  QAction* act = qobject_cast<QAction*>(sender());
  if( nullptr == act ){
    return;
  }
  if( nullptr != isoSettingsWidget_ ){
    QString atext = act->text();
    atext.replace( "&", "" );
    isoSettingsWidget_->setWidth(atext.toInt());
  }
}

void PlottingWidget::slotStepChanged()
{
  // QAction* act = static_cast<QAction*>(sender());
  // if( 0 == act || 0 == stepEdit_ ){
  //   return;
  // }

  // if (!act->text().isEmpty()) {
  //   stepEdit_->setValue(act->text().toDouble());
  // }
  // slotStepCustomChanged();
}

void PlottingWidget::slotStepCustomChanged()
{
//  if( 0 == stepEdit_ ){
//    return;
//  }
//  if( 0 != isoSettingsWidget_ ){
//    isoSettingsWidget_->setStep(stepEdit_->value());
//  }
}
void PlottingWidget::slotDoubleClicked(QTreeWidgetItem * item, int){
  if ( nullptr == item ) {
    return;
  }
  if ( hasLayer(item)) {
      slotRemoveLayer();
      return;
    }

  slotAddLayer();
}

void PlottingWidget::slotAddLayer()
{
  QTreeWidgetItem* item = ui_->treeWidget->currentItem();
  if( nullptr == item ){
    return;
  }
  addLayer(item);
}

void PlottingWidget::slotContextMenu(QPoint point)
{
  if( nullptr == mapdocument() ){
    return;
  }
  QMenu* menu = new QMenu(this);
  QAction* add = menu->addAction(QIcon(":/meteo/icons/tools/plus.png"), QObject::tr("Добавить слой") );
  connect( add, SIGNAL(triggered()), SLOT(slotAddLayer()));

  QAction* del = menu->addAction(QIcon(":/meteo/icons/tools/minus.png"), QObject::tr("Удалить слой") );
  connect( del, SIGNAL(triggered()), SLOT(slotRemoveLayer()));

  QTreeWidgetItem* item = ui_->treeWidget->currentItem();
  if( nullptr == item || nullptr == del ){
    return;
  }
  if( hasLayer(item) ){
    del->setDisabled(false);
    add->setDisabled(true);
  }
  else{
    del->setDisabled(true);
    add->setDisabled(false);
  }
  Layer* l = layerByItem(item);
  if ( nullptr != l ) {
    LayerMenu* lm = l->layerMenu();
    lm->setLayersForRemove( QList<Layer*>() << l  );
    lm->addActions(menu);
  }

  menu->exec(ui_->treeWidget->mapToGlobal(point));
  delete menu;
}

void PlottingWidget::slotRemoveLayer()
{
  QTreeWidgetItem* item = ui_->treeWidget->currentItem();
  if( nullptr == item ){
    return;
  }
  removeLayer(item);
}

void PlottingWidget::slotItemSelectionChanged(QTreeWidgetItem * item, QTreeWidgetItem* prev)
{
  if (prev == item) return;

  if( nullptr != isoSettingsWidget_ ){
    isoSettingsWidget_->setDefault();
    isoSettingsWidget_->setDisabled(true);
    ui_->fillBtn->setDisabled(true);
    ui_->gradAlpha->blockSignals(true);
    ui_->gradAlpha->setDisabled(true);
    ui_->gradAlpha->setCurrentIndex(3);
    ui_->gradAlpha->blockSignals(false);
  }
  if( nullptr == item ){
    return;
  }
  LayerIso* lay = nullptr;
  ui_->setLine->blockSignals(true);
  ui_->setFill->blockSignals(true);
  if(nullptr != ( lay = maplayer_cast<LayerIso*>(layerByItem(item)))) {
    ui_->addBtn->setDisabled(true);
    ui_->delBtn->setDisabled(false);

    Qt::CheckState state = Qt::Unchecked;
    std::unordered_set<IsoLine*> line = lay->objectsByType<IsoLine*>();


    if (line.size() != 0
        && *(line.begin()) != nullptr
        && (*line.begin())->visible()) {
      state = Qt::Checked;
    }
    ui_->setLine->setCheckState(state);
    state = Qt::Unchecked;
    std::unordered_set<GeoGradient*> grad = lay->objectsByType<GeoGradient*>();
    if (grad.size() != 0 && (*grad.begin()) != nullptr && (*grad.begin())->visible()) {
      state = Qt::Checked;
    }
    ui_->setFill->setCheckState(state);
  }
  else{
    ui_->addBtn->setDisabled(false);
    ui_->delBtn->setDisabled(true);
    ui_->setLine->setCheckState(Qt::Checked);
    ui_->setFill->setCheckState(Qt::Unchecked);
  }
  ui_->setLine->blockSignals(false);
  ui_->setFill->blockSignals(false);
  QString id = item->text(Id);
  int descr = item->text(MeteoDescriptor).toInt();
  field::DataDesc desc;
  global::arr2protomsg( item->data( Date, Qt::UserRole ).toByteArray(), &desc );
  currentLevel_ = desc.level();
  currentTypeLevel_= desc.level_type();

  ui_->fillBtn->setDisabled(false);
  ui_->gradAlpha->setDisabled(false);
  isoSettingsWidget_->setDisabled(false);

  if ( true == localparms_.contains(id) ) {
    isoparams_.CopyFrom(localparms_[id]);
  }
  else {
    isoparams_.CopyFrom(allparams_.protoParams(descr));
  }
  isoSettingsWidget_->setSettings(currentLevel_, currentTypeLevel_, isoparams_);

  ui_->gradAlpha->blockSignals(true);
  if (localOpacity_.contains(id)) {
    ui_->gradAlpha->setCurrentIndex(localOpacity_.value(id));
  }
  else {
    ui_->gradAlpha->setCurrentIndex(3);
  }
  ui_->gradAlpha->blockSignals(false);
}

void PlottingWidget::slotChangeSelection()
{
  if(ui_->treeWidget->selectionModel()->hasSelection()) {
    QTreeWidgetItem* current = ui_->treeWidget->selectedItems().first();
    if (current != nullptr) {
      QString id = current->text(Id);
      if ( false == id.isEmpty() ) {
        sendSelectPointsEvent(id);
      }
    }
  }
}

void PlottingWidget::sendSelectPointsEvent(QString fieldId) const
{
  Document* doc = mapdocument();
  if (doc != nullptr && doc->eventHandler() != nullptr) {
    field::SimpleDataRequest* req = new field::SimpleDataRequest();
    req->add_id(fieldId.toStdString());
    CustomEvent* ev = new CustomEvent(CustomEvent::SelectGeoPointsByField);
    ev->setOwnedMessage(req);
    doc->eventHandler()->postEvent(ev);
  }
}

void PlottingWidget::slotColorFillDialog()
{
  GradientEditor* dlg = nullptr;
  TColorGradList gradlist = allparams_.gradParams( currentLevel_, currentTypeLevel_, isoparams_ );
  if ( 0 != gradlist.size() ) {
    dlg = new GradientEditor( gradlist, this);
  }
  else {
    TColorGrad grad = allparams_.isoParams( currentLevel_, currentTypeLevel_, isoparams_ );
    dlg = new GradientEditor(
        grad.begval(),
        grad.begcolor(),
        grad.endval(),
        grad.endcolor(),
        isoparams_.step_grad(), this );
  }

  dlg->setWindowTitle("Настройка заливки");
  dlg->show();
  if (dlg->exec() == QDialog::Accepted) {
    allparams_.setGradColor( currentLevel_, currentTypeLevel_, dlg->gradient(), &isoparams_ );
    updateCurrentSettings();
    if (ui_->setFill->isChecked()) {
      repaintSelf(false, true);
    }
  }

  delete dlg;
}

ItemWidget::ItemWidget(QWidget* parent):
QWidget(parent)
{
  btn_ = new QToolButton(this);
  QHBoxLayout* hbl = new QHBoxLayout(this);
  hbl->addWidget(btn_);
  hbl->setMargin(5);
  //btn_->setAutoRaise(true);
  btn_->setCheckable(true);
  btn_->setMinimumSize(QSize(18,18));
  btn_->setMaximumSize(QSize(18,18));
  connect(btn_, SIGNAL(toggled(bool)), this, SIGNAL(toggled(bool)));
}

bool ItemWidget::isChecked()
{
  return btn_->isChecked();
}

void ItemWidget::setChecked(bool on)
{
  btn_->setChecked(on);
}

}
}
