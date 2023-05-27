#ifndef TEXPR_PARENT_H
#define TEXPR_PARENT_H

#include <mappi/ui/exprguid/exprdraw/texpr_class.h>

class TExpr_Parent : public TExpr_Class
{
public:
  TExpr_Parent(TExpr_Class* _son);
  virtual ~TExpr_Parent();
  void SetSon(TExpr_Class* _son);
  TExpr_Class* son() {return m_son;}
  TExpr_Class* CutOffSon();
  bool hasSon() const   {return m_son != 0;}
protected:
  void DynaSetFont();
  void DynaSetColor();
  void DynaSetPaintDevice();
private:
  TExpr_Class* m_son;
  virtual void SetSonFont();
  virtual void SetSonColor();
  virtual void SetSonPaintDevice();
};

#endif // TEXPR_PARENT_H
