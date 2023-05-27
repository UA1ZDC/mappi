#include "exprdraw/Simple/texpr_var.h"

#include <QPainter>

TExpr_Var::TExpr_Var(const QString &expr)
  : TExpr_Simple(expr)
{

}

TExpr_Var::TExpr_Var(const char _ch)
  : TExpr_Simple(_ch)
{

}

void TExpr_Var::SetPainterFont(QPainter* painter)
{
  QFont fn = TExpr_Simple::font();
  fn.setItalic(true);
  painter->setFont(fn);
}

QFont TExpr_Var::GetFont()
{
  QFont fn = m_font;
  fn.setItalic(true);
  return fn;
}

void TExpr_Var::CalcCapDX(int &DLeft, int &DRight)
{
  double DX;
  ushort code = m_text.isEmpty() ? 0 : m_text.at(0).unicode();

  switch (code) {
  case 'A': case 'f':
    DX = 5.0;
    break;
  case 'B': case 'D': case 'E': case 'F': case 'L': case 'P': case 'R':
  case 'S': case 'T': case 'Y': case 'Z': case 'q': case 'r': case 's':
  case 't': case 'y': case 'z':
    DX =1.0;
    break;
  case 'U': case 'V': case 'W':
    DX = 1.5;
    break;
  case 'u': case 'v': case 'w':
    DX = 0.5;
    break;
  default:
    DX = 0.0;
    break;
  }

  if (DX == 0.0){
    if (((code >= 'a') && (code <= 'e')) ||
        ((code >= 'g') && (code <= 'o')))
      DX = 1.0;
  }
  DLeft = qRound(DX*m_rwx);

  code = m_text.isEmpty() ? 0 : m_text.at(m_text.size()-1).unicode();
  switch (code) {
  case 'A': case 'f':
    DX = 5.0;
    break;
  case 'B': case 'D': case 'E': case 'F': case 'I': case 'P': case 'R': case 'X':
    DX =4.0;
    break;
  case 'C': case 'G': case 'H': case 'J': case 'K': case 'L': case 'O': case 'Q':
  case 'S': case 'U': case 'V': case 'W': case 'Y': case 'Z': case 'p':
    DX = 2.0;
    break;
  case 'M': case 'N':
    DX = 1.5;
    break;
  case 'T':
    DX = 2.4;
    break;
  case 'g': case 'h': case 'k': case 'm': case 'n': case 'o': case 'q': case 'r':
  case 's':
    DX = 1.0;
    break;
  case 'i': case 'j': case 'l': case 't':
    DX = 3.0;
    break;
  default:
    DX = 0.0;
    break;
  }
  if (DX == 0.0){
    if (((code >= 'a') && (code <= 'e')) ||
        ((code >= 'u') && (code <= 'z')))
      DX = 1.0;
  }
  DRight = qRound(DX*m_rwx);
}

int TExpr_Var::CalcIndexXPos()
{
  double DX;
  ushort code = m_text.isEmpty() ? 0 : m_text.at(m_text.size()-1).unicode();

  switch (code) {
  case 'R':
    DX = 1.5;
    break;
  case 'W':
    DX = 6.0;
    break;
  case 'x':
    DX = 1.0;
    break;
  default:
    DX = 3.0;
    break;
  }
  return TExpr_Simple::CalcIndexXPos() - qRound(DX*m_rwx);
}

int TExpr_Var::CalcPowerXPos()
{
  double DX;
  ushort code = m_text.isEmpty() ? 0 : m_text.at(m_text.size()-1).unicode();

  switch (code) {
  case 'f':
  case 'd':
    DX = 2.0;
    break;
  case 'r':
    DX = 1.0;
    break;
  default:
    DX = 0.0;
    break;
  }
  return TExpr_Simple::CalcPowerXPos() + qRound(DX*m_rwx);
}
