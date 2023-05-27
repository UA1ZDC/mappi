#ifndef TEXPR_TWINPARENT_H
#define TEXPR_TWINPARENT_H

#include <mappi/ui/exprguid/exprdraw/Parent/texpr_parent.h>

class TExpr_TwinParent : public TExpr_Parent
{
public:
  TExpr_TwinParent(TExpr_Class* _son,
                            TExpr_Class* _firstTwin,
                            TExpr_Class* _secondTwin);
  virtual ~TExpr_TwinParent();

  TExpr_Class* firstTwin() const {return m_firstTwin;}
  TExpr_Class* secondTwin() const {return m_secondTwin;}

  bool hasFirstTwin() const {return m_firstTwin != 0;}
  bool hasSecondTwin() const {return m_secondTwin != 0;}
  void setFirstTwin(TExpr_Class* _firstTwin);
  void setSecondTwin(TExpr_Class* _secondTwin);
protected:
  void DynaSetFont();
  void DynaSetColor();
  void DynaSetPaintDevice();
private:
  TExpr_Class* m_firstTwin;
  TExpr_Class* m_secondTwin;  
};

#endif // TEXPR_TWINPARENT_H
