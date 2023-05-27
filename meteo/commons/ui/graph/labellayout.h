#ifndef METEO_COMMONS_UI_GRAPH_LABELLAYOUT_H
#define METEO_COMMONS_UI_GRAPH_LABELLAYOUT_H

#include <qlist.h>
#include <qpair.h>
#include <qvector.h>

namespace meteo {

//! Класс LabelLayout отвечает за размещение подписей вдоль прямой таким образом, чтобы
//! расстояния до точки привязки было наименьшим.
class LabelLayout
{
public:
  LabelLayout();

  void setRamkaPos(double top, double bottom);
  void addLabel(double pos, double size);

  //! Возвращает список смещений относительно позиции подписи для её оптимального размещения.
  //! Значения в списке следуют в порядке добавления подписей.
  QVector<double> optimPosition() const;

private:
  bool hasIntersects(const QVector<double>& offsets) const;

private:
  double top_;
  double bot_;
  QVector< QPair<double,double> > labels_;
};

} // meteo

#endif // METEO_COMMONS_UI_GRAPH_LABELLAYOUT_H
