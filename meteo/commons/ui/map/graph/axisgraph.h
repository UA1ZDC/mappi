#ifndef METEO_COMMONS_UI_MAP_GRAPH_AXISGRAPH_H
#define METEO_COMMONS_UI_MAP_GRAPH_AXISGRAPH_H

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

#include <meteo/commons/proto/meteo.pb.h>

class QPoint;
class QPainter;

namespace meteo {
class Generalization;
namespace map {
class Document;
}
}


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
  QVector<float> ticks_;
  QBitArray lastDrawed_;
  QSize cacheTickLabelSize_;
  QVector<QString> cacheTickLabels_;
};

class AxisGraph
{
public:
  enum Position { kLeft, kRight, kTop, kBottom };
  enum LabelType { kltDateTime, kltNumber };
  enum TitlePosition { ktpBegin, ktpEnd, ktpNoDisplay };
  enum LabelAlign { pLeft, pCenter, pRight };

  explicit AxisGraph(const QSizeF& mapSize = QSizeF(500,500));

  void setMapSize(const QSizeF& size);
  void setOffset(float offset)                       { offset_ = offset; }

  bool isHorizontal() const                           { return (kTop == position_ || kBottom == position_); }

  float lower() const                                { return rangeLower_; }
  float upper() const                                { return rangeUpper_; }
  void setRange(float lower, float upper);

  Position position() const                           { return position_; }
  void setPosition(Position position);

  QString label() const                               { return axisLabel_; }
  void setLabel(const QString& label, TitlePosition lp = ktpEnd, LabelAlign align = pCenter)
  { axisLabel_ = label; axisLabelPosition_ = lp; alignLabel_ = align; }

  QString tickLabel(float scaleValue, const QString& def = "---") const;
  LabelType tickLabelType() const                     { return labelType_; }
  void setTickLabelType(LabelType type)               { labelType_ = type; }
  void setTickLabelFormat(const QString& format, int level = 20);

  int subTickCount() const                            { return subTickCount_; }
  void setSubTickCount(int n)                         { subTickCount_ = n; }

  int level() const                                   { return dataByLevel_[curLevel_].level; }

  QVector<float> ticks(int level) const;
  void setTickVector(const QVector<float>& ticks, int level = 20);

  void setAutoTicks(bool b);

  bool isInverted() const                             { return inverted_; }
  void setInverted(bool inverted)                     { inverted_ = inverted; }

  meteo::Property property() const               { return prop_; }
  void setProperty(const meteo::Property& prop)  { prop_ = prop; }

  float coord2scale(float c) const;
  float scale2coord(float c) const;

  bool isEqual(const AxisGraph& a) const;

  void paint(QPainter* painter, map::Document* doc, const QPoint& beg, const QPoint& end) const;
  QRect geometry(map::Document* doc, const QPoint& beg, const QPoint& end) const;

  QBitArray lastDrawed() const { return dataByLevel_[curLevel_].lastDrawed_; }

  void updateDocScale(float scale) const;
  QList<meteo::Generalization> dataLevels() const;

  int32_t dataSize() const;
  int32_t serializeToArray(char* data) const;
  int32_t parseFromArray(const char* data);

private:
  void updateFactor();
  void calcTicks();

  void clearCache();
  void updateTicksCache() const;

  bool compareTicks(const AxisGraph& a) const;

private:
  bool autoTicks_;
  float rangeLower_;
  float rangeUpper_;
  float factor_;
  Position position_;
  int tickSize_;
  int nSteps_;
  int subTickCount_;
  QFont labelFont_;
  LabelType labelType_;
  meteo::Property prop_;
  QString axisLabel_;
  TitlePosition axisLabelPosition_;
  LabelAlign alignLabel_;

  QSizeF mapSize_;
  float offset_ = 0.0;

  bool inverted_;
  int invertedVal_;

  int tickXDirection_;
  int tickYDirection_;
  int tickLabelAlignFlags_;
  int tickLabelXOffset_;
  int tickLabelYOffset_;
  float tickLabelXOffsetFactor_;
  float tickLabelYOffsetFactor_;

  QMargins margins_;

  mutable int curLevel_;
  mutable QMap<int,AxisData> dataByLevel_;
};

} // meteo

QDataStream& operator>>(QDataStream& in, meteo::AxisData& data);
QDataStream &operator<<(QDataStream &out, const meteo::AxisData& data);

#endif // METEO_COMMONS_UI_MAP_GRAPH_AXISGRAPH_H
