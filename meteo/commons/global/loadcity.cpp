#include "loadcity.h"

#include <fstream>

#include <qvariant.h>

#include <cross-commons/debug/tlog.h>

namespace meteo {
namespace map {


bool isEmpty(const Pen& pen)
{
  if ( !pen.has_color() &&
       !pen.has_style() &&
       !pen.has_width() ) {
    return true;
  }
  return false;
}


bool isEmpty(const Font& font)
{
  if ( !font.has_bold() &&
       !font.has_family() &&
       !font.has_italic() &&
       !font.has_pointsize() &&
       !font.has_strikeout() &&
       !font.has_underline() &&
       !font.has_weight() ) {//т.е. если никаких специфических параметров нет
    return true;
  }
  return false;
}


bool isEmpty( const meteo::map::proto::DisplaySetting& ds )
{
  if ( ds.has_font() == false &&
       ds.has_generalization_level() == false &&
       ds.has_pen() == false &&
       ds.has_position() == false &&
       ds.has_priority() == false &&
       ds.has_city_scale() == false &&
       ds.has_visible() == false &&
       ds.has_always_visible() == false &&
       ds.has_city_mark() == false ) {
    return true;
  }
  return false;
}



bool loadCitiesMap(QMap< int, meteo::map::proto::CitySetting>* citiesMap,
                              const QString& citiesFileName )
{
  std::fstream file(citiesFileName.toStdString().c_str(), std::ios::in | std::ios::binary);
  meteo::map::proto::Cities cities;
  if (!cities.ParseFromIstream(&file)) {
    error_log << QObject::tr("Ошибка загрузки списка городов из файла %1").arg(citiesFileName);
    return false;
  }

  citiesMap->clear();
  for (int i = 0; i < cities.city_setting_size(); i++) {
    int cityIndex = cities.city_setting(i).index();
    citiesMap->insert(cityIndex, cities.city_setting(i));
  }
  return true;
}

bool loadDisplaySettings( QMap<int, meteo::map::proto::DisplaySetting>* cityDisplaySettingsMap,
                          QMap<meteo::map::proto::CityMode, meteo::map::proto::DisplaySetting>* cityModeDisplaySettingsMap,
                          meteo::map::proto::DisplaySetting* defaultDisplaySettings,
                          const QString& displaySettingsFileName )
{
  //загружает список настроек отображения из proto-файла
  //QString filename(MnCommon::sharePath() + "/geo/displaysettings.pbf");
  std::fstream file(displaySettingsFileName.toStdString().c_str(), std::ios::in | std::ios::binary);
  meteo::map::proto::DisplaySettings displaySettings;
  if (!displaySettings.ParseFromIstream(&file)) {
    error_log << QObject::tr("Ошибка загрузки списка настроек отображения из файла").arg(displaySettingsFileName);
    return false;
  }
  // для удобства работы отдельные DisplaySetting'и хранятся в контейнерах-словарях для городов и групп городов
  // а DisplaySetting по умолчанию - в отдельной переменной
  // TODO пока что (не лучший вариант) здесь же производится (частичная) проверка на целостность и непротиворечивость
  // комплекта настроек отображения
  for (int i = 0; i < displaySettings.display_setting_size(); i++) {
    if (displaySettings.display_setting(i).has_is_default()) {
      if (displaySettings.display_setting(i).is_default() == false)
          debug_log << QObject::tr("Противоречие в файле настроек отображения: значение \"по умолчанию\" установлено в false");
      if (displaySettings.display_setting(i).has_city_mode() == true)
          debug_log << QObject::tr("Противоречие в файле настроек отображения: набор настроек одновременно является набором \"по умолчанию\" и набором для группы");
      if (displaySettings.display_setting(i).has_city_index() == true)
          debug_log << QObject::tr("Противоречие в файле настроек отображения: набор настроек одновременно является набором \"по умолчанию\" и набором для города");

      //TODO для проверки целостности дополнительно необходимо проверить наличие ВСЕХ полей (включая вложенные) в наборе по умолчанию

      if (displaySettings.display_setting(i).is_default() == true)
          defaultDisplaySettings->CopyFrom(displaySettings.display_setting(i));
      continue;
    }

    if (displaySettings.display_setting(i).has_city_mode()) {
      if (displaySettings.display_setting(i).has_is_default() == true)
          debug_log << QObject::tr("Противоречие в файле настроек отображения: набор настроек одновременно является набором \"по умолчанию\" и набором для группы");
      if (displaySettings.display_setting(i).has_city_index() == true)
          debug_log << QObject::tr("Противоречие в файле настроек отображения: набор настроек одновременно является набором для группы и набором для города");
      if (cityModeDisplaySettingsMap->contains(displaySettings.display_setting(i).city_mode()) == true)
          debug_log << QObject::tr("Противоречие в файле настроек отображения: больше одного набора настроек для группы %1").arg(displaySettings.display_setting(i).city_mode());
      if (meteo::map::isEmpty(displaySettings.display_setting(i)) == true)
          debug_log << QObject::tr("Нарушение целостности в файле настроек отображения: пустой набор настроек для группы %1").arg(displaySettings.display_setting(i).city_mode());

      cityModeDisplaySettingsMap->insert(displaySettings.display_setting(i).city_mode(),
                                       displaySettings.display_setting(i));
      continue;
    }

    if (displaySettings.display_setting(i).has_city_index()) {
      if (displaySettings.display_setting(i).has_is_default() == true) {
        debug_log << QObject::tr("Противоречие в файле настроек отображения: набор настроек одновременно является набором \"по умолчанию\" и набором для города");
      }
      if (displaySettings.display_setting(i).has_city_mode() == true) {
        debug_log << QObject::tr("Противоречие в файле настроек отображения: набор настроек одновременно является набором для группы и набором для города");
      }
      if (cityDisplaySettingsMap->contains(displaySettings.display_setting(i).city_index()) == true) {
        debug_log << QObject::tr("Противоречие в файле настроек отображения: больше одного набора настроек для города %1")
          .arg(QVariant(displaySettings.display_setting(i).city_index()).toString());
      }
      if (meteo::map::isEmpty(displaySettings.display_setting(i)) == true) {
        debug_log << QObject::tr("Нарушение целостности в файле настроек отображения: пустой набор настроек для города %1")
          .arg(QVariant(displaySettings.display_setting(i).city_index()).toString());
      }

      cityDisplaySettingsMap->insert(displaySettings.display_setting(i).city_index(),
                                   displaySettings.display_setting(i));
    }
  }
  return true;
}

}
}
