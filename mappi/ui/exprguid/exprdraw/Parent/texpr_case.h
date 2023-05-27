#ifndef TEXPR_CASE_H
#define TEXPR_CASE_H

#include <mappi/ui/exprguid/exprdraw/Parent/texpr_parent.h>


class TExpr_Case : public TExpr_Parent
{
public:
  TExpr_Case(TExpr_Class* _son);
protected:
  int CalcWidth();
  int CalcHeight();
  void Paint(QPainter* painter, int X, int Y);
};

#endif // TEXPR_CASE_H
