#include "weatheritem.h"

namespace meteo {
namespace map {
namespace weather {

WeatherItem::WeatherItem( int type )
  : QTreeWidgetItem(type)
{
}

WeatherItem::WeatherItem( const QStringList& strings, int type )
  : QTreeWidgetItem( strings, type)
{
}

WeatherItem::WeatherItem( QTreeWidget* parent, int type )
  : QTreeWidgetItem( parent, type )
{
}

WeatherItem::WeatherItem( QTreeWidget* parent, const QStringList& strings, int type )
  : QTreeWidgetItem( parent, strings, type)
{
}

WeatherItem::WeatherItem( QTreeWidget* parent, QTreeWidgetItem* preceding, int type )
  : QTreeWidgetItem( parent, preceding, type)
{
}

WeatherItem::WeatherItem( QTreeWidgetItem* parent, int type )
  : QTreeWidgetItem( parent, type )
{
}

WeatherItem::WeatherItem( QTreeWidgetItem* parent, const QStringList& strings, int type )
  : QTreeWidgetItem( parent, strings, type)
{
}
WeatherItem::WeatherItem( QTreeWidgetItem* parent, QTreeWidgetItem* preceding, int type )
  : QTreeWidgetItem( parent, preceding,  type )
{
}
WeatherItem::WeatherItem( const QTreeWidgetItem& other )
  : QTreeWidgetItem(other)
{
}

WeatherItem::~WeatherItem()
{
}

bool WeatherItem::operator<( const QTreeWidgetItem& item ) const
{
  int sortclmn = treeWidget()->sortColumn();
  if (   kHourClmn    == sortclmn
      || kModelClmn    == sortclmn
      || kLevelClmn    == sortclmn
      || kLvlTypeClmn  == sortclmn ) {
    return QTreeWidgetItem::text(sortclmn).toInt() < item.text(sortclmn).toInt();
  }
  return QTreeWidgetItem::operator<(item);
}

}
}
}
