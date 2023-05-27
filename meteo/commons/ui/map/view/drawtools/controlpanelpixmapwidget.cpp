#include "controlpanelpixmapwidget.h"
#include "ui_controlpanelpixmapwidget.h"
#include "textposdlg.h"
#include "iconsetwidget.h"

#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/paths.h>

#include <qgraphicsproxywidget.h>
#include <qcolordialog.h>
#include <qfiledialog.h>
#include <qsettings.h>
#include <qdir.h>

namespace meteo {
namespace map {

ControlPanelPixmapWidget::ControlPanelPixmapWidget(QWidget *parent) :
  QWidget(parent),
  ui_(new Ui::ControlPanelPixmapWidget)
{
  ui_->setupUi(this);
  setWindowFlags(Qt::Tool);
  posDlg_ = new TextPosDlg();
  posDlg_->resize(posDlg_->sizeHint());
  posDlg_->setCurrent(Position::kTopCenter);
  posDlg_->setAllowedAlign(Position::kNoPosition, false );
  pixmapSet_ = new QSettings(QDir::homePath() + "/." + MnCommon::applicationName() + "/pixmap.ini", QSettings::IniFormat);
  iconsetDlg_ = new IconsetWidget();
  iconsetDlg_->loadIconsFromPaths(QStringList() << ":/meteo/icons/geopixmap" << (MnCommon::sharePath() + "/iconset") );
  int ind = pixmapSet_->value("pos").toInt();
  iconsetDlg_->setCurrentIconIndx(ind);
  ui_->lockBtn->setChecked(true);
  slotChangeLock();
  ui_->lockBtn->setIconSize(QSize(16,16));
  ui_->pixBtn->setIcon(iconsetDlg_->currentIcon());
  ui_->pixBtn->setIconSize(QSize(32,32));
  ui_->colorBtn->setIcon(QIcon(":/meteo/icons/tools/fill.xpm"));
  ui_->colorBtn->setIconSize(QSize(32,32));
  ui_->addPixBtn->setIcon(QIcon(":/meteo/icons/add.png"));
  ui_->addPixBtn->setIconSize(QSize(32,32));
  ui_->deleteBtn->setIcon(QIcon(":/meteo/icons/delete-16.png"));
  ui_->deleteBtn->setIconSize(QSize(32,32));
  ui_->okBtn->setIcon(QIcon(":/meteo/icons/apply.png"));
  ui_->okBtn->setIconSize(QSize(32,32));
  ui_->posBtn->setIcon(QIcon(imgForAlign(posDlg_->currentPos())));
  ui_->posBtn->setIconSize(QSize(32,32));
  ui_->resetBtn->setIcon(QIcon(":/meteo/icons/drawtools/brush.png"));
  ui_->resetBtn->setIconSize(QSize(26,26));
  QObject::connect( ui_->lockBtn, SIGNAL(clicked(bool)), SLOT(slotChangeLock()) );
  QObject::connect( ui_->colorBtn, SIGNAL(clicked(bool)), SLOT(slotChangeColor()) );
  QObject::connect( ui_->addPixBtn, SIGNAL(clicked(bool)), SLOT(slotAddPix()));
  QObject::connect( ui_->deleteBtn, SIGNAL(clicked(bool)), SLOT(slotDelete()) );
  QObject::connect( ui_->okBtn, SIGNAL(clicked(bool)), SLOT(slotApply()) );
  QObject::connect( ui_->posBtn, SIGNAL(clicked(bool)), SLOT(slotSwitchPos()) );
  QObject::connect( ui_->pixBtn, SIGNAL(clicked(bool)), SLOT(slotSwitchPix()) );
  QObject::connect( ui_->resetBtn, SIGNAL(clicked(bool)), SLOT(slotReset()) );
  QObject::connect( posDlg_, SIGNAL(posChanged(int)), posDlg_, SLOT(hide()) );
  QObject::connect( posDlg_, SIGNAL(posChanged(int)), SLOT(slotChangePos()) );
  QObject::connect( iconsetDlg_, SIGNAL(iconChanged(int)), iconsetDlg_, SLOT(hide()) );
  QObject::connect( iconsetDlg_, SIGNAL(iconChanged(int)), SLOT(slotChangePix(int)) );
}

ControlPanelPixmapWidget::~ControlPanelPixmapWidget()
{
  delete ui_;
}

void ControlPanelPixmapWidget::setParentProxyWidget( QGraphicsProxyWidget* parentProxy )
{
  parentProxy_ = static_cast<GeoProxyWidget*>( parentProxy );
  if (nullptr != posProxy_  ) {
    delete posProxy_;
    posProxy_ = nullptr;
  }
  posProxy_ = new QGraphicsProxyWidget(parentProxy_);
  posProxy_->setWidget(posDlg_);
  posProxy_->setPos(0,0);
  posProxy_->hide();
  if ( nullptr != iconsetProxy_ ) {
    delete iconsetProxy_;
    iconsetProxy_ = nullptr;
  }
  iconsetProxy_ = new QGraphicsProxyWidget(parentProxy_);
  iconsetProxy_->setWidget(iconsetDlg_);
  iconsetProxy_->setPos(0,0);
  iconsetProxy_->hide();
}

void ControlPanelPixmapWidget::setColor( QColor color )
{
  color_ = color;
}

void ControlPanelPixmapWidget::slotSetPixPos( Position pos )
{
  posDlg_->setCurrent(pos);
  ui_->posBtn->setIcon(QIcon(imgForAlign(posDlg_->currentPos())));
}

void ControlPanelPixmapWidget::slotSetGeoPosProxyWidget(GeoPoint point)
{
  anchorPoit_ = point;
  if ( true == locked_ ) {
    parentProxy_->setGeoPos(anchorPoit_);
  }
}

void ControlPanelPixmapWidget::slotSetPixIndex(int index)
{
  iconsetDlg_->loadIconsFromPaths(QStringList() << ":/meteo/icons/geopixmap" << (MnCommon::sharePath() + "/iconset") );
  iconsetDlg_->setCurrentIconIndx(index);
  ui_->pixBtn->setIcon(iconsetDlg_->currentIcon());
}

void ControlPanelPixmapWidget::slotSetColor(QColor color)
{
  setColor(color);
}

void ControlPanelPixmapWidget::slotChangeLock()
{
  if ( true == ui_->lockBtn->isChecked() ) {
    ui_->lockBtn->setIcon(QIcon(":/meteo/icons/drawtools/padlock.png"));
    ui_->lockBtn->setToolTip(QObject::tr("Открепить панель"));
    locked_ = true;
    if ( nullptr != parentProxy_ ) {
      parentProxy_->setLock(locked_);
      parentProxy_->setGeoPos(anchorPoit_);
    }
  }
  else {
    ui_->lockBtn->setIcon(QIcon(":/meteo/icons/drawtools/padlock-unlocked.png"));
    ui_->lockBtn->setToolTip(QObject::tr("Закрепить панель"));
    locked_ = false;
    if ( nullptr != parentProxy_ ) {
      parentProxy_->setLock(locked_);
    }
  }
}

void ControlPanelPixmapWidget::slotChangeColor()
{
  QColorDialog dlg;
  dlg.setOption(QColorDialog::ShowAlphaChannel);
  if ( 0 == color_.alpha()) {
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

void ControlPanelPixmapWidget::slotSwitchPix()
{
  if ( nullptr == iconsetProxy_ ) {
    return;
  }
  if ( true == iconsetProxy_->isVisible() ) {
    iconsetProxy_->hide();
  }
  else {
    iconsetProxy_->show();
  }
}

void ControlPanelPixmapWidget::slotChangePix(int index)
{
  ui_->pixBtn->setIcon(iconsetDlg_->currentIcon());
  emit pixIndexChanged(index);
}

void ControlPanelPixmapWidget::slotSwitchPos()
{
  if (nullptr == posProxy_) {
    return;
  }
  if ( true == posProxy_->isVisible() ) {
    posProxy_->hide();
  }
  else {
    posProxy_->show();
  }
}

void ControlPanelPixmapWidget::slotChangePos()
{
  ui_->posBtn->setIcon(QIcon(imgForAlign(posDlg_->currentPos())));
  emit posChanged(posDlg_->currentPos());
}

void ControlPanelPixmapWidget::slotAddPix()
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
    QDir dir(MnCommon::sharePath() + "/iconset");
    if ( false == dir.exists() ) {
      dir.mkdir(dir.path());
    }
    QImage pix = QImage(str);
    if ( true == pix.save(MnCommon::sharePath() + "/iconset/" + str.split("/").last() ) ) {
      iconsetDlg_->loadIconsFromPaths(QStringList() << ":/meteo/icons/geopixmap" << (MnCommon::sharePath() + "/iconset") );
      iconsetDlg_->setCurrentIconIndx(iconsetDlg_->iconsSize()-1);
      ui_->pixBtn->setIcon( iconsetDlg_->currentIcon() );
      emit pixIndexChanged( iconsetDlg_->currentIndex() );
    }
    emit pixChanged(str);
  }
}

void ControlPanelPixmapWidget::slotReset()
{
  posDlg_->setCurrent(Position::kTopCenter);
  ui_->posBtn->setIcon(QIcon(imgForAlign(posDlg_->currentPos())));
  emit reset();
}

void ControlPanelPixmapWidget::slotApply()
{
  emit apply();
}

void ControlPanelPixmapWidget::slotDelete()
{
  emit forDelete();
}

QString ControlPanelPixmapWidget::imgForAlign(int pos)
{
  QString imgPath;
  switch (pos) {
    case kTopLeft:
      imgPath = QString(":/meteo/icons/text/text-pos-top-left.png");
      break;
    case kLeftCenter:
      imgPath = QString(":/meteo/icons/text/text-pos-mid-left.png");
      break;
    case kBottomLeft:
      imgPath = QString(":/meteo/icons/text/text-pos-bottom-left.png");
      break;
    case kBottomCenter:
      imgPath = QString(":/meteo/icons/text/text-pos-bottom-center.png");
      break;
    case kCenter:
      imgPath = QString(":/meteo/icons/text/text-pos-mid-center.png");
      break;
    case kTopCenter:
      imgPath = QString(":/meteo/icons/text/text-pos-top-center.png");
      break;
    case kBottomRight:
      imgPath = QString(":/meteo/icons/text/text-pos-bottom-right.png");
      break;
    case kRightCenter:
      imgPath = QString(":/meteo/icons/text/text-pos-mid-right.png");
      break;
    case kNoPosition:
      imgPath = QString(":/meteo/icons/misc/critical-message.png");
      break;
    case kTopRight:
    default:
      imgPath = QString(":/meteo/icons/text/text-pos-top-right.png");
      break;
  }
  return imgPath;
}

}
}
