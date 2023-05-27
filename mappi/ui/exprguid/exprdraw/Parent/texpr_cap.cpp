#include "exprdraw/Parent/texpr_cap.h"

#include <QFontMetrics>
#include <QPainter>
#include <QPolygon>

TExpr_Cap::TExpr_Cap(TExpr_Class* _son, ExprDraw::TExprCapStyle _capStyle,
                     int _count)
  : TExpr_Parent(_son)
{
 m_capStyle = _capStyle;
 m_count = _count;
}

int TExpr_Cap::CalcWidth()
{
  if (!hasSon())
    return 0;
  int result = son()->width();
  int DLeft = son()->capDXLeft();
  int DRight = son()->capDXRight();

  if ((m_capStyle == ExprDraw::ecVector) ||
      (m_capStyle == ExprDraw::ecLine)){
    if (DLeft < 0)
      result -= DLeft;
    if (DRight > 0)
      result += DRight;
  }
  else{
    int W = CapWidth() / 2;
    int CX = (DLeft + DRight + result) / 2;
    result = qMax(CX, W) + qMax(result - CX, W);
  }
  return result;
}

int TExpr_Cap::CalcHeight()
{
  int sH = hasSon() ? son()->height() : 0;
  return sH + SelfHeight();
}

int TExpr_Cap::CalcMidLine(ExprDraw::TExprOrigin origin)
{
  if (origin == ExprDraw::eoTop){
    int sM = hasSon() ? son()->midLineUp() : 0;
    return sM + SelfHeight();
  }

  return hasSon() ? son()->midLineDn() : 0;
}

int TExpr_Cap::CalcPowerXPos()
{
  int sW = 0;
  int sPX = 0;
  if (hasSon()){
    sW = son()->width();
    sPX = son()->powerXPos();
  }

  if (width() == sW)
    return sPX;

  return TExpr_Parent::CalcPowerXPos();
}

int TExpr_Cap::CalcPowerYPos()
{
  int sPY = hasSon() ? son()->powerYPos() : 0;
  return sPY + SelfHeight();
}

int TExpr_Cap::CalcIndexXPos()
{
  int sDXLeft = 0;
  int sDXRight = 0;
  int sW = 0;
  int sXPos = 0;
  if (hasSon()){
    sDXLeft = son()->GetCapDXLeft();
    sDXRight = son()->GetCapDXRight();
    sW = son()->width();
    sXPos = son()->CalcIndexXPos();
  }

  int DX = 0;
  if ((m_capStyle == ExprDraw::ecPoints) ||
      (m_capStyle == ExprDraw::ecCap) ||
      (m_capStyle == ExprDraw::ecTilde))
    DX = qMax(CapWidth()/2 - (sDXLeft + sW + sDXRight)/2, 0);
  else
    DX = qMax(0, -sDXLeft);
  return sXPos + DX;
}

int TExpr_Cap::CapWidth()
{
  QFontMetrics fm(font());
  switch (m_capStyle) {
  case ExprDraw::ecPoints:
    return m_wx * (4*m_count - 2);
  case ExprDraw::ecCap:
    return fm.width(QChar('^'));
  case ExprDraw::ecTilde:
    return fm.width(QChar('~'));
  default:
    return 0;
  }
}

int TExpr_Cap::CapHeight()
{
  switch (m_capStyle) {
  case ExprDraw::ecPoints:
    return 5*m_wy;
  case ExprDraw::ecVector:
  case ExprDraw::ecTilde:
    return 6*m_wy;
  case ExprDraw::ecCap:
    return 11*m_wy;
  case ExprDraw::ecLine:
    return 4*m_wy;
  default:
    return 0;
  }
}

int TExpr_Cap::SelfHeight()
{
  int sDY = hasSon() ? son()->capDY() : 0;
  return qMax(0, CapHeight() - sDY);
}

int TExpr_Cap::CalcCapDY()
{
  int sDY = hasSon() ? son()->capDY() : 0;
  return qMax(0, sDY - CapHeight());
}

int TExpr_Cap::FTType()
{
  if (hasSon())
    return son()->FTType();
  return TExpr_Parent::FTType();
}

void TExpr_Cap::Paint(QPainter *painter, int X, int Y)
{
  if (!hasSon())
    return;

  int DY = Y + SelfHeight();
  int DLeft = son()->capDXLeft();
  int DRight = son()->capDXRight();
  int W = width();
  int DX = 0;
  int TW = 0;
  int CX = 0;

  if ((m_capStyle == ExprDraw::ecPoints) ||
      (m_capStyle == ExprDraw::ecCap) ||
      (m_capStyle == ExprDraw::ecTilde)){
    TW = CapWidth()/2;
    CX = (DLeft + son()->width() + DRight)/2;
    DX = qMax(TW - CX, 0);
  }
  else
    DX = qMax(0, -DLeft);

  son()->Draw(painter, X + DX, DY, ExprDraw::ehLeft, ExprDraw::evTop);

  DY +=son()->capDY() - m_wy;
  int LX = X + qMax(0, DLeft);
  int RX = X + W+ qMin(0, DRight);

  painter->save();
  SetPenAndBrush(painter);
  SetPainterFont(painter);
  QPoint pos;
  QFontMetrics fm(font());

  switch (m_capStyle) {
  case ExprDraw::ecPoints:{
    QPen pen = painter->pen();
    for (int i = 0; i < m_count; ++i) {
      LX = X + DX + CX - TW;
      RX = LX + 2*m_wx;
      QRect rec;
      rec.setLeft(LX + 4*m_wx*i);
      rec.setTop(DY - 3*m_wy);
      rec.setRight(RX + 4*m_wx*i - pen.width());
      rec.setBottom(DY - m_wy - pen.width());
      painter->drawEllipse(rec);
    }
  }
    break;
  case ExprDraw::ecVector:{
    QPolygon pl;
    pl << QPoint(LX, DY - 3*m_wy);
    pl << QPoint(RX - 2*m_wx, pl.at(0).y());
    pl << QPoint(pl.at(1).x(), pl.at(1).y() - m_wy);

    if (m_wy&1){
      pl << QPoint(RX - 1, pl.at(1).y() + (m_wy / 2));
    }
    else{
      pl << QPoint(RX - 1, pl.at(1).y() + (m_wy / 2) - 1);
      pl << QPoint(pl.last().x(), pl.last().y() + 1);
    }

    pl << QPoint(pl.at(2).x(), pl.at(2).y() + 3*m_wy - 1);
    pl << QPoint(pl.last().x(), pl.last().y() - m_wy);
    pl << QPoint(pl.at(0).x(), pl.last().y());

    painter->setRenderHint(QPainter::Antialiasing, false);
    painter->drawPolygon(pl);
  }
    break;
  case ExprDraw::ecCap:{
    pos.setX(X + DX + CX - TW);
    pos.setY(DY - qRound(15.0*m_rwy) + fm.ascent());
    painter->drawText(pos, QString('^'));
  }
    break;
  case ExprDraw::ecTilde:{
    pos.setX(X + DX + CX - TW);
    pos.setY(DY - qRound(18.5*m_rwy) + fm.ascent());
    painter->drawText(pos, QString('~'));
  }
    break;
  case ExprDraw::ecLine:{
    QPen pen = painter->pen();
    QRect rec;
    rec.setLeft(LX);
    rec.setTop(DY - 2*m_wy);
    rec.setRight(RX - pen.width());
    rec.setBottom(DY - m_wy - pen.width());
    painter->drawRect(rec);
  }
    break;
  default:
    break;
  }

  painter->restore();
}
