#include "rotateaction.h"
#include <meteo/commons/ui/mainwindow/widgethandler.h>
#include "rotatewidget.h"

#include <cross-commons/debug/tlog.h>
#include <commons/geobasis/geopoint.h>
#include <meteo/commons/global/global.h>
#include <meteo/commons/ui/map/view/mapwindow.h>

#include "../mapview.h"
#include "../mapscene.h"
#include "../widgetitem.h"

#include <unistd.h>
#include <qprocess.h>

namespace
{
  const QIcon cwkButtonIcon() { return QIcon(":/meteo/icons/map/rotation_ckw.png"); }
  const QIcon acwkButtonIcon() { return QIcon(":/meteo/icons/map/rotation_ackw.png"); }
  const QString cwkButtonToolTip() { return QString::fromUtf8("Поворот по часовой стрелке"); }
  const QString acwkButtonToolTip() { return QString::fromUtf8("Поворот против часовой стрелки"); }
  const int kNextBtnStep = 6;
  const int kRotateAngle = 1;
  const int kTimerStep = 20;
  const int kTimerLong = 500;
  const QString kMenuItemName = QObject::tr("Параметры поворота карты");
  const QString kMenuName = QObject::tr("Параметры");
  const double kMaxRotateAngle = 360.0;
}

namespace meteo {
namespace map {

const QString kWidgetName = "rotatewidget";

RotateAction::RotateAction( MapScene* scene )
  : Action(scene, "rotateaction" ),
    ckwBtn_( new ActionButton ),
    ackwBtn_( new ActionButton ),
    timer_(new QTimer(this)),
    menucreated_(false),
    opt_(0),
    step_(kRotateAngle)
{
  setSize( QSize( size_.width()*2, size_.height() ) );
  ckwBtn_->setToolTip(cwkButtonToolTip());
  ckwBtn_->setIcon(cwkButtonIcon());
  ckwBtn_->setCheckable(true);
  ackwBtn_->setToolTip(acwkButtonToolTip());
  ackwBtn_->setIcon(acwkButtonIcon());
  ackwBtn_->setCheckable(true);

  connect(ckwBtn_, SIGNAL(pressed()), this, SLOT(slotStartCkw()));
  connect(ackwBtn_, SIGNAL(pressed()), this, SLOT(slotStartAckw()));
  connect(ckwBtn_, SIGNAL(released()), this, SLOT(slotStopCkw()));
  connect(ackwBtn_, SIGNAL(released()), this, SLOT(slotStopAckw()));
  connect(ckwBtn_, SIGNAL(clicked()), this, SLOT(slotCkw()));
  connect(ackwBtn_, SIGNAL(clicked()), this, SLOT(slotAckw()));

  connect(timer_, SIGNAL(timeout()), this, SLOT(slotTimeout()));

  ckwItem_ = scene_->addWidget(ckwBtn_);
  item_ = ckwItem_;
  QGraphicsProxyWidget* ackwItem = new QGraphicsProxyWidget(item_);
  ackwItem->setWidget(ackwBtn_);
  ackwItem->setPos(QPoint(cellsize_+kNextBtnStep,0));

  QString fileName = this->objectName().replace("::","_");
  settings_ = new QSettings(QDir::homePath() + "/.meteo/" + fileName + ".ini", QSettings::IniFormat);

  int step = settings_->value(this->objectName()+".step").toInt();
  if (step >= kRotateAngle)
  {
    step_ = step;
  }
}

RotateAction::~RotateAction()
{
//  delete ckwBtn_; ckwBtn_ = 0;
//  delete ackwBtn_; ackwBtn_ = 0;
  delete timer_; timer_ = 0;
  delete opt_; opt_ = 0;
}

void RotateAction::slotStartCkw()
{
  timer_->start(kTimerLong);
  ckwBtn_->setChecked(true);
}

void RotateAction::slotStartAckw()
{
  timer_->start(kTimerLong);
  ackwBtn_->setChecked(true);
}

void RotateAction::slotStopAckw()
{
  timer_->stop();
  ackwBtn_->setChecked(false);
}

void RotateAction::slotStopCkw()
{
  timer_->stop();
  ckwBtn_->setChecked(false);
}

void RotateAction::slotTimeout()
{
  if ( kTimerLong == timer_->interval() ) {
    timer_->stop();
    timer_->start(kTimerStep);
  }
  else {
    if ( true == ackwBtn_->isDown() ) {
      slotAckw();
    }
    else if ( true == ckwBtn_->isDown() ) {
      slotCkw();
    }
  }
}

void RotateAction::slotCkw()
{
  rotateAngle(+step_);
}

void RotateAction::slotAckw()
{
  rotateAngle(-step_);
}

double RotateAction::curMeridian()
{
  GeoPoint pnt;
  pnt.setLonDeg(scene_->document()->projection()->getMapCenter().lonDeg() + scene_->document()->rotateAngle());
  pnt.to180();
  return pnt.lonDeg();
}

void RotateAction::rotateAngle(int angle)
{
  if ( 0 == scene_ ) {
    return;
  }

  if ( 0 == scene_->document() ) {
    return;
  }
  //debug_log << "cur rotate angle = " << scene_->document()->rotateAngle() << "step angle = " << angle;
//  if (meteo::MERCAT == scene_->document()->projection()->type()) {
//     return;
//  }
  view()->setCacheMode( QGraphicsView::CacheNone );
  double newanglefull = scene_->document()->rotateAngle() + (double) angle;
  double newangle = fmod(newanglefull,kMaxRotateAngle);

  //debug_log << newanglefull << newangle;

  scene_->document()->setRotateAngle(newangle);
  scene_->setDrawState( MapScene::kRenderLayers );
  view()->setCacheMode( QGraphicsView::CacheBackground );
  //debug_log << "new rotate angle = " << scene_->document()->rotateAngle();
}

void RotateAction::showEvent( QShowEvent* event )
{
  if (0 == scene_)
  {
    return;
  }

  if (0 == scene_->document())
  {
    return;
  }
  Action::showEvent(event);
  if ( false == menucreated_ ) {
    menucreated_ = true;
    if ( 0 != view() ) {
      meteo::map::MapWindow* mainwidget = static_cast<meteo::map::MapWindow*>(view()->parent());
      if (0 == mainwidget)
      {
        //error
        return;
      }
      //mainwidget->setWindowTitle(QObject::tr("ARM"));
      QString menuname = kMenuName;
      QMenuBar* bar = mainwidget->menuBar();
      QList<QAction*> actlist = bar->actions();
      for ( int i = 0, sz = actlist.size(); i < sz; ++i ) {
        QAction* a = actlist[i];
        if (menuname == a->text())
        {
          QMenu* menu = a->menu();
          QAction* act = new QAction(kMenuItemName,menu);
          menu->addAction(act);
          connect(act,SIGNAL(triggered()),this,SLOT(slotShowSettings()));
        }
      }
    }
    else {
      //error
    }
  }
}

void RotateAction::slotShowSettings()
{
  if (0 == scene_)
  {
    return;
  }

  if (0 == scene_->document())
  {
    return;
  }
  if ( 0 == view() ) {
    //error
    return;
  }

  if (0 != opt_)
  {
    delete opt_;
  }

  meteo::map::MapWindow* mainwidget = static_cast<meteo::map::MapWindow*>(view()->parent());
  if (0 == mainwidget) {
    return;
  }
  opt_ = new RotateWidget(mainwidget);
  opt_->setMeridian((int) round(curMeridian()));
  opt_->setStep(step_);
  opt_->init();

  connect(opt_,SIGNAL(needSaveStep(int)),this,SLOT(slotSaveSettings(int)));
  connect(opt_,SIGNAL(needSaveMeridian(int)),this,SLOT(slotSetMeridian(int)));
  opt_->show();
}

void RotateAction::slotSaveSettings(int step)
{
  step_ = step;
  if (0 != settings_)
  {
    settings_->setValue(this->objectName()+".step",step_);
  }
}

void RotateAction::slotSetMeridian(int mer)
{
  if (0 == scene_)
  {
    return;
  }

  if (0 == scene_->document())
  {
    return;
  }
  if (0 == view())
  {
    return;
  }
  double cur = curMeridian();
  double meridian = (double) mer;
  if (meridian > cur)
  {
    double delta;
    delta = meridian - cur;
    view()->setCacheMode( QGraphicsView::CacheNone );
    scene_->document()->setRotateAngle(scene_->document()->rotateAngle() + delta);
    scene_->setDrawState( MapScene::kRenderLayers );
    view()->setCacheMode( QGraphicsView::CacheBackground );
  }
  else if (meridian < cur)
  {
    double delta;
    delta = abs(cur - meridian);
    view()->setCacheMode( QGraphicsView::CacheNone );
    scene_->document()->setRotateAngle(scene_->document()->rotateAngle() - delta);
    scene_->setDrawState( MapScene::kRenderLayers );
    view()->setCacheMode( QGraphicsView::CacheBackground );
  }
}

}
}
