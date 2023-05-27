#ifndef METEO_COMMONS_DOCUMENTVIEWER_CUSTOM_WGTHOVERINGWIDGET_H
#define METEO_COMMONS_DOCUMENTVIEWER_CUSTOM_WGTHOVERINGWIDGET_H

#include <qwidget.h>
#include <qmenu.h>
#include <qaction.h>
#include <meteo/commons/ui/documentviewer/datamodel/docviewtypes.h>

namespace Ui {
class WgtHoveringWidget;
}

namespace meteo {
namespace documentviewer {


class WgtHoveringWidget : public QWidget
{
  Q_OBJECT
public:  
  explicit WgtHoveringWidget(QWidget *parent = nullptr);
  ~WgtHoveringWidget();

  int getCurrentSlide( );
  void updateSlidesCount( int current, int total );

  bool isAnimationCycled();
  int getAnimationDelay();

  bool isControlsEnabled();
  void setControlsHidden( bool hidden );
  bool isToolbarEnabled();
  void setToolbarEnabled( bool enabled );
  bool isToolbaranimationEnabled();
  void setAnimationToolbarEnabled( bool enable );

  void restoreButtonIcons( bool isLeftWidgetsHidden);  
  void setPinningEnabled( bool enabled );
  void setAnimationEnabled( bool enabled );

  void setAnimationState( bool isAnimationActive );

  enum HoverWidgetActions {
    kAnimationTimeoutChanged,
    kHideLeftWidgets,
    kAnimationPlaySceneOnly,
    kAnimationPlayMapOnly,
    kAnimationPlaySceneAndMap,
    kAnimationPlayPause,
    kAnimationStop,
    kAnimationNext,
    kAnimationPrew,
    kAnimationSave,
    kPinningSettings,
    kPinningExtendedSettings,
    kCurrentSlideChanged,
    kRotateClockwise,
    kRotateAntiClockwise,
    kAnimationRepeatChanged,
    kScaleReset,
    kScaleOptimal,
    kDocumentPrint,
    kDocumentEdit
  };

protected:
  virtual void wheelEvent(QWheelEvent *event);

public slots:
  void slotActionToolbarHide( bool isEnabled );
  void slotActionOnlyImageTriggered( bool isEnabled );
  void slotActionAnimationToolbarHide( bool isEnabled );

signals:
  void signalAction( WgtHoveringWidget::HoverWidgetActions );

private slots:  
  void slotTimeoutChanged();
  void slotHideLeftWidgets();
  void slotPbAnimationPlaySceneAndMap();
  void slotPbAnimationPlaySceneOnly();
  void slotPbAnimationPlayMapOnly();
  void slotPbAnimationPrew();
  void slotPbAnimationNext();
  void slotPbAnimationStop();
  void slotSaveAnimation();
  void slotPinningSettingsPressed();
  void slotSliderAnimationSlideValueChanged(int value);
  void slotRotateClockwise();
  void slotRotateAntiClockwise();
  void slotPbRepeatToggled();
  void slotScaleReset();
  void slotScaleOptimal();
  void slotDocumentEdit();
  void slotDocumentPrint();
  void slotToolButtonActionTriggered(QAction* action);
  void slotPbAnimationPause();
  void slotPbPinningCtrlClicked();
private:
  void restoreControlsVisibility();

private:
  Ui::WgtHoveringWidget *ui;  
  QMenu pbPlayActionsMenu;
  bool isPinningEnabled_ = false;
  bool isAnimationActive_ = false;
  QAction *actionPlayAnimationOnSceneOnly_  = nullptr;
  QAction *actionPlayAnimationOnSceneAndMap_  = nullptr;
  QAction *actionPlayAnimationOnMapOnly_  = nullptr;
  QAction *actionPlayAnimationPause_  = nullptr;
  QAction *actionPbPlayAnimationOldDefault_ = nullptr;

  int defaultAction_;
};

}
}
#endif
