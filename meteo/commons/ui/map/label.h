#ifndef METEO_COMMONS_UI_MAP_LABEL_H
#define METEO_COMMONS_UI_MAP_LABEL_H

#include "object.h"

class QImage;

namespace meteo {
namespace map {

class FreezedLabelRect;

class Label : public Object
{
  public:
    Label( Layer* layer );
    Label( Object* parent );
    Label( Projection* proj );
    Label( const meteo::Property& prop );
    Label();
    ~Label();

    enum {
      Type = kLabel
    };
    virtual int type() const { return Type; }

    bool isLabel() const { return true; }
    
    bool render( QPainter* painter, const QRect& target, const QTransform& transform );

    void setScreenPos( const QPoint& pos );
    void removeScreenPos();

    /*! 
     * \brief - Рассчитать истинный размер метки
     * \return - размер метки
     */
    virtual QRect boundingRect() const = 0;

    /*! 
     * \brief - Рассчитать экранные координаты мест сцены, где должны быть отрисованы метки
     * \param transform - матрица преобразований карты
     * \return - список экранных координат и углов поворота метки
     */
    virtual QList< QPair< QPoint, float > > screenPoints( const QTransform& transform ) const = 0;

    const QList<QRect> drawedrects() const { return drawedrects_; }

  protected:
    /*! 
     * \brief - Рассчитать прямоугольник, в котором должна располагаться метка
     * \param pnt - экранная координата, к которй привязана метка
     * \param orient - ориентация метки относительно точки првязки
     * \param angle - угол поворта метки в точке привязки
     * \param oldtopleft - координата левого-верхнего угла метки при значении угла поворота = 0
     * \return - прямоугольник, описывающий метку
     */
    QRect boundingRect( const QPoint& pnt, Position orient, float angle, QPoint* oldtopleft = 0 ) const ;
    bool render( QPainter* painter, const QRect& r, const QPoint& pnt, const QPoint& oldtopleft, float angle );

    /*! 
     * \brief - Проверка пустой метки
     * \return - true - метка пустая, рисовать нечего, false - метка не пустая
     */
    virtual bool isEmpty() const = 0;

    /*! 
     * \brief - Подготовить картинку с содержимым Label 
     * \return true - картинка подготовлена, иначе - нет
     */
    virtual bool preparePixmap() = 0;

  protected:
    QImage* pix_;
    QPoint* screenpos_;

  private:
    bool freezed_ = false;
    QList<QRect> drawedrects_;

  friend class FreezedLabelRect;
};

}
}

#endif
