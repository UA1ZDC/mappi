#pragma once

#include "view.h"
#include <mappi/device/antenna/profile.h>
#include <qwidget.h>
#include <QHBoxLayout>


namespace mappi {

namespace antenna {

class Navigator :
  public QWidget
{
  Q_OBJECT
public :
  explicit Navigator(QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
  virtual ~Navigator();

  void setScope(float azimutCorrect, const Profile& profile);

signals :
  void newPosition(float az, float en);
  void keepMoving();
  void stop();

public slots :
  void setCurrentPositon(float az, float en);
  void ready();
  void waiting();

protected :
  virtual void paintEvent(QPaintEvent* event) override;

private :
  Scene* scene_;
  View* view_;
  QHBoxLayout* hLayout_;

  CoordSystem coord_;
};

}

}
