#include "exprdraw/texpr_space.h"

TExpr_Space::TExpr_Space(int _SpaceCount)
  :TExpr_Class()
{
  m_spaceCount = _SpaceCount;
}

int TExpr_Space::CalcWidth()
{
  return m_spaceCount * m_wx;
}
