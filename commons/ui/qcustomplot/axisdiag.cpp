#include "axisdiag.h"

AxisDiag::AxisDiag(QCPAxisRect *parent, AxisType type, bool rotate) :
  QCPAxis(parent, type),
  _isRotate(rotate)
{
}

AxisDiag::~AxisDiag()
{
}

void AxisDiag::draw(QCPPainter *painter)
{
  if (_isRotate) {
    drawDiag(painter);
  } else {
    drawPrime(painter);
  }	 
}

//диагональные
void AxisDiag::drawDiag(QCPPainter *painter)
{
  QRect axisRect = mAxisRect->rect();
  QPoint center = axisRect.center();
  float alpha = 45 * M_PI / 180;
  float coord = axisRect.width() * cos(alpha) / 2;
  int margin = 0;
  
  // draw baseline:
  QLineF baseLine;
  painter->setPen(basePen());
  if (mAxisType == QCPAxis::atLeft) {
    baseLine.setPoints(center + QPoint(-coord, -coord), center + QPoint(coord, coord));
  } else if (mAxisType ==  QCPAxis::atRight) {
    baseLine.setPoints(center + QPoint(coord, -coord), center + QPoint(-coord, coord)); 
  }
  painter->drawLine(baseLine);

  // axis label:
  QFont font = labelFont();
  font.setPointSize(font.pointSize()*1.2);
  font.setBold(true);
  painter->setFont(font);
  painter->setPen(QPen(labelColor()));
  QRect labelBounds = painter->fontMetrics().boundingRect(0, 0, 0, 0, Qt::TextDontClip, _labelTop);
  if (mAxisType ==  QCPAxis::atRight) {
    int xcor = labelBounds.width();
    int ycor = labelBounds.height();
    painter->drawText(baseLine.p1().x(), baseLine.p1().y() - ycor, labelBounds.width(), labelBounds.height(),
		      Qt::TextDontClip | Qt::AlignCenter, _labelTop);
    painter->drawText(baseLine.p2().x() - xcor, baseLine.p2().y(), labelBounds.width(), labelBounds.height(),
		      Qt::TextDontClip | Qt::AlignCenter, _labelBottom);
  } else if (mAxisType ==  QCPAxis::atLeft) {
    int xcor = labelBounds.width();
    int ycor = labelBounds.height();
    painter->drawText(baseLine.p1().x() - xcor, baseLine.p1().y() - ycor, labelBounds.width(), labelBounds.height(),
		      Qt::TextDontClip | Qt::AlignCenter, _labelTop);
    painter->drawText(baseLine.p2().x(), baseLine.p2().y(), labelBounds.width(), labelBounds.height(),
		      Qt::TextDontClip | Qt::AlignCenter, _labelBottom);
  }

  int tickDir = -1;
  if (mAxisType == QCPAxis::atLeft) {
    tickDir = 1;
  }
  
  drawTicksDiag(painter, tickDir, alpha);
  drawSubTicksDiag(painter, tickDir, alpha);  
}


void AxisDiag::drawTicksDiag(QCPPainter *painter, int tickDir, float alpha)
{
  //ticks
  QVector<QPointF> tickPositions;
  QVector<QString> tickLabels; 
  tickPositions.reserve(mTickVector.size());
  tickLabels.reserve(mTickVector.size());
 
  for (int i=0; i<mTickVector.size(); ++i) {
    QPointF pnt;
    pnt.setX(coordToPixel(tickDir*mTickVector.at(i) * cos(alpha)));
    pnt.setY(coordToPixel(mTickVector.at(i) * cos(alpha)));
    tickPositions.append(pnt);
    if (mTickLabels) {
      tickLabels.append(mTickVectorLabels.at(i));
    }
  }

  //  draw ticks:
  if (!tickPositions.isEmpty()) {
    painter->setPen(getTickPen());

    for (int i=0; i<tickPositions.size(); ++i) {
      painter->drawLine(QLineF(tickPositions.at(i).x() - tickLengthIn(),
      			       tickPositions.at(i).y() + tickDir*tickLengthIn(),
      			       tickPositions.at(i).x() + tickLengthIn(),
      			       tickPositions.at(i).y() - tickDir*tickLengthIn()));
    }
  }

  // tick labels: всегда lsOutside
  QRect oldClipRect;
  QSize tickLabelsSize(0, 0); // size of largest tick label, for offset calculation of axis label
  if (!tickLabels.isEmpty()) {   
    painter->setFont(getTickLabelFont());
    painter->setPen(QPen(getTickLabelColor()));
    const int maxLabelIndex = qMin(tickPositions.size(), tickLabels.size());
    int distanceToAxis = tickLengthIn() + tickLabelPadding();
    for (int i=0; i<maxLabelIndex; ++i) {
      placeTickLabelDiag(painter, tickPositions.at(i), distanceToAxis, tickLabels.at(i), &tickLabelsSize);
    }
  }
}

void AxisDiag::placeTickLabelDiag(QCPPainter *painter, QPointF position, int distanceToAxis, const QString &atext, QSize *tickLabelsSize)
{
  if (atext.isEmpty() || atext == "0") return;
  QString text = atext;
  text.remove('-');
  
  QSize finalSize;
  QPointF labelAnchor;
  float xcor = 0;
  switch (mAxisType) {
  case QCPAxis::atLeft:   labelAnchor = QPointF(position.x(), position.y() + distanceToAxis*0.5); break;
  case QCPAxis::atRight:  labelAnchor = QPointF(position.x(), position.y() + distanceToAxis*0.5); break;
  }
 
  QPointF finalPosition = labelAnchor;

  QRect totalBounds = QFontMetrics(painter->font()).boundingRect(0, 0, 0, 0, Qt::TextDontClip | Qt::AlignHCenter, text);
  totalBounds.moveTopLeft(QPoint(0, 0)); // want bounding box aligned top left at origin, independent of how it was created, to make further processing simpler

  if (mAxisType == QCPAxis::atLeft) {
    xcor = -totalBounds.width();
  }
  
  QTransform oldTransform = painter->transform();
  painter->translate(labelAnchor.x(), labelAnchor.y());
  painter->drawText(xcor, 0, totalBounds.width(), totalBounds.height(),
		    Qt::TextDontClip | Qt::AlignHCenter, text);
  painter->setTransform(oldTransform);
}

void AxisDiag::drawSubTicksDiag(QCPPainter *painter, int tickDir, float alpha)
{
  QVector<QPointF> subTickPositions;
  subTickPositions.reserve(mSubTickVector.size());

  if (mSubTicks) {
    const int subTickCount = mSubTickVector.size();
    for (int i=0; i<subTickCount; ++i) {
      QPointF pnt;
      pnt.setX(coordToPixel(tickDir*mSubTickVector.at(i) * cos(alpha)));
      pnt.setY(coordToPixel(mSubTickVector.at(i) * cos(alpha)));
      subTickPositions.append(pnt);
    }
  }

  // draw subticks:
  if (!subTickPositions.isEmpty()) {
    painter->setPen(subTickPen());
    for (int i=0; i<subTickPositions.size(); ++i) {
      painter->drawLine(QLineF(subTickPositions.at(i).x() - subTickLengthIn(),
  			       subTickPositions.at(i).y() + tickDir*subTickLengthIn(),
  			       subTickPositions.at(i).x() + subTickLengthIn(),
  			       subTickPositions.at(i).y() - tickDir*subTickLengthIn()));
    }
  }
}

void AxisDiag::drawPrime(QCPPainter *painter)
{
  QRect axisRect = mAxisRect->rect();
  QPoint center = axisRect.center();
  float coord = axisRect.width() / 2;

  // draw baseline:
  QLineF baseLine;
  painter->setPen(basePen());
  if (mAxisType == QCPAxis::atLeft) {
    baseLine.setPoints(center + QPoint(0, -coord), center + QPoint(0, coord));
  } else if (mAxisType ==  QCPAxis::atBottom) {
    baseLine.setPoints(center + QPoint(coord, 0), center + QPoint(-coord, 0)); 
  }
  painter->drawLine(baseLine);

  // axis label:
  QFont font = labelFont();
  font.setPointSize(font.pointSize()*1.2);
  font.setBold(true);
  painter->setFont(font);
  painter->setPen(QPen(labelColor()));
  QRect labelBounds = painter->fontMetrics().boundingRect(0, 0, 0, 0, Qt::TextDontClip, _labelTop);
  
  if (mAxisType ==  QCPAxis::atBottom) {
    int ycor = -labelBounds.height() - tickLengthIn();
    painter->drawText(baseLine.p2().x(), baseLine.p2().y() + ycor,
		      labelBounds.width(), labelBounds.height(),
		      Qt::TextDontClip | Qt::AlignCenter, _labelTop);
    painter->drawText(baseLine.p1().x() - labelBounds.width(), baseLine.p1().y() + ycor,
		      labelBounds.width(), labelBounds.height(),
		      Qt::TextDontClip | Qt::AlignCenter, _labelBottom);
    
  } else if (mAxisType ==  QCPAxis::atLeft) {
    int xcor = labelBounds.width() / 2 + tickLengthIn();
    painter->drawText(baseLine.p1().x() + xcor, baseLine.p1().y(),
		      labelBounds.width(), labelBounds.height(),
		      Qt::TextDontClip | Qt::AlignCenter, _labelTop);
    painter->drawText(baseLine.p2().x() + xcor, baseLine.p2().y() - labelBounds.height(),
		      labelBounds.width(), labelBounds.height(),
		      Qt::TextDontClip | Qt::AlignCenter, _labelBottom);
  }

  
  drawTicksPrime(painter);
  drawSubTicksPrime(painter);
}

void AxisDiag::drawTicksPrime(QCPPainter *painter)
{
  QPoint center = mAxisRect->rect().center();

  QVector<float> tickPositions;
  QVector<QString> tickLabels; 
  tickPositions.reserve(mTickVector.size());
  tickLabels.reserve(mTickVector.size());
  
  for (int i=0; i<mTickVector.size(); ++i) {
    float pnt = coordToPixel(mTickVector.at(i));
    tickPositions.append(pnt);
    if (mTickLabels) {
      tickLabels.append(mTickVectorLabels.at(i));
    }
  }

  float xcor = -1;
  // draw ticks:
  if (!tickPositions.isEmpty()) {
    painter->setPen(getTickPen());

    for (int i=0; i<tickPositions.size(); ++i) {
      if (QCPAxis::orientation(mAxisType) == Qt::Horizontal) {
  	painter->drawLine(QLineF(tickPositions.at(i) + xcor, center.y() - tickLengthIn(),
  				 tickPositions.at(i) + xcor, center.y() + tickLengthIn()));
      } else {
	painter->drawLine(QLineF(center.x() + tickLengthIn(), tickPositions.at(i),
  				 center.x() - tickLengthIn(), tickPositions.at(i)));
      }
    }
  }

  //tick labels: всегда lsOutside
  QRect oldClipRect;
  QSize tickLabelsSize(0, 0); // size of largest tick label, for offset calculation of axis label
  if (!tickLabels.isEmpty()) {
    painter->setFont(getTickLabelFont());
    painter->setPen(QPen(getTickLabelColor()));
    const int maxLabelIndex = qMin(tickPositions.size(), tickLabels.size());
    int distanceToAxis = tickLabelPadding();
    for (int i=0; i<maxLabelIndex; ++i) {
      placeTickLabelPrime(painter, center, tickPositions.at(i), distanceToAxis, tickLabels.at(i), &tickLabelsSize);
    }
    // margin += tickLabelsSize.width();
  }

}

void AxisDiag::drawSubTicksPrime(QCPPainter *painter)
{
  QPoint center = mAxisRect->rect().center();
  
  QVector<float> subTickPositions;
  subTickPositions.reserve(mSubTickVector.size());
  
  if (mSubTicks) {
    const int subTickCount = mSubTickVector.size();
    for (int i = 0; i < subTickCount; ++i) {
      float pnt = coordToPixel(mSubTickVector.at(i));
      subTickPositions.append(pnt);
    }
  }
  
  float xcor = -1;
  if (!subTickPositions.isEmpty()) {
    painter->setPen(subTickPen());
    for (int i = 0; i < subTickPositions.size(); ++i) {
      if (QCPAxis::orientation(mAxisType) == Qt::Horizontal) {
  	painter->drawLine(QLineF(subTickPositions.at(i) + xcor, center.y() + subTickLengthIn(),
  				 subTickPositions.at(i) + xcor, center.y() - subTickLengthIn()));
      } else {
  	painter->drawLine(QLineF(center.x() - subTickLengthIn(), subTickPositions.at(i),
  				 center.x() + subTickLengthIn(), subTickPositions.at(i)));
      }
    }
  }
}

void AxisDiag::placeTickLabelPrime(QCPPainter *painter, const QPointF& center, float position,
				   int distanceToAxis, const QString &atext, QSize *tickLabelsSize)
{
  if (atext.isEmpty() || atext == "0") return;
  QString text = atext;
  text.remove('-');
  
  QSize finalSize;
  QPointF labelAnchor;
  float xcor = 0, ycor = 0;
  switch (mAxisType) {
  case QCPAxis::atLeft:   labelAnchor = QPointF(center.x() - distanceToAxis, position); break;
  case QCPAxis::atBottom: labelAnchor = QPointF(position, center.y() + distanceToAxis); break;
  }
 
  QPointF finalPosition = labelAnchor;

  QRect totalBounds = QFontMetrics(painter->font()).boundingRect(0, 0, 0, 0, Qt::TextDontClip | Qt::AlignHCenter, text);
  totalBounds.moveTopLeft(QPoint(0, 0)); // want bounding box aligned top left at origin, independent of how it was created, to make further processing simpler

  if (mAxisType == QCPAxis::atLeft) {
    xcor = -totalBounds.width();
    ycor = -totalBounds.height() / 2;
  } else if (mAxisType == QCPAxis::atBottom) {
    xcor = -totalBounds.width() / 2;
  }
    
  QTransform oldTransform = painter->transform();
  painter->translate(labelAnchor.x(), labelAnchor.y());
  painter->drawText(xcor, ycor, totalBounds.width(), totalBounds.height(),
		    Qt::TextDontClip | Qt::AlignHCenter, text);
  //painter->drawRect(totalBounds);
  painter->setTransform(oldTransform);
}
