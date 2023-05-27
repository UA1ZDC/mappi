#include <QMessageBox>
#include <QTemporaryFile>

#include "documentparams.h"
#include "ui_documentparams.h"

#include <meteo/commons/global/dateformat.h>
#include <meteo/commons/proto/surface_service.pb.h>
#include "meteo/commons/services/sprinf/sprinfservice.h"
#include <meteo/commons/global/global.h>
#include <meteo/commons/ui/custom/userselectform.h>

namespace meteo {

DocumentParams::DocumentParams(QWidget *parent) :
  QWidget(parent),
  ui_(new Ui::DocumentParams)
{
  ui_->setupUi(this);
  QDateTime dt = QDateTime::currentDateTimeUtc();
  QTime t = dt.time();
  t.setHMS(t.hour(),0,0);
  dt.setTime(t);
  ui_->dateTimeEdit->setDateTime(dt);
  ui_->stormToDatetime->setDateTime(dt);
  ui_->stormTextButton->setIcon(QIcon(":/meteo/icons/manual.png"));
  ui_->clearStormTextBtn->setIcon(QIcon(":/meteo/icons/clear.png"));
  ui_->selectUsersBtn->setIcon(QIcon(":/meteo/icons/users.png"));

  ui_->dateTimeEdit->setDisplayFormat(meteo::dtHumanFormatTimeShort);
  ui_->stormToDatetime->setDisplayFormat(meteo::dtHumanFormatTimeShort);

  forecastForm_ = new ForecastFormWidget();
  forecastForm_->setWindowTitle(QObject::tr("Состав прогноза"));
  QObject::connect( forecastForm_, SIGNAL(submitFormText(QString)), this, SLOT(slotOnStormText(QString)));

  QValidator* intValidator = new QIntValidator(0,99,this);
  ui_->arbitraryNumberEdit->setValidator(intValidator);
  QObject::connect(ui_->createButton, SIGNAL(clicked(bool)), this, SLOT(slotOnCreateButtonClicked()));
  QObject::connect(ui_->cancelButton, SIGNAL(clicked(bool)), this, SLOT(close()));
  QObject::connect(ui_->arbitraryNumberEdit, SIGNAL(textEdited(QString)), this, SLOT(slotArbitraryEdited()));
  QObject::connect( ui_->stormTextButton, SIGNAL(clicked(bool)), SLOT(slotShowForecastForm()));
  QObject::connect( ui_->clearStormTextBtn, SIGNAL(clicked(bool)), SLOT(slotClearStormText()));

  ui_->av6TemplateComboBox->setEnabled(false);
  QObject::connect(ui_->station, SIGNAL(changed()), this, SLOT(slotOnComplete()));

  formals_ = meteo::map::WeatherLoader::instance()->formals();
  ui_->hydroTree->hide();
  QObject::connect(ui_->stationMobCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(slotHydroChanged(int)));
  QObject::connect(ui_->dateTimeEdit, SIGNAL(dateTimeChanged(const QDateTime&)), this, SLOT(slotDateTimeChanged(const QDateTime&)));

  QObject::connect( ui_->selectStationGroupBox, SIGNAL( clicked() ), this, SLOT( slotShowSelectUsers() ) );
  QObject::connect( ui_->selectUsersBtn, SIGNAL(clicked()), this, SLOT(slotShowSelectUsers() ) );

  QList<meteo::sprinf::MeteostationType> stationsToLoad;
  stationsToLoad << meteo::sprinf::MeteostationType::kStationSynop
                 << meteo::sprinf::MeteostationType::kStationSynmob
                 << meteo::sprinf::MeteostationType::kStationAirport
                 << meteo::sprinf::MeteostationType::kStationAero
                 << meteo::sprinf::MeteostationType::kStationHydro
                 << meteo::sprinf::MeteostationType::kStationOcean
                 << meteo::sprinf::MeteostationType::kStationAerodrome;
  ui_->station->loadStation(stationsToLoad);

  setVisibleUserBtn(isInterAvailable());

  this->adjustSize();
}

DocumentParams::~DocumentParams()
{
  delete forecastForm_;
  delete ui_;
  delete usersWidget_;
}
void DocumentParams::slotOnCreateButtonClicked()
{
  meteo::map::proto::StartArguments sa;
  auto s = sa.add_stations();
  if ( false == synMobSea_ ) {
    auto station = ui_->station->toStation();
    if ( true == station.has_index() ) {
      s->set_index(QString::number(station.index()).toStdString());
    }
    else if ( true == station.has_cccc() ) {
      s->set_index(station.cccc());
    }
    else {
      return;
    }
    if ( true == station.has_type() ) {
      s->set_type(station.type());
    }
    if ( true == station.has_name() && true == station.name().has_rus() ) {
      s->set_name(station.name().rus());
    }
    else if ( true == station.has_name() && true == station.name().has_international() ) {
      s->set_name(station.name().international());
    }
    else if ( true == station.has_name() && true == station.name().has_short_() ) {
      s->set_name(station.name().short_());
    }
    if ( true == station.has_position() ) {
      s->set_fi(station.position().lat_radian());
      s->set_la(station.position().lon_radian());
      if ( true == station.position().has_height_meters() ) {
        s->set_height(station.position().height_meters());
      }
    }
    sa.set_datetime(ui_->dateTimeEdit->dateTime().toString(Qt::ISODate).toStdString());
  }
  else {
    QByteArray arr = ui_->hydroTree->currentItem()->data(0, Qt::UserRole).toByteArray();
    meteo::surf::OceanAvailablePoint p;
    p.ParseFromString(std::string( arr.data(), arr.size()));
    s->set_index(p.name());
    s->set_name(p.name());
    s->set_fi(p.lat());
    s->set_la(p.lon());
    s->set_type(1);
    sa.set_datetime(p.dt());
  }
  if (kForecasts == type_) {
    if (ui_->hour2->isChecked()) {
      sa.set_forecast(2);
      docName_ = QObject::tr("Прогноз погоды на 2 часа");
    }
    if (ui_->hour24->isChecked()){
      sa.set_forecast(24);
      docName_ = QObject::tr("Прогноз погоды на 24 часа");
    }
  }
  auto map = formals_[docName_];

  if (kBulletin == type_) {
    sa.set_number(ui_->arbitraryNumberEdit->text().toInt());
  }
  if ( kStorm == type_ ) {
    sa.set_datetime_end(ui_->stormToDatetime->dateTime().toString(Qt::ISODate).toStdString());
    sa.set_text(ui_->stormText->toPlainText().toStdString());
  }
  if ( kStormForecast == type_ ) {
    sa.set_datetime_end(ui_->stormToDatetime->dateTime().toString(Qt::ISODate).toStdString());
    sa.set_text(ui_->stormText->toPlainText().toStdString());
    sa.set_forecast(1);
  }
  if ( kAv6 == type_ ) {
    sa.set_av6_full( 1 == ui_->av6TemplateComboBox->currentIndex() );
    sa.set_av6_all_hours( 1 == ui_->av6HourComboBox->currentIndex() );
  }
  if ( kGidroinfo == type_ ) {
    sa.set_datetime_end(ui_->stormToDatetime->dateTime().toString(Qt::ISODate).toStdString());
    sa.set_text(ui_->freeText->text().toStdString());
  }

  sa.set_department(meteo::global::currentUserDepartment().toStdString());
  sa.set_user_name(meteo::global::currentUserShortName().toStdString());
  sa.set_user_rank(meteo::global::currentUserRank().toStdString());
  map.set_datetime(ui_->dateTimeEdit->dateTime().toString(Qt::ISODate).toStdString());
  meteo::sprinf::Station *st = map.add_station();
  st->set_station(sa.stations(0).index());
  st->set_index(QString::fromStdString(sa.stations(0).index()).toInt());
  st->mutable_name()->set_international(sa.stations(0).eng_name());
  st->mutable_name()->set_rus(sa.stations(0).name());
  st->mutable_name()->set_short_(sa.stations(0).name());

  map.mutable_formal()->set_proto_args(sa.SerializeAsString());
  map.set_format( map.formal().format() );
  QStringList logins;
  if ( ui_->sendUsersCheck->isChecked() ) {
    if ( nullptr == usersWidget_ ) {
       QMessageBox::critical(0, QObject::tr("Внимание"), QObject::tr("Ошибка. Пользователи не выбраны") );
       return;
    }
    else {
      logins = usersWidget_->getCurrentUsers();
      if ( logins.size() == 0 ) {
        QMessageBox::critical(0, QObject::tr("Внимание"), QObject::tr("Ошибка. Пользователи не выбраны") );
        return;
      }
    }
  }
  formaldoc_.slotCreateFormalDoc(map, logins);

}

void DocumentParams::show()
{
  ui_->freeInfoBox->hide();
  ui_->stationMobCombo->setCurrentIndex(0);
  QList<meteo::sprinf::MeteostationType> stationsToLoad;
  stationsToLoad << meteo::sprinf::MeteostationType::kStationSynop
                 << meteo::sprinf::MeteostationType::kStationSynmob
                 << meteo::sprinf::MeteostationType::kStationAirport
                 << meteo::sprinf::MeteostationType::kStationAero
                 << meteo::sprinf::MeteostationType::kStationHydro
                 << meteo::sprinf::MeteostationType::kStationOcean
                 << meteo::sprinf::MeteostationType::kStationAerodrome;
  ui_->station->loadStation(stationsToLoad);
  switch (type_) {
  case kSimple:
    ui_->bulletinGroupBox->hide();
    ui_->forecastGroupBox->hide();
    ui_->av6GroupBox->hide();
    ui_->stormBox->hide();
    ui_->createButton->setEnabled(true);
    ui_->label_2->setText(QObject::tr("Срок данных: "));
    ui_->hydroMobStation->hide();
    ui_->station->show();
    break;
  case kForecasts:
    ui_->bulletinGroupBox->hide();
    ui_->stormBox->hide();
    ui_->av6GroupBox->hide();
    ui_->forecastGroupBox->show();
    ui_->createButton->setEnabled(true);
    ui_->label_2->setText(QObject::tr("Срок данных: "));
    ui_->hydroMobStation->hide();
    ui_->station->show();
    break;
  case kBulletin:
    ui_->forecastGroupBox->hide();
    ui_->stormBox->hide();
    ui_->av6GroupBox->hide();
    ui_->bulletinGroupBox->show();
    if (ui_->arbitraryNumberEdit->text().isEmpty()) {
      ui_->createButton->setEnabled(false);
    }
    ui_->label_2->setText(QObject::tr("Срок данных: "));
    ui_->hydroMobStation->hide();
    ui_->station->show();
    break;
  case kStorm:
  case kStormForecast:
    ui_->bulletinGroupBox->hide();
    ui_->forecastGroupBox->hide();
    ui_->av6GroupBox->hide();
    ui_->stormBox->show();
    ui_->clearStormTextBtn->show();
    ui_->stormText->show();
    ui_->stormTextButton->show();
    ui_->hydroMobStation->show();
    ui_->label_2->setText(QObject::tr("На период с "));
    ui_->hydroMobStation->hide();
    ui_->station->show();
    break;
  case kAv6:
    ui_->bulletinGroupBox->hide();
    ui_->forecastGroupBox->hide();
    ui_->av6GroupBox->show();
    ui_->stormBox->hide();
    ui_->createButton->setEnabled(true);
    ui_->label_2->setText(QObject::tr("Срок данных: "));
    ui_->hydroMobStation->hide();
    ui_->station->show();
    break;
  case kFactUnderlying:
    ui_->bulletinGroupBox->hide();
    ui_->forecastGroupBox->hide();
    ui_->av6GroupBox->hide();
    ui_->stormBox->hide();
    ui_->createButton->setEnabled(true);
    ui_->label_2->setText(QObject::tr("Срок данных: "));
    ui_->hydroMobStation->show();
    ui_->station->show();
    break;
   case kOnlyDate:
    ui_->bulletinGroupBox->hide();
    ui_->forecastGroupBox->hide();
    ui_->av6GroupBox->hide();
    ui_->stormBox->hide();
    ui_->createButton->setEnabled(true);
    ui_->label_2->setText(QObject::tr("Срок данных: "));
    ui_->hydroMobStation->hide();
    ui_->station->hide();
    break;
  case kGidroinfo:
    ui_->bulletinGroupBox->hide();
    ui_->forecastGroupBox->hide();
    ui_->av6GroupBox->hide();
    ui_->stormBox->hide();
    ui_->createButton->setEnabled(true);
    ui_->label_2->setText(QObject::tr("На период с "));
    ui_->stormBox->show();
    ui_->clearStormTextBtn->hide();
    ui_->stormText->hide();
    ui_->stormTextButton->hide();
    ui_->hydroMobStation->hide();
    ui_->station->show();
    ui_->freeInfoBox->show();
    ui_->freeLabel->setText(QObject::tr("Водный объект:"));
    stationsToLoad.clear();
    stationsToLoad << meteo::sprinf::MeteostationType::kStationHydro;
    ui_->station->loadStation(stationsToLoad, true);
   break;
  default:
    break;
  }
  switch (script_) {
  case kMeteosummary:
    docName_ = QObject::tr("Метеосводка");
    break;
  case kForecast:
    docName_ = QObject::tr("Прогноз погоды");
    break;
  case kMeteo11:
    docName_ = QObject::tr("Бюллетень Метео-11");
    break;
  case kMeteo44:
    docName_ = QObject::tr("Бюллетень Метео-44");
    break;
  case kWindDoc:
    docName_ = QObject::tr("Прогноз параметров ветра");
    break;
  case kAV6:
    docName_ = QObject::tr("Дневник погоды");
    break;
  case kLayer:
    docName_ = QObject::tr("Бюллетень Слой");
    break;
  case kStormAlert:
    docName_ = QObject::tr("Штормовое предупреждение");
    break;
  case kForecastWindRegion:
    docName_ = QObject::tr("Прогноз ветра по району");
    break;
  case kFactConditionUnderlying:
    docName_ = QObject::tr("ОГМУ по району");
    break;
  case kFloodSituation:
    docName_ = QObject::tr("Паводковая обстановка");
    break;
  case kHSPrirodaVoiska:
    docName_ = QObject::tr("Донесение о возникновении ЧС природного характера в местах дислокации");
    break;
  case kFactyavleniavoiska:
    docName_ = QObject::tr("Сведения о наблюдавшихся стихийных гидрометеорологических явлениях");
    break;
  case kStormvoiska:
    docName_ = QObject::tr("Сведения о количестве разработанных ШП");
    break;
  case kSharopilot:
    docName_ = QObject::tr("Сведения о потребности в шаропилотных оболочках");
    break;
  case kGidrometeotech:
    docName_ = QObject::tr("Сведения по обеспеченности ГМТ");
    break;
  case kGidrospravka:
    docName_ = QObject::tr("Гидрологическая сводка");
    break;
  case kFireHazardReport:
    docName_ = QObject::tr("Доклад по пожароопасности");
    break;
  case kHydroMeteoReport:
    docName_ = QObject::tr("Доклад гидрометеорологических условий");
    break;
  case kFactCondition:
    docName_ = QObject::tr("Фактические гидрометеорологические условия");
    break;
  case kForecastCondition:
    docName_ = QObject::tr("Прогноз гидрометеорологических условий");
    break;
  case kDokladOperBoi:
    docName_ = QObject::tr("Доклад об обеспечении мероприятий оперативной и боевой подготовки");
    break;
  case kOprppsp:
    docName_ = QObject::tr("Данные по оправдываемости прогнозов погоды и штормовых предупреждений");
    break;
  case kGidrometeomerop:
    docName_ = QObject::tr("Отчёт об участии в гидрометеорологическом обеспечении мероприятий оперативной подготовки");
    break;
  case kZaprosgmiwar:
    docName_ = QObject::tr("Заявка на получение ГМИ от Росгидромета в военное время");
    break;
  case kZaprosgmimir:
    docName_ = QObject::tr("Заявка на получение ГМИ от Росгидромета в мирное время");
    break;
  case kShortConditionForecast:
    docName_ = QObject::tr("Краткосрочный прогноз гидрометеорологических условий по району до 36 часов");
    break;
  }
  setWindowTitle(docName_);
  QWidget::show();
}

void DocumentParams::slotArbitraryEdited()
{
  if (ui_->arbitraryNumberEdit->text().isEmpty()) {
    ui_->createButton->setEnabled(false);
  }
  else {
    ui_->createButton->setEnabled(true);
  }
}

void DocumentParams::slotShowForecastForm()
{
  forecastForm_->show();
}

void DocumentParams::slotClearStormText()
{
  ui_->stormText->clear();
  delete forecastForm_;
  forecastForm_ = new ForecastFormWidget();
  forecastForm_->setWindowTitle(QObject::tr("Состав прогноза"));
  connect( forecastForm_, SIGNAL(submitFormText(QString)), this, SLOT(slotOnStormText(QString)));
}

void DocumentParams::slotOnStormText(QString text)
{
  forecastForm_->close();
  ui_->stormText->setText(text);
}

void DocumentParams::slotOnComplete()
{
  if ( 2 == ui_->station->stationType() ) {
    ui_->av6TemplateComboBox->setEnabled(true);
  }
  else {
    ui_->av6TemplateComboBox->setCurrentIndex(0);
    ui_->av6TemplateComboBox->setEnabled(false);
  }
}

void DocumentParams::slotHydroChanged(int index)
{
  if ( 0 == index ) {
    ui_->station->show();
    ui_->hydroTree->hide();
    synMobSea_ = false;
  }
  else if ( 1 == index )
  {
    ui_->station->hide();
    ui_->hydroTree->show();
    loadAvailableOceanStations();
    synMobSea_ = true;
  }
  this->adjustSize();
}

void DocumentParams::slotDateTimeChanged(const QDateTime& datetime)
{
  Q_UNUSED(datetime);
  if ( true == synMobSea_ ) {
    loadAvailableOceanStations();
  }
}

void DocumentParams::slotShowSelectUsers()
{
  if ( nullptr == usersWidget_) {
    usersWidget_ = new meteo::UserSelectForm();
  }
  usersWidget_->show();
}

void DocumentParams::loadAvailableOceanStations()
{
  ui_->hydroTree->clear();
  meteo::surf::DataRequest request;
  request.set_date_start(ui_->dateTimeEdit->dateTime().toString(Qt::ISODate).toStdString());
  request.add_meteo_descrname("HHHH");
  request.add_meteo_descrname("S");
  request.add_meteo_descrname("Hw");
  request.add_meteo_descrname("Tw");
  request.add_meteo_descrname("zi");
  request.add_meteo_descrname("ci");

  meteo::rpc::Channel * ctrl_src = meteo::global::serviceChannel(meteo::settings::proto::kSrcData);
  if(nullptr == ctrl_src ) {
    error_log.msgBox() << meteo::msglog::kNoConnect.arg(meteo::global::serviceTitle(meteo::settings::proto::kSrcData));
    return;
  }

  meteo::surf::OceanAvailableReply* reply = ctrl_src->remoteCall(&meteo::surf::SurfaceService::GetAvailableSynMobSea, request, 120000);
  delete ctrl_src;ctrl_src=nullptr;

  if ( nullptr == reply ) {
    error_log << QObject::tr("При попытке получить данные в коде FM64 Ext. для ДСМ ответ от сервиса данных не получен.");
    return;
  }

  enum Tree {
    kName = 0,
    kLat = 1,
    kLon = 2,
  };

  bool first = true;
  for ( auto point : reply->point() ) {
    QTreeWidgetItem* item = new QTreeWidgetItem(ui_->hydroTree);
    if ( true == first ) {
      ui_->hydroTree->setCurrentItem(item);
      first = false;
    }
    item->setText(kName, QString::fromStdString(point.name()));
    meteo::GeoPoint g = meteo::GeoPoint::fromDegree(point.lat(), point.lon());
    item->setText(kLat, g.strLat());
    item->setText(kLon, g.strLon());
    std::string str;
    point.SerializeToString(&str);
    QByteArray arr(str.data(), str.size());
    item->setData(0, Qt::UserRole, QVariant(arr));
  }
}

bool DocumentParams::isInterAvailable()
{
  settings::proto::Service service = global::Settings::instance()->service(settings::proto::ServiceCode::kInterMessagesService);
  return service.has_host() && service.has_port();
}

void DocumentParams::setVisibleUserBtn(bool visible)
{
  ui_->sendUsersCheck->setVisible( visible );
  ui_->selectUsersBtn->setVisible( visible );
}

void DocumentParams::keyReleaseEvent(QKeyEvent *event)
{
  if ( event->key() == Qt::Key_Escape ) {
    this->close();
  }
}

}

