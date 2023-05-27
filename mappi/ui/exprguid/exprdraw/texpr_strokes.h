#ifndef TEXPR_STROKES_H
#define TEXPR_STROKES_H

#include <mappi/ui/exprguid/exprdraw/texpr_class.h>


class TExpr_Strokes : public TExpr_Class
{
public:
  TExpr_Strokes(uint _countStrokes);
private:
  uint m_countStrokes;
protected:
  void Paint(QPainter* painter, int X, int Y);
  int CalcWidth();
  int CalcHeight();
};

#endif // TEXPR_STROKES_H
