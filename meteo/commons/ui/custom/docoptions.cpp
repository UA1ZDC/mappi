#include "ui_docoptions.h"
#include "docoptions.h"

#include <cross-commons/debug/tlog.h>
#include <commons/mathtools/mnmath.h>
#include <meteo/commons/global/common.h>
#include <meteo/commons/ui/map/geopolygon.h>

#include <qobject.h>
#include <qevent.h>
#include <qdebug.h>
#include <qstring.h>

#include <meteo/commons/global/global.h>
#include <commons/textproto/tprototext.h>

DocOptions::DocOptions(QWidget* p):
  QWidget(p),
  ui_( new Ui::DocOptions )
{
  ui_->setupUi( this );

  ui_->cache->setVisible(false);
  ui_->lblcache->setVisible(false);

  QStringList geo = meteo::global::geoLoaders();
  ui_->geoloader->insertItems(0,geo);
  ui_->userValueEdit->setDisabled(true);
  QObject::connect( ui_->projgroup, SIGNAL(buttonClicked(QAbstractButton*)), this, SLOT(slotChanged()) );
  QObject::connect( ui_->scalegroup, SIGNAL(buttonClicked(QAbstractButton*)), this, SLOT(slotSwitch()) );
  QObject::connect( ui_->mapcenter, SIGNAL(changed()), this, SLOT(slotChanged()) );
  QObject::connect( ui_->cmbpunchorient, SIGNAL( currentIndexChanged(int) ), this, SLOT( slotChanged() ) );
  QObject::connect( ui_->cmbpunchquality, SIGNAL( currentIndexChanged(int) ), this, SLOT( slotChanged() ) );
  QObject::connect( ui_->cmbisolabel, SIGNAL( currentIndexChanged(int) ), this, SLOT( slotChanged() ) );
  QObject::connect( ui_->cmbisosmooth, SIGNAL( currentIndexChanged(int) ), this, SLOT( slotChanged() ) );
  QObject::connect( ui_->geoloader, SIGNAL( currentIndexChanged(int) ), this, SLOT( slotChanged() ) );
  QObject::connect( ui_->cmbintersect, SIGNAL( currentIndexChanged(int) ), this, SLOT( slotChanged() ) );
  QObject::connect( ui_->cmboutline, SIGNAL( currentIndexChanged(int) ), this, SLOT( slotChanged() ) );
  QObject::connect( ui_->btnoutlinecolor, SIGNAL( colorChanged(QRgb) ), this, SLOT( slotChanged() ) );
  QObject::connect( ui_->btnbackgroundcolor, SIGNAL( colorChanged(QRgb) ), this, SLOT(slotChanged()));
  QObject::connect( ui_->cache, SIGNAL( currentIndexChanged(int) ), this, SLOT( slotChanged() ) );
  QFile file(meteo::global::kMapColorSettingsFilename);
  if ( false == file.exists() ) {
    meteo::createDefaultColorSettings(meteo::global::kMapColorSettingsFilename);
  }
  loadMapSettings();
}

DocOptions::~DocOptions()
{
  delete ui_;
  ui_ = nullptr;
}

meteo::ProjectionType DocOptions::projection() const
{
  meteo::ProjectionType proj = meteo::kStereo;
  QAbstractButton* btn = ui_->projgroup->checkedButton();
  if ( ui_->mercator == btn ) {
    proj = meteo::kMercat;
  }
  else if ( ui_->conich == btn ) {
    proj = meteo::kConical;
  }
  return proj;
}

double DocOptions::scale() const
{
  QAbstractButton* btn = ui_->scalegroup->checkedButton();
  double sc = 16.0;
  if ( btn == ui_->polush ) {
    sc = 17.0;
  }
  else if ( btn == ui_->region ) {
    sc = 15.0;
  }
  else if ( btn == ui_->ring ) {
    sc = 14.0;
  }
  else if ( btn == ui_->microring ) {
    sc = 13.0;
  }
  else if (btn == ui_->userValue) {
    sc=ui_->userValueEdit->value();
  }
  return sc;
}

double DocOptions::lat_radian() const
{
  return ui_->mapcenter->coord().lat();
}

double DocOptions::lon_radian() const
{
  return ui_->mapcenter->coord().lon();
}

double DocOptions::lat_degree() const
{
  return ui_->mapcenter->latDegree();
}

double DocOptions::lon_degree() const
{
  return ui_->mapcenter->lonDegree();
}

int DocOptions::punchOrient() const
{
  return ui_->cmbpunchorient->currentIndex();
}

int DocOptions::ruleVeiw() const
{
  return ui_->cmbpunchquality->currentIndex();
}

int DocOptions::isoLabel() const
{
  return ui_->cmbisolabel->currentIndex();
}

bool DocOptions::isoSmooth() const
{
  if ( 0 == ui_->cmbisosmooth->currentIndex() ) {
    return false;
  }
  return true;
}

QString DocOptions::loader() const
{
  return ui_->geoloader->currentText();
}

int DocOptions::crossType() const
{
  meteo::map::proto::CrossType t = meteo::map::proto::kCrossLayer;
  if ( 1 == ui_->cmbintersect->currentIndex() ) {
    t = meteo::map::proto::kCrossDocument;
  }
  return t;
}

bool DocOptions::outline() const
{
  bool fl = false;
  if ( 1 == ui_->cmboutline->currentIndex() ) {
    fl = true;
  }
  return fl;
}

QRgb DocOptions::outlineColor() const
{
  return ui_->btnoutlinecolor->color();
}

QRgb DocOptions::backgroundColor() const
{
  return ui_->btnbackgroundcolor->color();
}

bool DocOptions::cache() const
{
  bool fl = false;
  if ( 0 == ui_->cache->currentIndex() ) {
    fl = true;
  }
  return fl;
}

void DocOptions::setCoord(const meteo::GeoPoint &pnt)
{
  ui_->mapcenter->setCoord(pnt);
}

void DocOptions::setProjection(const meteo::ProjectionType &proj)
{
  switch ( proj ) {
    case meteo::kMercat:
      ui_->mercator->setChecked(true);
      break;
    case meteo::kConical:
      ui_->conich->setChecked(true);
      break;
    case meteo::kStereo:
      ui_->stereo->setChecked(true);
      break;
    default:
      ui_->stereo->setChecked(true);
      break;
  }
}

void DocOptions::setScale( double sc )
{
  ui_->userValueEdit->setDisabled(true);
  if (sc < 0.5 ) {
    sc = 0.5;
  }
  if ( true == MnMath::isEqual( sc, 17.0) ) {
    ui_->polush->setChecked(true);
  }
  else if ( true == MnMath::isEqual( sc, 15.0) ) {
    ui_->region->setChecked(true);
  }
  else if ( true == MnMath::isEqual( sc, 14.0) ) {
    ui_->ring->setChecked(true);
  }
  else if ( true == MnMath::isEqual( sc, 13.0) ) {
    ui_->microring->setChecked(true);
  }
  else {
    ui_->userValueEdit->setDisabled(false);
    ui_->userValueEdit->setValue(sc);
    ui_->userValue->setChecked(true);
  }
}

void DocOptions::setPunchOrient( int orient )
{
  ui_->cmbpunchorient->setCurrentIndex(orient);
}

void DocOptions::setRuleVeiw( int qual )
{
  ui_->cmbpunchquality->setCurrentIndex(qual);
}

void DocOptions::setIsoLabel( int orient )
{
  ui_->cmbisolabel->setCurrentIndex(orient);
}

void DocOptions::setIsoSmooth( bool fl )
{
  if ( false == fl ) {
    ui_->cmbisosmooth->setCurrentIndex(0);
  }
  else {
    ui_->cmbisosmooth->setCurrentIndex(1);
  }
}

void DocOptions::setLoader( const QString& str )
{
  int indx = 0;
  if ( false == str.isEmpty() ) {
    indx = ui_->geoloader->findText(str);
  }
  if ( -1 == indx ) {
    error_log.msgBox() << QObject::tr("Загрузчик %1 не найден.")
      .arg(str);
    indx = 0;
  }
  ui_->geoloader->setCurrentIndex(indx);
}

void DocOptions::setCrossType( int t )
{
  switch (t) {
    case meteo::map::proto::kCrossDocument:
      t = 1;
      break;
    case meteo::map::proto::kCrossLayer:
    default:
      t = 0;
      break;
  };
  ui_->cmbintersect->setCurrentIndex(t);
}

void DocOptions::setOutline( bool fl )
{
  int indx = 0;
  if ( true == fl ) {
    indx = 1;
  }
  ui_->cmboutline->setCurrentIndex(indx);
}

void DocOptions::setOutlineColor( QRgb clr )
{
  ui_->btnoutlinecolor->setColor( clr );
}

void DocOptions::setBackgroundColor(QRgb color)
{
  ui_->btnbackgroundcolor->setColor(color);
}

void DocOptions::loadMapSettings()
{
  settings_ = meteo::mapSettings();
  if ( false == settings_.contains(meteo::kMAPCOLORSETTINGS_IDENT) ) {
    ui_->btnbackgroundcolor->setColor(QColor(meteo::kMAP_ISLAND_COLOR).rgb());
  }
  else {
    ui_->btnbackgroundcolor->setColor(settings_[meteo::kMAPCOLORSETTINGS_IDENT].brush().color());
  }
}

bool DocOptions::saveMapSettings()
{
  settings_[meteo::kMAPCOLORSETTINGS_IDENT].mutable_brush()->set_color(ui_->btnbackgroundcolor->color());
  settings_[meteo::kMAPISLAND31_IDENT].mutable_brush()->set_color(ui_->btnbackgroundcolor->color());
  meteo::Properties properties;

  for(const auto& prop: settings_) {
    properties.add_property()->CopyFrom(prop);
  }

  QFile file(meteo::global::kMapColorSettingsFilename);
  if ( false == file.exists() ) {
    error_log << QObject::tr("Файл с параметрами картографических документов не существует\n\t Имя файла - %1")
      .arg(meteo::global::kMapColorSettingsFilename);
    return false;
  }
  file.setPermissions( QFileDevice::ReadOwner
                      |QFileDevice::WriteOwner
                      |QFileDevice::ReadGroup
                      |QFileDevice::WriteGroup
                      |QFileDevice::ReadOther
                      |QFileDevice::WriteOther);
  return TProtoText::toFile(properties, meteo::global::kMapColorSettingsFilename);
}

void DocOptions::setCache( bool fl )
{
  int indx = 1;
  if ( true == fl ) {
    indx = 0;
  }
  ui_->cache->setCurrentIndex(indx);
}

QString DocOptions::projectionText() const
{
  QString projstr = QObject::tr("Неизвестно");
  if ( 0 != ui_->projgroup->checkedButton() ) {
    projstr = ui_->projgroup->checkedButton()->text();
  }
  return projstr;
}

QString DocOptions::scaleText() const
{
  QString regstr = QObject::tr("Неизвестно");
  if ( 0 != ui_->scalegroup->checkedButton() ) {
    regstr = ui_->scalegroup->checkedButton()->text();
  }
  return regstr;
}

bool DocOptions::setDoc(const meteo::map::proto::Document &doc)
{
  if ( false == doc.IsInitialized() ) {
    return false;
  }
  meteo::GeoPoint pnt( 0.0, 0.0 );
  if ( true == doc.has_doc_center() ) {
    if ( true == doc.doc_center().has_lat_radian() ) {
      pnt = meteo::GeoPoint( doc.doc_center().lat_radian(), doc.doc_center().lon_radian() );
    }
    else if ( true == doc.doc_center().has_lat_deg()  ) {
      pnt = meteo::GeoPoint::fromDegree( doc.doc_center().lat_deg(), doc.doc_center().lon_deg() );
    }
  }
  else if ( true == doc.has_map_center() ) {
    if ( doc.map_center().has_lat_radian() ) {
      pnt = meteo::GeoPoint( doc.map_center().lat_radian(), doc.map_center().lon_radian() );
    }
    else if ( true == doc.map_center().has_lat_deg()  ) {
      pnt = meteo::GeoPoint::fromDegree( doc.map_center().lat_deg(), doc.map_center().lon_deg() );
    }
  }
  setCoord(pnt);
  if ( true == doc.has_projection() ) {
    setProjection( doc.projection() );
  }
  if ( true == doc.has_scale() ) {
    setScale(doc.scale());
    ui_->userValueEdit->setValue(doc.scale());
  }
  setPunchOrient( doc.punchorient() );
  setRuleVeiw( doc.ruleview() );
  int indx = 0;
  if ( true == doc.text_towards_iso() ) {
    indx = 1;
  }
  setIsoLabel(indx);
  setIsoSmooth( doc.smooth_iso() );
  setLoader( QString::fromStdString( doc.geoloader() ) );
  setCrossType( doc.cross_type() );
  setOutline( doc.text_outline() );
  setOutlineColor( doc.outlinecolor() );
  setCache( doc.cache() );
  return true;
}

meteo::map::proto::Document DocOptions::doc() const
{
  meteo::map::proto::Document doc;
  doc.set_projection( projection() );
  doc.set_scale( scale() );
  doc.mutable_map_center()->set_lat_radian( lat_radian() );
  doc.mutable_map_center()->set_lon_radian( lon_radian() );
  doc.mutable_doc_center()->set_lat_radian( lat_radian() );
  doc.mutable_doc_center()->set_lon_radian( lon_radian() );

  doc.set_punchorient( static_cast<meteo::map::proto::PunchOrient>( punchOrient() ) );
  doc.set_ruleview( static_cast<meteo::RuleView>( ruleVeiw() ) );
  bool isolabel = false;
  if ( 1 == isoLabel() ) {
    isolabel = true;
  }
  doc.set_text_towards_iso(isolabel);
  doc.set_smooth_iso( isoSmooth() );
  doc.set_geoloader( loader().toStdString() );
  doc.set_cross_type( static_cast<meteo::map::proto::CrossType>( crossType() ) );
  doc.set_text_outline( outline() );
  doc.set_outlinecolor( outlineColor() );

  doc.set_gridloader( meteo::global::kGridLoaderCommon.toStdString() );
  doc.set_cache( cache() );

  return doc;
}

void DocOptions::slotChanged()
{
  emit changed();
}

void DocOptions::slotSwitch()
{
  if (true==ui_->userValue->isChecked()) {
    ui_->userValueEdit->setDisabled(false);
  }
  else {
    ui_->userValueEdit->setDisabled(true);
  }
  ui_->userValueEdit->setValue(scale());
}
