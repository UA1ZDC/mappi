#include "climatsettings.h"
#include "stationsearch.h"
#include "settingsfuncs.h"
#include "datasortitem.h"

#include <commons/textproto/tprototext.h>
#include <meteo/commons/global/global.h>
#include <meteo/commons/global/common.h>


namespace meteo {

ClimatSettings::ClimatSettings(const meteo::settings::Location& loc,
                           QWidget *parent):
  StationSelectForm (loc, parent)
{
  setObjectName(QObject::tr("Настройки климата"));
}

QString ClimatSettings::stationListLabelText()
{
  return QObject::tr("Станции, по которым выполняется рассчет климатических характеристик");
}

QList<sprinf::MeteostationType> ClimatSettings::stationTypes()
{
  return { sprinf::MeteostationType::kStationSynop,
        sprinf::MeteostationType::kStationAero };
}

QList<sprinf::Station> ClimatSettings::loadStations()
{
  meteo::climat::ClimatSaveProp prop;
  prop = meteo::global::loadClimatStations();
  
  QList<sprinf::Station> stations;
  for ( auto station: prop.stations() ){
    stations << station.station();    

    this->stationsUtcs_[QString::fromStdString(station.station().station())] = qRound(station.utc() * 60);
  }
  return stations;
}

bool ClimatSettings::saveStations(const QList<sprinf::Station> &stations)
{
  meteo::climat::ClimatSaveProp prop;

  for ( auto station: stations ) {
    climat::ClimatStation* neib = prop.add_stations();
    neib->mutable_station()->CopyFrom(station);
    auto ownIndex = QString::fromUtf8(own().index().c_str(),
                                      own().index().size());

    auto stationIndex = QString::fromStdString(station.station());
    float h = 60;
    float utc = this->stationsUtcs_[stationIndex] / h;
    neib->set_utc(utc);

    auto clType = station.type() == sprinf::MeteostationType::kStationAero?
          sprinf::MeteostationType::kStationAero:
          sprinf::MeteostationType::kStationSynop;

    neib->set_cltype(clType);

    if ( station.index() == ownIndex.toInt() ) {
      neib->set_isown(true);
    }
    else {
      neib->set_isown(false);
    }
  }  


  if ( false == meteo::global::saveClimatStations(prop) ) {
    error_log.msgBox() << meteo::msglog::kFileWriteFailed
                          .arg(meteo::global::kClimatConfPath())
                          .arg(QString());
    return false;
  }
  return true;
}


QStringList ClimatSettings::additionalStationParams()
{
  static QStringList params( {
                               QObject::tr("Часовая зона")
                             });
  return params;
}


void ClimatSettings::onUtcComboChanged(int index)
{
  auto sender = qobject_cast<QComboBox*>(QObject::sender());
  auto station = sender->objectName();

  this->stationsUtcs_[station] = sender->itemData(index, Qt::UserRole).toInt();
  this->stateChanged();
}

QWidget* ClimatSettings::getAdditionalColumnWidget(int index, const meteo::sprinf::Station& station)
{
  auto stationIndex = QString::fromStdString(station.station());

  switch(index) {
  case 0:{
    auto timeZoneTemplate = QObject::tr("UTC%1%2");
    auto timeZoneWithTimeTemplate = QObject::tr("UTC%1%2");

    QComboBox *cb = new QComboBox();

    if ( false == this->stationsUtcs_.contains(stationIndex) ) {
      this->stationsUtcs_[stationIndex] = 180;
    }

    QString selectedText = QObject::tr("UTC+4");
    for ( int  i = -12 * 60; i <= 12*60; i += 15 ){
      int hours = i / 60;
      int mins = abs(i % 60);
      auto text = 0 == mins? QObject::tr("UTC%1%2")
                             .arg(hours>=0?'+':'-')
                             .arg(abs(hours))
                           :
                             QObject::tr("UTC%1%2:%3")
                             .arg(hours>=0?'+':'-')
                             .arg(abs(hours))
                             .arg(mins);
      cb->addItem(text, i);

      if (  i == this->stationsUtcs_[stationIndex] ){
        selectedText = text;
      }
    }
    cb->setObjectName(stationIndex);
    QObject::connect(cb, SIGNAL(currentIndexChanged(int)), this, SLOT(onUtcComboChanged(int)) );


    cb->setCurrentText(selectedText);

    return cb;
  }
  default:
    return nullptr;
  }
}

}
