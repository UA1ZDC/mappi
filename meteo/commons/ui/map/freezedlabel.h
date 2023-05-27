#ifndef METEO_COMMONS_UI_MAP_FREEZEDLABEL_H
#define METEO_COMMONS_UI_MAP_FREEZEDLABEL_H

#include <qpoint.h>
#include <qrect.h>
#include <qpainter.h>
#include <qmap.h>

//FreezedLabelRect, FreezedLabel, FreezedLabelList - классы, предназначенные для корректной отрисовки меток поверх всей карты
//с учетом их приоритета и пересечения. Создаются и 'живут' только в течение выполнения функций Document::drawDocument и MapScene::drawBackground

namespace meteo {
namespace map {

class Document;
class Label;
class FreezedLabel;

/*! 
 * \brief FreezedLabelRect - Обеспечивает отрисовку метки в одном прямоугольнике с учетом его параметров поворота
 */
class FreezedLabelRect
{
  public:
    FreezedLabelRect( Label* parent, const QRect& rect, const QPoint& pntnew, const QPoint& pntold, float angle );
    ~FreezedLabelRect();

    bool render( QPainter* pntr ) const ;

  private:
    Label* parent_;
    QRect rect_;
    QPoint pntnew_;
    QPoint pntold_;
    float angle_;
};

/*! 
 * \brief FreezedLabel - Обеспечивает отрисовку метки во всех прямоугольниках, в которых она должна быть отрисована
 */
class FreezedLabel
{
  public:
    FreezedLabel() {}
    FreezedLabel( Label* label );
    ~FreezedLabel();

    Label* label() const { return label_; }
    void setLabel( Label* lbl ) { label_ = lbl; }

    void render( QPainter* pntr, const QRect& target, Document* doc );

    void addRect( const QRect& rect, const QPoint& pntnew, const QPoint& pntold, float angle );

  private:
    Label* label_;
    QList< FreezedLabelRect > rects_;

};

/*! 
 * \brief FreezedLabelList - Набор всех меток, которые должны быть отрисованы на карте
 */
class FreezedLabelList
{
  public:
    FreezedLabelList() {}
    FreezedLabelList( Document* doc, const QRect& target );
    ~FreezedLabelList();

    void render( QPainter* pntr );

    bool addRect( Label* lbl, const QRect& r, const QPoint& pntnew, const QPoint& pntold, float angle );

  private:
    Document* doc_ = nullptr;
    QMap< int, QMap< Label*, FreezedLabel > > labels_;
    QList<QRect> labelrects_;
    FreezedLabel* lastlbl_ = nullptr;
    QRect target_;
};

}
}

#endif
