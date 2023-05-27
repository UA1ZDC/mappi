#include "exprdraw/Parent/TwinParent/texpr_index.h"

#include "exprdraw/Simple/texpr_funcname.h"

TExpr_Index::TExpr_Index(TExpr_Class *_son,
                         TExpr_Class *_firstTwin,
                         TExpr_Class *_secondTwin)
  : TExpr_TwinParent(_son, _firstTwin, _secondTwin)
{

}

int TExpr_Index::CalcWidth()
{
  int sW = 0;
  int sIndexX = 0;
  int sPowerX = 0;
  if (hasSon()){
    sW =son()->width();
    sIndexX = son()->indexXPos();
    sPowerX = son()->powerXPos();
  }

  int W1 = 0;
  if (hasFirstTwin())
    W1 = sIndexX + firstTwin()->width();

  int W2 = 0;
  if (hasSecondTwin())
    W2 = sPowerX + secondTwin()->width();

  sW = qMax(sW, W1);
  return qMax(sW, W2);
}

int TExpr_Index::CalcHeight()
{
  int sH = 0;
  int sIndexY = 0;
  int sPowerY = 0;
  if (hasSon()){
    sH =son()->height();
    sIndexY = son()->indexYPos();
    sPowerY = son()->powerYPos();
  }

  if (hasFirstTwin())
    sH += qMax(0, firstTwin()->height() - sH + sIndexY);
  if (hasSecondTwin())
    sH += qMax(0, secondTwin()->height() - sPowerY);
  return sH;
}

int TExpr_Index::CalcMidLine(ExprDraw::TExprOrigin origin)
{
  if (origin == ExprDraw::eoTop){
    int sMidUp = 0;
    int sPowerY = 0;
    if (hasSon()){
      sMidUp = son()->midLineUp();
      sPowerY = son()->powerYPos();
    }

    if (hasSecondTwin())
      return sMidUp + qMax(0, secondTwin()->height() - sPowerY);
    return sMidUp;
  }

  int sH = 0;
  int sMidDn = 0;
  int sIndexY = 0;
  if (hasSon()){
    sH = son()->height();
    sMidDn = son()->midLineDn();
    sIndexY = son()->indexYPos();
  }

  if (hasFirstTwin())
    return sMidDn - qMax(0, firstTwin()->height() - sH + sIndexY);
  return sMidDn;
}

int TExpr_Index::CalcCapDY()
{
  if (hasSecondTwin())
    return secondTwin()->capDY();

  if (hasSon())
    return son()->capDY();
  return 0;
}

bool TExpr_Index::ArgNeedBrackets()
{
  TExpr_FuncName* fName = dynamic_cast<TExpr_FuncName*>(son());
  return (fName == 0);
}

int TExpr_Index::FTType()
{
  int sFTT = hasSon() ? son()->FTType() : 0;
  return sFTT | ExprDraw::efRight;
}

void TExpr_Index::Paint(QPainter *painter, int X, int Y)
{
  int DY = 0;

  if (hasSecondTwin()){
    int sPowerX = 0;
    int sPowerY = 0;
    if (hasSon()){
      sPowerX = son()->powerXPos();
      sPowerY = son()->powerYPos();
    }

    DY = qMax(0, secondTwin()->height() - sPowerY);
    secondTwin()->Draw(painter, X + sPowerX, Y + DY + sPowerY,
                       ExprDraw::ehLeft, ExprDraw::evBottom);
  }

  if (hasSon())
    son()->Draw(painter, X, Y + DY, ExprDraw::ehLeft, ExprDraw::evTop);

  if (hasFirstTwin()){
    int sIndexX = 0;
    int sIndexY = 0;
    if (hasSon()){
      sIndexX = son()->indexXPos();
      sIndexY = son()->indexYPos();
    }

    firstTwin()->Draw(painter, X + sIndexX, Y + DY + sIndexY,
                      ExprDraw::ehLeft, ExprDraw::evTop);
  }
}
