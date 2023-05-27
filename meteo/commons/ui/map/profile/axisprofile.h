#ifndef METEO_COMMONS_UI_MAP_PROFILE_AXISPROFILE_H
#define METEO_COMMONS_UI_MAP_PROFILE_AXISPROFILE_H

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

class LayerProfile;
class Document;
class Generalization;

} // map
} // meteo


namespace meteo {
namespace map {

//! Структура PixItem хранит отрисованый пуансон и его
//! описывающий прямоугольник (для привильного центрирования при отрисовке на шкале)
struct PixItem
{
  QPixmap pixmap;
  QRect   rect;
};

//! Класс AxisProfile отвечает за отрисовку шкала с пуансонами в качестве подписей для насечек.
//! В качестве исходных данных для пуансонов используются заначения получаемый от слоя LayerProfile.
class AxisProfile : public AxisSimple
{
public:
  enum { Type = UserType + 1 };

public:
  explicit AxisProfile(LayerProfile* layer = nullptr, const QSizeF& mapSize = QSizeF(500,500));

  virtual int type() const { return Type; }

  void setPuanson(const meteo::puanson::proto::Puanson& puanson);

  //! Обновляет метеоданные, используемые для отрисовки пуансонов.
  //! Для горизонтальной шкалы используется занчение coord.lon(), для вертикальной coord.lat()
  //! Данные берутся из слоя LayerProfile, установленного для данной шкалы.
  void updateData(const GeoPoint& coord, Document* doc);
  //! Угол поворота ветра.
  void setAngleWindCorrection( float angle ) { angleWindCorrection_ = angle; }

  virtual void paint(QPainter* painter, const QPoint& beg, const QPoint& end, map::Document* doc) const;
  //! @arg beg  экранная координата начала оси
  //! @arg end  экранная координата конца оси
  virtual QRect boundingRect(Document* doc) const;

  virtual int32_t dataSize() const;
  virtual int32_t serializeToArray(char* data) const;
  virtual int32_t parseFromArray(const char* data);

private:
  QString key(const GeoPoint& pos, int generalizationLevel, int tickNumber) const;
  PixItem* renderTickImage(const TMeteoData& md) const;

  QVector<GeoPoint> mkProfile(const GeoPoint& beg, const QVector<float>& ticks) const;

  LayerProfile* profileLayer() const;

private:
  meteo::puanson::proto::Puanson puanson_;

  GeoPoint dataPos_;
  QVector<TMeteoData> data_;
  mutable QCache<QString,PixItem> pixCache_;
  float angleWindCorrection_ = 0;
};

} // map
} // meteo

//QDataStream& operator>>(QDataStream& in, meteo::AxisData& data);
//QDataStream &operator<<(QDataStream &out, const meteo::AxisData& data);

#endif // METEO_COMMONS_UI_MAP_PROFILE_AXISPROFILE_H
