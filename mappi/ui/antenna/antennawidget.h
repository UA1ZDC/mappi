#pragma once

#include "navigator.h"
#include "clientstub.h"
#include <mappi/device/antenna/antenna.h>
#include <qwidget.h>
#include <qbuttongroup.h>
#include <qtimer.h>


namespace Ui {
  class AntennaWidget;
}


namespace mappi {

namespace antenna {

class AntennaWidget :
  public QWidget
{
  Q_OBJECT

public :
  explicit AntennaWidget(QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
  virtual ~AntennaWidget();

protected :
  virtual void closeEvent(QCloseEvent* event);
  virtual void showEvent(QShowEvent* event);

private :
  void setError(const QString& text);
  void setStatus(const QString& text);
  void setCurrentPos(float azimut, float elevat);

private slots :
  void serviceMode(bool checked);
  void positionActivated(bool checked);
  void monkeyTestActivated(bool checked);

  void start();
  void stop();

  void healthCheck();
  void update(conf::AntennaResponse* resp);

private :
  Ui::AntennaWidget* ui_;
  QButtonGroup* groupScript_;
  QTimer* timer_;

  Navigator* navigator_;
  ClientStub* client_;

  float currentDSA_;
  float currentDSE_;
  float currentAzimut_;
  float currentElevat_;

  antenna::Profile profile_;
};

}

}
