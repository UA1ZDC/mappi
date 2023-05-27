#include "preparewidget.h"
#include "ui_preparewidget.h"
#include <meteo/commons/global/dateformat.h>

PrepareWidget::PrepareWidget(QWidget *parent) :
  QDialog(parent),
  ui_(new Ui::PrepareWidget)
{
  ui_->setupUi(this);
  ui_->begDt->setDisplayFormat(meteo::dtHumanFormatTimeShort);
  ui_->endDt->setDisplayFormat(meteo::dtHumanFormatTimeShort);

  ui_->stationGr->hide();

  setMode(kTransfer);

  connect( ui_->begDt, SIGNAL(dateTimeChanged(QDateTime)), SLOT(slotDateTimeChanged()) );
  connect( ui_->endDt, SIGNAL(dateTimeChanged(QDateTime)), SLOT(slotDateTimeChanged()) );

  connect( ui_->runBtn, SIGNAL(clicked(bool)), SIGNAL(run()) );
  connect( ui_->closeBtn, SIGNAL(clicked(bool)), SIGNAL(close()) );

  connect( ui_->checkP, SIGNAL(toggled(bool)), SIGNAL(paramChanged()) );
  connect( ui_->checkPQNH, SIGNAL(toggled(bool)), SIGNAL(paramChanged()) );
  connect( ui_->checkU, SIGNAL(toggled(bool)), SIGNAL(paramChanged()) );
  connect( ui_->checkT, SIGNAL(toggled(bool)), SIGNAL(paramChanged()) );
  connect( ui_->checkTd, SIGNAL(toggled(bool)), SIGNAL(paramChanged()) );
  connect( ui_->checkWind, SIGNAL(toggled(bool)), SIGNAL(paramChanged()) );

  connect( ui_->stationRadio, SIGNAL(toggled(bool)), SLOT(slotUpdateAnalyseState(bool)) );
  connect( ui_->analyseRadio, SIGNAL(toggled(bool)), ui_->centerCombo, SLOT(setEnabled(bool)) );
  connect( ui_->analyseRadio, SIGNAL(toggled(bool)), ui_->centerLabel, SLOT(setEnabled(bool)) );

  setByStation(true);
}

PrepareWidget::~PrepareWidget()
{
  delete ui_;
}

QDateTime PrepareWidget::beginDateTime() const
{
  return ui_->begDt->dateTime();
}

void PrepareWidget::setBeginDateTime(const QDateTime& dt)
{
  return ui_->begDt->setDateTime(dt);
}

QDateTime PrepareWidget::endDateTime() const
{
  return ui_->endDt->dateTime();
}

void PrepareWidget::setEndDateTime(const QDateTime& dt)
{
  ui_->endDt->setDateTime(dt);
}

bool PrepareWidget::isByStation() const
{
  return ui_->stationRadio->isChecked();
}

void PrepareWidget::setByStation(bool v)
{
  ui_->stationRadio->setChecked(v);
  ui_->analyseRadio->setChecked(!v);
}

QCheckBox* PrepareWidget::checkT() const
{
  return ui_->checkT;
}

QCheckBox* PrepareWidget::checkTd() const
{
  return ui_->checkTd;
}

QCheckBox* PrepareWidget::checkU() const
{
  return ui_->checkU;
}

QCheckBox* PrepareWidget::checkP() const
{
  return ui_->checkP;
}

QCheckBox*PrepareWidget::checkPQNH() const
{ return ui_->checkPQNH; }

QCheckBox* PrepareWidget::checkWind() const
{
  return ui_->checkWind;
}

StationWidget* PrepareWidget::stationEdit() const
{
  return ui_->stationWidget;
}

StationWidget*PrepareWidget::setStationWidget(StationWidget* w)
{
  if ( nullptr == w ) { return ui_->stationWidget; }

  StationWidget* old = ui_->stationWidget;
  ui_->stationWidget = w;
  return old;
}

bool PrepareWidget::isForecastTime() const
{
  return QDateTime::currentDateTimeUtc() < endDateTime();
}

QRadioButton* PrepareWidget::analyseRadio() const
{
  return ui_->analyseRadio;
}

QRadioButton* PrepareWidget::stationRadio() const
{
  return ui_->stationRadio;
}

QComboBox* PrepareWidget::centerCombo() const
{
  return ui_->centerCombo;
}

QComboBox* PrepareWidget::levelCombo() const
{
  return ui_->levelCombo;
}

void PrepareWidget::slotDateTimeChanged()
{
  if ( beginDateTime().secsTo(endDateTime()) < 3600 ) {
    if ( sender() == ui_->begDt ) {
      ui_->endDt->setDateTime(beginDateTime().addSecs(3600));
    }
    else if (sender() == ui_->endDt ) {
      ui_->begDt->setDateTime(endDateTime().addSecs(-3600));
    }
  }

  emit dateTimeChanged(beginDateTime(), endDateTime(), isForecastTime());
}

void PrepareWidget::slotUpdateAnalyseState(bool toggled)
{
  ui_->analyseRadio->setChecked(!toggled);
}

void PrepareWidget::setupGui()
{
  QList<QWidget*> shown;
  QList<QWidget*> hidden;

  hidden += ui_->paramGr;
  hidden += ui_->mgLabel;
  hidden += ui_->stationRadio;
  hidden += ui_->analyseRadio;
  hidden += ui_->fieldsTree;
  hidden += ui_->fieldsLable;
  hidden += ui_->transfGr;

  if ( kTransfer == mode_ ) {
    ui_->levelCombo->clear();
    ui_->levelCombo->addItems(QStringList() << "925" << "850" << "700" << "500");
    ui_->levelCombo->setCurrentIndex(1);

    shown += ui_->fieldsTree;
    shown += ui_->fieldsLable;
    shown += ui_->transfGr;

    ui_->runBtn->setText(tr("Добавить"));
  }
  else if ( kMeteogram == mode_ ) {
    shown += ui_->paramGr;
    shown += ui_->mgLabel;
    shown += ui_->stationRadio;
    shown += ui_->analyseRadio;

    ui_->runBtn->setText(tr("Сформировать"));
  }
  else if ( kSloy == mode_ ) {
    shown += ui_->mgLabel;
    shown += ui_->stationRadio;
    shown += ui_->analyseRadio;

    ui_->runBtn->setText(tr("Сформировать"));
  }

  for ( int i=0,isz=hidden.size(); i<isz; ++i ) {
    hidden[i]->hide();
  }
  for ( int i=0,isz=shown.size(); i<isz; ++i ) {
    shown[i]->show();
  }
}

bool PrepareWidget::isStationGroupVisible() const
{
  return ui_->stationGr->isVisible();
}

void PrepareWidget::setStationGroupVisible(bool visible)
{
  ui_->stationGr->setVisible(visible);
}

void PrepareWidget::setMargins(int left, int top, int right, int bottom)
{
  ui_->mainLayout->setContentsMargins(left, top, right, bottom);
}
