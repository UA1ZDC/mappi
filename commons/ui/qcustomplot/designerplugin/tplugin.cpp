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
    + QString("\">")

    + QString( "  <property name=\"geometry\">\n")
    + QString("   <rect>\n")
    + QString("    <x>0</x>\n")
    + QString("    <y>0</y>\n")
    + QString("    <width>200</width>\n")
    + QString("    <height>200</height>\n")
    + QString("   </rect>\n")
    + QString("  </property>\n")

    + QString("</widget>");
}

QString TPlugin::includeFile() const {
  return includefile_;
}


