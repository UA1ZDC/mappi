#include "tplugin.h"

TPlugin::TPlugin( const QString& include, const QString& n)
  :iscontainer_(false),
  icon_( QIcon() ),
  group_("[OPG]"),
  includefile_(include),
  name_(n),
  tooltip_( QObject::tr("Класс - ") + name_ ),
  whatsthis_( QObject::tr("Класс - ") + name_ )
{
}

QString TPlugin::name() const {
  return name_;
}

QString TPlugin::group() const {
  return group_;
}

QIcon TPlugin::icon() const {
  return icon_;
}

QString TPlugin::toolTip() const {
  return tooltip_;
}

QString TPlugin::whatsThis() const {
  return whatsthis_;
}

bool TPlugin::isContainer() const {
  return iscontainer_;
}

QString TPlugin::domXml() const {
  return QString("<widget class=\"") 
    + name_ 
    + QString("\" name=\"") 
    + name_
    + QString("\"/>");
}

QString TPlugin::includeFile() const {
  return includefile_;
}
