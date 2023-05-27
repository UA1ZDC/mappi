#include "exprdraw/Parent/TwinParent/texpr_twinparent.h"

#include <QFont>

TExpr_TwinParent::TExpr_TwinParent(TExpr_Class *_son,
                                   TExpr_Class *_firstTwin,
                                   TExpr_Class *_secondTwin)
  : TExpr_Parent(_son)
{
  m_firstTwin = 0;
  m_secondTwin = 0;

  setFirstTwin(_firstTwin);
  setSecondTwin(_secondTwin);
}

TExpr_TwinParent::~TExpr_TwinParent()
{
  delete m_firstTwin;
  delete m_secondTwin;
}

void TExpr_TwinParent::setFirstTwin(TExpr_Class *_firstTwin)
{
  delete m_firstTwin;
  m_firstTwin = _firstTwin;

  if (hasFirstTwin()){
    m_firstTwin->SetParent(this);
    m_firstTwin->SetPaintDevice(paintDevice());
    QFont fn = font();
    fn.setPointSize(qRound(0.7 * fn.pointSize()));
    m_firstTwin->SetFont(fn);
  }

  m_toChange = ExprDraw::CHANGED;
}

void TExpr_TwinParent::setSecondTwin(TExpr_Class *_secondTwin)
{
  delete m_secondTwin;
  m_secondTwin = _secondTwin;

  if (hasSecondTwin()){
    m_secondTwin->SetParent(this);
    m_secondTwin->SetPaintDevice(paintDevice());
    QFont fn = font();
    fn.setPointSize(qRound(0.7 * fn.pointSize()));
    m_secondTwin->SetFont(fn);
  }

  m_toChange = ExprDraw::CHANGED;
}

void TExpr_TwinParent::DynaSetFont()
{
  TExpr_Parent::DynaSetFont();
  QFont fn = font();
  fn.setPointSize(qRound(0.7 * fn.pointSize()));

  if (hasFirstTwin())
    m_firstTwin->SetFont(fn);
  if (hasSecondTwin())
    m_secondTwin->SetFont(fn);
}

void TExpr_TwinParent::DynaSetColor()
{
  TExpr_Parent::DynaSetColor();

  if (hasFirstTwin())
    m_firstTwin->SetColor(m_color);
  if (hasSecondTwin())
    m_secondTwin->SetColor(m_color);
}

void TExpr_TwinParent::DynaSetPaintDevice()
{
  TExpr_Parent::DynaSetPaintDevice();
  if (hasFirstTwin())
    m_firstTwin->SetPaintDevice(paintDevice());
  if (hasSecondTwin())
    m_secondTwin->SetPaintDevice(paintDevice());
}
