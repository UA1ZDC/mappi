#ifndef TEXPR_BIGPARENT_H
#define TEXPR_BIGPARENT_H

#include <mappi/ui/exprguid/exprdraw/Parent/texpr_parent.h>


class TExpr_BigParent : public TExpr_Parent
{
public:
  TExpr_BigParent(TExpr_Class* _son, TExpr_Class* _daughter);
  virtual ~TExpr_BigParent();
  void SetDaughter(TExpr_Class* _daughter);
  TExpr_Class* daughter() {return m_daughter;}
  TExpr_Class* CutOffDaughter();

protected:
  void DynaSetFont();
  void DynaSetColor();
  void DynaSetPaintDevice();
  bool hasDaughter() const   {return m_daughter != 0;}

private:
  TExpr_Class* m_daughter;
  virtual void SetDaughterFont();
  virtual void SetDaughterColor();
  virtual void SetDaughterPaintDevice();
};

#endif // TEXPR_BIGPARENT_H
