#include "wgthoveringwidget.h"
#include "ui_wgthoveringwidget.h"
#include <cross-commons/debug/tlog.h>
#include <meteo/commons/ui/documentviewer/custom/ctrlclickbutton.h>
#include <meteo/commons/proto/documentviewer.pb.h>
#include <meteo/commons/global/global.h>

static const QIcon iconArrowLeft(":/meteo/icons/arrow/darkarrowleft.png");
static const QIcon iconArrowRight(":/meteo/icons/arrow/darkarrowright.png");

static const QIcon iconPlayPause(":/meteo/icons/media/media-playback-pause.png");
static const QIcon iconPlayStart(":/meteo/icons/media/media-playback-start.png");
static const QIcon iconPlayStop(":/meteo/icons/media/media-playback-stop.png");
static const QIcon iconPlayRepeat(":/meteo/icons/media/media-playlist-repeat.png");
static const QIcon iconPlayBackward(":/meteo/icons/media/media-seek-backward.png");
static const QIcon iconPlayForward(":/meteo/icons/media/media-seek-forward.png");
static const QIcon iconSaveGif(":/meteo/icons/map/save.png");
static const QIcon iconAddItem (":/meteo/icons/plus.png");
static const QIcon iconRmItem (":/meteo/icons/minus.png");
static const QIcon iconPinImageToMapSettings (":/meteo/icons/tools/target.png");

static const QIcon iconRotateClockwise(":/meteo/icons/tools/faxview/rotate_clockwise.png");
static const QIcon iconRotateAntiClockwise(":/meteo/icons/tools/faxview/rotate_anticlockwise.png");
static const QIcon iconZoomOriginal(":/meteo/icons/tools/faxview/zoom_original.png");
static const QIcon iconZoomBest(":/meteo/icons/tools/faxview/zoom_best.png");
static const QIcon iconDocumentEdit(":/meteo/icons/misc/text-editor.png");
static const QIcon iconDocumentPrint(":/meteo/icons/map/print.png");

static const QString configPath = meteo::global::kDocumentViewerConfigPath + QObject::tr("hover.bin");

namespace meteo {
namespace documentviewer {

WgtHoveringWidget::WgtHoveringWidget(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::WgtHoveringWidget)
{
  ui->setupUi(this);
  this->setAttribute( Qt::WA_TranslucentBackground );

  this->ui->btnHideLeftWidgets->setIcon(iconArrowLeft);
  QObject::connect( this->ui->btnHideLeftWidgets, SIGNAL(pressed()), this, SLOT(slotHideLeftWidgets()) );
  QObject::connect( ui->sbAnimationSpeed, SIGNAL(valueChanged(int)), this, SLOT(slotTimeoutChanged()) );

  this->ui->pbNextImage->setIcon(iconPlayForward);
  this->ui->pbPrewImage->setIcon(iconPlayBackward);
  this->ui->pbStopAnimation->setIcon(iconPlayStop);
  this->ui->pbRepeat->setIcon(iconPlayRepeat);
  this->ui->pbSaveAnimation->setIcon(iconSaveGif);

  QObject::connect( this->ui->pbPlayAnimation, &QToolButton::triggered, this, &WgtHoveringWidget::slotToolButtonActionTriggered );

  this->actionPlayAnimationOnSceneAndMap_ = new QAction( iconPlayStart, QObject::tr("Синхнонно"), this);
  this->actionPlayAnimationOnSceneOnly_ = new QAction( iconPlayStart, QObject::tr("В просмотре документов"),this );
  this->actionPlayAnimationOnMapOnly_ = new QAction( iconPlayStart, QObject::tr("На карте"), this );
  this->actionPlayAnimationPause_ = new QAction( iconPlayPause, QObject::tr("Пауза"), this );
  this->pbPlayActionsMenu.addAction(this->actionPlayAnimationOnSceneAndMap_);
  this->pbPlayActionsMenu.addAction(this->actionPlayAnimationOnSceneOnly_);
  this->pbPlayActionsMenu.addAction(this->actionPlayAnimationOnMapOnly_);

  this->ui->pbPlayAnimation->setDefaultAction( this->actionPlayAnimationOnSceneOnly_ );

  QObject::connect( actionPlayAnimationOnSceneAndMap_, SIGNAL(triggered()), this, SLOT(slotPbAnimationPlaySceneAndMap()));
  QObject::connect( actionPlayAnimationOnSceneOnly_, SIGNAL(triggered()), this, SLOT(slotPbAnimationPlaySceneOnly()));
  QObject::connect( actionPlayAnimationOnMapOnly_, SIGNAL(triggered()), this, SLOT(slotPbAnimationPlayMapOnly()));
  QObject::connect( actionPlayAnimationPause_, SIGNAL(triggered()), this, SLOT(slotPbAnimationPause()) );
  QObject::connect( ui->pbPrewImage, SIGNAL(pressed()), this, SLOT(slotPbAnimationPrew()) );
  QObject::connect( ui->pbNextImage, SIGNAL(pressed()), this, SLOT(slotPbAnimationNext()) );
  QObject::connect( ui->pbStopAnimation, SIGNAL(pressed()), this, SLOT(slotPbAnimationStop()) );
  QObject::connect( ui->pbSaveAnimation, SIGNAL(pressed()), this, SLOT(slotSaveAnimation()) );

  auto pinningSettingsMenu = new QMenu(this);
  pinningSettingsMenu->setFocusPolicy(Qt::NoFocus);
  //this->ui->pbPinningSettings->setMenu(pinningSettingsMenu);
  this->ui->pbPinningSettings->setIcon( iconPinImageToMapSettings );
  this->ui->pbPinningSettings->setCheckable(false);
  QObject::connect( this->ui->pbPinningSettings, &QPushButton::pressed, this, &WgtHoveringWidget::slotPinningSettingsPressed );
  //QObject::connect( pinningSettingsMenu, SIGNAL(aboutToShow()), this, SLOT(slotPinningSettingsPressed()) );
  QObject::connect( ui->pbPinningSettings, &::meteo::commons::CtrlClickButton::signalCtrlClick, this, &WgtHoveringWidget::slotPbPinningCtrlClicked );


  QObject::connect( ui->sliderAnimationSlide, SIGNAL(valueChanged(int)), this, SLOT(slotSliderAnimationSlideValueChanged(int)) );

  QObject::connect( this->ui->pb1RotateNotClock, SIGNAL(pressed()), this, SLOT(slotRotateAntiClockwise()) );
  QObject::connect( this->ui->pb2RotateClock, SIGNAL(pressed()), this, SLOT(slotRotateClockwise()) );
  QObject::connect( this->ui->pb5ScaleReset, SIGNAL(pressed()), this, SLOT(slotScaleReset()) );
  QObject::connect( this->ui->pb6ScaleOptimal, SIGNAL(pressed()), this, SLOT(slotScaleOptimal()) );
  QObject::connect( this->ui->pb7DocumentEdit, SIGNAL(pressed()), this, SLOT(slotDocumentEdit()) );
  QObject::connect( this->ui->pb8DocumentPrint, SIGNAL(pressed()), this, SLOT(slotDocumentPrint()) );
  QObject::connect( this->ui->pbRepeat, SIGNAL(toggled(bool)), this, SLOT(slotPbRepeatToggled()) );

  this->ui->pb1RotateNotClock->setIcon(iconRotateAntiClockwise);
  this->ui->pb2RotateClock->setIcon(iconRotateClockwise);

  this->ui->pb5ScaleReset->setIcon(iconZoomOriginal);
  this->ui->pb6ScaleOptimal->setIcon(iconZoomBest);
  this->ui->pb7DocumentEdit->setIcon(iconDocumentEdit);
  this->ui->pb8DocumentPrint->setIcon(iconDocumentPrint);

  this->setContextMenuPolicy(Qt::CustomContextMenu);  

  QFile inputFile(configPath);
  if ( true == inputFile.open(QIODevice::ReadOnly) ){
    meteo::proto::HoverWidgetSettings settings;
    auto data = inputFile.readAll();
    settings.ParseFromString(data.toStdString());

    this->defaultAction_ = settings.default_play_button_state();
  }
  else {
    this->defaultAction_ = 0;
  }
}

WgtHoveringWidget::~WgtHoveringWidget()
{
  meteo::proto::HoverWidgetSettings settings;


  if ( true == this->isPinningEnabled_ ){
    auto action = this->ui->pbPlayAnimation->defaultAction();
    if ( action == this->actionPlayAnimationOnMapOnly_ ){
      settings.set_default_play_button_state(meteo::proto::kPlayMapOnly);
    }
    else if (action == this->actionPlayAnimationOnSceneAndMap_ ){
      settings.set_default_play_button_state(meteo::proto::kPlaySceneAndMap);
    } else {
      settings.set_default_play_button_state(meteo::proto::kPlaySceneOnly);
    }
  }
  else {
    settings.set_default_play_button_state(static_cast<meteo::proto::PlayButtonState>(defaultAction_));
  }
  delete ui;

  QFile settingsFile(configPath);
  if ( true == settingsFile.open(QIODevice::WriteOnly) ){
    auto data = QByteArray::fromStdString(settings.SerializeAsString());
    settingsFile.write(data);
  }
}

void WgtHoveringWidget::slotToolButtonActionTriggered(QAction* action)
{
  this->ui->pbPlayAnimation->removeAction(this->ui->pbPlayAnimation->defaultAction());
  this->ui->pbPlayAnimation->setDefaultAction(action);
}

void WgtHoveringWidget::slotPbAnimationPlaySceneOnly()
{
  emit signalAction(kAnimationPlaySceneOnly);  
}

void WgtHoveringWidget::slotPbAnimationPlayMapOnly()
{
  emit signalAction(kAnimationPlayMapOnly);  
}

void WgtHoveringWidget::slotPbAnimationPlaySceneAndMap()
{
  emit signalAction(kAnimationPlaySceneAndMap);  
}

void WgtHoveringWidget::slotPbAnimationPrew()
{
  emit signalAction(kAnimationPrew);
}

void WgtHoveringWidget::slotPbAnimationNext()
{
  emit signalAction(kAnimationNext);
}

void WgtHoveringWidget::slotHideLeftWidgets()
{
  emit signalAction(kHideLeftWidgets);
}

void WgtHoveringWidget::slotActionOnlyImageTriggered( bool isHidden )
{
  this->ui->wgtCenterContent->setEnabled( isHidden );
  this->restoreControlsVisibility();
}

bool WgtHoveringWidget::isToolbaranimationEnabled()
{
  return this->ui->wgtToolbarAnimation->isEnabled();
}

void WgtHoveringWidget::slotActionToolbarHide( bool isHidden )
{
  this->ui->wgtToolBarCentered->setEnabled(isHidden)  ;
  this->restoreControlsVisibility();
}

void WgtHoveringWidget::slotActionAnimationToolbarHide( bool isEnabled )
{
  this->ui->wgtToolbarAnimation->setEnabled(isEnabled);
  this->ui->wgtAnimationSlideMovement->setEnabled(isEnabled);
  this->restoreControlsVisibility();
}


void WgtHoveringWidget::restoreControlsVisibility()
{
  this->ui->wgtToolBarCentered->setHidden( this->ui->wgtToolBarCentered->isEnabled() == false );
  this->ui->wgtAnimationSlideMovement->setHidden( this->ui->wgtAnimationSlideMovement->isEnabled() == false );
  this->ui->pbPinningSettings->setHidden( false );
  this->ui->wgtToolbarAnimation->setHidden( this->ui->wgtToolbarAnimation->isEnabled() == false );
}


void WgtHoveringWidget::updateSlidesCount( int current, int total )
{
  static const QString textTemplate = QObject::tr("%1/%2");
  this->ui->sliderAnimationSlide->blockSignals(true);
  if ( ( total > current ) && ( current >= 0 ) ) {
    this->ui->sliderAnimationSlide->setMinimum(0);
    this->ui->sliderAnimationSlide->setMaximum( total - 1);
    this->ui->sliderAnimationSlide->setValue(current);
    this->ui->wgtAnimationSlideMovement->setHidden(false);
    this->ui->lbCurrentSlide->setText( textTemplate.arg(current + 1).arg(total) );
  }
  else {
    this->ui->lbCurrentSlide->setText(QString());
    this->ui->wgtAnimationSlideMovement->setHidden( this->ui->wgtAnimationSlideMovement->isEnabled() == false );
  }
  this->ui->sliderAnimationSlide->blockSignals(false);
}

void WgtHoveringWidget::slotTimeoutChanged()
{
  emit signalAction(kAnimationTimeoutChanged);
}

int WgtHoveringWidget::getAnimationDelay()
{
  return 1000 * 100 / ui->sbAnimationSpeed->value() ;
}

bool WgtHoveringWidget::isAnimationCycled()
{
  return this->ui->pbRepeat->isChecked();
}

void WgtHoveringWidget::restoreButtonIcons(bool isLeftWidgetsHidden)
{
  this->ui->btnHideLeftWidgets->setIcon( (true == isLeftWidgetsHidden)? iconArrowRight: iconArrowLeft);
}

bool WgtHoveringWidget::isControlsEnabled()
{
  return this->ui->wgtCenterContent->isEnabled();
}

void WgtHoveringWidget::setControlsHidden( bool hidden )
{
  this->ui->wgtCenterContent->setHidden(hidden);
}

bool WgtHoveringWidget::isToolbarEnabled()
{
  return this->ui->wgtToolBarCentered->isEnabled();
}

void WgtHoveringWidget::setToolbarEnabled( bool enabled )
{
  this->ui->wgtToolBarCentered->setEnabled(enabled);
  this->restoreControlsVisibility();
}

void WgtHoveringWidget::setAnimationToolbarEnabled( bool enable )
{
  this->ui->wgtToolbarAnimation->setEnabled(enable);
  this->restoreControlsVisibility();
}

void WgtHoveringWidget::slotPbAnimationStop()
{
  emit signalAction(kAnimationStop);  
}

void WgtHoveringWidget::slotSaveAnimation()
{
  emit signalAction(kAnimationSave);
}

void WgtHoveringWidget::slotPinningSettingsPressed()
{  
  emit signalAction(kPinningSettings);
}

void WgtHoveringWidget::slotSliderAnimationSlideValueChanged( int )
{
  emit signalAction(kCurrentSlideChanged);
}

int WgtHoveringWidget::getCurrentSlide()
{
  return this->ui->sliderAnimationSlide->value();
}

void WgtHoveringWidget::slotRotateClockwise()
{
  emit signalAction(kRotateClockwise);
}

void WgtHoveringWidget::slotRotateAntiClockwise()
{
  emit signalAction(kRotateAntiClockwise);
}

void WgtHoveringWidget::slotPbRepeatToggled()
{
  emit signalAction(kAnimationRepeatChanged);
}

void WgtHoveringWidget::slotScaleReset()
{
  emit signalAction(kScaleReset);
}

void WgtHoveringWidget::slotScaleOptimal()
{
  emit signalAction(kScaleOptimal);
}
void WgtHoveringWidget::slotDocumentEdit()
{
  emit signalAction(kDocumentEdit);
}

void WgtHoveringWidget::slotDocumentPrint()
{
  emit signalAction(kDocumentPrint);
}

void WgtHoveringWidget::setPinningEnabled( bool enabled )
{
  this->isPinningEnabled_ = enabled;
  this->setAnimationState( this->isAnimationActive_ );
  this->restoreControlsVisibility();
}

void WgtHoveringWidget::setAnimationEnabled( bool enabled )
{
  this->ui->wgtToolbarAnimation->setEnabled(enabled);
  this->ui->wgtAnimationSlideMovement->setEnabled(enabled);
  this->restoreControlsVisibility();
}

void WgtHoveringWidget::setAnimationState( bool isActive )
{
  this->isAnimationActive_ = isActive;
  if ( true == isActive ) {
    //кнопка паузы
    if ( this->actionPlayAnimationPause_ != this->ui->pbPlayAnimation->defaultAction() ){
      this->ui->pbPlayAnimation->setMenu(nullptr);
      this->ui->pbPlayAnimation->setPopupMode(QToolButton::ToolButtonPopupMode::InstantPopup);
      this->actionPbPlayAnimationOldDefault_ = this->ui->pbPlayAnimation->defaultAction();
      this->ui->pbPlayAnimation->removeAction(this->ui->pbPlayAnimation->defaultAction());
      this->ui->pbPlayAnimation->setDefaultAction(this->actionPlayAnimationPause_);
    }
  }
  else {
    if ( false == this->isPinningEnabled_ ) {
      //Кнопка анимации на сцене
      this->ui->pbPlayAnimation->setPopupMode(QToolButton::ToolButtonPopupMode::InstantPopup);
      this->ui->pbPlayAnimation->setMenu(nullptr);
      this->ui->pbPlayAnimation->removeAction(this->ui->pbPlayAnimation->defaultAction());
      this->ui->pbPlayAnimation->setDefaultAction(this->actionPlayAnimationOnSceneOnly_);
      this->actionPbPlayAnimationOldDefault_ = nullptr;
    }
    else {
      //Кнопка анимации с выбором вариантов
      this->ui->pbPlayAnimation->setPopupMode(QToolButton::ToolButtonPopupMode::MenuButtonPopup);
      this->ui->pbPlayAnimation->setMenu(&this->pbPlayActionsMenu);
      this->ui->pbPlayAnimation->removeAction(this->ui->pbPlayAnimation->defaultAction());
      if ( nullptr != this->actionPbPlayAnimationOldDefault_ ) {
        this->ui->pbPlayAnimation->setDefaultAction(this->actionPbPlayAnimationOldDefault_);
      }
      else {
        if ( this->defaultAction_ == meteo::proto::kPlayMapOnly ){
          this->ui->pbPlayAnimation->setDefaultAction(this->actionPlayAnimationOnMapOnly_);
        }
        else if ( this->defaultAction_ == meteo::proto::kPlaySceneAndMap ){
          this->ui->pbPlayAnimation->setDefaultAction(this->actionPlayAnimationOnSceneAndMap_);
        }
        else {
          this->ui->pbPlayAnimation->setDefaultAction(this->actionPlayAnimationOnSceneOnly_);
        }
      }
      this->actionPbPlayAnimationOldDefault_ = nullptr;
    }
  }
}


void WgtHoveringWidget::slotPbAnimationPause()
{
  emit signalAction(kAnimationPlayPause);  
}

void WgtHoveringWidget::slotPbPinningCtrlClicked()
{
  emit signalAction(kPinningExtendedSettings);
}

void WgtHoveringWidget::wheelEvent(QWheelEvent *event)
{
  QWidget::wheelEvent(event);
}

}
}
