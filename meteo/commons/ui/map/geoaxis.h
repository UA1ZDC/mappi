#ifndef METEO_COMMONS_UI_GEOAXIS_H
#define METEO_COMMONS_UI_GEOAXIS_H

#include <qvector.h>

#include <commons/meteo_data/meteo_data.h>
#include "object.h"
#include "geotext.h"


namespace meteo {
namespace map {
class Puanson;
} // map
} // meteo


namespace meteo {
namespace map {

const int kGeoAxisType = kPolygon + 9999;
class DecartData
{
public:
  DecartData(){x=0.;y=0.;fun=0.;}
  DecartData(float ax, float ay, float af){
    x=ax; y=ay; fun = af;
  }
  
  float x;
  float y;
  float fun;
};
//! Класс GeoAxis позволяет отобразить на плоскости координатную ось.
class GeoAxis : public Object
{
public:
  //! Данный тип используется для обозначения ориентации оси.
  enum Orientation { kVertical, kHorizontal };
  //! Данный тип используется для обозначения расположения подписей.
  enum LabelPosition { kBegin, kEnd };

  //! Конструируем объект, который будет размещён на слое layer.
  GeoAxis(Layer* layer, Orientation orientation = kVertical);
  //! Конструируем дочерний объект для объекта parent, который будет размещён на том же слое, что и parent.
  GeoAxis(Object* parent, Orientation orientation = kVertical);
  //!
  GeoAxis(Projection* proj, Orientation orientation = kVertical);
  //!
  GeoAxis(Orientation orientation = kVertical, const meteo::Property& prop = meteo::Property() );
  //!
  ~GeoAxis();

  enum {
     Type = kGeoAxisType
  };
  virtual int type() const { return Type; }

  //! Создаём копию объекта на слое layer.
  virtual Object* copy(Layer* layer) const;
  //! Создаём дочерний объект, для объекта parent, который является копией текущего объекта.
  virtual Object* copy(Object* parent) const;
  //!
  virtual Object* copy(Projection* proj) const;

  //! Возвращает список ограничивающих прямоугольников объекта.
  virtual QList<QRect> boundingRect(const QTransform& transform) const;
  //! Отрисовывает объект на painter.
  //! target - область экрана требующая перерисовки
  //! transform - ???
  virtual bool render(QPainter* painter, const QRect& target, const QTransform& transform);
  //!
  virtual QList<GeoVector> skeletInRect(const QRect& rect, const QTransform& transform) const;
  //! Первая координата skelet задаёт позицию оси, а вторая определяет длину. Для горизонтальной ориентации
  //! используется значение GeoPoint::lat(), для вертикальной GeoPoint::lon().
  virtual void setSkelet(const GeoVector& skelet);
  //! Изменяет ориентацию координатной оси на orientation.
  void setOrientation(Orientation orientation);

  //! Изменяет врхнее и нижнее значение шкалы на lower и upper.
  void setRange(float lower, float upper);
  //! Устанавливает значения для которых будут отрисовываться засечки.
  void setTickValues(const QVector<float>& values);
  //! Устанавливает подписи для засечек на шкале.
  void setTickLables(const QVector<QString>& labels);
  //! Устанавливает подписи для засечек на шкале.
  void setTickLables(const QMap< float , TMeteoData >& );
  //! Устанавливает объект-прототип для подписей засечек.
  void setTickLabelPrototype(const GeoText& prototype);
  //! Устанавливает объект-прототип для подписей засечек.
  void setTickLabelPrototype(const Puanson& prototype);
  //! Добавляет объект object в качестве подписи для оси, со смещением screenOffset относительно
  //! позиции position.
  void addAxisLabel(const Object& object, const QPoint& screenOffset, LabelPosition position);

  //!
  virtual bool underGeoPoint(const GeoPoint& gp) const;
  //!
  virtual int minimumScreenDistance(const QPoint& pos, QPoint* cross = 0 ) const;

  //! Не используется. Всегда возвращает false.
  virtual bool hasValue() const;
  //! Не используется. Всегда возвращает -9999.
  virtual float value() const;
  //! Не используется.
  virtual void setValue(float val, const QString& format = "4' '.2'0'", const QString& unit = QString());

protected:
  //! Возвращает координату засечки для значения value. Если значение value выходит за пределы диапазона оси или
  //! не задана ось, то вернёт точку с координатой (0,0).
  GeoPoint tickGeoPoint(float value) const;
  //! Возвращает координату засечки для значения value. Если значение value выходит за пределы диапазона оси или
  //! не задана ось, то вернёт точку с координатой (0,0).
  QPoint tickCartesianPoint(float value) const;

private:
  void init();
  //
  void moveToPoint(Object* object, const GeoPoint& point) const;
  //
  void updateLabelsCoord();

private:
  Orientation orientation_;
  float rangeLower_;
  float rangeUpper_;
  QVector<float> tickValues_;
  QVector<QString> tickLabels_;
  //TMeteoData tickPuanson_;
  QMap< float , TMeteoData > meteo_data_;
  //meteo::zond::PlaceData zond;
  Object* labelTickPrototype_;
  QList<QRect>  boundingRects_;

  // список объектов и смещений (в экранных координатах), отрисовываемых в начале оси
  QList<QPair<Object*,QPoint> > beginLabels_;
  // список объектов и смещений (в экранных координатах), отрисовываемых в конце оси
  QList<QPair<Object*,QPoint> > endLabels_;
};

} // map
} // meteo

#endif // METEO_COMMONS_UI_VERTICALCUT_GUI_GEOAXIS_H
