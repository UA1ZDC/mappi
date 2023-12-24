#ifndef MAPPI_THEMATIC_THEMATICCALC_TEMATICCALC_H
#define MAPPI_THEMATIC_THEMATICCALC_TEMATICCALC_H

#include <cmath>
#include <functional>
#include <qmap.h>
#include <qvector.h>

#include <qstring.h>
#include <qtextstream.h>
#include <qdatastream.h>

#include <mappi/thematic/algs_calc/channel.h>
#include "exprtk.hpp"

namespace mappi {

namespace thematic {
class ThematicCalc
{
public:
  ThematicCalc(){}
  ~ThematicCalc() {}

  void bindArray(const QString& name, to::Channel& vec);
  bool parsingExpression(const QString& data_);
  bool prepareASMFunction();
  QVector<uchar> dataProcessing();
  void processVector();

  QVector<float> getResult() { return result_; }

private:
  QVector<uchar> fitTo8Bit();
  QVector<uchar> stretchTo8Bit();
  template<typename T, typename U>
  T normalize(U value);
  template<typename T, typename U>
  T normalize(U value, U old_min, U old_max);
private:
  QString stringExpression_;
  QMap<QString, to::Channel> map_;
  int size_ = 0;
  float min_value_ = std::numeric_limits<float>::max();
  float max_value_ = std::numeric_limits<float>::min();
  QVector<float> result_;
};

} //thematic
} //mappi

#endif // MAPPI_THEMATIC_THEMATICCALC_TEMATICCALC_H
