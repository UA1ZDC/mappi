#include "appcontainer_linux.h"

#include <qpainter.h>
#include <qtimer.h>
#include <qevent.h>

#include "preloader.h"

AppContainer::AppContainer(QWidget* parent) :
  QX11EmbedContainer(parent),
  timer_(new QTimer),
  preloader_(new meteo::app::Preloader)
{
  connect(timer_, SIGNAL(timeout()), SLOT(repaint()));
  connect(this, SIGNAL(clientIsEmbedded()), timer_, SLOT(stop()));
  connect(this, SIGNAL(clientIsEmbedded()), SLOT(showMaximized()));
  preloader_->setTextLog(QObject::tr("Загрузка приложения..."));
  timer_->start(100);
}

AppContainer::~AppContainer()
{
  delete timer_;
  delete preloader_;
}

void AppContainer::paintEvent(QPaintEvent* e)
{
  QPainter img_painter(this);
  preloader_->render(&img_painter, QPoint(), QRegion() );
  QX11EmbedContainer::paintEvent(e);
}

void AppContainer::resizeEvent(QResizeEvent* e)
{
  QX11EmbedContainer::resizeEvent(e);
  preloader_->setGeometry(geometry());
}
