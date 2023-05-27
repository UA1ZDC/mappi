#ifndef MAPPI_DEVICE_ANTENNA_SMALLANT_H
#define MAPPI_DEVICE_ANTENNA_SMALLANT_H

#include "suzhetant.h"


namespace mappi {

namespace receive {

// статус
enum AntStatusMask {
  kAzTempHi = 0x02, //!< высокая температура драйвера, азимут (8-канальный МОП мост выключен, управление шаговым двигателем прервано)
  kAzTempCritic = 0x04, //!< критическая температура драйвера, азимут (сброс питания драйвера, управление прервано)
  kAzNoAnsw = 0x08, //!< драйвер не отвечает на комманды, азимут
  kAzFail = 0x10, //!< неисправен датчик азимута
  kElFail = 0x20, //!< неисправен датчик угла места
  kInit = 0x100, //!< выполняется инициализация (калибровка)
  kAntSensor = 0x200, //!< сработал датчик излучателя антены (антена свёрнута)
  kAzMin = 0x400, //!< сработал датчик азимута 1 (двигатель остановлен) (ушла за минимальный технический угол)
  kAzMax = 0x800, //!< сработал датчик азимута 2 (двигатель остановлен) (ушла за максимальный технический угол)
  kElMin = 0x1000, //!< cработал датчик угла места 1 (двигатель остановлен) (ушла за минимальный технический угол)
  kElmax = 0x2000, //!< cработал датчик угла места 2 (двигатель остановлен) (ушла за максимальный технический угол)
  kNotInit = 0x4000, //!< привод не инициализирован(произошёл сброс питания)
  kElTempHi = 0x100000, //!< высокая температура драйвера, азимут (8-канальный МОП мост выключен, управление шаговым двигателем прервано)
  kElTempCritic = 0x200000, //!< критическая температура драйвера, азимут (сброс питания драйвера, управление прервано)
  kElNoAnsw = 0x400000, //!< драйвер не отвечает на комманды, азимут
};


class SmallAnt :
  public SuzhetAnt
{
public :
  explicit SmallAnt(QObject* parent = nullptr);
  virtual ~SmallAnt();

  virtual bool isInit() override;
  virtual bool setPower(bool v) override;

  bool status(uint32_t* status);
  bool regenerate();
  bool setTravelPosition();
  bool setWorkPosition();
};

}

}

#endif // MAPPI_DEVICE_ANTENNA_SMALLANT_H
