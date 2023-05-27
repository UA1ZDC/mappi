#pragma once

#include <mappi/proto/reception.pb.h>
#include <qstring.h>


namespace mappi {

namespace antenna {

/*
 * Профиль антенны - перечень заводских параметров (выделенных в отдельный класс), который необходим при:
 * - построении координатного поля в GUI;
 * - реализации скриптов управления антенной.
 *
 * Информация в профиле - только для чтения. Поэтому из всего перечня профилей загружается только тот,
 * который указан в конфигурации антенны.
 * В ходе работы программы профиль не создается.
 */
class Profile
{
public :
  struct range_t {
    float min;
    float max;
  };

public :
  static void toProto(const Profile& other, conf::AntennaProfile* profile);
  static void fromProto(const conf::AntennaProfile& other, Profile* profile);

public :
  Profile();
  ~Profile();

  // для антенн с раздельным управлением приводами
  bool isValidAzimut(float v) const;
  bool isValidElevat(float v) const;

  // для антенн с совмещенным управлением приводами
  bool isValidRange(float azimut, float elevat) const;

  bool isValidDSA(float v) const;
  bool isValidDSE(float v) const;

  float dsaTempo(float v) const;
  float dsaTempo() const;

  float dseTempo(float v) const;
  float dseTempo() const;

  QString toString() const;

public :
  conf::AntennaType type;
  range_t azimut;
  range_t elevat;
  float dsa;
  float dse;
  float timeSlot;
  bool hasTravelMode;
  bool hasFeedhornControl;
};

}

}
