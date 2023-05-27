#include "gmiform.h"
#include "ui_gmiform.h"
#include <meteo/commons/settings/tusersettings.h>
#include <meteo/commons/global/dateformat.h>

enum kHourIndex {
  k0000 = 0,
  k0600 = 1,
  k1200 = 2,
  k1800 = 3
};

GmiForm::GmiForm(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::GmiForm),
  doc_(this),
  usersWidget_(nullptr)
{
  ui->setupUi(this);
  ui->dtDateBegin->setDisplayFormat(meteo::dtHumanFormatDateOnly);
  QPushButton* btnCreate = ui->confirmButtons->button(QDialogButtonBox::StandardButton::Ok);
  QPushButton* btnCancel = ui->confirmButtons->button(QDialogButtonBox::StandardButton::Cancel);
  btnCreate->setText(QObject::tr("Сформировать"));
  btnCancel->setText(QObject::tr("Отмена"));

  this->dtBegin_ = QDateTime::currentDateTimeUtc();
  this->dtBegin_.setTime(QTime(0,0,0));
  this->dtEnd_ = this->dtBegin_.addSecs(3600*36);

  this->ui->dtDateBegin->setDateTime(this->dtBegin_);
  QObject::connect(btnCreate, &QPushButton::clicked, this, &GmiForm::slotCreateDocumentButtonClicked);
  QObject::connect(btnCancel, &QPushButton::clicked, this, &GmiForm::close);
  QTimer::singleShot(0, this, &GmiForm::initialLoad);\

  this->setWindowTitle(QObject::tr("Прогноз гидрометеорологических условий по району"));

  ui->selectUsersBtn->setIcon(QIcon(":/meteo/icons/users.png"));
  QObject::connect(ui->selectUsersBtn, &QPushButton::clicked, this, &GmiForm::onSelectUsersBtnClicked);
}

GmiForm::~GmiForm()
{
  delete ui;
}

void GmiForm::initialLoad()
{
  QApplication::setOverrideCursor(Qt::WaitCursor);
  this->ui->stationSelect->loadStation({ meteo::sprinf::kStationSynop,
                                         meteo::sprinf::kStationSynmob,
                                         meteo::sprinf::kStationAirport,
                                         meteo::sprinf::kStationAero,
                                         meteo::sprinf::kStationAeromob,
                                         meteo::sprinf::kStationHydro,
                                         meteo::sprinf::kStationOcean,
                                         meteo::sprinf::kStationTyphoon,
                                         meteo::sprinf::kStationRadarmap,
                                         meteo::sprinf::kStationGeophysics,
                                         meteo::sprinf::kStationVa,
                                         meteo::sprinf::kStationFir,
                                         meteo::sprinf::kStationAirplane,
                                         meteo::sprinf::kStationNode,
                                         meteo::sprinf::kStationAerodrome
                                       });
  QApplication::restoreOverrideCursor();
}

void GmiForm::slotCreateDocumentButtonClicked()
{
  auto hour = static_cast<kHourIndex>(this->ui->cbTimeBegin->currentIndex());
  QTime time;

  switch (hour){
  case k0000:{
    time = QTime(0,0);
    break;
  }
  case k0600:{
    time = QTime(6,0);
    break;
  }
  case k1200:{
    time = QTime(12,0);
    break;
  }
  case k1800:{
    time = QTime(18,0);
    break;
  }
  default:{
    error_log << QObject::tr("Неверный индекс времени. Проверьте виджет");
    return;
  }
  }

  QDateTime dt = QDateTime( this->ui->dtDateBegin->date(), time,Qt::TimeSpec::UTC );

  auto hoursIndex = static_cast<meteo::map::proto::GmiArguments_GmiType>(ui->cbLength->currentIndex());

  meteo::map::proto::GmiArguments args;
  args.set_dt_start(dt.toString(Qt::ISODate).toStdString());
  args.set_doc_type(hoursIndex);
  args.set_document_number(ui->leRequestId->text().toUtf8());
  auto stationIndex = ui->stationSelect->stationIndex();
  auto stWgt = ui->stationSelect;
  auto ruName = stWgt->ruName();

  args.set_station_name(ruName.toUtf8().data());
  args.set_station_h(stWgt->coord().alt());
  args.set_station_index(stWgt->stationIndex().toStdString());
  args.set_lat(stWgt->coord().fi());
  args.set_lon(stWgt->coord().la());
  auto authorUserName = meteo::TUserSettings::instance()->getCurrentUserLogin();

  args.set_author(authorUserName.toStdString());

  meteo::map::proto::Map request;
  request.set_datetime(dtBegin_.toString(Qt::ISODate).toStdString());
  request.mutable_formal()->set_proto_args(args.SerializeAsString());
  request.set_format( meteo::map::proto::DocumentFormat::kOdt );
  request.mutable_document()->set_doctype(meteo::map::proto::DocumentType::kFormalDocument);
  request.mutable_formal()->set_module_path("shortgmi36h.py");
  request.mutable_formal()->set_format(meteo::map::proto::DocumentFormat::kOdt);

  switch (hoursIndex) {
  case meteo::map::proto::GmiArguments_GmiType_kShort:{
    static const auto name = QObject::tr("Краткосрочный прогноз ГМИ");
    static const auto mapName = QObject::tr("gmi-short.map");
    request.mutable_formal()->set_template_(QObject::tr("gmi.odt").toStdString());
    request.mutable_formal()->set_name(name.toStdString());
    request.set_name(mapName.toStdString());
    request.set_title(name.toStdString());
    request.set_hour(36);
    break;
  }
  case meteo::map::proto::GmiArguments_GmiType_kMedium:{
    static const auto name = QObject::tr("Среднесрочный прогноз ГМИ");
    static const auto mapName = QObject::tr("gmi-medium.map");
    request.mutable_formal()->set_template_(QObject::tr("gmi.odt").toStdString());
    request.mutable_formal()->set_name(name.toStdString());
    request.set_name(mapName.toStdString());
    request.set_title(name.toStdString());
    request.set_hour(72);
    break;
  }
  case meteo::map::proto::GmiArguments_GmiType_kLong:{
    static const auto name = QObject::tr("Долгосрочный прогноз ГМИ");
    static const auto mapName = QObject::tr("gmi-long.map");
    request.mutable_formal()->set_template_(QObject::tr("gmilong.odt").toStdString());
    request.mutable_formal()->set_name(name.toStdString());
    request.set_name(mapName.toStdString());
    request.set_title(name.toStdString());
    request.set_hour(168);
    break;
  }
  }

  request.set_datetime(dt.toString(Qt::ISODate).toStdString());
  meteo::sprinf::Station *st = request.add_station();
  st->set_station(stWgt->stationIndex().toStdString());
  st->set_index(stWgt->stationIndex().toInt());
  st->mutable_name()->set_international(stWgt->enName().toStdString());
  st->mutable_name()->set_rus(stWgt->ruName().toStdString());
  st->mutable_name()->set_short_(stWgt->ruName().toStdString());

  auto oldTimeout = doc_.setTimeout(600000);
  QStringList logins;
  if ( this->ui->sendUsersCheck->isChecked() ) {
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
  this->doc_.slotCreateFormalDoc(request, logins);
  //doc_.slotCreateFormalDoc(request);
  doc_.setTimeout(oldTimeout);
}

void GmiForm::onSelectUsersBtnClicked()
{
  if ( nullptr == this->usersWidget_) {
    this->usersWidget_ = new meteo::UserSelectForm(this);
    this->usersWidget_->setWindowFlags(Qt::Tool);
  }
  this->usersWidget_->show();
}
