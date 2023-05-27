#include "jobwidget.h"
#include "selectwidget.h"
#include "ui_job.h"
#include <qpushbutton.h>
#include <qfile.h>
#include <qmessagebox.h>
#include <qdatetime.h>
#include <qfiledialog.h>
#include <QTreeWidgetItem>
#include <meteo/commons/global/global.h>
#include <meteo/commons/proto/surface_service.pb.h>
#include <meteo/commons/proto/meteo.pb.h>
#include <meteo/commons/proto/sprinf.pb.h>

namespace {

const QStringList& basicCenters()
{
  static QStringList bs;
  if( true == bs.isEmpty() ){
    bs.append("26");
    bs.append("30");
    bs.append("93");
    bs.append("34");
    bs.append("7");
    bs.append("4");
    bs.append("74");
    bs.append("98");
    bs.append("250");
  }
  return bs;
}

const QMap< int, int > storetypes()
{
  static QMap< int, int > types;
  types.insert( 0, meteo::map::proto::kNoSave );
  types.insert( 1, meteo::map::proto::kSaveServerFile );
  types.insert( 2, meteo::map::proto::kSaveClientFile );
  types.insert( 3, meteo::map::proto::kSaveServerFile | meteo::map::proto::kSaveClientFile );
  return types;
}

const QStringList& basicSizes()
{
  static QStringList bs;
  if( true == bs.isEmpty() ){
    bs.append("640 x 480");
    bs.append("800 x 600");
    bs.append("1024 x 768");
    bs.append("1600 x 1200");
    bs.append("1920 x 1080");
  }
  return bs;
}

QStringList centerModels(int center_id)
{
  QStringList models;
  switch(center_id){
    case 4 :
      models.append("11");
      models.append("2");
      models.append("15");
      models.append("1");
      break;
    case 34 :
      models.append("4");
      break;
    case 7 :
      models.append("81");
      models.append("96");
      break;
    case 74 :
      models.append("15");
      models.append("45");
      break;
    case 98:
      models.append("146");
      break;
    case 250:
      models.append("250");
//      models.append("251");
      break;
  }
  return models;
}

const QStringList& basicHours(){
  static QStringList lst;
  if( true == lst.isEmpty() ){
    lst.append("0");
    lst.append("6");
    lst.append("12");
    lst.append("18");
    lst.append("24");
    lst.append("30");
    lst.append("36");
    lst.append("42");
    lst.append("48");
    lst.append("54");
    lst.append("60");
    lst.append("66");
    lst.append("72");
    lst.append("78");
    lst.append("84");
    lst.append("90");
    lst.append("96");
    lst.append("120");
    lst.append("144");
    lst.append("168");
    lst.append("192");
    lst.append("216");
    lst.append("240");
  }
  return lst;
}
}

namespace meteo {
namespace map {

enum{
  TemplateName   = 0,
  ViewMode       = 1,
  TemplateTitle  = 2,
  Level          = 3,
  TypeLevel      = 4,
  LevelTitle     = 5,
  TypeLevelTitle = 6,
  DataType       = 7,
  Layer          = 8,
  Transparency   = 9
};

JobWidget::JobWidget(QWidget *parent) :
  QDialog(parent),
  ui_(new Ui::Job),
  hourMenu_(0),
  addLayer_(0)
{
  ui_->setupUi(this);

  ui_->sizeTemplate->addItems(::basicSizes());
  QObject::connect( ui_->sizeTemplate, SIGNAL(currentIndexChanged(const QString&)), SLOT(slotChangeSize(const QString&)));
  QObject::connect( ui_->centerBox, SIGNAL(currentIndexChanged(QString)), SLOT(slotCenterChange(QString)));
  QObject::connect( ui_->addDateBtn, SIGNAL(clicked()), SLOT(slotAddDate()));
  QObject::connect( ui_->hourButton, SIGNAL(clicked()), SLOT(slotHourButtonClicked()));
  QObject::connect( ui_->btnfile, SIGNAL(clicked()), SLOT(chooseFile()));
  QObject::connect( ui_->okbtn, SIGNAL( clicked() ), this, SLOT( slotOkButton() ) );
  QObject::connect( ui_->nobtn, SIGNAL( clicked() ), this, SLOT( slotNoButton() ) );
  QObject::connect( ui_->cmbstoretype, SIGNAL( currentIndexChanged( int ) ), this, SLOT( slotChangeStoreType() ) );

  ui_->edit_task_name->setPlaceholderText(tr("Введите имя"));
  QObject::connect(ui_->edit_task_name, SIGNAL(textChanged(QString)), SLOT(slotNameChanged()));

  ui_->mapName->setPlaceholderText(tr("Введите имя"));
  QObject::connect(ui_->mapName, SIGNAL(textChanged(QString)), SLOT(slotNameChanged()));

  QObject::connect(ui_->layerTree, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)), SLOT(slotItemChanged()));

  ui_->editTimeSheet->setIcon(QIcon(":/meteo/icons/edit.png"));
  QObject::connect(ui_->editTimeSheet, SIGNAL(clicked()), SLOT(slotEditTimeSheet()));

  slotChangeSize(ui_->sizeTemplate->currentText());
  ui_->addDateEdit->setDate(QDate::currentDate());

  QObject::connect(ui_->addDateBtn, SIGNAL(clicked()), SLOT(slotAddDate()));
  QObject::connect(ui_->clearBtn, SIGNAL(clicked()), this, SLOT( slotRemoveDate() ) );
  ui_->hourButton->setIcon(QIcon(":/meteo/icons/settings.png"));
  ui_->file_path->setText(QDir::homePath());

  ui_->incutVisibilityButton->setIcon(QIcon(":/meteo/icons/tools/layer_visible.png"));
  ui_->legendVisibilityButton->setIcon(QIcon(":/meteo/icons/tools/layer_visible.png"));
  QObject::connect(ui_->incutVisibilityButton, SIGNAL(clicked()),SLOT(slotOnIncutVisible()));
  QObject::connect(ui_->legendVisibilityButton, SIGNAL(clicked()),SLOT(slotOnLegendVisible()));

  getMeteoCenters();
  for( const QString& center_id : basicCenters() ){
    if( centers_.contains(center_id) ){
      ui_->centerBox->addItem(centers_[center_id]);
    }
    else{
      ui_->centerBox->addItem(center_id);
    }
  }
  ui_->tabWidget->setStyleSheet(QString("QComboBox,QSpinBox,QLineEdit,QLabel,QCheckBox,QDateEdit,QToolButton{min-height:20px}"));

  ui_->addLayerBtn->setIcon(QIcon(":/meteo/icons/plus.png"));
  ui_->editLayerBtn->setIcon(QIcon(":/meteo/icons/edit.png"));
  ui_->delLayerBtn->setIcon(QIcon(":/meteo/icons/minus.png"));
  connect(ui_->addLayerBtn, SIGNAL(clicked()), SLOT(slotAddLayer()));
  connect(ui_->editLayerBtn, SIGNAL(clicked()), SLOT(slotEditLayer()));
  connect(ui_->delLayerBtn, SIGNAL(clicked()), SLOT(slotRemoveLayer()));

  connect(ui_->layerTree, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)), SLOT(slotEditLayer()));
  ui_->layerTree->header()->setVisible(true);

  incutInit();
  legendInit();
  createMenu();

//  ui_->tab_5->setVisible(false); ///TODO: доделать врезку
  ui_->fileNameEdit->setVisible(false); ///TODO: доделать изменение имени файла
  ui_->label_3->setVisible(false);
}

JobWidget::~JobWidget()
{

}

proto::Job JobWidget::job()
{
  proto::Job job;
  QString name, title;
  title = (ui_->edit_task_name->text().isEmpty() ? "unknown" : ui_->edit_task_name->text());

  if ( false == jobName_.isEmpty() ) {
    name = jobName_;
  }
  else {
    QString mapName;
    if ( false == mapName_.isEmpty() ) {
      mapName = mapName_;
    }
    else {
      mapName = global::kTranslitFunc(ui_->mapName->text());
    }
    name = QString("%1_%2.job").arg(global::kTranslitFunc(title)).arg(mapName);
  }
  job.set_name(name.toStdString());
  job.set_title(title.toStdString());
  proto::Document document;
  document.CopyFrom( ui_->docoptions->doc() );
  document.set_cache(true);
  GeoPointPb gp;
  document.mutable_docsize()->set_height(ui_->doc_height_spin->value());
  document.mutable_docsize()->set_width(ui_->doc_width_spin->value());
  switch( ui_->formatBox->currentIndex() ){
    case 0 :
      job.set_format(proto::kBmp);
      break;
    case 1 :
      job.set_format(proto::kPng);
      break;
    case 2 :
      job.set_format(proto::kJpeg);
      break;
    case 3 :
      job.set_format(proto::kSxf);
      break;
    case 4 :
      job.set_format(proto::kPtkpp);
  }
  job.set_map_id(mapid_.toStdString());
  foreach( QCheckBox* check, checklist_ ){
    if( true == check->isChecked() ){
      job.add_hour(check->text().toInt());
    }
  }

  if( 0 == job.hour_size() ){
    job.add_hour(0);
  }

  if( -1 != ui_->modelBox->currentIndex() ){
    job.set_model(ui_->modelBox->currentText().toInt());
  }
  if( -1 != ui_->centerBox->currentIndex() ){
    if( centers_.values().contains(ui_->centerBox->currentText()) ){
      job.set_center(centers_.key(ui_->centerBox->currentText()).toInt());
    }
    else{
      job.set_center(ui_->centerBox->currentText().toInt());
    }
  }
  for ( int i = 0, sz = ui_->treedates->topLevelItemCount(); i < sz; ++i ) {
    QTreeWidgetItem* item = ui_->treedates->topLevelItem(i);
    job.add_additional_date( item->text(0).toStdString() );
  }

  job.mutable_document()->MergeFrom(document);
  job.mutable_document()->mutable_legend()->set_pos(doc_.legend().pos());

  job.mutable_document()->set_geoloader(ui_->docoptions->loader().toStdString());
  proto::StoreType storetype = static_cast<proto::StoreType>( storetypes()[ui_->cmbstoretype->currentIndex()] );
  job.set_storetype(storetype);
  if ( proto::kSaveClientFile == ( storetype & proto::kSaveClientFile ) ) {
    job.set_clientsavepath( ui_->file_path->text().toStdString() );
  }
  if ( proto::kSaveClientFile == ( storetype & proto::kSaveClientFile ) ) {
    ui_->file_path->setDisabled(false);
    ui_->btnfile->setDisabled(false);
  }
  else {
    ui_->file_path->setDisabled(true);
    ui_->btnfile->setDisabled(true);
  }
  job.set_timesheet(sheet_.toStdString());
  job.mutable_map()->CopyFrom(map());
  job.set_period(period_);
  job.set_await(await_);
  job.set_cron_shift(cron_);

  proto::IncutParams incut;
  if ( false == ui_->incutVisibilityButton->isChecked() ) {
    incut.set_visible(false);
  }
  else {
    incut.set_visible(true);
    incut.set_pos(static_cast<Position>(ui_->incutPosition->currentData().toInt()));
  }
  job.mutable_document()->mutable_incut()->CopyFrom(incut);

  proto::LegendParams legend;
  if ( false == ui_->legendVisibilityButton->isChecked() ) {
    legend.set_pos(Position::kNoPosition);
  }
  else {
    legend.set_pos(static_cast<Position>(ui_->legendPosition->currentData().toInt()));
    legend.set_orient(static_cast<Orientation>(ui_->legendOrientation->currentData().toInt()));
  }
  job.mutable_document()->mutable_legend()->CopyFrom(legend);

  if ( ProjectionType::kMercat == job.document().projection() && "ptkpp" == job.document().geoloader() ) {
    job.mutable_document()->mutable_map_center()->set_lat_radian(0);
    job.mutable_document()->mutable_map_center()->set_lon_radian(0);
  }
  job.mutable_map()->set_spline_koef(0);

  job.set_enabled(ui_->cbEnabled_->checkState() == Qt::Checked);
  return job;
}

proto::Map JobWidget::map()
{
  proto::Map map;
  QString title = ui_->mapName->text();
  if( true == title.isEmpty() ){
    title = "unknown";
  }
  map.set_title(title.toStdString());
  // map.set_spline_koef(ui_->splineBox->currentText().toInt());
  for( int i = 0; i < ui_->layerTree->topLevelItemCount(); i++ ){
    proto::WeatherLayer* l = map.add_data();
    proto::ViewMode mode = (proto::ViewMode)ui_->layerTree->topLevelItem(i)->text(ViewMode).toInt();
    l->set_mode(mode);
    l->set_template_name(ui_->layerTree->topLevelItem(i)->text(TemplateName).toStdString());
    switch( mode ){
      case proto::kPuanson : {
        l->set_level(ui_->layerTree->topLevelItem(i)->text(Level).toInt());
        l->set_type_level(ui_->layerTree->topLevelItem(i)->text(TypeLevel).toInt());
        if( proto::kPuanson == mode ){
          QStringList types = ui_->layerTree->topLevelItem(i)->text(DataType).split(";");
          for( const auto &one_type : types ){
            if( false == one_type.isEmpty() ){
              l->add_data_type( static_cast<meteo::surf::DataType>(one_type.toInt()) );
            }
          }
        }
        break;
      }
      case proto::kIsoline :
      case proto::kIsoGrad :
      case proto::kGradient : {
        l->set_level(ui_->layerTree->topLevelItem(i)->text(Level).toInt());
        l->set_type_level(ui_->layerTree->topLevelItem(i)->text(TypeLevel).toInt());
        l->set_transparency(ui_->layerTree->topLevelItem(i)->text(Transparency).toInt());
        break;
      }
      case proto::kRadar : {
        QStringList hh = ui_->layerTree->topLevelItem(i)->text(Layer).split("-");
        if( 2 == hh.count() ){
          l->set_h1(hh[0].toInt());
          l->set_h2(hh[1].toInt());
          l->set_type(kLayerMrl);
          l->set_mode(proto::kRadar);
          l->set_source(proto::kSurface);
          l->add_data_type(surf::kRadarMapType);
        }
        break;
      }
      case proto::kSigwx : {
        l->set_source(proto::kSurface);
        l->add_data_type(surf::kFlightSigWx);
        l->set_type(kLayerSigwx);
        QStringList hh = ui_->layerTree->topLevelItem(i)->text(Layer).split("-");
        l->set_h1(hh[0].toInt());
        l->set_h2(hh[1].toInt());
      }
      break;
      default:
        break;
    }
  }
  return map;
}

void JobWidget::clearForm()
{
  GeoPoint gp;
  gp.setLatDeg(60);
  gp.setLonDeg(30);
  ui_->docoptions->setCoord(gp);
  ui_->docoptions->setProjection(kMercat);
  ui_->docoptions->setScale(12);
  ui_->sizeTemplate->setCurrentIndex(1);
  ui_->formatBox->setCurrentIndex(1);
  ui_->doc_width_spin->setValue(800);
  ui_->doc_height_spin->setValue(600);
  ui_->modelBox->setCurrentIndex(0);
  ui_->centerBox->setCurrentIndex(0);
  ui_->edit_task_name->clear();
  ui_->treedates->clear();
  foreach( QCheckBox* check, checklist_ ){
    check->setChecked(false);
  }
  ui_->mapName->clear();
  ui_->layerTree->clear();
  //ui_->splineBox->setCurrentIndex(0);
  ui_->delLayerBtn->setEnabled(false);
  ui_->editLayerBtn->setEnabled(false);


  if ( nullptr != timeSheet_ ) {
    timeSheet_->clear();
  }

  ui_->timeSheetLine->clear();
  jobName_.clear();
  mapName_.clear();
}

void JobWidget::setJob(const proto::Job& job)
{
  clearForm();
  ui_->doc_width_spin->setValue(job.document().docsize().width());
  ui_->doc_height_spin->setValue(job.document().docsize().height());

  bool isEnabled = job.has_enabled() ? job.enabled() : false;
  ui_->cbEnabled_->setCheckState(isEnabled ? Qt::Checked : Qt::Unchecked);

  QString sizetext = QString("%1 x %2")
                     .arg(job.document().docsize().width())
                     .arg(job.document().docsize().height());
  int indx = ui_->sizeTemplate->findText(sizetext);
  if ( -1 == indx ) {
    ui_->sizeTemplate->addItem( sizetext );
  }
  indx = ui_->sizeTemplate->findText(sizetext);
  ui_->sizeTemplate->setCurrentIndex(indx);

  for ( int i = 0, sz = job.additional_date_size(); i < sz; ++i ) {
    QDate dt = QDate::fromString( QString::fromStdString( job.additional_date(i) ), "yyyy-MM-dd" );
    if ( false == dt.isValid() ) {
      continue;
    }
    QTreeWidgetItem* item = new QTreeWidgetItem( ui_->treedates );
    item->setText( 0, dt.toString("yyyy-MM-dd") );
  }
  doc_.CopyFrom(job.document());
  ui_->docoptions->setDoc( job.document() );

  if ( true == job.has_name()) {
    ui_->edit_task_name->setText( QString::fromStdString( job.title() ) );
  }

  switch( job.format() ){
    case proto::kBmp :
      ui_->formatBox->setCurrentIndex(0);
      break;
    case proto::kPng :
      ui_->formatBox->setCurrentIndex(1);
      break;
    case proto::kJpeg :
      ui_->formatBox->setCurrentIndex(2);
      break;
    case proto::kSxf :
      ui_->formatBox->setCurrentIndex(3);
      break;
    case proto::kPtkpp :
      ui_->formatBox->setCurrentIndex(4);
      break;
    default:
      break;
    case proto::kOdp:
      break;
  }

  QString hourstxt;
  foreach( QCheckBox* check, checklist_ ){
    if( true == checkHour(job, check->text().toInt()) ){
      check->setChecked(true);
      hourstxt.append(check->text()).append(";");
    }
    else{
      check->setChecked(false);
    }
  }
  ui_->hoursLabel->setText(hourstxt);

  if( false == job.has_center() ){
    ui_->centerBox->setCurrentIndex(0);
  }
  else{
    ui_->centerBox->setCurrentIndex(::basicCenters().indexOf(QString::number(job.center())));
  }
  if( false == job.has_model() ){
    ui_->modelBox->setCurrentIndex(0);
  }
  else{
    ui_->modelBox->setCurrentIndex(ui_->modelBox->findText(QString::number(job.model())));
  }
  proto::StoreType storetype = job.storetype();
  ui_->cmbstoretype->setCurrentIndex( storetypes().key(storetype) );
  if ( proto::kSaveClientFile == ( storetype & proto::kSaveClientFile ) ) {
    ui_->file_path->setDisabled(false);
    ui_->btnfile->setDisabled(false);
  }
  else {
    ui_->file_path->setDisabled(true);
    ui_->btnfile->setDisabled(true);
  }
  if ( true == job.has_clientsavepath() ) {
    ui_->file_path->setText( QString::fromStdString( job.clientsavepath() ) );
  }
  else {
    ui_->file_path->setText( QDir::homePath() );
  }
  if ( true == job.has_timesheet() ) {
    sheet_ = QString::fromStdString(job.timesheet());
  }
  setMap(job.map());
  period_ = job.period();
  await_ = job.await();
  cron_ = job.cron_shift();
  if ( true == job.has_name() ) {
    jobName_ = QString::fromStdString(job.name());
  }
  parseSheet();

  incut_.CopyFrom(job.document().incut());
  if ( true == incut_.has_visible() ) {
    ui_->incutVisibilityButton->setChecked(incut_.visible());
    auto pos = incut_.pos();
    ui_->incutPosition->setCurrentIndex(static_cast<int>(pos));
  }
  else {
    ui_->incutVisibilityButton->setChecked(false);
  }

  slotOnIncutVisible();

  legend_.CopyFrom(job.document().legend());
  if ( false == legend_.has_pos() || legend_.pos() == kNoPosition) {
    ui_->legendVisibilityButton->setChecked(false);
  }
  else {
    ui_->legendVisibilityButton->setChecked(true);
  }
  slotOnLegendVisible();
}

void JobWidget::setMode(automap::Mode mode)
{
  mode_ = mode;
  if( automap::Add == mode_ ){
    ui_->task_name->setVisible(false);
    ui_->edit_task_name->setVisible(true);
    ui_->lbl_task_name->setVisible(true);
  }
  else if(automap::Edit == mode_){
    ui_->task_name->setVisible(true);
    ui_->lbl_task_name->setVisible(false);
  }
}

void JobWidget::getMeteoCenters()
{
  rpc::Channel* ctrl = meteo::global::serviceChannel(meteo::settings::proto::kSprinf );
  if ( 0 == ctrl ) {
    return;
  }
  meteo::sprinf::MeteoCenterRequest req;
  for( const QString& center_id : ::basicCenters() ){
    req.add_center_id(center_id.toInt());
  }
  meteo::sprinf::MeteoCenters* res = ctrl->remoteCall(&meteo::sprinf::SprinfService::GetMeteoCenters, req, 5000);
  if (res != 0) {
    for (int j = 0, jsz = res->center_size(); j < jsz; ++j) {
      centers_.insert(QString::number(res->center(j).id()), QString::fromStdString(res->center(j).short_name()));
    }
  }
  delete res;
  delete ctrl;
}

void JobWidget::createMenu()
{
  foreach( QCheckBox* check, checklist_ ){
    delete check;
  }
  checklist_.clear();
  hourMenu_ = new QMenu( this );
  QAction* act = hourMenu_->addAction(QObject::tr("Срок ч."));
  QFont font = act->font();
  font.setBold(true);
  act->setFont(font);
  hourMenu_->addSeparator();
  foreach(const QString& name, ::basicHours()){
    QCheckBox* check = new QCheckBox(hourMenu_);
    checklist_.append(check);
    check->setText(name);
    QWidgetAction *chkBoxAction  = new QWidgetAction(hourMenu_);
    chkBoxAction->setDefaultWidget(check);
    hourMenu_->addAction(chkBoxAction);
    QObject::connect(check, SIGNAL(toggled(bool)), SLOT(slotCheckToggled(bool)));
  }
}

bool JobWidget::checkHour(const proto::Job& job, int hour)
{
  for( int i = 0; i < job.hour_size(); i++ ){
    if( hour == job.hour(i) ){
      return true;
    }
  }
  return false;
}

void JobWidget::setMap(const proto::Map &map)
{
//  clearForm();
  QMap< QString, meteo::puanson::proto::Puanson > punchlibrary = WeatherLoader::instance()->punchlibrary();
  QMap<QString, meteo::map::proto::FieldColor> isolibrary = WeatherLoader::instance()->isolibrary();
  QMap< QString, meteo::map::proto::RadarColor > radarlibrary = meteo::map::WeatherLoader::instance()->radarlibrary();
  ui_->mapName->setText(QString::fromStdString(map.title()));
  //ui_->splineBox->setCurrentIndex(map.spline_koef());
  QString layer_name, layer_title;
  for( int i = 0; i < map.data_size(); i++){
    QTreeWidgetItem* item = new QTreeWidgetItem( ui_->layerTree );
    item->setSizeHint(2, QSize(250, 20));
    layer_name = QString::fromStdString(map.data(i).template_name());
    item->setText(0, layer_name);
    item->setText(ViewMode, QString::number(map.data(i).mode()));

    switch( map.data(i).mode() ){
      case proto::kPuanson : {
        item->setText(Level, QString::number(map.data(i).level()));
        item->setText(TypeLevel, QString::number(map.data(i).type_level()));
        item->setText(LevelTitle, SelectWidget::levelHuman(map.data(i).level(), map.data(i).type_level()));
        item->setText(TypeLevelTitle, SelectWidget::typeLevelHuman(map.data(i).type_level()));
        QString typestext;
        for( int k = 0; k < map.data(i).data_type_size(); k++ ){
          meteo::surf::DataType t = map.data(i).data_type(k);
          typestext.append(QString::number(t)).append(";");
        }
        item->setText(DataType, typestext);
        if( punchlibrary.contains(layer_name) ){
          layer_title = QString::fromStdString(punchlibrary[layer_name].name());
        }
        else{
          layer_title = layer_name;
        }
        break;
      }
      case proto::kSigwx:
        {
          item->setText(TemplateName, QObject::tr("Опасные явления"));
          item->setText(Layer, QString("%1-%2").arg(map.data(i).h1()).arg(map.data(i).h2()));
          break;
        }
      case proto::kIsoline :
      case proto::kGradient :
      case proto::kIsoGrad :
        item->setText(Level, QString::number(map.data(i).level()));
        item->setText(TypeLevel, QString::number(map.data(i).type_level()));
        item->setText(LevelTitle, SelectWidget::levelHuman(map.data(i).level(), map.data(i).type_level()));
        item->setText(TypeLevelTitle, SelectWidget::typeLevelHuman(map.data(i).type_level()));
        item->setText(Transparency, QString::number(map.data(i).transparency()));
        if( isolibrary.contains(layer_name) ){
          layer_title = QString::fromStdString(isolibrary[layer_name].name());
        }
        else{
          layer_title = layer_name;
        }
        break;
      case proto::kRadar :
        item->setText(Layer, QString("%1-%2").arg(map.data(i).h1()).arg(map.data(i).h2()));
        if( isolibrary.contains(layer_name) ){
          layer_title = QString::fromStdString(radarlibrary[layer_name].name());
        }
        else{
          layer_title = layer_name;
        }
        break;
      default :
        break;
    }
    item->setText(TemplateTitle, layer_title );
  }
  if ( true == map.has_name() ) {
    mapName_ = QString::fromStdString(map.name());
  }
}

void JobWidget::parseSheet()
{
  debug_log << sheet_;
  QString readable;
  QStringList fields = sheet_.split(QRegExp("[\t ]+"), QString::SkipEmptyParts);
  if ( fields.size() < 5 ) {
    return;
  }

  QString minutes = fields.at(0);
  QString hours = fields.at(1);
  QString weekDays = fields.at(4);

  auto period = period_;
  if ( period < 60 ) {
    readable.append(QObject::tr("Выполняется каждые %1 минут").arg(period));
  }
  else {
    readable.append(QObject::tr("Выполняется раз в %1 час(а/ов)").arg(period/60));
  }

  if ( "*" != weekDays ) {
    QStringList wdNames = QStringList() << QObject::tr("пн") << QObject::tr("вт") << QObject::tr("ср") << QObject::tr("чт")
                                       << QObject::tr("пт") << QObject::tr("сб") << QObject::tr("вс");
    readable.append("; ");
    QString separator;
    if ( true == weekDays.contains("-") ) {
      separator = "-";
    }
    else if ( true == weekDays.contains(",") ) {
      separator = ",";
    }
    QStringList wd = weekDays.split(separator);
    for ( const auto &s : wd ) {
      readable.append(wdNames[s.toInt() - 1]);
      readable.append(separator);
    }
    readable.remove(readable.size() - 1, 1);
  }
  auto await = cron_;
  if ( 0 != await ) {
    auto hour = await/60;
    auto minute = await%60;
    QString str = QObject::tr("; спустя %1%2");
    if ( 0 != hour ) {
      str = str.arg(QString("%1 %2").arg(hour).arg(QObject::tr("часов")));
    }
    else {
      str = str.arg("");
    }
    if ( 0 != minute ) {
      str = str.arg(QString(" %1 %2").arg(minute).arg(QObject::tr("минут")));
    }
    else {
      str = str.arg("");
    }
    str.append(QObject::tr(" после наступления срока"));
    readable.append(str);
  }
  ui_->timeSheetLine->setText(readable);
}

void JobWidget::incutInit()
{
  ui_->incutPosition->addItem(QIcon(":/meteo/icons/text/text-pos-top-center.png"), QObject::tr("В центре"), kCenter);
  ui_->incutPosition->addItem(QIcon(":/meteo/icons/text/text-pos-top-center.png"), QObject::tr("Наверху"), kTopCenter);
  ui_->incutPosition->addItem(QIcon(":/meteo/icons/text/text-pos-mid-left.png"), QObject::tr("Слева"), kLeftCenter);
  ui_->incutPosition->addItem(QIcon(":/meteo/icons/text/text-pos-top-left.png"), QObject::tr("Слева вверху"), kTopLeft);
  ui_->incutPosition->addItem(QIcon(":/meteo/icons/text/text-pos-bottom-center.png"), QObject::tr("Внизу"), kBottomCenter);
  ui_->incutPosition->addItem(QIcon(":/meteo/icons/text/text-pos-bottom-left.png"), QObject::tr("Слева внизу"), kBottomLeft);
  ui_->incutPosition->addItem(QIcon(":/meteo/icons/text/text-pos-mid-right.png"), QObject::tr("Справа"), kRightCenter);
  ui_->incutPosition->addItem(QIcon(":/meteo/icons/text/text-pos-top-right.png"), QObject::tr("Справа вверху"), kTopRight);
  ui_->incutPosition->addItem(QIcon(":/meteo/icons/text/text-pos-bottom-right.png"), QObject::tr("Справа внизу"), kBottomRight);
}

void JobWidget::legendInit()
{
  ui_->legendPosition->addItem(QIcon(":/meteo/icons/text/text-pos-top-left.png"), QObject::tr("Слева вверху"), kTopLeft);
  ui_->legendPosition->addItem(QIcon(":/meteo/icons/text/text-pos-top-right.png"), QObject::tr("Справа вверху"), kTopRight);
  ui_->legendPosition->addItem(QIcon(":/meteo/icons/text/text-pos-bottom-left.png"), QObject::tr("Слева внизу"), kBottomLeft);
  ui_->legendPosition->addItem(QIcon(":/meteo/icons/text/text-pos-bottom-right.png"), QObject::tr("Справа внизу"), kBottomRight);
  ui_->legendPosition->addItem(QIcon(":/meteo/icons/text/text-pos-top-center.png"), QObject::tr("Наверху"), kTopCenter);
  ui_->legendPosition->addItem(QIcon(":/meteo/icons/text/text-pos-bottom-center.png"), QObject::tr("Внизу"), kBottomCenter);
  ui_->legendPosition->addItem(QIcon(":/meteo/icons/text/text-pos-mid-left.png"), QObject::tr("Слева"), kLeftCenter);
  ui_->legendPosition->addItem(QIcon(":/meteo/icons/text/text-pos-mid-right.png"), QObject::tr("Справа"), kRightCenter);
  ui_->legendPosition->addItem(QIcon(":/meteo/icons/text/text-pos-mid-right.png"), QObject::tr("Плавающая"), kFloat);

  ui_->legendOrientation->addItem(QObject::tr("Горизонтальная"), kHorizontal);
  ui_->legendOrientation->addItem(QObject::tr("Вертикальная"), kVertical);
}

void JobWidget::accept()
{
  QDialog::accept();
}

void JobWidget::slotChangeSize(const QString& str)
{
  int w = str.split("x").first().toInt();
  int h = str.split("x").last().toInt();

  ui_->doc_width_spin->setValue(w);
  ui_->doc_height_spin->setValue(h);
}

void JobWidget::slotAddDate()
{
  QString date = ui_->addDateEdit->date().toString("yyyy-MM-dd");

  for ( int i = 0,  sz = ui_->treedates->topLevelItemCount(); i < sz; ++i ) {
    QTreeWidgetItem* item = ui_->treedates->topLevelItem(i);
    if ( item->text(0) == date ) {
      return;
    }
  }
  QTreeWidgetItem* item = new QTreeWidgetItem( ui_->treedates );
  item->setText( 0, date );
}

void JobWidget::slotRemoveDate()
{
  QTreeWidgetItem* item = ui_->treedates->currentItem();
  if ( 0 == item ) {
    return;
  }
  delete item;
}

void JobWidget::slotCenterChange(const QString& text)
{
  ui_->modelBox->clear();
  int center_id;
  if( centers_.values().contains(text) ){
    center_id = centers_.key(text).toInt();
  }
  else{
    center_id = text.toInt();
  }
  ui_->modelBox->addItems(centerModels(center_id));
}

void JobWidget::slotHourButtonClicked()
{
  QToolButton* btn = qobject_cast<QToolButton*>(sender());
  if( 0 == btn ){
    return;
  }
  if( 0 != hourMenu_ ){
    QRect widgetRect = btn->geometry();
    widgetRect.moveTopLeft(btn->parentWidget()->mapToGlobal(widgetRect.topLeft()));
    hourMenu_->exec( widgetRect.bottomLeft() );
  }
}


void JobWidget::slotEditNameBtnClicked()
{
    ui_->edit_task_name->setVisible(!ui_->edit_task_name->isVisible());

}

void JobWidget::slotCheckToggled(bool /*on*/)
{
  QString hourstxt;
  foreach( QCheckBox* check, checklist_ ){
    if( true == check->isChecked() ){
      hourstxt.append(check->text()).append(";");
    }
  }
  ui_->hoursLabel->setText(hourstxt);
}

void JobWidget::chooseFile()
{
  QString caption = QString::fromUtf8("Выбор каталога для сохранения");
  QString dirName = QFileDialog::getExistingDirectory(this,caption, ui_->file_path->text());
  if (!dirName.isEmpty()) {
    ui_->file_path->setText(dirName);
  }
}

void JobWidget::slotOkButton()
{
  QDialog::accept();
}

void JobWidget::slotNoButton()
{
  QDialog::reject();
}

void JobWidget::slotChangeStoreType()
{
  proto::StoreType storetype = static_cast<proto::StoreType>( storetypes()[ui_->cmbstoretype->currentIndex()] );
  if ( proto::kSaveClientFile == ( storetype & proto::kSaveClientFile ) ) {
    ui_->file_path->setDisabled(false);
    ui_->fileNameEdit->setDisabled(false);
    ui_->btnfile->setDisabled(false);
  }
  else {
    ui_->file_path->setDisabled(true);
    ui_->fileNameEdit->setDisabled(true);
    ui_->btnfile->setDisabled(true);
  }
}

void JobWidget::slotNameChanged()
{
  QString title = ui_->edit_task_name->text();
  QString mapTitle = ui_->mapName->text();

  ui_->task_name->setText(QString("%1 %2")
    .arg(mapTitle)
    .arg(title)
  );

  ui_->okbtn->setEnabled(!title.isEmpty() && !mapTitle.isEmpty());
}

void JobWidget::slotAddLayer()
{
  if( 0 == addLayer_ ){
    addLayer_ = new SelectWidget(this);
  }
  addLayer_->clearForm();
  if( QDialog::Accepted == addLayer_->exec() ){
    QTreeWidgetItem* item = new QTreeWidgetItem( ui_->layerTree );
    item->setText(TemplateName, addLayer_->templateName());
    item->setText(TemplateTitle, addLayer_->templateNameHuman());
    proto::ViewMode mode = addLayer_->type();
    item->setText(ViewMode, QString::number(mode));
    switch ( mode ){
      case proto::kSigwx:
      {
        item->setText(TemplateName, QObject::tr("Опасные явления"));
        item->setText(Layer, addLayer_->hh());
        break;
      }
      case proto::kPuanson :
      {
        item->setText(Level, QString::number(addLayer_->level()));
        item->setText(TypeLevel, QString::number(addLayer_->type_level()));
        item->setText(LevelTitle, addLayer_->levelHuman());
        item->setText(TypeLevelTitle, addLayer_->typeLevelHuman());
        item->setText(DataType, addLayer_->data_type());
        break;
      }
      case proto::kGradient :
      case proto::kIsoline :
      case proto::kIsoGrad :
      {
        item->setText(Level, QString::number(addLayer_->level()));
        item->setText(TypeLevel, QString::number(addLayer_->type_level()));
        item->setText(LevelTitle, addLayer_->levelHuman());
        item->setText(TypeLevelTitle, addLayer_->typeLevelHuman());
        item->setText(Transparency, QString::number(addLayer_->transparency()));
        break;
      }
      case proto::kRadar:
      {
        item->setText(Layer, addLayer_->hh());
        break;
      }
      default :
        break;
    }
    item->setSizeHint(TemplateTitle, QSize(250, 20));
    for( int i = 0; i < ui_->layerTree->topLevelItemCount(); i++ ){
      ui_->layerTree->resizeColumnToContents(i);
    }
  }
}

void JobWidget::slotEditLayer()
{
  QTreeWidgetItem* item = ui_->layerTree->currentItem();
  if( 0 == item ){
    return;
  }
  meteo::map::proto::Map m = map();
  proto::WeatherLayer layer;
  layer = m.data(ui_->layerTree->indexOfTopLevelItem(item));
  if( 0 == addLayer_ ){
    addLayer_ = new SelectWidget(this);
  }
  addLayer_->fillForm(layer);
  if( QDialog::Accepted == addLayer_->exec() ){
    delete item;
    item = new QTreeWidgetItem( ui_->layerTree );
    item->setText(TemplateName, addLayer_->templateName());
    item->setText(TemplateTitle, addLayer_->templateNameHuman());
    proto::ViewMode mode = addLayer_->type();
    item->setText(ViewMode, QString::number(mode));
    switch ( mode ){
      case proto::kSigwx :
      {
        item->setText(TemplateName, QObject::tr("Опасные явления"));
        item->setText(Layer, addLayer_->hh());
        break;
      }
      case proto::kPuanson :
      {
        item->setText(Level, QString::number(addLayer_->level()));
        item->setText(TypeLevel, QString::number(addLayer_->type_level()));
        item->setText(LevelTitle, addLayer_->levelHuman());
        item->setText(TypeLevelTitle, addLayer_->typeLevelHuman());
        item->setText(DataType, addLayer_->data_type());
        break;
      }
      case proto::kGradient :
      case proto::kIsoline :
      case proto::kIsoGrad :
      {
        item->setText(Level, QString::number(addLayer_->level()));
        item->setText(TypeLevel, QString::number(addLayer_->type_level()));
        item->setText(LevelTitle, addLayer_->levelHuman());
        item->setText(TypeLevelTitle, addLayer_->typeLevelHuman());
        item->setText(Transparency, QString::number(addLayer_->transparency()));
        break;
      }
      case proto::kRadar:
      {
        item->setText(Layer, addLayer_->hh());
        break;
      }
      default :
        break;
    }
    item->setSizeHint(TemplateTitle, QSize(250, 20));
    for( int i = 0; i < ui_->layerTree->topLevelItemCount(); i++ ){
      ui_->layerTree->resizeColumnToContents(i);
    }
  }
}

void JobWidget::slotRemoveLayer()
{
  QTreeWidgetItem* item = ui_->layerTree->currentItem();
  if( 0 == item ){
    return;
  }else{
    delete item;
  }

  if( 0 == ui_->layerTree->topLevelItemCount() ){
    ui_->delLayerBtn->setEnabled(false);
    ui_->editLayerBtn->setEnabled(false);
  }
}

void JobWidget::slotItemChanged()
{
  if( 0 != ui_->layerTree->currentItem() ){
    ui_->delLayerBtn->setEnabled(true);
    ui_->editLayerBtn->setEnabled(true);
  }
}

void JobWidget::slotEditTimeSheet()
{
  if ( nullptr == timeSheet_ ) {
    timeSheet_ = new TimeSheet(this);
  }
  timeSheet_->setSheet(sheet_);
  timeSheet_->setPeriod(period_);
  timeSheet_->setAwait(await_);
  timeSheet_->setCron(-1);
  timeSheet_->setCron(cron_);
  if ( QDialog::Accepted == timeSheet_->exec() ) {
    sheet_ = timeSheet_->sheet();
    period_ = timeSheet_->period();
    await_ = timeSheet_->await();
    cron_ = timeSheet_->cron();
    parseSheet();
  }
}

void JobWidget::slotOnIncutVisible()
{
  if ( true == ui_->incutVisibilityButton->isChecked() ) {
    ui_->incutVisibilityButton->setIcon(QIcon(":/meteo/icons/tools/layer_visible.png"));
    ui_->incutPosition->setDisabled(false);
  }
  else {
    ui_->incutVisibilityButton->setIcon(QIcon(":/meteo/icons/tools/set_layer_hidden.png"));
    ui_->incutPosition->setDisabled(true);
  }
}

void JobWidget::slotOnLegendVisible()
{
  if ( true == ui_->legendVisibilityButton->isChecked() ) {
    ui_->legendVisibilityButton->setIcon(QIcon(":/meteo/icons/tools/layer_visible.png"));
    ui_->legendOrientation->setDisabled(false);
    ui_->legendPosition->setDisabled(false);
  }
  else {
    ui_->legendVisibilityButton->setIcon(QIcon(":/meteo/icons/tools/set_layer_hidden.png"));
    ui_->legendOrientation->setDisabled(true);
    ui_->legendPosition->setDisabled(true);
  }
}

}
}
