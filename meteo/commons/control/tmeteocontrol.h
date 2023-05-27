#ifndef METEO_COMMONS_CONTROL_TMETEOCONTROL_H
#define METEO_COMMONS_CONTROL_TMETEOCONTROL_H

#include <cross-commons/singleton/tsingleton.h>
#include <commons/meteo_data/meteo_data.h>

#include <qmap.h>

class TMeteoData;
class TControlMethod;
class TControlMethodBase;

namespace control {

  extern const float gamma;
  extern const float Rd;
  extern const float g;
  extern const float R;
  extern const float Cp; //!< удельная теплоёмкость при пост. давлении. сухой воздух = 1.005; 100% влажность = 1.0301 
}

namespace tmcontrol {

class TMeteoControl {
public:
  TMeteoControl();
  virtual ~TMeteoControl();

  //TODO проверка заголовка сводки - номера блоков, местоположение


  bool control(ushort ctrlMask, TMeteoData* data);
  bool control(ushort ctrlMask, TMeteoData* cur, TMeteoData* prev);

  bool registerMethod(control::ControlType type, TControlMethodBase* method);

private:

  QMap<control::ControlType, TControlMethod*> _methods; //!< все зарегистрированные методы по типу контроля

};

}

typedef TSingleton<tmcontrol::TMeteoControl> TMeteoControl;

#endif
