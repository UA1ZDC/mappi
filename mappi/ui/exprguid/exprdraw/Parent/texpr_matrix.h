#ifndef TEXPR_MATRIX_H
#define TEXPR_MATRIX_H

#include <mappi/ui/exprguid/exprdraw/Parent/texpr_parent.h>

class TExpr_Matrix : public TExpr_Parent
{
public:
  TExpr_Matrix(TExpr_Class* _son,
                        int _horizontalSize, int _verticalSize);
protected:
  void Paint(QPainter *painter, int X, int Y);
  int CalcWidth();
  int CalcHeight();

  void GetCellSize(int& CX, int& CY);
  int GetCellWidth();
  int GetCellHeight();
private:
  int m_hSize;
  int m_vSize;
  int m_CX;
  int m_CY;

};

#endif // TEXPR_MATRIX_H
