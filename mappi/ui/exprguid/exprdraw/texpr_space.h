#ifndef TEXPR_SPACE_H
#define TEXPR_SPACE_H

#include <mappi/ui/exprguid/exprdraw/texpr_class.h>

class TExpr_Space : public TExpr_Class
{
public:
  TExpr_Space(int _SpaceCount);
protected:
  int CalcWidth();
private:
  int m_spaceCount;
};

#endif // TEXPR_SPACE_H
