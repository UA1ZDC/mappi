#include "sloywidget.h"
#include "ui_sloywidget.h"

#include <cross-commons/debug/tlog.h>

Q_DECLARE_METATYPE( meteo::surf::OneZondValueOnStation )

SloyWidget::SloyWidget(QWidget *parent) :
  QWidget(parent),
  ui_(new Ui::SloyWidget)
{
  ui_->setupUi(this);

  stationWidget_ = 0;

  connect( ui_->dateEdit, SIGNAL(dateTimeChanged(QDateTime)), SIGNAL(changed()) );
  connect( ui_->stationRadio, SIGNAL(toggled(bool)), SIGNAL(changed()) );
  connect( ui_->analyseRadio, SIGNAL(toggled(bool)), SIGNAL(changed()) );
  connect( ui_->centerCombo, SIGNAL(currentIndexChanged(int)), SIGNAL(changed()) );

  setExternalStationWidget(0);
}

SloyWidget::~SloyWidget()
{
  delete ui_;
}

void SloyWidget::setExternalStationWidget(StationWidget* w)
{
  if ( 0 != stationWidget_ ) {
    disconnect( stationWidget_ );
  }

  if ( 0 == w ) {
    ui_->stationWidgetGr->show();
    stationWidget_ = ui_->stationWidget;
  }
  else {
    ui_->stationWidgetGr->hide();
    stationWidget_ = w;
  }

  if ( 0 != stationWidget_ ) {
    connect( stationWidget_, SIGNAL(changed()), SIGNAL(changed()) );
  }
}

QComboBox* SloyWidget::centerCombo() const
{
  return ui_->centerCombo;
}

void SloyWidget::setData(const QScriptValue& data)
{
  meteo::surf::OneZondValueOnStation d = qscriptvalue_cast<meteo::surf::OneZondValueOnStation>(data);
  ui_->zond->setData(d);
}

QScriptValue SloyWidget::zondData() const
{
  return engine()->toScriptValue(ui_->zond->toZond());
}

void SloyWidget::hideZondColumn(int column)
{
  ui_->zond->hideColumn(column);
}

void SloyWidget::showZondColumn(int column)
{
  ui_->zond->showColumn(column);
}

QDateTime SloyWidget::observationDt() const
{
  return ui_->dateEdit->dateTime();
}

void SloyWidget::setObservationDt(const QDateTime& dt)
{
  if ( dt == ui_->dateEdit->dateTime() ) { return; }

  ui_->dateEdit->setDateTime(dt);
}

int SloyWidget::center() const
{
  int idx = ui_->centerCombo->currentIndex();

  if ( -1 == idx ) { return -1; }

  return ui_->centerCombo->itemData(idx, kCenterIdRole).toInt();
}

bool SloyWidget::isByStation() const
{
  return ui_->stationRadio->isChecked();
}

void SloyWidget::setByStation(bool check)
{
  ui_->stationRadio->setChecked(check);
}

bool SloyWidget::isByAnalyse() const
{
  return ui_->analyseRadio->isChecked();
}

void SloyWidget::setByAnalyse(bool check)
{
  ui_->analyseRadio->setChecked(check);
}

bool SloyWidget::isPostGrVisible() const
{
  return ui_->postGr->isVisible();
}

void SloyWidget::setPostGrVisible(bool visible)
{
  ui_->postGr->setVisible(visible);
}

bool SloyWidget::isPostAltVisible() const
{
  return ui_->altSpin->isVisible();
}

void SloyWidget::setPostAltVisible(bool visible)
{
  ui_->altLabel->setVisible(visible);
  ui_->altSpin->setVisible(visible);
}

bool SloyWidget::isPostNumVisible() const
{
  return ui_->postSpin->isVisible();
}

void SloyWidget::setPostNumVisible(bool visible)
{
  ui_->postLabel->setVisible(visible);
  ui_->postSpin->setVisible(visible);
}

int SloyWidget::postNum() const
{
  return ui_->postSpin->value();
}

void SloyWidget::setPostNum(int num)
{
  ui_->postSpin->setValue(num);
}

int SloyWidget::postAlt() const
{
  return ui_->altSpin->value();
}

void SloyWidget::setPostAlt(int alt)
{
  ui_->altSpin->setValue(alt);
}

void SloyWidget::setZondLabel(const QString& text)
{
  ui_->zond->setLabel(text);
}

void SloyWidget::slotSetCenters(const QStringList& names, const QList<int>& ids)
{
  if ( names.size() != ids.size() ) { trc; return; }

  QString last = ui_->centerCombo->currentText();

  ui_->centerCombo->blockSignals(true);

  ui_->centerCombo->clear();
  for ( int i=0,isz=ids.size(); i<isz; ++i ) {
    ui_->centerCombo->addItem(names[i], ids[i]);
  }

  ui_->centerCombo->blockSignals(false);

  for ( int i=0,isz=ui_->centerCombo->count(); i<isz && !last.isEmpty(); ++i ) {
    if ( ui_->centerCombo->itemText(i) == last ) {
      ui_->centerCombo->setCurrentIndex(i);
      break;
    }
  }
}
