#include "appcontainer_windows.h"

#include "preloader.h"

AppContainer::AppContainer(QWidget* parent)
  : QWidget(parent)
{
  preloader_ = new meteo::app::Preloader;
  preloader_->setTextLog(QObject::tr("Встраивание для ОС Windows не реализовано!"));
}

AppContainer::~AppContainer()
{
  delete preloader_;
}
