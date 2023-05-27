#include "stationdlg.h"
#include "ui_stationdlg.h"

namespace meteo {

StationDlg::StationDlg(QWidget *parent) :
  QDialog(parent),
  ui_(new Ui::StationDlg)
{
  ui_->setupUi(this);

  QObject::connect( ui_->addBtn, SIGNAL(clicked()), SIGNAL(addClicked()) );
}

StationDlg::~StationDlg()
{
  delete ui_;
}

void StationDlg::init()
{
  setCursor(Qt::WaitCursor);
  ui_->stationWidget->loadStation(QList<meteo::sprinf::MeteostationType>()
				  << sprinf::kStationSynop   << sprinf::kStationSynmob
				  << sprinf::kStationAirport << sprinf::kStationAerodrome
				  << sprinf::kStationHydro   << sprinf::kStationOcean);
  unsetCursor();
}

void StationDlg::setStatus(const QString& text, int timeoutSec)
{
  ui_->statusLabel->setText(text);

  if ( -1 != timeoutSec ) {
    QTimer::singleShot( timeoutSec * 1000, this, SLOT(slotClearStatus()) );
  }
}

sprinf::Station StationDlg::getStation() const
{
  return ui_->stationWidget->toStation();
}

void StationDlg::slotClearStatus()
{
  ui_->statusLabel->clear();
}

void StationDlg::closeEvent(QCloseEvent* e)
{
  reject();
  QDialog::closeEvent(e);
}

} // meteo
