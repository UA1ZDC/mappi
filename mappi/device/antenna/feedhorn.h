#pragma once

#include <qstring.h>
#include <array>


namespace mappi {

namespace antenna {

/** Состояние облучателя антенны. */
class Feedhorn
{
public :
  // идентификатор доп. выхода
  enum output_t {
    OUTPUT_1 = 1,
    OUTPUT_2,
    OUTPUT_3,
    OUTPUT_4
  };

public :
  Feedhorn();
  ~Feedhorn();

  inline void turnOn(bool flag) { turnOn_ = flag; }
  inline bool isTurnOn() const { return turnOn_; }

  inline void setActiveOutput(output_t n, bool flag) { output_[n - 1] = flag; }
  inline bool isActiveOutput(output_t n) const { return output_[n - 1]; }

  QString toString() const;

public :
  bool turnOn_; // управление питанием
  std::array<bool, 4> output_{ {false, false, false, false} };
};

}

}
