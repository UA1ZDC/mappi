#include "controlpaneltextwidget.h"
#include "ui_controlpaneltextwidget.h"
#include "textposdlg.h"
#include "symbdlg.h"

#include <qcolordialog.h>

#include <meteo/commons/global/common.cpp>
#include <meteo/commons/ui/map/view/mapscene.h>
#include <meteo/commons/ui/map/view/actions/action.h>



namespace meteo {
namespace map {

ControlPanelTextWidget::ControlPanelTextWidget(QWidget *parent) :
  QWidget(parent),
  ui_(new Ui::ControlPanelTextWidget)
{
  ui_->setupUi(this);
  setWindowFlags(Qt::Tool);
  posDlg_ = new TextPosDlg();
  posDlg_->resize(posDlg_->sizeHint());
  posDlg_->setCurrent(Position::kTopCenter);
  posDlg_->setAllowedAlign(Position::kNoPosition, false );
  symbDlg_ = new SymbDlg();
  QFont font = font2qfont( prop_.font()) ;
  font.setBold(false);
  font.setItalic(false);
  font.setUnderline(false);
  font.setStrikeOut(false);
  symbDlg_->slotSetFont( font );
  ui_->posBtn->setIconSize(QSize(32,32));
  ui_->okBtn->setIcon(QIcon(":/meteo/icons/apply.png"));
  ui_->okBtn->setIconSize(QSize(32,32));
  ui_->delBtn->setIcon(QIcon(":/meteo/icons/delete-16.png"));
  ui_->delBtn->setIconSize(QSize(32,32));
  setDrawAlwaysIcon();
  ui_->drawBtn->setIconSize(QSize(32,32));
  ui_->lockBtn->setChecked(true);
  slotLockChanged();
  ui_->lockBtn->setIconSize(QSize(16, 16));
  QObject::connect( ui_->underlineBtn, SIGNAL(clicked(bool)), SLOT(slotFontChanged()) );
  QObject::connect( ui_->strikeBtn, SIGNAL(clicked(bool)), SLOT(slotFontChanged()) );
  QObject::connect( ui_->italicBtn, SIGNAL(clicked(bool)), SLOT(slotFontChanged()) );
  QObject::connect( ui_->boldBtn, SIGNAL(clicked(bool)), SLOT(slotFontChanged()) );
  QObject::connect( ui_->symbolBtn, SIGNAL(clicked(bool)), SLOT(slotSwitchSymb()) );
  QObject::connect( ui_->lockBtn, SIGNAL(clicked(bool)), SLOT(slotLockChanged()) );
  QObject::connect( ui_->drawBtn, SIGNAL(clicked(bool)), SLOT(slotDrawAlways()) );
  QObject::connect( ui_->textColorBtn, SIGNAL(clicked(bool)), SLOT(slotTextColorDialog()) );
  QObject::connect( ui_->fonColorBtn, SIGNAL(clicked(bool)), SLOT(slotFonColorDialog()) );
  QObject::connect( ui_->okBtn, SIGNAL(clicked(bool)), SLOT(slotOnAction()));
  QObject::connect( ui_->okBtn, SIGNAL(clicked(bool)), SIGNAL(apply()));
  QObject::connect( ui_->delBtn, SIGNAL(clicked(bool)), SLOT(slotOnAction()) );
  QObject::connect( ui_->delBtn, SIGNAL(clicked(bool)), SIGNAL(forDelete()) );
  QObject::connect( ui_->posBtn, SIGNAL(clicked(bool)), SLOT(slotSwitchPos()));
  QObject::connect( ui_->fontComboBox, SIGNAL(currentFontChanged(QFont)), SLOT(slotFontChanged()));
  QObject::connect( ui_->fontSizeBox, SIGNAL(valueChanged(int)), SLOT(slotFontChanged()));
  QObject::connect( ui_->priorBox, SIGNAL(valueChanged(int)), SLOT(slotPriorChanged()));
  QObject::connect( ui_->ramkaBox, SIGNAL(currentIndexChanged(int)), SLOT(slotRamkaChanged()));
  QObject::connect( posDlg_, SIGNAL(posChanged(int)), posDlg_, SLOT(hide()) );
  QObject::connect( posDlg_, SIGNAL(posChanged(int)), SLOT(slotTextPosChanged()) );
  QObject::connect( symbDlg_, SIGNAL(symbChanged()), SLOT(slotSymbolInsert()));
}

ControlPanelTextWidget::~ControlPanelTextWidget()
{
  delete ui_;
}

void ControlPanelTextWidget::setParentProxyWidget(QGraphicsProxyWidget* parentProxy)
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
  QObject::connect( parentProxy_, SIGNAL(enter()), SLOT(slotOffAction()) );
  QObject::connect( parentProxy_, SIGNAL(leave()), SLOT(slotOnAction()) );
  if ( nullptr != symbProxy_ ) {
    delete symbProxy_;
    symbProxy_ = nullptr;
  }
  symbProxy_ = new QGraphicsProxyWidget(parentProxy_);
  symbProxy_->setWidget(symbDlg_);
  symbProxy_->setPos(0,0);
  symbProxy_->hide();

}

void ControlPanelTextWidget::setProperty(const meteo::Property& prop)
{
  prop_.CopyFrom(prop);
  QPalette p;
  p = ui_->textColorBtn->palette();
  p.setColor( ui_->textColorBtn->backgroundRole(), pen2qpen(prop_.pen()).color() );
  ui_->textColorBtn->setPalette(p);
  p = ui_->fonColorBtn->palette();
  p.setColor( ui_->fonColorBtn->backgroundRole(), brush2qbrush(prop_.brush()).color() );
  ui_->fonColorBtn->setPalette(p);
  posDlg_->setCurrent(prop_.pos());
  ui_->posBtn->setIcon(QIcon(imgForAlign(posDlg_->currentPos())));
  QFont font = font2qfont(prop_.font());
  ui_->priorBox->setValue(prop_.priority());
  ui_->fontSizeBox->setValue(font.pointSize());
  ui_->fontComboBox->setCurrentFont(font);
  ui_->ramkaBox->setCurrentIndex(prop_.ramka());
  ui_->italicBtn->setChecked( font.italic() );
  ui_->boldBtn->setChecked( font.bold() );
  ui_->strikeBtn->setChecked( font.strikeOut() );
  ui_->underlineBtn->setChecked( font.underline() );
  ui_->drawBtn->setChecked( prop_.draw_always() );
  setDrawAlwaysIcon();
}

meteo::Property ControlPanelTextWidget::toProperty() const
{
  return prop_;
}

void ControlPanelTextWidget::slotSetProperty(meteo::Property prop)
{
  setProperty(prop);
}

void ControlPanelTextWidget::slotSetParentProxyPos(GeoPoint point)
{
  anchorPoint_ = point;
  if ( true == locked_) {
    parentProxy_->setGeoPos(anchorPoint_);
  }
}

void ControlPanelTextWidget::slotFontChanged()
{
  QFont font = font2qfont(prop_.font());
  font = ui_->fontComboBox->currentFont();
  font.setPointSize(ui_->fontSizeBox->value());
  font.setBold(ui_->boldBtn->isChecked());
  font.setItalic(ui_->italicBtn->isChecked());
  font.setUnderline(ui_->underlineBtn->isChecked());
  font.setStrikeOut(ui_->strikeBtn->isChecked());
  prop_.mutable_font()->CopyFrom( qfont2font(font) );
  emit propertyChange( prop_ );
}

void ControlPanelTextWidget::slotLockChanged()
{
  if ( true == ui_->lockBtn->isChecked() ) {
    ui_->lockBtn->setIcon(QIcon(":/meteo/icons/drawtools/padlock.png"));
    ui_->lockBtn->setToolTip(QObject::tr("Открепить панель"));
    locked_ = true;
    if ( nullptr != parentProxy_ ) {
      parentProxy_->setLock(locked_);
      parentProxy_->setGeoPos(anchorPoint_);
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

void ControlPanelTextWidget::slotSymbolInsert()
{
  if ( nullptr != symbDlg_ ) {
    emit insertSymb(symbDlg_->currentSymb());
  }
}

void ControlPanelTextWidget::slotRamkaChanged()
{
  prop_.set_ramka(static_cast<TextRamka>(ui_->ramkaBox->currentIndex()) );
  emit propertyChange( prop_ );
}

void ControlPanelTextWidget::slotPriorChanged()
{
  prop_.set_priority(ui_->priorBox->value());
  emit propertyChange( prop_ );
}

void ControlPanelTextWidget::slotDrawAlways()
{
  setDrawAlwaysIcon();
  prop_.set_draw_always(ui_->drawBtn->isChecked());
  emit propertyChange( prop_ );
}

void ControlPanelTextWidget::slotTextColorDialog()
{
  QColorDialog dlg;
  if ( dlg.exec() == QDialog::Accepted ) {
    QPalette p;
    p = ui_->textColorBtn->palette();
    p.setColor( ui_->textColorBtn->backgroundRole(), dlg.currentColor() );
    ui_->textColorBtn->setPalette(p);
    QPen pen = pen2qpen(prop_.pen());
    pen.setColor(ui_->textColorBtn->palette().color(ui_->textColorBtn->backgroundRole()));
    prop_.mutable_pen()->CopyFrom(qpen2pen(pen));
    emit propertyChange( prop_ );
  }
}

void ControlPanelTextWidget::slotFonColorDialog()
{
  QColorDialog dlg;
  if ( dlg.exec() == QDialog::Accepted ) {
    QPalette p;
    p = ui_->fonColorBtn->palette();
    p.setColor( ui_->fonColorBtn->backgroundRole(), dlg.currentColor() );
    ui_->fonColorBtn->setPalette(p);
    QBrush brush = brush2qbrush(prop_.brush());
    brush.setColor(ui_->fonColorBtn->palette().color(ui_->fonColorBtn->backgroundRole()));
    prop_.mutable_brush()->CopyFrom(qbrush2brush(brush));
    emit propertyChange( prop_ );

  }
}

void ControlPanelTextWidget::slotTextPosChanged()
{
  ui_->posBtn->setIcon(QIcon(imgForAlign(posDlg_->currentPos())));
  prop_.set_pos(posDlg_->currentPos());
  emit propertyChange( prop_ );
}

void ControlPanelTextWidget::slotSwitchPos()
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

void ControlPanelTextWidget::slotSwitchSymb()
{
  if (nullptr == symbProxy_) {
    return;
  }
  if ( true == symbProxy_->isVisible() ) {
    symbProxy_->hide();
  }
  else {
    QFont font = font2qfont( prop_.font()) ;
    font.setBold(false);
    font.setItalic(false);
    font.setUnderline(false);
    font.setStrikeOut(false);
    symbDlg_->slotSetFont( font );
    symbProxy_->show();
  }
}

void ControlPanelTextWidget::slotOnAction()
{
  MapScene* scene = qobject_cast<MapScene*>(parentProxy_->scene());
  Action* a = scene->getAction("scaleaction");
  if ( nullptr == a ) {
    return;
  }
  a->setDisabled(false);
}

void ControlPanelTextWidget::slotOffAction()
{
  MapScene* scene = qobject_cast<MapScene*>(parentProxy_->scene());
  Action* a = scene->getAction("scaleaction");
  if ( nullptr == a ) {
    return;
  }
  a->setDisabled(true);
}

void ControlPanelTextWidget::defaultProperty()
{
  QFont font = font2qfont(prop_.font());
  font = ui_->fontComboBox->currentFont();
  font.setPointSize(14);
  prop_.mutable_font()->CopyFrom( qfont2font(font) );
//  slotAlignChanged();
  QPen pen = pen2qpen(prop_.pen());
  pen.setColor(ui_->textColorBtn->palette().color(ui_->textColorBtn->backgroundRole()));
  prop_.mutable_pen()->CopyFrom(qpen2pen(pen));
  QBrush brush = brush2qbrush(prop_.brush());
  brush.setColor(ui_->fonColorBtn->palette().color(ui_->fonColorBtn->backgroundRole()));
  prop_.mutable_brush()->CopyFrom(qbrush2brush(brush));
  posDlg_->setCurrent(Position::kTopRight);
  prop_.set_pos(posDlg_->currentPos());
  ui_->posBtn->setIcon(QIcon(imgForAlign(posDlg_->currentPos())));
}

QString ControlPanelTextWidget::imgForAlign(int pos)
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

void ControlPanelTextWidget::setDrawAlwaysIcon()
{
  if ( true == ui_->drawBtn->isChecked() ) {
    ui_->drawBtn->setIcon(QIcon(":/meteo/icons/tools/layer_visible.png"));
    ui_->drawBtn->setToolTip(QObject::tr("Текст всегда отображается"));
  }
  else {
    ui_->drawBtn->setIcon(QIcon(":/meteo/icons/tools/set_layer_hidden.png"));
    ui_->drawBtn->setToolTip(QObject::tr("Текст скрывается"));
  }
}

}
}
