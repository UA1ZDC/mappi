#ifndef METEO_COMMONS_UI_PLUGINS_WEATHER_WEATHERITEM_H
#define METEO_COMMONS_UI_PLUGINS_WEATHER_WEATHERITEM_H

#include <qtreewidget.h>

namespace meteo {
namespace map {
namespace weather {

class WeatherItem : public QTreeWidgetItem
{
  public:
    enum ColumnNames {
      kTermClmn     =       0,
      kCenterClmn   =       1,
      kHourClmn     =       2,
      kModelClmn    =       3,
      kLevelClmn    =       4,
      kLvlTypeClmn  =       5
    };
    WeatherItem( int type = Type );
    WeatherItem( const QStringList& strings, int type = Type );
    WeatherItem( QTreeWidget* parent, int type = Type );
    WeatherItem( QTreeWidget* parent, const QStringList& strings, int type = Type );
    WeatherItem( QTreeWidget* parent, QTreeWidgetItem* preceding, int type = Type );
    WeatherItem( QTreeWidgetItem* parent, int type = Type );
    WeatherItem( QTreeWidgetItem* parent, const QStringList& strings, int type = Type );
    WeatherItem( QTreeWidgetItem* parent, QTreeWidgetItem* preceding, int type = Type );
    WeatherItem( const QTreeWidgetItem& other );
    ~WeatherItem();

    bool operator<( const QTreeWidgetItem& item ) const ;

};

}
}
}

#endif
