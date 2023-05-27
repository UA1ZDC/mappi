#include "pixmappropwidget.h"
#include "iconsetwidget.h"
#include "drawtoolbarwidget.h"
#include "ui_pixmapminwidget.h"
#include "textposdlg.h"

#include <qevent.h>
#include <qdir.h>
#include <qfiledialog.h>
#include <qpixmap.h>
#include <cross-commons/debug/tmap.h>
#include <qgraphicsproxywidget.h>
#include <qsettings.h>
#include <qcolordialog.h>

#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/paths.h>
#include <meteo/commons/ui/map/view/mapwindow.h>
#include <meteo/commons/ui/map/view/mapview.h>
#include <meteo/commons/ui/map/view/actionbutton.h>

namespace meteo {
namespace map {

PixmapPropWidget::PixmapPropWidget(QWidget* parent) :
  PropWidget(parent),
  ui_(new Ui::PixmapMinWidget)
{
  ui_->setupUi(this);
  setWindowFlags(Qt::Tool);
  setAttribute( Qt::WA_TranslucentBackground, true );
  pixmapSet_ = new QSettings(QDir::homePath() + "/." + MnCommon::applicationName() + "/pixmap.ini", QSettings::IniFormat);
  iconsetBtn_ = new ActionButton(this);
  iconsetBtn_->setToolTip(QObject::tr("Выбрать изображение"));
  colorBtn_ = new ActionButton(this);
  colorBtn_->setCheckable(false);
  colorBtn_->setToolTip(QObject::tr("Задать цвет"));

  ui_->iconsetLayout->addWidget(iconsetBtn_);
  ui_->colorLayout->addWidget(colorBtn_);

  ui_->delBtn->setEnabled(false);
  ui_->delBtn->setIcon(QIcon(":/meteo/icons/delete-16.png"));
  ui_->okBtn->setIcon(QIcon(":/meteo/icons/apply.png"));
  ui_->addPixBtn->setIcon(QIcon(":/meteo/icons/add.png"));
  colorBtn_->setIcon( QIcon(":/meteo/icons/tools/fill.xpm") );
  loadSettings();
  QObject::connect(colorBtn_, SIGNAL(clicked(bool)), SLOT(slotShowColorDlg()));
}

PixmapPropWidget::~PixmapPropWidget()
{
  saveSettings(toProperty());
  delete ui_;
  ui_ = nullptr;
  delete this->pixmapSet_;
  this->pixmapSet_ = nullptr;
}

meteo::Property PixmapPropWidget::toProperty() const
{
  return prop_;
}

void PixmapPropWidget::setProperty(const meteo::Property& prop)
{
  prop_.CopyFrom(prop);
  emit posChanged(prop_.pos());
}

void PixmapPropWidget::slotPixmapChanged( int index )
{
  Q_UNUSED(index);
  pix_ = QImage( iconsetDlg_->currentIconPath());
  QString str = iconsetDlg_->currentIconPath();
  setPixmapIconPath(str);
  pixmapSet_->setValue("pos", iconsetDlg_->currentIndex());
  emit valueChanged();
  emit pixIndexChanged(iconsetDlg_->currentIndex());
}

void PixmapPropWidget::slotSetPixmapPos( Position pos )
{
  prop_.set_pos(pos);
  emit valueChanged();
}

void PixmapPropWidget::slotShowColorDlg()
{
  QColorDialog dlg;
  dlg.setOption(QColorDialog::ShowAlphaChannel);
  if ( 0 == color_.alpha() ) {
    QColor color = color_;
    color.setAlpha(255);
    dlg.setCurrentColor(color);
  }
  else {
    dlg.setCurrentColor(color_);
  }
  if ( dlg.exec() == QDialog::Accepted ) {
    color_ = dlg.currentColor();
    emit colorChanged( color_ );
  }
}

void PixmapPropWidget::slotSwitchIconDlg()
{
  if (true == iconsetBtn_->isChecked()) {
    slotShowIconDlg();
  }
  else {
    iconsetDlgPW_->hide();
  }
}

void PixmapPropWidget::slotShowIconDlg()
{
  iconsetDlgPW_->show();
}

void PixmapPropWidget::slotHideIconDlg()
{
  iconsetBtn_->setChecked(false);
  iconsetBtn_->setIcon( iconsetDlg_->currentIcon() );
  iconsetDlgPW_->hide();
}

void PixmapPropWidget::slotAddCommonPix()
{
  QFileDialog dlg;
  dlg.setDirectory(QDir::homePath());
  dlg.setNameFilter(tr("Изображения (*.tiff *.png *.xpm *.jpg *.jpeg *.bmp)"));
  if ( dlg.exec() == QDialog::Accepted ) {
    QStringList files = dlg.selectedFiles();
    if ( true == files.isEmpty() ) {
      return;
    }
    QString str = files.first();
    pix_ = QImage( str);
    QDir dir(MnCommon::sharePath() + "/iconset");
    if ( false == dir.exists() ) {
      dir.mkdir(dir.path());
    }
    if ( true == pix_.save(MnCommon::sharePath() + "/iconset/" + str.split("/").last() ) ) {
      iconsetDlg_->loadIconsFromPaths(QStringList() << ":/meteo/icons/geopixmap" << (MnCommon::sharePath() + "/iconset") );
      iconsetDlg_->setCurrentIconIndx(iconsetDlg_->iconsSize()-1);
      pixmapSet_->setValue("pos", iconsetDlg_->currentIndex());
      iconsetBtn_->setIcon( iconsetDlg_->currentIcon() );
      slotPixmapChanged(iconsetDlg_->currentIndex());
      emit pixIndexChanged(iconsetDlg_->currentIndex());
    }
    setPixmapIconPath(str);
  }
  emit valueChanged();
}

void PixmapPropWidget::slotDefaultProp()
{
  prop_.set_pos(Position::kTopCenter);
  prop_.set_rotateangle( 0 );
  double scalexy = 1;
  prop_.mutable_scalexy()->CopyFrom( qpointf2pointf( QPointF( scalexy, scalexy ) ) );
  prop_.mutable_translatexy()->CopyFrom( qpoint2point( QPoint( 0, 0 ) ) );
  color_.setRgb(0,0,0,0);
  emit colorChanged(color_);
  emit valueChanged();
}

void PixmapPropWidget::slotSetProperty(meteo::Property prop)
{
  setProperty(prop);
}

void PixmapPropWidget::slotSetColor(QColor color)
{
  if ( true == color.isValid() ) {
    color_ = color;
    emit colorChanged(color_);
  }
}

void PixmapPropWidget::slotSetPix(QString str)
{
  if ( false == str.isEmpty() ) {
    pix_ = QImage( str);
    setPixmapIconPath(str);
    iconsetDlg_->loadIconsFromPaths(QStringList() << ":/meteo/icons/geopixmap" << (MnCommon::sharePath() + "/iconset") );
    iconsetDlg_->setCurrentIconIndx(iconsetDlg_->iconsSize()-1);
    pixmapSet_->setValue("pos", iconsetDlg_->currentIndex());
    iconsetBtn_->setIcon(iconsetDlg_->currentIcon());
    emit valueChanged();
  }
}

void PixmapPropWidget::slotSetPixIndex(int index)
{
  iconsetDlg_->setCurrentIconIndx(index);
  iconsetBtn_->setIcon( iconsetDlg_->currentIcon() );
  pix_ = QImage( iconsetDlg_->currentIconPath());
  QString str = iconsetDlg_->currentIconPath();
  setPixmapIconPath(str);
  pixmapSet_->setValue("pos", iconsetDlg_->currentIndex());
  emit valueChanged();
}

void PixmapPropWidget::slotOnDelBtn()
{
  ui_->delBtn->setEnabled(true);
}

void PixmapPropWidget::slotOffDelBtn()
{
  ui_->delBtn->setEnabled(false);
}

void PixmapPropWidget::initWidget()
{
  QGraphicsProxyWidget* parentPW = getProxyWidget();
  if ( nullptr == parentPW ) {
    return;
  }

  iconsetDlg_ = new IconsetWidget();
  iconsetDlg_->loadIconsFromPaths(QStringList() << ":/meteo/icons/geopixmap" << (MnCommon::sharePath() + "/iconset") );
  int ind = pixmapSet_->value("pos").toInt();
  iconsetDlg_->setCurrentIconIndx(ind);
  iconsetBtn_->setIcon( iconsetDlg_->currentIcon() );
  iconsetDlgPW_ = new QGraphicsProxyWidget( parentPW );
  iconsetDlgPW_->setWidget(iconsetDlg_);
  iconsetDlgPW_->setPos(30, 0);
  iconsetDlgPW_->hide();
  pix_ = QImage( iconsetDlg_->currentIconPath());

  QObject::connect( ui_->okBtn, SIGNAL(clicked(bool)), SIGNAL(finish()) );
  QObject::connect( ui_->delBtn, SIGNAL(clicked(bool)), SIGNAL(deleteCurrent()) );
  QObject::connect( iconsetBtn_, SIGNAL(clicked(bool)), SLOT(slotSwitchIconDlg()));
  QObject::connect( iconsetDlg_, SIGNAL(iconChanged(int)), SLOT(slotHideIconDlg()) );
  QObject::connect( iconsetDlg_, SIGNAL(iconChanged(int)), SLOT(slotPixmapChanged(int)) );
  QObject::connect( ui_->addPixBtn, SIGNAL(clicked(bool)), SLOT(slotAddCommonPix()));
  loadSettings();
}

QIcon PixmapPropWidget::icoForAlign( int a )
{
  QIcon ico;
  switch (a) {
    case kTopLeft:
      ico = QIcon(":/meteo/icons/text/text-pos-top-left.png");
      break;
    case kLeftCenter:
      ico = QIcon(":/meteo/icons/text/text-pos-mid-left.png");
      break;
    case kBottomLeft:
      ico = QIcon(":/meteo/icons/text/text-pos-bottom-left.png");
      break;
    case kBottomCenter:
      ico = QIcon(":/meteo/icons/text/text-pos-bottom-center.png");
      break;
    case kCenter:
      ico = QIcon(":/meteo/icons/text/text-pos-mid-center.png");
      break;
    case kTopCenter:
      ico = QIcon(":/meteo/icons/text/text-pos-top-center.png");
      break;
    case kBottomRight:
      ico = QIcon(":/meteo/icons/text/text-pos-bottom-right.png");
      break;
    case kRightCenter:
      ico = QIcon(":/meteo/icons/text/text-pos-mid-right.png");
      break;
    case kNoPosition:
      ico = QIcon(":/meteo/icons/misc/critical-message.png");
      break;
    case kTopRight:
    default:
      ico = QIcon(":/meteo/icons/text/text-pos-top-right.png");
      break;
  }
  return ico;
}

void PixmapPropWidget::loadSettings()
{
  if ( nullptr == opt_ ) {
    error_log << QObject::tr("Ошибка. Отсутвует файл настроек.");
    slotDefaultProp();
    return;
  }
  if (true == opt_->contains("pixmapprop")) {
    QByteArray propArr = opt_->value("pixmapprop").toByteArray();
    std::string string( propArr.constData(), propArr.length() );
    meteo::Property prop;
    prop.ParseFromString(string);
    setProperty(prop);
  }
  else {
    prop_.set_pos(Position::kTopCenter);
    prop_.set_rotateangle( 0 );
    double scalexy = 1;
    prop_.mutable_scalexy()->CopyFrom( qpointf2pointf( QPointF( scalexy, scalexy ) ) );
    prop_.mutable_translatexy()->CopyFrom( qpoint2point( QPoint( 0, 0 ) ) );
  }
  if ( true == opt_->contains("pixmap-color") ) {
    QRgb rgb = opt_->value("pixmap-color").toUInt();
    color_.setRgba(rgb);
  }
  else {
      color_.setRgb(0,0,0,0);
  }
  emit colorChanged(color_);
  emit valueChanged();
}

void PixmapPropWidget::saveSettings(const meteo::Property &prop )
{
  if ( nullptr == opt_ ) {
    error_log << QObject::tr("Ошибка. Отсутвует файл настроек.");
    return;
  }
  std::string propStr;
  if (true == prop.SerializeToString(&propStr)) {
    opt_->setValue("pixmapprop", QByteArray(propStr.c_str(), propStr.length()));
  }
  else {
    error_log << QObject::tr("Ошибка сериализациии настроек.");
  }
  if ( true == color_.isValid() ) {
    opt_->setValue("pixmap-color", color_.rgba());
  }
  opt_->sync();
}

}
}


