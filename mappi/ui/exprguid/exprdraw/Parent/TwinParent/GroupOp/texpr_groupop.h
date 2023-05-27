#ifndef TEXPR_GROUPOP_H
#define TEXPR_GROUPOP_H

#include <mappi/ui/exprguid/exprdraw/Parent/TwinParent/texpr_twinparent.h>
#include <qchar.h>
#include <qfont.h>

class TExpr_GroupOp : public TExpr_TwinParent
{
public:
  TExpr_GroupOp(TExpr_Class* _son,
                         TExpr_Class* _firstTwin,
                         TExpr_Class* _secondTwin);
protected:
  QChar m_ch;
  QFont m_symbol_font;

  void DynaSetFont();
  int CalcSymbolHeight();
  virtual int CalcSymbolWidth();
  virtual void DrawSymbol(QPainter* painter, int X, int Y);
  void Paint(QPainter* painter, int X, int Y);
  int GetSymbolWidth();
  int GetSymbolHeight();
  int CalcWidth();
  int CalcHeight();
  int CalcMidLine(ExprDraw::TExprOrigin origin);
private:
  int m_symbolHeight;
  int m_symbolWidth;
};

#endif // TEXPR_GROUPOP_H
