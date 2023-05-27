#include "exprdraw/Parent/TwinParent/GroupOp/texpr_circ.h"

#include <QPaintDevice>
#include <QPainter>
#include <QFontMetrics>

TExpr_Circ::TExpr_Circ(TExpr_Class* _son,
                       TExpr_Class* _firstTwin,
                       TExpr_Class* _secondTwin)
  : TExpr_GroupOp(_son, _firstTwin, _secondTwin)
{
  m_ch = QChar(8747);
}

void TExpr_Circ::DrawSymbol(QPainter *painter, int X, int Y)
{
  // X,Y середина символа.
  TExpr_GroupOp::DrawSymbol(painter, X, Y);

  QFontMetrics fm(m_symbol_font);
  int rX = fm.boundingRect(m_ch).width()/2;

  double logicalDpi = 1.0;
  if (m_paintDevice != 0){
    logicalDpi = m_paintDevice->logicalDpiX()/m_paintDevice->logicalDpiY();
  }
  int rY = qRound(rX * logicalDpi);

  painter->save();

  QPen pen;
  pen.setStyle(Qt::SolidLine);
  pen.setWidth(m_wx);
  pen.setColor(color());
  painter->setPen(pen);

  QBrush brush;
  brush.setStyle(Qt::NoBrush);
  painter->setBrush(brush);

  painter->setRenderHint(QPainter::Antialiasing);


  QRect rec;
  X += qRound(painter->pen().width()/2.0);
  Y += qRound(painter->pen().width()/2.0);
  rec.setLeft(X - rX);
  rec.setTop(Y - rY);
  rec.setRight(X + rX - m_wx);
  rec.setBottom(Y + rY - m_wx);

  painter->drawEllipse(rec);
  painter->restore();
}
