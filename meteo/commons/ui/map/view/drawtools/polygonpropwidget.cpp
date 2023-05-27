#include "polygonpropwidget.h"
#include "ui_polygonpropwidget.h"

#include <qcolordialog.h>
#include <qpainter.h>
#include <qstandarditemmodel.h>
#include <qsettings.h>

#include <cross-commons/debug/tmap.h>
#include <cross-commons/debug/tlog.h>

#include <commons/textproto/pbtools.h>
#include <meteo/commons/global/weatherloader.h>
#include <meteo/commons/ui/map/geopolygon.h>
#include <commons/geobasis/generalproj.h>

namespace meteo {
namespace map {

const QString kNoOrnament = QString("kNoOrnament");
const QString kSolidLine = QObject::tr("Сплошная");
const QString kDashLine  = QObject::tr("Штриховой пунктир");
const QString kDotLine = QObject::tr("Точечный пунктир");
const QString kDashDotLine = QObject::tr("Штрихпунктир");
const QString kDashDotDotLine = QObject::tr("Штрихпунктир с двумя точками");
const QString kDblSolidLine = QObject::tr("Двойная сплошная");
const QString kDblDashLine  = QObject::tr("Двойная пунктирная");

const QStringList kLineTypes = QStringList()
  << kSolidLine
  << kDashLine
  << kDotLine
  << kDashDotLine
  << kDashDotDotLine
//  << kDblSolidLine
//  << kDblDashLine
;

// расположение стрелок на линии
const QString kNoArr    = QObject::tr("Без стрелок");
const QString kBeginArr = QObject::tr("В начале");
const QString kEndArr   = QObject::tr("В конце");
const QString kBothArr  = QObject::tr("На концах");

const TMap<ArrowPlace,QString> kArrPlace = TMap<ArrowPlace,QString>()
  << qMakePair(kNoArrow, kNoArr)
  << qMakePair(kBeginArrow, kBeginArr)
  << qMakePair(kEndArrow, kEndArr)
  << qMakePair(kBothArrow, kBothArr)
;


PolygonPropWidget::PolygonPropWidget(Preset preset, QWidget* parent)
  : PropWidget(parent)
{
  preset_ = preset;
  ui_ = new Ui::PolygonPropWidget;
  ui_->setupUi(this);
  setWindowFlags(Qt::Tool);
  ui_->delBtn->setEnabled(false);

  setLineColor(QColor(0,0,0));
  setFillColor(QColor(0,0,0));
  slotInitArrowPlaces();
  fillLineCombo();
  loadOrnaments();
  ui_->widthSpin->setValue(1);
  ui_->lineCombo->setCurrentIndex(kLineTypes.indexOf(kSolidLine));
  ui_->labelButton->setChecked(false);
  ui_->labelButton->setIcon(QIcon(":/meteo/icons/drawtools/tag_off.png"));
  ui_->lockBtn->setIcon(QIcon(":/meteo/icons/drawtools/padlock-unlocked.png"));
  ui_->lockBtn->setIconSize(QSize(12,12));
  ui_->arrTypeBtn->setProperty("arr-type", kSimpleArrow);
  ui_->arrTypeBtn->setIcon(QIcon(ArrowDlg::icoForType(kSimpleArrow)));
  ui_->finishBtn->setIcon(QIcon(":/meteo/icons/apply.png"));
  ui_->arrTypeBtn->hide();
  ui_->arrTypeBtn->setVisible(false);
  arrowDlg_ = new ArrowDlg();
  arrowDlg_->move(mapToGlobal(ui_->arrTypeBtn->pos()));
  arrowDlg_->resize(QSize(28,28));
  loadSettings();
  QObject::connect( ui_->colorBtn, SIGNAL(clicked(bool)), SLOT(slotSelectColor()) );
  QObject::connect( ui_->fillBtn, SIGNAL(clicked(bool)), SLOT(slotSelectColor()) );
  QObject::connect( ui_->widthSpin, SIGNAL(valueChanged(int)), SLOT(slotSaveSettings()) );
  QObject::connect( ui_->splineSpin, SIGNAL(valueChanged(int)), SLOT(slotSaveSettings()) );
  QObject::connect( ui_->lineCombo, SIGNAL(currentIndexChanged(int)), SLOT(slotSaveSettings()) );
  QObject::connect( ui_->patternCombo, SIGNAL(currentIndexChanged(int)), SLOT(slotSaveSettings()) );
  QObject::connect( ui_->arrPlaceCombo, SIGNAL(currentIndexChanged(int)), SLOT(slotSaveSettings()) );
  QObject::connect( ui_->labelButton, SIGNAL(toggled(bool)), SLOT(slotLabelSwitch(bool)) );
  QObject::connect( ui_->valueSpin, SIGNAL(valueChanged(double)), SLOT(slotSaveSettings()) );
  QObject::connect( ui_->unitEdit, SIGNAL(textChanged(QString)), SLOT(slotSaveSettings()) );
  QObject::connect( ui_->splineSpin, SIGNAL(valueChanged(int)), SLOT(slotSaveSettings()) );
  QObject::connect( ui_->ornamentCombo, SIGNAL(currentIndexChanged(int)), SLOT(slotSaveSettings()) );
  QObject::connect( ui_->ornamentMirrorBtn, SIGNAL(toggled(bool)), SLOT(slotSaveSettings()) );
  QObject::connect( ui_->arrTypeBtn, SIGNAL(clicked(bool)), SLOT(slotShowArrowDlg()) );
  QObject::connect( ui_->finishBtn, SIGNAL(clicked(bool)), SIGNAL(finish()) );
  QObject::connect( ui_->delBtn, SIGNAL(clicked(bool)), SIGNAL(deleteCurrent()) );
  QObject::connect( arrowDlg_, SIGNAL(arrowTypeChanged()), SLOT(slotArrowTypeChanged()) );
  QObject::connect( ui_->lockBtn, SIGNAL(clicked(bool)), SLOT(slotChangeLock()) );
  ui_->widthSpin->installEventFilter(this);
  ui_->splineSpin->installEventFilter(this);
  ui_->lineCombo->installEventFilter(this);
  ui_->patternCombo->installEventFilter(this);
  ui_->arrPlaceCombo->installEventFilter(this);
  ui_->valueSpin->installEventFilter(this);
  ui_->ornamentCombo->installEventFilter(this);

  switch ( preset ) {
    case kLine: {
    ui_->fillBtn->hide();
    ui_->patternCombo->hide();
    ui_->lockBtn->hide();
    updateGeometry();
    } break;
    case kPolygon: {
    ui_->fillBtn->show();
    ui_->patternCombo->show();
    ui_->lockBtn->show();
    slotUpdateFillBtnPalette(true);
    updateGeometry();
    } break;
  }
  slotLabelSwitch(ui_->labelButton->isChecked());
  ui_->patternCombo->installEventFilter(this);
}

PolygonPropWidget::~PolygonPropWidget()
{
  delete ui_;
  ui_ = nullptr;
}

bool PolygonPropWidget::hasValue() const
{
  return ui_->labelButton->isChecked();
}

void PolygonPropWidget::setValue(double value, bool enable)
{
  ui_->labelButton->setChecked(enable);
  ui_->valueSpin->setValue(value);
}

double PolygonPropWidget::value() const
{
  return ui_->valueSpin->value();
}

QString PolygonPropWidget::unit() const
{
  return ui_->unitEdit->text();
}

meteo::Property PolygonPropWidget::toProperty() const
{
  meteo::Property p;
  QPen pen;
  pen.setWidth(lineWidth());
  pen.setStyle(lineStyle());
  pen.setColor(lineColor());
  p.mutable_pen()->CopyFrom( qpen2pen(pen) );
  QBrush brush;
  brush.setColor(fillColor());
  brush.setStyle(fillStyle());
  p.mutable_brush()->CopyFrom( qbrush2brush(brush) );
  p.set_closed(closedFlag());
  p.set_double_line(isDouble());
  p.set_splinefactor(factor());
  p.set_arrow_place(arrowPlace());
  p.set_arrow_type(arrowType());
  int idx = ui_->ornamentCombo->currentIndex();
  QString name = ui_->ornamentCombo->itemData(idx, kNameRole).toString();
  if ( name != kNoOrnament ) {
    p.set_ornament(pbtools::toString(name));
  }
  p.set_ornament_mirror(ui_->ornamentMirrorBtn->isChecked());

  return p;
}

void PolygonPropWidget::setProperty(const meteo::Property& prop)
{
  ui_->widthSpin->setValue(prop.pen().width());
  ui_->fillBtn->setVisible(prop.closed());
  ui_->patternCombo->setVisible(prop.closed());
  ui_->splineSpin->setValue(prop.splinefactor());

  QPen pen = pen2qpen(prop.pen());
  QBrush brush = brush2qbrush(prop.brush());
  int idx = kLineTypes.indexOf(kSolidLine);
  if ( pen.style() == Qt::SolidLine ) {
    idx = ( true == prop.double_line() ) ? kLineTypes.indexOf(kDblSolidLine) : kLineTypes.indexOf(kSolidLine);
  }
  else if ( pen.style() == Qt::DashLine ) {
    idx = ( true == prop.double_line() ) ? kLineTypes.indexOf(kDblDashLine) : kLineTypes.indexOf(kDashLine);
  }
  else if ( pen.style() == Qt::DotLine ) {
    idx = kLineTypes.indexOf(kDotLine);
  }
  else if ( pen.style() == Qt::DashDotLine ) {
    idx = kLineTypes.indexOf(kDashDotLine);
  }
  else if ( pen.style() == Qt::DashDotDotLine ) {
    idx = kLineTypes.indexOf(kDashDotDotLine);
  }
  ui_->lineCombo->setCurrentIndex(idx);
  setLineColor(pen.color());
  setFillColor(brush.color());
  setPatternStyle(brush.style());

  for ( int i=0,isz=ui_->arrPlaceCombo->count(); i<isz; ++i ) {
    if ( ArrowPlace(ui_->arrPlaceCombo->itemData(i).toInt()) == prop.arrow_place() ) {
      ui_->arrPlaceCombo->setCurrentIndex(i);
      break;
    }
  }

  ui_->arrTypeBtn->setIcon(QIcon(ArrowDlg::icoForType(prop.arrow_type())));

  if ( prop.has_ornament() ) {
    QString name = pbtools::toQString(prop.ornament());
    for ( int i=0,isz=ui_->ornamentCombo->count(); i<isz; ++i ) {
      if ( ui_->ornamentCombo->itemData(i, kNameRole).toString() == name ) {
        ui_->ornamentCombo->setCurrentIndex(i);
      }
    }
  }
  ui_->ornamentMirrorBtn->setChecked(prop.ornament_mirror());
}

void PolygonPropWidget::initWidget()
{
  QGraphicsProxyWidget* parentP = getProxyWidget();
  if ( nullptr != parentP ) {
    arrowProxy_ = new QGraphicsProxyWidget(parentP);
    arrowProxy_->setWidget(arrowDlg_);
    arrowProxy_->setPos(ui_->arrTypeBtn->pos()+QPoint(0,32));
    arrowProxy_->hide();
  }
}

bool PolygonPropWidget::eventFilter(QObject* obj, QEvent* e)
{
  if ( obj == ui_->patternCombo ) {
    if ( QEvent::Resize == e->type() ) {
      Qt::BrushStyle s = fillStyle();
      slotInitPatterns();
      setPatternStyle(s);
    }
  }
  if ( QEvent::Wheel == e->type() ) {
    if ( blockWheel() ) {
      return true;
    }
  }
  return false;
}

void PolygonPropWidget::slotOnDelBtn()
{
  ui_->delBtn->setEnabled(true);
}

void PolygonPropWidget::slotOffDelBtn()
{
  ui_->delBtn->setEnabled(false);
}

void PolygonPropWidget::slotSaveSettings()
{
  saveSettings(toProperty());
  emit valueChanged();
}

void PolygonPropWidget::slotSelectColor()
{
  QPushButton* btn = qobject_cast<QPushButton*>(sender());
  if ( nullptr == btn ) {
    return;
  }

  QColorDialog dlg;
  dlg.setOption(QColorDialog::ShowAlphaChannel);

  if ( ui_->fillBtn == btn ) {
    dlg.setCurrentColor(fillColor());
  }
  else if ( ui_->colorBtn == btn ) {
    dlg.setCurrentColor(lineColor());
  }

  if ( dlg.exec() == QDialog::Accepted ) {
    if ( ui_->fillBtn == btn ) {
      setFillColor(dlg.currentColor());
      if ( true == locked_ ) {
        setLineColor(dlg.currentColor());
      }
    }
    else if ( ui_->colorBtn == btn ) {
      setLineColor(dlg.currentColor());
      if ( true == locked_ ) {
        setFillColor(dlg.currentColor());
      }
    }
  }
  saveSettings(toProperty());
}

void PolygonPropWidget::slotInitPatterns()
{
  ui_->patternCombo->clear();
  QBrush brush;
  brush.setColor(fillColor());
  QPixmap pix(QSize(ui_->patternCombo->size().width() - 20, 24));
  pix.fill(Qt::white);
  ui_->patternCombo->addItem(QIcon(pix), " ", int(Qt::NoBrush));
  for (int i=Qt::SolidPattern; i<Qt::DiagCrossPattern; ++i ) {
    ui_->patternCombo->setIconSize(pix.size());
    pix.fill(Qt::white);
    QPainter painter(&pix);
    brush.setStyle(Qt::BrushStyle(i));
    painter.fillRect(QRect(QPoint(0,0),pix.size()), brush);
    ui_->patternCombo->addItem(QIcon(pix),"",int(i));
  }
}

void PolygonPropWidget::slotInitArrowPlaces()
{
  ui_->arrPlaceCombo->clear();
  ui_->arrPlaceCombo->addItem(kNoArr, kNoArrow);
  ui_->arrPlaceCombo->addItem(kBeginArr, kBeginArrow);
  ui_->arrPlaceCombo->addItem(kEndArr, kEndArrow);
  ui_->arrPlaceCombo->addItem(kBothArr, kBothArrow);
}

void PolygonPropWidget::slotUpdateFillBtnPalette(bool enable)
{
  QPushButton* btn = qobject_cast<QPushButton*>(sender());
  if ( nullptr == btn ) {
    return;
  }

  if ( enable ) {
    setFillColor(btn->property("prop-color").value<QColor>());
  }
  else {
    btn->setPalette(QApplication::palette());
  }
}

void PolygonPropWidget::slotShowArrowDlg()
{
  if ( nullptr != arrowProxy_ ) {
    arrowProxy_->setPos(ui_->arrTypeBtn->pos()+QPoint(0,32));
    arrowProxy_->show();
  }
}

void PolygonPropWidget::slotArrowTypeChanged()
{
    ui_->arrTypeBtn->setProperty("arr-type", arrowDlg_->currentArrowType());
    ui_->arrTypeBtn->setIcon(QIcon(ArrowDlg::icoForType(arrowDlg_->currentArrowType())));
}

void PolygonPropWidget::slotLabelSwitch(bool enable)
{
  ui_->valueSpin->setEnabled(enable);
  ui_->unitEdit->setEnabled(enable);
  if ( true == enable ) {
    ui_->labelButton->setIcon(QIcon(":/meteo/icons/drawtools/tag_on.png"));
    ui_->labelButton->setToolTip(QObject::tr("Убрать значение"));
  }
  else {
    ui_->labelButton->setIcon(QIcon(":/meteo/icons/drawtools/tag_off.png"));
    ui_->labelButton->setToolTip(QObject::tr("Установить значение"));
  }
  emit valueChanged();
}

void PolygonPropWidget::slotChangeLock()
{
  if ( true == ui_->lockBtn->isChecked() ) {
    ui_->lockBtn->setIcon(QIcon(":/meteo/icons/drawtools/padlock.png"));
    ui_->lockBtn->setToolTip(QObject::tr("Раздельное изменение цвета линии и заливки"));
    locked_ = true;
  }
  else {
    ui_->lockBtn->setIcon(QIcon(":/meteo/icons/drawtools/padlock-unlocked.png"));
    ui_->lockBtn->setToolTip(QObject::tr("Одновременное изменение цвета линии и заливки"));
    locked_ = false;
  }
}

int PolygonPropWidget::lineWidth() const
{
  return ui_->widthSpin->value();
}

bool PolygonPropWidget::closedFlag() const
{
  return ui_->patternCombo->isVisible();
}

Qt::PenStyle PolygonPropWidget::lineStyle() const
{
  int curIdx = ui_->lineCombo->currentIndex();
  if ( kLineTypes.indexOf(kSolidLine) == curIdx ) {
    return Qt::SolidLine;
  }
  else if ( kLineTypes.indexOf(kDashLine) == curIdx ) {
    return Qt::DashLine;
  }
  else if ( kLineTypes.indexOf(kDblSolidLine) == curIdx ) {
    return Qt::SolidLine;
  }
  else if ( kLineTypes.indexOf(kDblDashLine) == curIdx ) {
    return Qt::DashLine;
  }
  else if ( kLineTypes.indexOf(kDotLine) == curIdx ) {
    return Qt::DotLine;
  }
  else if ( kLineTypes.indexOf(kDashDotLine) == curIdx ) {
    return Qt::DashDotLine;
  }
  else if ( kLineTypes.indexOf(kDashDotDotLine) == curIdx ) {
    return Qt::DashDotDotLine;
  }

  error_log << tr("Не удалось определить тип линии");
  return Qt::NoPen;
}

bool PolygonPropWidget::isDouble() const
{
  int curIdx = ui_->lineCombo->currentIndex();
  if ( kLineTypes.indexOf(kSolidLine) == curIdx
       || kLineTypes.indexOf(kDashLine) == curIdx
       || kLineTypes.indexOf(kDotLine) == curIdx
       || kLineTypes.indexOf(kDashDotLine) == curIdx
       || kLineTypes.indexOf(kDashDotDotLine) == curIdx
     )
  {
    return false;
  }
  if ( kLineTypes.indexOf(kDblSolidLine) == curIdx
       || kLineTypes.indexOf(kDblDashLine) == curIdx
      )
  {
    return true;
  }

  error_log << tr("Не удалось определить тип линии");
  return false;
}

int PolygonPropWidget::factor() const
{
  return ui_->splineSpin->value();
}

QColor PolygonPropWidget::lineColor() const
{
  return ui_->colorBtn->palette().color(ui_->colorBtn->backgroundRole());
}

QColor PolygonPropWidget::fillColor() const
{
  return ui_->fillBtn->palette().color(ui_->fillBtn->backgroundRole());
}

Qt::BrushStyle PolygonPropWidget::fillStyle() const
{
  int i = ui_->patternCombo->currentIndex();
  return Qt::BrushStyle(ui_->patternCombo->itemData(i).toInt());
}

ArrowPlace PolygonPropWidget::arrowPlace() const
{

  int i = ui_->arrPlaceCombo->currentIndex();
  return ArrowPlace(ui_->arrPlaceCombo->itemData(i).toInt());
}

ArrowType PolygonPropWidget::arrowType() const
{
  return kSimpleArrow;
}

void PolygonPropWidget::setLineColor(const QColor& color)
{
  setBackgroundColor(ui_->colorBtn, color);
  updateIco(ui_->colorBtn);

  emit valueChanged();
}

void PolygonPropWidget::setFillColor(const QColor& color)
{
  setBackgroundColor(ui_->fillBtn, color);
  ui_->fillBtn->setProperty("prop-color", color);
  updateIco(ui_->fillBtn);
  Qt::BrushStyle s = fillStyle();
  slotInitPatterns();
  setPatternStyle(s);

  emit valueChanged();
}

void PolygonPropWidget::setPatternStyle(Qt::BrushStyle style)
{
  for ( int i=0,isz=ui_->patternCombo->count(); i<isz; ++i ) {
    if ( Qt::BrushStyle(ui_->patternCombo->itemData(i).toInt()) == style ) {
      ui_->patternCombo->setCurrentIndex(i);
      break;
    }
  }
}

void PolygonPropWidget::updateIco(QPushButton* btn)
{
  int l = btn->palette().color(btn->backgroundRole()).lightness();
  QString icoFile;
  if ( ui_->colorBtn == btn ) {
    icoFile = ":/meteo/icons/map/paint";
  }
  else if ( ui_->fillBtn == btn ) {
    icoFile = ":/meteo/icons/drawtools/fill-color";
  }
  icoFile += ( l > 110 ) ? ".png" : "-light.png";
  btn->setIcon(QIcon(icoFile));
}

void PolygonPropWidget::loadOrnaments()
{
  ui_->ornamentCombo->clear();

  QMap<QString,QString> map; // для сортировки по алфавиту

  QMap<QString,proto::Ornament> ornaments = WeatherLoader::instance()->ornamentlibrary();
  foreach ( const QString& name, ornaments.keys() ) {
    map[pbtools::toQString(ornaments[name].title())] = name;
  }
  map.insert("Без орнамента", kNoOrnament);
  foreach ( const QString& title, map.keys() ) {
    QPixmap icon(400, 70);
    icon.fill(Qt::white);
    if ( "Без орнамента" != title ) {
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
      painter.setPen(Qt::red);
      meteo::Property prop;
      prop.set_closed(false);
      prop.set_ornament(map[title].toStdString());
      QPen pen;
      pen.setWidth(3);
      pen.setStyle(Qt::PenStyle::SolidLine);
      pen.setColor(Qt::black);
      prop.mutable_pen()->CopyFrom( qpen2pen(pen) );
      QBrush brush;
      prop.mutable_brush()->CopyFrom( qbrush2brush(brush) );
      polygon.setProperty(prop);
      polygon.render(&painter, icon.rect(), QTransform() );
      painter.end();
    }
    ui_->ornamentCombo->setIconSize(QSize(100,17));
    ui_->ornamentCombo->addItem(QIcon(icon)," ",map[title]);
    int index = ui_->ornamentCombo->count() - 1;
    ui_->ornamentCombo->setItemData( index, title, Qt::ToolTipRole  );
  }
}

void PolygonPropWidget::fillLineCombo()
{
  Qt::PenStyle penStyle;
  foreach (QString type, kLineTypes) {
    penStyle = Qt::NoPen;
    if ( kSolidLine == type ) {
      penStyle = Qt::SolidLine;
    }
    else if ( kDashLine == type ) {
      penStyle = Qt::DashLine;
    }
    else if ( kDotLine== type ) {
      penStyle = Qt::DotLine;
    }
    else if ( kDashDotLine == type ) {
      penStyle = Qt::DashDotLine;
    }
    else if ( kDashDotDotLine == type ) {
      penStyle = Qt::DashDotDotLine;
    }
    else {
      error_log << QObject::tr("Неопределённый тип линии");
      ui_->lineCombo->addItem(type);
    }
    if ( Qt::NoPen == penStyle ) {
      continue;
    }
    QPixmap icon(40, 10);
    icon.fill(Qt::white);
    QPainter painter;
    painter.begin(&icon);
    QPen pen(Qt::black);
    pen.setStyle(penStyle);
    pen.setWidth(2);
    painter.setPen(pen);
    painter.drawLine(0,5,40,5);
    painter.end();
    ui_->lineCombo->setIconSize(QSize(40,10));
    ui_->lineCombo->addItem(QIcon(icon)," ");
    int index = ui_->lineCombo->count() - 1;
    ui_->lineCombo->setItemData( index, type, Qt::ToolTipRole  );
  }
}

void PolygonPropWidget::loadSettings()
{
  if ( nullptr == opt_ ) {
    error_log << QObject::tr("Ошибка. Отсутвует файл настроек.");
    return;
  }
  if (true == opt_->contains("polygonprop" + QString::number( preset_ ) )) {
    QByteArray propArr = opt_->value("polygonprop" + QString::number( preset_ ) ).toByteArray();
    std::string string( propArr.constData(), propArr.length() );
    meteo::Property prop;
    prop.ParseFromString(string);
    setProperty(prop);
    saveSettings(prop);
  }
}

void PolygonPropWidget::saveSettings(const meteo::Property &prop )
{
  if ( nullptr == opt_ ) {
    error_log << QObject::tr("Ошибка. Отсутвует файл настроек.");
    return;
  }
  std::string propStr;
  if (true == prop.SerializeToString(&propStr)) {
    opt_->setValue("polygonprop" + QString::number( preset_ ) , QByteArray(propStr.c_str(), propStr.length()));
  }
  else {
    error_log << QObject::tr("Ошибка сериализациии настроек.");
    return;
  }
}

bool PolygonPropWidget::fF2X_one( const Projection& proj, const GeoPoint& geoCoord, QPoint* meterCoord )
{
  Q_UNUSED(proj);
  meterCoord->setX(geoCoord.latDeg());
  meterCoord->setY(geoCoord.lonDeg());
  return true;
}

bool PolygonPropWidget::fX2F_one( const Projection& proj, const QPoint& meterCoord, GeoPoint* geoCoord )
{
  Q_UNUSED(proj);
  geoCoord->setLatDeg(meterCoord.x());
  geoCoord->setLonDeg(meterCoord.y());
  return true;
}

bool PolygonPropWidget::fF2X_onef( const Projection& proj, const GeoPoint& geoCoord, QPointF* meterCoord )
{
  Q_UNUSED(proj);
  meterCoord->setX(geoCoord.latDeg());
  meterCoord->setY(geoCoord.lonDeg());
  return true;
}

bool PolygonPropWidget::fX2F_onef( const Projection& proj, const QPointF& meterCoord, GeoPoint* geoCoord )
{
  Q_UNUSED(proj);
  geoCoord->setLatDeg(meterCoord.x());
  geoCoord->setLonDeg(meterCoord.y());
  return true;
}

}
}
