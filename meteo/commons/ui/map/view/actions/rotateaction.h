#ifndef ROTATEACTION_H
#define ROTATEACTION_H

#include "action.h"
#include <qwidget.h>
#include <QSettings>
#include "../actionbutton.h"

namespace meteo {
namespace map {

class MapView;
class WidgetItem;
class RotateWidget;

class RotateAction : public Action
{
  Q_OBJECT
public:
  RotateAction( MapScene* scene );
  ~RotateAction();

  virtual void showEvent( QShowEvent* event );

public  slots:
  void slotSetMeridian(int mer);

private slots:
  void slotCkw();
  void slotAckw();
  void slotStartCkw();
  void slotStartAckw();
  void slotStopCkw();
  void slotStopAckw();
  void slotTimeout();
  void slotShowSettings();
  void slotSaveSettings(int step);


private:
  void rotateAngle(int angle);  
  double curMeridian();

private:
  ActionButton* ckwBtn_;
  QGraphicsProxyWidget* ckwItem_;
  ActionButton* ackwBtn_;
  QTimer* timer_;
  bool menucreated_;
  RotateWidget* opt_;
  QSettings* settings_;
  int step_;
};

}
}
#endif // ROTATEACTION_H
