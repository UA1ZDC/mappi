#include "exprdraw/Simple/texpr_customtext.h"

#include <QPainter>
#include <QFont>

TExpr_CustomText::TExpr_CustomText(const QString& expr,
                                   const int _fontStyles /*= fsBold*/,
                                   const QString& _fontFamily /*= QString()*/)
  : TExpr_Simple(expr)
{
  m_fontFamily = _fontFamily;
  m_fontStyles = _fontStyles;
}

QFont TExpr_CustomText::GetFont()
{
  QFont fn = m_font;
  if (!m_fontFamily.isEmpty())
    fn.setFamily(m_fontFamily);
  fn.setBold(m_fontStyles & ExprDraw::fsBold);
  fn.setItalic(m_fontStyles & ExprDraw::fsItalic);
  fn.setStrikeOut(m_fontStyles & ExprDraw::fsStrikeOut);
  fn.setUnderline(m_fontStyles & ExprDraw::fsUnderline);
  return fn;
}
