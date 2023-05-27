#ifndef COMMONS_UI_QCUSTOMPLOT_AXISWINDROSE_H
#define COMMONS_UI_QCUSTOMPLOT_AXISWINDROSE_H

#include <commons/ui/qcustomplot/qcustomplot.h>
#include <commons/ui/qcustomplot/axisdiag.h>

class AxisWindRose : QCPAbstractItem {
public:
  AxisWindRose(QCustomPlot* plot);
  virtual ~AxisWindRose();

  void setCalm(float val);
  void setCurve(const QMap<int, float>& table);

  void setDistance(float dist);
  
  QCPAxis* xAxis() { return _x; }
  QCPAxis* yAxis() { return _y; }
  QCPItemText* calmText() { return _calm; }
  QCPCurve* curve() { return _curve; }
  

  double selectTest(const QPointF&, bool, QVariant* =0)const { return 0; }
  
protected:
  void	draw(QCPPainter */*painter*/) {}
    
private:
  void createAxis(QCustomPlot* plot);

private:
  AxisDiag* _x = nullptr;
  AxisDiag* _y = nullptr;
  AxisDiag* _left = nullptr; //верх-лево до низ-право
  AxisDiag* _right = nullptr;//низ-право до верх-лево

  QCPItemText *_calm = nullptr; //!< повторяемость штилей
  QCPCurve *_curve = nullptr;
};

#endif

