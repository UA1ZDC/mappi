#ifndef MAPPI_UI_PLUGINS_UHDCONTROL_AXISSPECTR_H
#define MAPPI_UI_PLUGINS_UHDCONTROL_AXISSPECTR_H

#include <cmath>

#include <qmap.h>
#include <qpair.h>
#include <qbitarray.h>
#include <qdatetime.h>
#include <qfont.h>
#include <qlist.h>
#include <qmargins.h>
#include <qrect.h>
#include <qsize.h>
#include <qvector.h>

#include <meteo/commons/proto/map_document.pb.h>

class QPoint;
class QPainter;

namespace meteo {
  class Generalization;
  namespace map {
    class Document;
  } // map
} // meteo


namespace meteo {

class AxisData
{
public:
  AxisData(int lvl = 20)
  {
    level = lvl;
    format_ = "f,2";
  }

  int level;

  QString format_;
  QVector<double> ticks_;
  QBitArray lastDrawed_;
  QSize cacheTickLabelSize_;
  QVector<QString> cacheTickLabels_;
};

class AxisSpectr
{
public:
  enum Position { kLeft, kRight, kTop, kBottom };
  enum LabelType { kltDateTime, kltNumber };
  enum TitlePosition { ktpBegin, ktpEnd, ktpNoDisplay };

  explicit AxisSpectr(const QSizeF& mapSize = QSizeF(500,500));

  void setMapSize(const QSizeF& size);

  bool isHorizontal() const                           { return (kTop == position_ || kBottom == position_); }

  double lower() const                                { return rangeLower_; }
  double upper() const                                { return rangeUpper_; }
  void setRange(double lower, double upper);
  void setScale(double scale) const;

  Position position() const                           { return position_; }
  void setPosition(Position position);

  QString label() const                               { return axisLabel_; }
  void setLabel(const QString& label, TitlePosition lp = ktpEnd) { axisLabel_ = label; axisLabelPosition_ = lp; }

  QString tickLabel(double scaleValue, const QString& def = "---") const;
  LabelType tickLabelType() const                     { return labelType_; }
  void setTickLabelType(LabelType type)               { labelType_ = type; }
  void setTickLabelFormat(const QString& format, int level = 20);

  int subTickCount() const                            { return subTickCount_; }
  void setSubTickCount(int n)                         { subTickCount_ = n; }

  int level() const                                   { return dataByLevel_[curLevel_].level; }

  QVector<double> ticks(int level) const;
  void setTickVector(const QVector<double>& ticks, int level = 20);

  void setAutoTicks(bool b);

  bool isInverted() const                             { return inverted_; }
  void setInverted(bool inverted)                     { inverted_ = inverted; }

  meteo::Property property() const               { return prop_; }
  void setProperty(const meteo::Property& prop)  { prop_ = prop; }


  bool isEqual(const AxisSpectr& a) const;

  void paint(QPainter* painter, map::Document* doc, const QPoint& beg, const QPoint& end) const;
  QRect geometry(map::Document* doc, const QPoint& beg, const QPoint& end) const;

  QBitArray lastDrawed() const { return dataByLevel_[curLevel_].lastDrawed_; }

  void updateDocScale(double scale) const;
  QList<meteo::Generalization> dataLevels() const;

  int32_t dataSize() const;
  int32_t serializeToArray(char* data) const;
  int32_t parseFromArray(const char* data);
  void calcTicks() const;

private:
  void updateFactor();

  void clearCache() const;
  void updateTicksCache() const;

  bool compareTicks(const AxisSpectr& a) const;

private:
  bool autoTicks_;
  double rangeLower_;
  double rangeUpper_;
  double factor_;
  Position position_;
  int tickSize_;
  mutable int nSteps_;
  mutable char scale_;
  int subTickCount_;
  QFont labelFont_;
  LabelType labelType_;
  meteo::Property prop_;
  QString axisLabel_;
  TitlePosition axisLabelPosition_;

  QSizeF mapSize_;

  bool inverted_;
  int invertedVal_;

  int tickXDirection_;
  int tickYDirection_;
  int tickLabelAlignFlags_;
  int tickLabelXOffset_;
  int tickLabelYOffset_;
  double tickLabelXOffsetFactor_;
  double tickLabelYOffsetFactor_;

  QMargins margins_;

  mutable int curLevel_;
  mutable QMap<int,AxisData> dataByLevel_;
};

} // meteo

QDataStream& operator>>(QDataStream& in, meteo::AxisData& data);
QDataStream &operator<<(QDataStream &out, const meteo::AxisData& data);


#endif // MAPPI_UI_PLUGINS_UHDCONTROL_AXISSPECTR_H
