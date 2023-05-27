#include "exprdraw/Parent/BigParent/texpr_bigparent.h"

TExpr_BigParent::TExpr_BigParent(TExpr_Class* _son, TExpr_Class* _daughter)
  :TExpr_Parent(_son)
{
  m_daughter = 0;
  SetDaughter(_daughter);
}

TExpr_BigParent::~TExpr_BigParent()
{
  delete m_daughter;
}

void TExpr_BigParent::SetDaughter(TExpr_Class *_daughter)
{
  delete m_daughter;
  m_daughter = _daughter;
  if (hasDaughter()){
    m_daughter->SetParent(this);
    SetDaughterFont();
    SetDaughterPaintDevice();
  }
  m_toChange = ExprDraw::CHANGED;
}

void TExpr_BigParent::DynaSetFont()
{
  TExpr_Parent::DynaSetFont();
  SetDaughterFont();
}

void TExpr_BigParent::DynaSetColor()
{
  TExpr_Parent::DynaSetColor();
  SetDaughterColor();
}

void TExpr_BigParent::DynaSetPaintDevice()
{
  TExpr_Parent::DynaSetPaintDevice();
  SetDaughterPaintDevice();
}

void TExpr_BigParent::SetDaughterFont()
{
  if (hasDaughter() && (m_daughter->font() != m_font))
    m_daughter->AssignFont(m_font,m_wx, m_wy, m_rwx, m_rwy);
}

void TExpr_BigParent::SetDaughterColor()
{
  if (hasDaughter())
    m_daughter->AssignColor(m_color);
}

void TExpr_BigParent::SetDaughterPaintDevice()
{
  if (hasDaughter() && (m_daughter->paintDevice() != m_paintDevice))
    m_daughter->AssignPaintDevice(m_paintDevice,m_wx, m_wy, m_rwx, m_rwy);
}

TExpr_Class* TExpr_BigParent::CutOffDaughter()
{
  TExpr_Class* result = m_daughter;
  m_daughter = 0;
  m_toChange = ExprDraw::CHANGED;
  return result;
}
