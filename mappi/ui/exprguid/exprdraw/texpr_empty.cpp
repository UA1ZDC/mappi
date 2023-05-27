#include "exprdraw/texpr_empty.h"
#include <QFontMetrics>

TExpr_Empty::TExpr_Empty()
  :TExpr_Class()
{

}

int TExpr_Empty::CalcHeight()
{
  QFontMetrics fm(font());
  return fm.height();
}
