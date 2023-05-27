#ifndef COMMONS_UI_QCUSTOMPLOT_CUSTOMAXIS_H
#define COMMONS_UI_QCUSTOMPLOT_CUSTOMAXIS_H

#include <commons/ui/qcustomplot/qcustomplot.h>

class AxisDiag : public QCPAxis {
public:
  AxisDiag(QCPAxisRect *parent, AxisType type, bool rotate);
  virtual ~AxisDiag();

  void draw(QCPPainter *painter);
  void setLabels(const QString& top, const QString& bottom) {
    _labelTop = top; _labelBottom = bottom;
  }
  
private:
  void drawDiag(QCPPainter *painter);
  void drawTicksDiag(QCPPainter *painter, int tickDir, float alpha);
  void drawSubTicksDiag(QCPPainter *painter, int tickDir, float alpha);
  void placeTickLabelDiag(QCPPainter *painter, QPointF position, int distanceToAxis, const QString &text, QSize *tickLabelsSize);

  void drawPrime(QCPPainter *painter);
  void drawTicksPrime(QCPPainter *painter);
  void drawSubTicksPrime(QCPPainter *painter);
  void placeTickLabelPrime(QCPPainter *painter, const QPointF& center, float position,
			   int distanceToAxis, const QString &atext, QSize *tickLabelsSize);

private:
  bool _isRotate; //!< диагональные оси или основные
  QString _labelTop; //!< подпись к верхней или левой половинке
  QString _labelBottom; //!< подпись к нижней или правой
  
};

#endif 
