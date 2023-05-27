#include "exprdraw/Parent/texpr_parent.h"


TExpr_Parent::TExpr_Parent(TExpr_Class* _son)
  : TExpr_Class()
{
  m_son = 0;
  SetSon(_son);
}

TExpr_Parent::~TExpr_Parent()
{
  delete m_son;
}

void TExpr_Parent::SetSon(TExpr_Class *_son)
{
  delete m_son;
  m_son = _son;

  if (hasSon()){
    m_son->SetParent(this);
    SetSonFont();
    SetSonPaintDevice();
  }    
  m_toChange = ExprDraw::CHANGED;
}

void TExpr_Parent::DynaSetFont()
{
  TExpr_Class::DynaSetFont();
  SetSonFont();
}

void TExpr_Parent::DynaSetColor()
{
  TExpr_Class::DynaSetColor();
  SetSonColor();
}

void TExpr_Parent::DynaSetPaintDevice()
{
  TExpr_Class::DynaSetPaintDevice();
  SetSonPaintDevice();
}

void TExpr_Parent::SetSonFont()
{
  if (hasSon() && (m_son->font() != m_font))
    m_son->AssignFont(m_font,m_wx, m_wy, m_rwx, m_rwy);
}

void TExpr_Parent::SetSonColor()
{
  if (hasSon())
    m_son->AssignColor(m_color);
}

void TExpr_Parent::SetSonPaintDevice()
{
  if (hasSon() && (m_paintDevice != m_son->paintDevice()))
    m_son->AssignPaintDevice(m_paintDevice,m_wx, m_wy, m_rwx, m_rwy);
}

TExpr_Class* TExpr_Parent::CutOffSon()
{
  TExpr_Class* result = m_son;
  m_son = 0;
  m_toChange = ExprDraw::CHANGED;
  return result;
}
