#include "locsettings.h"
#include "ui_locsettings.h"

#include <qlayout.h>
#include <qlineedit.h>
#include <qmessagebox.h>
#include <qvalidator.h>

#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/paths.h>
#include <meteo/commons/global/global.h>

#include <commons/textproto/pbtools.h>
#include <meteo/commons/global/common.h>
#include <meteo/commons/settings/settings.h>
#include <meteo/commons/settings/tusersettings.h>

#include <commons/textproto/tprototext.h>

namespace {
  const QString settingsHumanName() { return QString::fromUtf8("Пункт"); }
}

namespace meteo {

static const QString& userConfigPath() {
  static auto value = (MnCommon::etcPath() + "/location.conf");
  return value;
}



LocSettings::LocSettings(QWidget* parent) :
  SettingsWidget(parent),
  ui_(new Ui::LocSettings)
{
  setObjectName(::settingsHumanName());
  ui_->setupUi(this);

  ui_->wmoEdit->setHidden(true);
  ui_->wmoIdLabel->setHidden(true);
  //ui_->wmoEdit->setValidator(new QRegExpValidator(QRegExp("[A-Za-z]{4}")));
  //ui_->hmsEdit->setValidator(new QRegExpValidator(QRegExp(tr("[А-Яа-я]{4}"))));
  ui_->unitNumber->setValidator(new QRegExpValidator(QRegExp(tr("[0-9\\-]+"))));
  QObject::connect( ui_->wmoEdit, SIGNAL(textChanged(QString)), SLOT(slotCorrectInput(QString)) );
  QObject::connect( ui_->hmsEdit, SIGNAL(textChanged(QString)), SLOT(slotCorrectInput(QString)) );
}

LocSettings::~LocSettings()
{
  delete ui_;
  ui_ = 0;
}

void LocSettings::load()
{
  QMap<int,QString> ranks = meteo::TUserSettings::instance()->ranks();

  foreach( int id, ranks.keys() ){
    ui_->rankCmdr->addItem(ranks.value(id), id);
  }
  meteo::settings::Location loc;
  if ( false == loadLocationSettigns(&loc) ){
    warning_log.msgBox() << QObject::tr("Не удалось получить параметры пункта");
  }
  setLoc(loc);
  QObject::connect(ui_->coord, SIGNAL(changed()), SLOT(slotChangeState()));
  QObject::connect(ui_->fioCmdr, SIGNAL(textChanged(QString)), SLOT(slotChangeState()));
  QObject::connect(ui_->locIndex, SIGNAL(textChanged(QString)), SLOT(slotChangeState()));
  QObject::connect(ui_->locName, SIGNAL(textChanged(QString)), SLOT(slotChangeState()));
  QObject::connect(ui_->wmoEdit, SIGNAL(textChanged(QString)), SLOT(slotChangeState()));
  QObject::connect(ui_->hmsEdit, SIGNAL(textChanged(QString)), SLOT(slotChangeState()));
  QObject::connect(ui_->unitNumber, SIGNAL(textChanged(QString)), SLOT(slotChangeState()));
  QObject::connect(ui_->rankCmdr, SIGNAL(currentIndexChanged(int)), SLOT(slotChangeState()));
  QObject::connect(ui_->stationType, SIGNAL(currentIndexChanged(int)), SLOT(slotChangeState()));
}

bool LocSettings::setLoc(const meteo::settings::Location &loc)
{
  if (false == loc.IsInitialized()) {
    return false;
  }
  setCoord( pbgeopoint2geopoint( loc.coord() ) );
  if (loc.has_index()) {
    setLocIndex(QString::fromUtf8(loc.index().c_str()));
  }
  if (loc.has_name()) {
    setLocName(QString::fromUtf8(loc.name().c_str()));
  }
  if (loc.has_unit_number()) {
    setUnitNumber(QString::fromUtf8(loc.unit_number().c_str()));
  }
  if (loc.has_fio_cmdr()) {
    setFioCmdr(QString::fromUtf8(loc.fio_cmdr().c_str()));
  }
  if (loc.has_rank_cmdr()) {
    setRankCmdr(loc.rank_cmdr());
  }
  if (loc.has_station_type()) {
    setStatonType(loc.station_type());
  }
  ui_->wmoEdit->setText(pbtools::toQString(loc.wmo_id()));
  ui_->hmsEdit->setText(pbtools::toQString(loc.hms_id()));

  return true;
}

QString LocSettings::fioCmdr() const
{
  return ui_->fioCmdr->text();
}

int LocSettings::rankCmdr() const
{
  return ui_->rankCmdr->itemData(ui_->rankCmdr->currentIndex()).toInt();
}


void LocSettings::setStatonType(meteo::settings::NodeType stationtype)
{
  if(ui_->stationType->count()>stationtype){
    ui_->stationType->setCurrentIndex(stationtype);
  }
}

void LocSettings::setRankCmdr(int rank)
{
   for( int i=0, isz=ui_->rankCmdr->count(); i<isz; ++i ){
    if (ui_->rankCmdr->itemData(i).toInt() == rank) {
      ui_->rankCmdr->setCurrentIndex(i);
      break;
    }
  }
}

meteo::settings::NodeType LocSettings::stationType() const
{
  return static_cast<meteo::settings::NodeType>(ui_->stationType->currentIndex());
}

QString LocSettings::locName() const
{
  return ui_->locName->text();
}

meteo::GeoPoint LocSettings::coord() const
{
  return ui_->coord->coord();
}

QString LocSettings::locIndex() const
{
  return ui_->locIndex->text();
}

QString LocSettings::unitNumber() const
{
  return ui_->unitNumber->text();
}

void LocSettings::setFioCmdr(const QString &fio)
{
  ui_->fioCmdr->setText(fio);
}

void LocSettings::setLocName(const QString &name)
{
  ui_->locName->setText(name);
}

void LocSettings::setCoord(const meteo::GeoPoint coord)
{
  ui_->coord->setCoord(coord);
}

void LocSettings::setLocIndex(const QString &index)
{
  ui_->locIndex->setText(index);
}

void LocSettings::setUnitNumber(const QString &number)
{
  ui_->unitNumber->setText(number);
}

meteo::settings::Location LocSettings::loc() const
{
  meteo::settings::Location loc;
  loc.set_fio_cmdr(fioCmdr().toUtf8());
  loc.set_index(locIndex().toUtf8());
  loc.set_rank_cmdr(rankCmdr());
  loc.set_unit_number(unitNumber().toUtf8());
  loc.set_name(locName().toUtf8());
  loc.set_station_type(stationType());
  loc.mutable_coord()->CopyFrom( geopoint2pbgeopoint( coord() ) );
//  loc.mutable_coord()->set_lat_radian(coord().lat());
//  loc.mutable_coord()->set_lon_radian(coord().lon());
//  loc.mutable_coord()->set_height_meters(coord().());
  loc.set_wmo_id(pbtools::toString(ui_->wmoEdit->text()));
  loc.set_hms_id(pbtools::toString(ui_->hmsEdit->text()));

  return loc;
}

void LocSettings::save()
{
  if (changed_ == false) {
    return;
  }
  meteo::settings::Location loc = this->loc();
  if (false == loc.has_coord()) {
    error_log.msgBox() << QString::fromUtf8("Не удалось сохранить параметры пункта. Не получены координаты пункта");
    return;
  }
  if ( writeLocationSettings(loc) ) {
    changed_ = false;
    info_log.msgBox() << QObject::tr("Настройки пункта успешно сохранены");
  }
  else {
    error_log.msgBox() << QString::fromUtf8("Не удалось сохранить параметры пункта");
  }

}

void LocSettings::slotChangeState()
{
  changed_ = true;
  emit changed();
  emit locationChanged(loc());
}

void LocSettings::slotCorrectInput(const QString& text)
{
  QLineEdit* le = qobject_cast<QLineEdit*>(sender());

  if ( nullptr == le ) {
    return;
  }

  le->blockSignals(true);
  le->setText(text.toUpper());
  le->blockSignals(false);
}

bool LocSettings::writeLocationSettings(const meteo::settings::Location& loc)
{
  QFile ofile(userConfigPath());
  if ( false == ofile.open(QIODevice::WriteOnly)){
    error_log << meteo::msglog::kFileWriteFailed
                              .arg(userConfigPath())
                              .arg(ofile.errorString());
    return false;
  }

  auto data = TProtoText::toText(loc);
  auto written = ofile.write(data);
  if ( written != data.size() ){
    error_log << QObject::tr("Ошибка при записи файла. Ожидалось, что будет записано %1 символов, записано %2 символов")
                 .arg(data.size())
                 .arg(written);
    return false;
  }
  return true;
}

bool LocSettings::loadLocationSettigns(meteo::settings::Location* loc)
{
  QFile ifile(userConfigPath());
  if ( false == ifile.open(QIODevice::ReadOnly)){
    error_log << meteo::msglog::kFileReadFailed
                       .arg(userConfigPath())
                       .arg(ifile.errorString());
    return false;;
  }

  auto data = QString::fromUtf8( ifile.readAll() );
  ifile.close();

  bool result = TProtoText::fillProto(data, loc);
  return result;
}

} // meteo
