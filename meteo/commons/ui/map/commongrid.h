#ifndef METEO_COMMONS_UI_MAP_LOADER_COMMONGRID_H
#define METEO_COMMONS_UI_MAP_LOADER_COMMONGRID_H

#include <qstring.h>
#include <qmap.h>

#include <meteo/commons/proto/map_city.pb.h>

namespace meteo {
namespace map {

class Document;

bool loadCoordLayer( Document* doc );

bool loadCitiesLayer(meteo::map::Document *doc);

QString cityMarkIconByIndex(meteo::map::proto::CityMark cityMark);

meteo::map::proto::DisplaySetting fillDisplaySetting
(const proto::DisplaySetting fillDispSetting, const proto::DisplaySetting baseDispSetting);

bool isEqual(const GeoPointPb gp1, const GeoPointPb gp2);

meteo::map::proto::DisplaySetting differDisplaySetting
(const proto::DisplaySetting tocutDispSetting, const proto::DisplaySetting fullDispSetting);

Font fillFont(const Font fillFont, const Font baseFont);

Font differFont(const Font tocutFont, const Font fullFont);

Pen fillPen( const Pen fillPen, const Pen basePen);

Pen differPen( const Pen tocutPen, const Pen fullPen);

//bool isEmpty(const proto::DisplaySetting ds);
//bool isEmpty(const Font font);
//bool isEmpty(const Pen pen);

class DisplaySettingsOperator {
public:
    DisplaySettingsOperator();
    DisplaySettingsOperator(QString citySettingsProtoFileName, QString displaySettingsProtoFileName);
    //~DisplaySettingsOperator();
    void init(QString citySettingsProtoFileName, QString displaySettingsProtoFileName);
    meteo::map::proto::DisplaySetting filledCityDisplaySetting(int cityIndex);
    meteo::map::proto::DisplaySetting differedCityDisplaySetting(const meteo::map::proto::DisplaySetting cityDisplaySetting);
    meteo::map::proto::DisplaySetting filledCityModeDisplaySetting(meteo::map::proto::CityMode cityMode);
    meteo::map::proto::DisplaySetting differedCityModeDisplaySetting(const meteo::map::proto::DisplaySetting cityModeDisplaySetting);
    meteo::map::proto::DisplaySetting defaultDisplaySetting();
    //citySettingsMap_ - это отображение в классе нужно для простоты и удобства, при этом поэлементно перебирать его удобнее, обращаясь напрямую
    QMap<int, meteo::map::proto::CitySetting> *citiesMap();
    QMap <int, meteo::map::proto::DisplaySetting> *cityDisplaySettingMap();
    QMap <meteo::map::proto::CityMode, meteo::map::proto::DisplaySetting> * cityModeDisplaySettingMap();
    bool loadedOK();
    void setDefaultDisplaySetting(meteo::map::proto::DisplaySetting defaultDS);
    void setCityMapModified(bool modified);
    bool cityMapModified();
    void setDisplaySettingListModified(bool modified);
    bool displaySettingListModified();
    bool flushSettingsToFiles();
    bool flushSettingsToFiles(QString citySettingsProtoFileName, QString displaySettingsProtoFileName);

private:
    QMap <int, meteo::map::proto::CitySetting> citiesMap_;
    QMap <int, meteo::map::proto::DisplaySetting> cityDisplaySettingMap_;
    QMap <meteo::map::proto::CityMode, meteo::map::proto::DisplaySetting> cityModeDisplaySettingMap_;
    meteo::map::proto::DisplaySetting defaultDisplaySetting_;
    bool loadedOK_;
    bool cityMapModified_;// = false;
    bool displaySettingListModified_;// = false;
    QString citySettingsFileName_;
    QString displaySettingsFileName_;
};

}
}

#endif
