#ifndef MAPPI_SETTINGS_MAPPISETTINGS_H
#define MAPPI_SETTINGS_MAPPISETTINGS_H

#include <cross-commons/singleton/tsingleton.h>
#include <meteo/commons/settings/settings.h>

#include <commons/funcs/tcolorgrad.h>

#include <mappi/proto/reception.pb.h>
#include <mappi/proto/satellite.pb.h>
#include <mappi/proto/thematic.pb.h>


namespace mappi {
  class MappiSettings : public meteo::Settings
  {
  public:
    explicit MappiSettings();
    virtual ~MappiSettings();

    //! Конфигурации приемника
    const ::mappi::conf::Reception reception() const { return reception_; }
    //! Конфигурации спутников
    const ::mappi::conf::Pretreatment satellites() const { return satellites_; }
    //! Конфигурации инструментов
    const ::mappi::conf::InstrCollect instruments() const { return instruments_; }
    //! Предустановленные тематические обработки
    //const ::mappi::conf::ThematicConf thematics() const { return thematics_; }
    const ::mappi::conf::ThematicProcs thematicsCalc() const { return thematics_calc_; }
    const ::mappi::conf::ThematicCoef coef() const { return them_coef_; }
    const ::mappi::conf::ThematicThresholds thres() const { return them_thresholds_; }

    //! Возвращает палитру для тематической обработки с именем thematicName.
    TColorGradList palette(const std::string &themname) const;
    void setPalette(const std::string &themname, const TColorGradList& grad);

    bool satellite(const QString& satelliteName, ::mappi::conf::PretrSat* satellite);
    bool radiometer(const QString& satelliteName, ::mappi::conf::Instrument* instrument);
    bool instrumentByName(const std::string& instrName, ::mappi::conf::Instrument* instrument);
    bool instrument(::mappi::conf::InstrumentType type, ::mappi::conf::Instrument* instrument);

    bool saveReception(::mappi::conf::Reception* reception);
    bool saveThematics(::mappi::conf::ThematicProcs* thematics);
    bool saveThresholds(::mappi::conf::ThematicThresholds* thematics);
    bool saveCoef(::mappi::conf::ThematicCoef* thematics);
  private:
    
    bool saveFile(const QString& fileName, google::protobuf::Message* message);
    
  private:
    ::mappi::conf::Reception      reception_;    //!< Конфигурация приёмника
    ::mappi::conf::Pretreatment   satellites_;   //!< Параметры спутников
    ::mappi::conf::InstrCollect   instruments_;  //!< Параметры инструментов
    //::mappi::conf::ThematicConf thematics_;    //!< Предустановленные тематические обработки
    ::mappi::conf::ThematicProcs thematics_calc_;    //!< Предустановленные тематические обработки (калькулятор)
    
    ::mappi::conf::ThematicPalettes palettes_; //палитра

    ::mappi::conf::ThematicCoef them_coef_; //!<коэффициенты для методов тематической обработки
    ::mappi::conf::ThematicThresholds them_thresholds_ ; //!<пороговые значения для методов тематической обработки

    friend class TSingleton<MappiSettings>;
  };

  namespace inter {
    using Settings = TSingleton<mappi::MappiSettings>;
  }
}

#endif
