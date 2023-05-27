#include "frontpropwidget.h"
#include "ui_frontpropwidget.h"

#include "arrowdlg.h"

#include <qcolordialog.h>
#include <qpainter.h>
#include <qabstractitemview.h>
#include <qsettings.h>

#include <cross-commons/debug/tmap.h>
#include <cross-commons/debug/tlog.h>

#include <commons/textproto/pbtools.h>
#include <meteo/commons/global/weatherloader.h>
#include <meteo/commons/ui/map/geopolygon.h>
#include <commons/geobasis/generalproj.h>


namespace meteo {
namespace map {

FrontPropWidget::FrontPropWidget(QWidget* parent)
  : PropWidget(parent)
{
  ui_ = new Ui::FrontPropWidget;
  ui_->setupUi(this);
  ui_->frontCombo->view()->setSelectionBehavior(QAbstractItemView::SelectRows);
  ui_->delBtn->setEnabled(false);

  setWindowFlags(Qt::Tool);

  initOrnaments();
  ui_->finishBtn->setIcon(QIcon(":/meteo/icons/apply.png"));
  ui_->mirrorBtn->setIcon(QIcon(":/meteo/icons/drawtools/mirror.png"));
  QObject::connect( ui_->splineSpin, SIGNAL(valueChanged(int)), SLOT(slotSaveSettings()) );
  QObject::connect( ui_->frontCombo, SIGNAL(currentIndexChanged(int)), SLOT(slotSaveSettings()) );
  QObject::connect( ui_->frontCombo, SIGNAL(activated(int)), SLOT(slotSaveSettings()) );
  QObject::connect( ui_->widthSpin, SIGNAL(valueChanged(int)), SLOT(slotSaveSettings()) );
  QObject::connect( ui_->mirrorBtn, SIGNAL(clicked(bool)), SLOT(slotSaveSettings()) );
  QObject::connect( ui_->finishBtn, SIGNAL(clicked(bool)), SIGNAL(finish()) );
  QObject::connect( ui_->delBtn, SIGNAL(clicked(bool)), SIGNAL(deleteCurrent()) );
  ui_->splineSpin->installEventFilter(this);
  ui_->frontCombo->installEventFilter(this);
  ui_->widthSpin->installEventFilter(this);
  loadSettings();
}

FrontPropWidget::~FrontPropWidget()
{
  delete ui_;
  ui_ = 0;
}

meteo::Property FrontPropWidget::toProperty() const
{
  int idx = ui_->frontCombo->currentIndex();
  QString frontName = ui_->frontCombo->itemData(idx, kNameRole).toString();

  const meteo::Property& preset = presets_[frontName].prop;  prop_.set_pos(Position::kTopCenter);
  prop_.set_rotateangle( 0 );
  double scalexy = 1;
  prop_.mutable_scalexy()->CopyFrom( qpointf2pointf( QPointF( scalexy, scalexy ) ) );
  prop_.mutable_translatexy()->CopyFrom( qpoint2point( QPoint( 0, 0 ) ) );

  QPen pen = pen2qpen(preset.pen());
  pen.setWidth(ui_->widthSpin->value());
  meteo::Property p;
  p.set_ornament(preset.ornament());
  p.set_splinefactor(factor());
  p.mutable_pen()->CopyFrom( qpen2pen(pen) );
  p.set_ornament_mirror(ui_->mirrorBtn->isChecked());
  prop_.CopyFrom(p);
  return p;
}

void FrontPropWidget::setProperty(const meteo::Property& prop)
{
  ui_->splineSpin->setValue(prop.splinefactor());
  QString name = pbtools::toQString(prop.ornament());
  selectFrontByName(name);
 
  ui_->mirrorBtn->setChecked(prop.ornament_mirror());
  QPen pen = pen2qpen( prop.pen());
  ui_->widthSpin->setValue( pen.width() );
}

void FrontPropWidget::slotOnDelBtn()
{
  ui_->delBtn->setEnabled(true);
}

void FrontPropWidget::slotOffDelBtn()
{
  ui_->delBtn->setEnabled(false);
}

void FrontPropWidget::slotSaveSettings()
{
  saveSettings(toProperty());
  emit valueChanged();
}

void FrontPropWidget::initOrnaments()
{
  meteo::Property base;
  base.mutable_pen()->set_color(qRgba(0,0,0,255));
  base.mutable_pen()->set_style(kSolidLine);
  base.mutable_pen()->set_width(2);

  QMap<QString,QString> map; // для сортировки по алфавиту

  QMap<QString,proto::Ornament> ornaments = WeatherLoader::instance()->ornamentlibrary();
  foreach ( const QString& name, ornaments.keys()  )
  {
    if ( !name.endsWith("front") ) {
      continue;
    }

    QString title = pbtools::toQString(ornaments[name].title());
    map[title] = name;

    FrontInfo fi;
    fi.prop = base;
    fi.prop.set_ornament(pbtools::toString(name));
    fi.title = title;

    if ( name.contains("cold") || name.contains("inactive") ) {
      fi.prop.mutable_pen()->set_color(qRgba(0,50,220,255));
    }
    if ( name.contains("warm") ) {
      fi.prop.mutable_pen()->set_color(qRgba(200,0,0,255));
    }
    if ( name.contains("occlusion") ) {
      fi.prop.mutable_pen()->set_color(qRgba(100,50,30,255));
    }
    if ( name.contains("coldsec") ) {
      fi.prop.mutable_pen()->set_style(kDashLine);
    }

    presets_[name] = fi;
  }

  foreach ( const QString& title, map.keys() ) {
    QPixmap icon(400, 70);
    icon.fill(Qt::white);
    QPainter painter;
    GeneralProj projection;
    projection.setFuncTransform( fF2X_one, fX2F_one );
    projection.setFuncTransform( fF2X_onef, fX2F_onef );
    GeoPolygon polygon(&projection);
    GeoVector skel;
    GeoPoint point;
    point.setLatDeg(0.0);
    point.setLonDeg(35.0);
    skel.append(point);
    point.setLatDeg(50.0);
    skel.append(point);
    point.setLatDeg(400.0);
    skel.append(point);
    polygon.setSkelet(skel);
    painter.begin(&icon);
    polygon.setProperty(presets_[map[title]].prop);
    polygon.render(&painter, icon.rect(), QTransform() );
    painter.end();
    ui_->frontCombo->setIconSize(QSize(100,17));
    ui_->frontCombo->addItem(QIcon(icon)," ",map[title]);
    int index = ui_->frontCombo->count() - 1;
    ui_->frontCombo->setItemData( index, title, Qt::ToolTipRole  );
  }
}

void FrontPropWidget::selectFrontByName(const QString& name)
{
  for ( int i=0,isz=ui_->frontCombo->count(); i<isz; ++i ) {
    if ( name == ui_->frontCombo->itemData(i).toString() ) {
      ui_->frontCombo->setCurrentIndex(i);
      break;
    }
  }
}

void FrontPropWidget::updateIco(QToolButton* btn)
{
  int l = btn->palette().color(btn->backgroundRole()).lightness();
  QString icoFile = ":/meteo/icons/map/paint";
  icoFile += ( l > 110 ) ? ".png" : "-light.png";
  btn->setIcon(QIcon(icoFile));
}

int FrontPropWidget::factor() const
{
  return ui_->splineSpin->value();
}

void FrontPropWidget::loadSettings()
{
  if ( nullptr == opt_ ) {
    error_log << QObject::tr("Ошибка. Отсутвует файл настроек.");
    return;
  }
  if (true == opt_->contains("frontprop")) {
    QByteArray propArr = opt_->value("frontprop").toByteArray();
    std::string string( propArr.constData(), propArr.length() );
    meteo::Property prop;
    prop.ParseFromString(string);
    setProperty(prop);
  }
}

void FrontPropWidget::saveSettings(const meteo::Property &prop )
{
  if ( nullptr == opt_ ) {
    error_log << QObject::tr("Ошибка. Отсутвует файл настроек.");
    return;
  }
  std::string propStr;
  if (true == prop.SerializeToString(&propStr)) {
    opt_->setValue("frontprop", QByteArray(propStr.c_str(), propStr.length()));
  }
  else {
    error_log << QObject::tr("Ошибка сериализациии настроек.");
    return;
  }
}

bool FrontPropWidget::fF2X_one( const Projection& proj, const GeoPoint& geoCoord, QPoint* meterCoord )
{
  Q_UNUSED(proj);
  meterCoord->setX(geoCoord.latDeg());
  meterCoord->setY(geoCoord.lonDeg());
  return true;
}

bool FrontPropWidget::fX2F_one( const Projection& proj, const QPoint& meterCoord, GeoPoint* geoCoord )
{
  Q_UNUSED(proj);
  geoCoord->setLatDeg(meterCoord.x());
  geoCoord->setLonDeg(meterCoord.y());
  return true;
}

bool FrontPropWidget::fF2X_onef( const Projection& proj, const GeoPoint& geoCoord, QPointF* meterCoord )
{
  Q_UNUSED(proj);
  meterCoord->setX(geoCoord.latDeg());
  meterCoord->setY(geoCoord.lonDeg());
  return true;
}

bool FrontPropWidget::fX2F_onef( const Projection& proj, const QPointF& meterCoord, GeoPoint* geoCoord )
{
  Q_UNUSED(proj);
  geoCoord->setLatDeg(meterCoord.x());
  geoCoord->setLonDeg(meterCoord.y());
  return true;
}


} // map
} // meteo
