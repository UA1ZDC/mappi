#include "coloralertwidget.h"
#include "ui_coloralertwidget.h"

#include <qfilesystemwatcher.h>

#include <commons/meteo_data/tmeteodescr.h>
#include <commons/textproto/pbtools.h>
#include <commons/textproto/tprototext.h>

#include <meteo/commons/proto/sprinf.pb.h>
#include <meteo/commons/proto/surface_service.pb.h>
#include <meteo/commons/rpc/rpc.h>

#include <meteo/commons/global/global.h>

namespace meteo {

namespace coloralert {
QString confFile() { return QDir::homePath() + "/.meteo/meteotablo.conf"; }
} // coloralert

ColorAlertWidget::ColorAlertWidget(QWidget *parent) :
  QWidget(parent),
  ui_(new Ui::ColorAlertWidget)
{
  ui_->setupUi(this);

  connect( ui_->radiusSlider, SIGNAL(valueChanged(int)), SLOT(slotRadiusChanged(int)) );
  connect( ui_->radiusSlider, SIGNAL(sliderReleased()), SLOT(slotEmitChanged()) );
  connect( ui_->alphaSlider, SIGNAL(valueChanged(int)), SLOT(slotAlphaChanged(int)) );
  connect( ui_->alphaSlider, SIGNAL(sliderReleased()), SLOT(slotEmitChanged()) );

  //connect( ui_->distSlider, SIGNAL(valueChanged(int)), SLOT(slotDistChanged(int)) );
  //connect( ui_->distSlider, SIGNAL(sliderReleased()), SLOT(slotEmitChanged()) );


  connect( ui_->redCheck, SIGNAL(toggled(bool)), SLOT(slotEmitChanged()) );
  connect( ui_->yellowCheck, SIGNAL(toggled(bool)), SLOT(slotEmitChanged()) );
  connect( ui_->greenCheck, SIGNAL(toggled(bool)), SLOT(slotEmitChanged()) );

  if ( !QFile::exists(coloralert::confFile()) ) {
    QFile f(coloralert::confFile());
    f.open(QFile::WriteOnly);
    f.close();
  }

  watcher_ = new QFileSystemWatcher(this);
  watcher_->addPath(coloralert::confFile());
  connect( watcher_, SIGNAL(fileChanged(QString)), SLOT(slotLoadSettings()) );
  connect( watcher_, SIGNAL(fileChanged(QString)), SLOT(slotEmitChanged()) );

  // init
  ui_->errorGr->hide();
  ui_->radiusSlider->setValue(20);
  ui_->alphaSlider->setValue(30);
  //ui_->distSlider->setValue(50);
  slotLoadSettings();
}

ColorAlertWidget::~ColorAlertWidget()
{
  delete ui_;
}

int ColorAlertWidget::radius() const
{
  return ui_->radiusSlider->value() * 1000;
}

void ColorAlertWidget::blockGui(bool block)
{
  ui_->sliderGr->setDisabled(block);
  ui_->colorGr->setDisabled(block);
  ui_->scroll->setVisible(!block);
  ui_->errorGr->setVisible(block);

  if ( false == block ) {
    slotLoadSettings();
  }
}

bool ColorAlertWidget::isGuiBlocked() const
{
  return ui_->errorGr->isVisible();
}

void ColorAlertWidget::setAlpha(int percent)
{
  ui_->alphaSlider->setValue(percent);
}

void ColorAlertWidget::checkParams(bool checked)
{
  for ( int i=0,isz=ui_->paramsLayout->count(); i<isz; ++i ) {
    QCheckBox* cb = qobject_cast<QCheckBox*>(ui_->paramsLayout->itemAt(i)->widget());
    if ( nullptr == cb ) {
      continue;
    }
    cb->setChecked(checked);
  }
}

QString ColorAlertWidget::descrUndeCursor() const
{

  QCheckBox* check = qobject_cast<QCheckBox*>(childAt(mapFromGlobal(QCursor::pos())));
  if ( nullptr == check ) {
    return QString();
  }

  return check->property("descrname").toString();
}

tablo::Settings ColorAlertWidget::pattern() const
{
  tablo::Settings pattern;

  if ( QFile::exists(coloralert::confFile()) && !TProtoText::fromFile(coloralert::confFile(), &pattern) ) {
    debug_log << tr("Не удалось загрузить настройки (%1).").arg(coloralert::confFile());
  }

  pattern.set_radius_meter(radius());
 // pattern.set_max_dist_meter(ui_->distSlider->value());
  pattern.set_alpha(ui_->alphaSlider->value());

  QList<tablo::Color> list;
  if ( ui_->redCheck->isChecked() ) { list << tablo::kRed; }
  if ( ui_->yellowCheck->isChecked() ) { list << tablo::kYellow; }
  if ( ui_->greenCheck->isChecked() ) { list << tablo::kGreen; }

  QStringList checked = checkedParams();

  for ( int i=0,isz=pattern.alert_size(); i<isz; ++i ) {
    QString descrname = pbtools::toQString(pattern.alert(i).descrname());
    if ( !checked.contains(descrname) ) {
      pattern.mutable_alert(i)->clear_condition();
    }

    for ( int j=0,jsz=pattern.alert(i).condition_size(); j<jsz; ++j ) {
      if ( !list.contains(pattern.alert(i).condition(j).color()) ) {
        pattern.mutable_alert(i)->mutable_condition(j)->set_color(tablo::kNoColor);
      }
    }
  }

  return pattern;
}

QStringList ColorAlertWidget::paramList() const
{
  QStringList list;
  for ( int i=0,isz=ui_->paramsLayout->count(); i<isz; ++i ) {
    QCheckBox* cb = qobject_cast<QCheckBox*>(ui_->paramsLayout->itemAt(i)->widget());

    if ( nullptr == cb ) { continue; }

    list << cb->property("descrname").toString();
  }
  return list;
}

void ColorAlertWidget::slotRadiusChanged(int value)
{
  ui_->radiusLabel->setText(tr("%1 км").arg(value));

 // emit changed(pattern());
}

void ColorAlertWidget::slotAlphaChanged(int value)
{
  ui_->alphaLabel->setText(tr("%1 %").arg(value));
}

void ColorAlertWidget::slotDistChanged(int )
{
 //weather ui_->distLabel->setText(tr("%1 ").arg(value));
}

void ColorAlertWidget::slotEmitChanged()
{
  emit changed(pattern());
}

void ColorAlertWidget::slotLoadSettings()
{
  tablo::Settings pattern;
  if ( QFile::exists(coloralert::confFile()) && !TProtoText::fromFile(coloralert::confFile(), &pattern) ) {
    debug_log << tr("Не удалось загрузить настройки (%1).").arg(coloralert::confFile());
  }

  QStringList list;
  for ( int i=0,isz=pattern.alert_size(); i<isz; ++i ) {
    list << pbtools::toQString(pattern.alert(i).descrname());
  }
  updateParamList(list);

}

void ColorAlertWidget::updateParamList(const QStringList& newParams)
{
  QStringList checked = checkedParams();

  while ( ui_->paramsLayout->count() > 0 ) {
    delete ui_->paramsLayout->takeAt(0)->widget();
  }

  foreach ( QString name, newParams ) {
    QCheckBox* cb = new QCheckBox(ui_->scrollAreaWidgetContents);
    cb->setProperty("descrname", name);
    cb->setChecked(checked.contains(name));
    cb->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
    ui_->paramsLayout->addWidget(cb);
    connect( cb, SIGNAL(toggled(bool)), SLOT(slotEmitChanged()) );

    meteodescr::Property prop;
    bool ok = TMeteoDescriptor::instance()->property(name, &prop);

    if ( ok && !prop.unitsRu.isEmpty() && "NO" != prop.unitsRu ) {
      name += ", " + prop.unitsRu;
    }
    cb->setText(name);
    if ( ok ) {
      cb->setText(cb->text() + " - " + prop.description);
    }
  }

  ui_->paramsLayout->addItem(new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding));
}

QStringList ColorAlertWidget::checkedParams() const
{
  QStringList list;
  for ( int i=0,isz=ui_->paramsLayout->count(); i<isz; ++i ) {
    QCheckBox* cb = qobject_cast<QCheckBox*>(ui_->paramsLayout->itemAt(i)->widget());
    if ( nullptr == cb ) {
      continue;
    }

    if ( cb->isChecked() ) {
      list << cb->property("descrname").toString();
    }
  }
  return list;
}

void ColorAlertWidget::setDataInfo(const QString &text){
  ui_->info_label->setText(text);
}


} // meteo
