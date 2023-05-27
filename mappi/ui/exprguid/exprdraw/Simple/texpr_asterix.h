#ifndef TEXPR_ASTERIX_H
#define TEXPR_ASTERIX_H

#include <mappi/ui/exprguid/exprdraw/Simple/texpr_simple.h>

class TExpr_Asterix : public TExpr_Simple
{
public:
  TExpr_Asterix();
protected:
  void Paint(QPainter* painter, int X, int Y);
};

#endif // TEXPR_ASTERIX_H
