#ifndef TEXPR_SIMPLE_H
#define TEXPR_SIMPLE_H

#include <mappi/ui/exprguid/exprdraw/texpr_class.h>

class QString;

class TExpr_Simple : public TExpr_Class
{
public:
  TExpr_Simple(const QString& expr);
  TExpr_Simple(const char _ch);
protected:
  QString m_text;
  void Paint(QPainter* painter, int x, int y);
  int CalcWidth();
  int CalcHeight();
  int CalcCapDY();
};

#endif // TEXPR_SIMPLE_H
