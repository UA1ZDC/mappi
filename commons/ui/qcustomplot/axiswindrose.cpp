#include "axiswindrose.h"
#include "axisdiag.h"

AxisWindRose::AxisWindRose(QCustomPlot* plot):
  QCPAbstractItem(plot)
{
  createAxis(plot);
}

AxisWindRose::~AxisWindRose()
{
}

void AxisWindRose::setDistance(float dist)
{
  if (nullptr != _x) {
    _x->setRange(-dist, dist);
  }
  if (nullptr != _y) {
    _y->setRange(-dist, dist);
  }
  if (nullptr != _left) {
    _left->setRange(-dist, dist);
  }
  if (nullptr != _right) {
    _right->setRange(-dist, dist);
  }

}

void AxisWindRose::createAxis(QCustomPlot* plot)
{
  if (nullptr == plot) return;

  plot->xAxis->setVisible(false);
  plot->yAxis->setVisible(false);
  
  _x = new AxisDiag(plot->axisRect(), QCPAxis::atBottom, false);
  plot->axisRect()->addAxis(QCPAxis::atBottom, _x);
  _x->setVisible(true);
  _x->setSubTicks(true);
  _x->setLabels(QObject::tr("З"), QObject::tr("В"));
  
  _y = new AxisDiag(plot->axisRect(), QCPAxis::atLeft, false);
  plot->axisRect()->addAxis(QCPAxis::atLeft, _y);
  _y->setVisible(true);
  _y->setSubTicks(true);
  _y->setLabels(QObject::tr("С"), QObject::tr("Ю"));
  
  _left = new AxisDiag(plot->axisRect(), QCPAxis::atLeft, true);
  plot->axisRect()->addAxis(QCPAxis::atLeft, _left);
  _left->setVisible(true);
  _left->setSubTicks(true);
  _left->setLabels(QObject::tr("СЗ"), QObject::tr("ЮВ"));
  
  _right = new AxisDiag(plot->axisRect(), QCPAxis::atRight, true);
  plot->axisRect()->addAxis(QCPAxis::atRight, _right);
  _right->setVisible(true);
  _right->setSubTicks(true);
  _right->setLabels(QObject::tr("СВ"), QObject::tr("ЮЗ"));

  plot->axisRect()->setAutoMargins(QCP::msNone);
  QMargins marg = plot->plotLayout()->margins();
  marg.setLeft(15);
  marg.setRight(15);
  marg.setTop(15);
  marg.setBottom(15);
  plot->plotLayout()->setMargins(marg);

  QCPItemEllipse* circle = new QCPItemEllipse(plot);
  QBrush brush = circle->brush();
  brush.setStyle(Qt::SolidPattern);
  brush.setColor(Qt::white);
  circle->setBrush(brush);
  QPointF diff = QPointF(plot->width() / 30, plot->height() / 30);
  QPointF center = QPointF(plot->rect().center());
  circle->topLeft->setType(QCPItemPosition::ptAbsolute);
  circle->topLeft->setCoords(center - diff);
  circle->bottomRight->setType(QCPItemPosition::ptAbsolute);
  circle->bottomRight->setCoords(center + diff);

  _calm = new QCPItemText(plot);
  _calm->position->setType(QCPItemPosition::ptAbsolute);
  _calm->position->setCoords(center + QPoint(0, -1)); 
  _calm->setPositionAlignment(Qt::AlignCenter);
  _calm->setText("");
  _calm->setFont(QFont(plot->font().family(), 12));

  _curve = new QCPCurve(_x, _y);
  QPen pen = _curve->pen();
  pen.setWidth(2);
  _curve->setPen(pen);
  QBrush brushc;
  brushc.setStyle(Qt::SolidPattern);
  brushc.setStyle(Qt::SolidPattern);
  brushc.setColor(QColor(0, 80, 200, 35));
  _curve->setBrush(brushc);  
}

void AxisWindRose::setCalm(float val)
{
  if (nullptr == _calm) return;

  _calm->setText(QString::number(val, 'f', 0));
}

//0 - север, дальше по часовой, всего 8 значений
void AxisWindRose::setCurve(const QMap<int, float>& table)
{
  if (nullptr == _curve) return;
  
  QVector<QCPCurveData> data(9);
  float max = 0;

  for (int idx = 1; idx < 9; idx++) {
    if (table.contains(idx)) {
      float alpha = (360 - (idx-2)*45) * M_PI / 180;
      float x = table.value(idx) * cos(alpha);
      float y = table.value(idx) * sin(alpha);
      max = max < table.value(idx) ? table.value(idx) : max;
      data[idx - 1] = QCPCurveData(idx - 1, x, y);
    } else {
      data[idx - 1] = QCPCurveData(idx - 1, 0, 0);
    }
  }
  data[8] = data[0];
  _curve->data()->set(data, true);
  
  setDistance(max + 1);
}
