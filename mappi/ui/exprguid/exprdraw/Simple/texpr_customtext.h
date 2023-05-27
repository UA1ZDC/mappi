#ifndef TEXPR_CUSTOMTEXT_H
#define TEXPR_CUSTOMTEXT_H

#include <mappi/ui/exprguid/exprdraw/Simple/texpr_simple.h>

#include <QString>

class TExpr_CustomText : public TExpr_Simple
{
public:
  TExpr_CustomText(const QString& expr,
                            const int _fontStyles = ExprDraw::fsBold,
                            const QString& _fontFamily = QString());
protected:
  QFont GetFont();
private:
  int  m_fontStyles;
  QString m_fontFamily;
};

#endif // TEXPR_CUSTOMTEXT_H
