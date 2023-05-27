#include "script.h"


namespace mappi {

namespace antenna {

/**
 * "Ловец" - сценарий определяет(ловит) текущее положение антенны после открытия порта.
 * Определение текущего положения явдляется сложной процедурой для антенны niitv:
 * - необходима отправка команды и ожидание;
 * - в ответ может прийти мусор, т.е. может понадобиться несколько попыток.
 */
class Catcher :
  public Script
{
  Q_OBJECT
public :
  static const int TRY_COUNT = 10;

public :
  explicit Catcher(QObject* parent = nullptr);
  virtual ~Catcher();

  virtual Script::id_t id() const { return Script::CATCHER; }
  virtual bool exec(Antenna* antenna);

private :
  int try_;
};

}

}