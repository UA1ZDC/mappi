#ifndef METEO_COMMONS_UI_MAP_AXIS_AXISPUANSON_H
#define METEO_COMMONS_UI_MAP_AXIS_AXISPUANSON_H

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
#include <qpixmapcache.h>
#include <qcache.h>

#include <commons/meteo_data/meteo_data.h>

#include <meteo/commons/proto/meteo.pb.h>
#include <meteo/commons/proto/puanson.pb.h>
#include <meteo/commons/ui/map/axis/axissimple.h>

class QPoint;
class QPainter;

namespace meteo {
namespace map {

class LayerAxis;
class Document;
class Generalization;

} // map
} // meteo


namespace meteo {
namespace map {

//! Структура PixItem хранит отрисованый пуансон и его
//! описывающий прямоугольник (для привильного центрирования при отрисовке на шкале)
struct PuanPixItem
{
  QPixmap pixmap;
  QRect   rect;
};

struct PuanDataItem
{
  TMeteoData data;
  GeoPoint   pos;

  bool operator ==(const PuanDataItem& d) const
  {
    if ( d.data.count() != data.count() ) {
      return false;
    }

    QByteArray ba[2];
    d.data >> ba[0];
    data >> ba[1];
    return ba[0] == ba[1];
  }
};

//! Класс AxisProfile отвечает за отрисовку шкала с пуансонами в качестве подписей для насечек.
//! В качестве исходных данных для пуансонов используются заначения получаемый от слоя LayerProfile.
class AxisPuanson : public AxisSimple
{
public:
  enum { Type = UserType + 2 };

public:
  explicit AxisPuanson(LayerAxis* layer = nullptr, const QSizeF& mapSize = QSizeF(500,500));

  virtual int type() const { return Type; }

  void setPuanson(const meteo::puanson::proto::Puanson& puanson);

  //! Обновляет метеоданные, используемые для отрисовки пуансонов.
  //! Для горизонтальной шкалы используется занчение coord.lon(), для вертикальной coord.lat()
  //! Данные берутся из слоя LayerProfile, установленного для данной шкалы.
  void updateData(const GeoPoint& coord, Document* doc);

  //! @arg beg  экранная координата начала оси
  //! @arg end  экранная координата конца оси
  virtual void paint(QPainter* painter, const QPoint& beg, const QPoint& end, map::Document* doc) const;
  virtual QRect boundingRect(Document* doc) const;

  virtual bool isEqual(const AxisSimple& a) const;

  virtual int32_t dataSize() const;
  virtual int32_t serializeToArray(char* data) const;
  virtual int32_t parseFromArray(const char* data);

private:
  QString key(const GeoPoint& pos, int generalizationLevel, int tickNumber) const;
  PuanPixItem* renderTickImage(const TMeteoData& md, const QColor& bg, float scale) const;

  QVector<GeoPoint> mkProfile(const GeoPoint& beg, const QVector<float>& ticks) const;

  LayerAxis* axisLayer() const;

private:
  meteo::puanson::proto::Puanson puanson_;

  QVector<PuanDataItem> data_;
  mutable QCache<QString,PuanPixItem> puanCache_;
  mutable QCache<QString,PuanPixItem> textCache_;
};

} // map
} // meteo

//QDataStream& operator>>(QDataStream& in, meteo::AxisData& data);
//QDataStream &operator<<(QDataStream &out, const meteo::AxisData& data);

#endif // METEO_COMMONS_UI_MAP_AXIS_AXISPUANSON_H
