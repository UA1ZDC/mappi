#include "exprdraw/Parent/texpr_matrix.h"

TExpr_Matrix::TExpr_Matrix(TExpr_Class* _son,
                           int _horizontalSize, int _verticalSize)
  : TExpr_Parent(_son)
{
  m_hSize = _horizontalSize;
  m_vSize = _verticalSize;
}

void TExpr_Matrix::GetCellSize(int &CX, int &CY)
{
  CX = 0;

  int Over = 0;
  int Above = 0;

  TExpr_Class* P = son();
  while (P != 0) {
    Over = qMax(Over, P->midLineUp()+1);
    Above = qMax(Above, P->height() - P->midLineUp() - 1);
    CX = qMax(CX, P->width());
    P = P->next();
  }

  CY = Over + Above;
}

int TExpr_Matrix::GetCellWidth()
{
  if ((m_toChange & ExprDraw::tcCellSize) != 0){
    GetCellSize(m_CX, m_CY);
    m_toChange = m_toChange & ~ExprDraw::tcCellSize;
  }
  return m_CX;
}

int TExpr_Matrix::GetCellHeight()
{
  if ((m_toChange & ExprDraw::tcCellSize) != 0){
    GetCellSize(m_CX, m_CY);
    m_toChange = m_toChange & ~ExprDraw::tcCellSize;
  }
  return m_CY;
}

int TExpr_Matrix::CalcWidth()
{
  return GetCellWidth() * m_hSize + m_wx*(4 + 6*(m_hSize - 1));
}

int TExpr_Matrix::CalcHeight()
{
  return GetCellHeight() * m_vSize;
}

void TExpr_Matrix::Paint(QPainter *painter, int X, int Y)
{
  if (!hasSon())
    return;

  int CX;
  int CY;
  GetCellSize(CX, CY); // наверно эта функция за один раз пересчитает
                       // width и height, и сл. 2 функции выполнятся быстрее
  CX = GetCellWidth();
  CY = GetCellHeight();

  TExpr_Class* P = son();
  Y += CY / 2;

  for (int J = 0; J < m_vSize; ++J) {
    int DX = X + 2*m_wx + CX / 2;

    for (int I = 0; I < m_hSize; ++I) {
      if (P != 0){
        P->Draw(painter, DX, Y, ExprDraw::ehCenter, ExprDraw::evCenter);
        P = P->next();
        DX += CX + 6*m_wx;
      }
    }

    Y += CY;
  }
}
