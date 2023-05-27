#ifndef METEO_COMMONS_UI_MAP_AXIS_AXISSIMPLE_H
#define METEO_COMMONS_UI_MAP_AXIS_AXISSIMPLE_H

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

#include <commons/geobasis/geopoint.h>
#include <meteo/commons/proto/meteo.pb.h>

class QPoint;
class QPainter;

namespace meteo {
class Generalization;
namespace map {

class Layer;
class Document;

}
}


namespace meteo {
namespace map {

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
  QVector<QString> labels_;
  QBitArray lastDrawed_;
  QSize cacheTickLabelSize_;
  QVector<QString> cacheTickLabels_;
};

class AxisSimple
{
public:
  //! Тип AxisType определяет вид шкалы и её ориентацию
  enum AxisType {
    kLeft,    //!< вертикальная ориентация, насечки слева от оси
    kRight,   //!< вертикальная ориентация, насечки справа от оси
    kCenter,  //!< вертикальная ориентация, насечки по центру оси
    kBottom,  //!< горизонтальная ориентация, насечки снизу оси
    kTop,     //!< горизонтальная ориентация, насечки сверху оси
    kMiddle,  //!< горизонтальная ориентация, насечки по середине оси
  };
  //! Тип подписи для насечек шкалы
  enum LabelType { kltDateTime, kltNumber, kltText };
  //! Положение подписи шкалы
  enum TitlePosition { ktpBegin, ktpEnd, ktpNoDisplay };

  enum {
    Type = 0,
    UserType = 65536
  };

public:
  explicit AxisSimple(const QSizeF& mapSize = QSizeF(500,500));
  virtual ~AxisSimple();

  virtual int type() const { return Type; }

  void setLayer(map::Layer* layer)                    { layer_ = layer; }
  map::Layer* layer() const                           { return layer_;  }

  void setGeoPos(const GeoPoint& pos)                 { pos_ = pos;  }
  GeoPoint geoPos() const                             { return pos_; }
  GeoPoint begGeoPos() const                          { return geoPos(); }
  GeoPoint endGeoPos() const;

  //! @length длина оси в координатах проекции
  void setLength(float length)                       { length_ = length; }
  float length() const                               { return length_;   }

  //! Требуется для корректной работы scale2coord(), coord2scale().
  void setMapSize(const QSizeF& size);
  //! Устанавливает смещение шкалы относительно начала координат
  //! Требуется для корректной работы scale2coord(), coord2scale().
  void setOffset(float offset)                       { offset_ = offset; }
  float offset() const                               { return offset_;   }

  bool isHorizontal() const                           { return (kTop == axisType_ || kBottom == axisType_ || kMiddle == axisType_); }

  bool isVisible() const                              { return prop_.visible();     }
  void setVisible(bool visible)                       { prop_.set_visible(visible); }

  bool drawOnRamka() const                            { return drawOnRamka_; }
  void setDrawOnRamka(bool f)                         { drawOnRamka_ = f;    }

  float lower() const                                { return rangeLower_; }
  float upper() const                                { return rangeUpper_; }
  void setRange(float lower, float upper);

  AxisType axisType() const                           { return axisType_; }
  void setAxisType(AxisType axisType);

  //! Возвращает подпись шкалы
  QString label() const                               { return axisLabel_; }
  void setLabel(const QString& label, TitlePosition lp = ktpEnd) { axisLabel_ = label; axisLabelPosition_ = lp; }

  //! Возвращает текстовую строку для заданного значения шкалы
  QString tickLabel(float scaleValue, const QString& def = "---") const;
  LabelType tickLabelType() const                     { return labelType_; }
  void setTickLabelType(LabelType type)               { labelType_ = type; }
  void setTickLabelFormat(const QString& format, int level = 20);

  int subTickCount() const                            { return subTickCount_; }
  void setSubTickCount(int n)                         { subTickCount_ = n; }

  QVector<float> ticks(int level) const;
  void setTickVector(const QVector<float>& ticks, int level = 20);
  //! Изменяет подписи для насечек на labels.
  //! @note: изменяет тип подписи насечки на kltText.
  void setTickLabelVector(const QVector<QString>& labels, int level = 20);

  void setAutoTicks(bool b);

  bool isInverted() const                             { return inverted_; }
  void setInverted(bool inverted)                     { inverted_ = inverted; }

  meteo::Property property() const               { return prop_; }
  void setProperty(const meteo::Property& prop)  { prop_ = prop; }

  float coord2scale(float c) const;
  float scale2coord(float scaleValue) const;

  virtual bool isEqual(const AxisSimple& a) const;

  //! Обновляет метеоданные, используемые для отрисовки пуансонов.
  //! Для горизонтальной шкалы используется занчение coord.lon(), для вертикальной coord.lat()
  //! Данные берутся из слоя LayerProfile, установленного для данной шкалы.
  virtual void updateData(const GeoPoint& coord, map::Document* doc)      { Q_UNUSED( coord ); Q_UNUSED( doc ); }

  //! @arg beg  экранная координата начала оси
  //! @arg end  экранная координата конца оси
  virtual void paint(QPainter* painter, const QPoint& beg, const QPoint& end, Document* doc) const;
  //! @arg beg  экранная координата начала оси
  //! @arg end  экранная координата конца оси
  virtual QRect boundingRect(map::Document* doc) const;

  QList<int> levels() const { return dataByLevel_.keys(); }

//  QBitArray lastDrawed() const { return dataByLevel_[curLevel_].lastDrawed_; }

  void updateDocScale(float scale) const;
  QList<meteo::Generalization> dataLevels() const;
  int generalizationLevel() const                     { return curLevel_; }

  virtual int32_t dataSize() const;
  virtual int32_t serializeToArray(char* data) const;
  virtual int32_t parseFromArray(const char* data);

protected:
  int tickSize() const                                { return tickSize_;               }
  int tickXDirection() const                          { return tickXDirection_;         }
  int tickYDirection() const                          { return tickYDirection_;         }
  int tickLabelAlignFlags() const                     { return tickLabelAlignFlags_;    }
  int tickLabelXOffset() const                        { return tickLabelXOffset_;       }
  int tickLabelYOffset() const                        { return tickLabelYOffset_;       }
  float tickLabelXOffsetFactor() const               { return tickLabelXOffsetFactor_; }
  float tickLabelYOffsetFactor() const               { return tickLabelYOffsetFactor_; }
  QFont labelFont() const                             { return labelFont_;              }
  float rangeLower() const                           { return rangeLower_;             }
  float rangeUpper() const                           { return rangeUpper_;             }
  float factor() const                               { return factor_;                 }
  float axisLabelPosition() const                    { return axisLabelPosition_;      }

  QMap<int,AxisData> dataByLevel() const              { return dataByLevel_; }

private:
  void updateFactor();
  void calcTicks();

  void clearCache();
  void updateTicksCache() const;

  bool compareTicks(const AxisSimple& a) const;

private:
  bool autoTicks_;
  float rangeLower_;
  float rangeUpper_;
  float factor_;
  AxisType axisType_;
  int tickSize_;
  int nSteps_;
  int subTickCount_;
  QFont labelFont_;
  LabelType labelType_;
  meteo::Property prop_;
  QString axisLabel_;
  TitlePosition axisLabelPosition_;
  bool drawOnRamka_ = false;

  float length_ = 0.0;
  GeoPoint pos_;
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
  map::Layer* layer_ = nullptr;

  mutable int curLevel_;
  mutable QMap<int,AxisData> dataByLevel_;
};

}
}

QDataStream& operator>>(QDataStream& in, meteo::map::AxisData& data);
QDataStream &operator<<(QDataStream &out, const meteo::map::AxisData& data);

template <class T> inline T axis_cast( meteo::map::AxisSimple* a )
{
  return int(static_cast<T>(0)->Type) == int(meteo::map::AxisSimple::Type)
    || ( 0 != a && int(static_cast<T>(0)->Type) == a->type() ) ? static_cast<T>(a) : 0;
}

template <class T> inline T axis_cast(const meteo::map::AxisSimple* a )
{
  return int(static_cast<T>(0)->Type) == int(meteo::map::AxisSimple::Type)
    || ( 0 != a && int(static_cast<T>(0)->Type) == a->type() ) ? static_cast<T>(a) : 0;
}

#endif // METEO_COMMONS_UI_MAP_AXIS_AXISSIMPLE_H
