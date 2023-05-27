#ifndef TEXPR_BRACKETED_H
#define TEXPR_BRACKETED_H

#include <mappi/ui/exprguid/exprdraw/Parent/Chain/texpr_chain.h>

#include <qchar.h>
#include <qfont.h>

class QPainter;

class TExpr_Bracketed : public TExpr_Chain
{
public:
  TExpr_Bracketed(TExpr_Class* _son,
                           ExprDraw::TExprBracket LeftBracket,
                           ExprDraw::TExprBracket RightBracket);
  int FTType();
  void removeBrackets();
protected:
  void Paint(QPainter* painter, int X, int Y);
  virtual bool isBracketed();
  int CalcCapDY();
  void CalcCapDX(int &DLeft, int &DRight);
  int CalcWidth();
  int CalcHeight();
  int CalcMidLine(ExprDraw::TExprOrigin origin);
private:
  enum Bracket {brLeft, brRigth};

  ExprDraw::TExprBracket m_leftBracket;
  ExprDraw::TExprBracket m_rightBracket;
  QChar m_leftCh;
  QChar m_rightCh;

  void DrawBracket(QPainter* painter, int X, int Y, int& dX, Bracket _bracket);
  QFont getBracketFont(const QChar& bracketChar);
};

#endif // TEXPR_BRACKETED_H
