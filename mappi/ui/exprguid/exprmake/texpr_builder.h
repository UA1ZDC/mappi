#ifndef TEXPR_BUILDER_H
#define TEXPR_BUILDER_H

#include <mappi/ui/exprguid/exprdraw/texpr_class.h>
#include <mappi/ui/exprguid/exprmake/texpr_error.h>

#include <QString>

class TExpr_Builder
{
public:
  TExpr_Builder();

  const TExpr_Error& lastError() const {return m_error;}

  TExpr_Class* BuildExpr(const QString& _expr);

  void setVariableAutoIndex(bool _value) {m_VarAutoIndex = _value;}
  void setFunctionAutoIndex(bool _value) {m_FuncAutoIndex = _value;}
  void setPostSymbols(bool _value) {m_PostSymbols = _value;}

  bool variableAutoIndex() {return m_VarAutoIndex;}
  bool functionAutoIndex() {return m_FuncAutoIndex;}
  bool postSymbols() {return m_PostSymbols;}

private:
  enum TExprGroupOpType {gtSumma, gtProd, gtCirc, gtInt, gtIntM};

  int m_Pos;       // Текущая позиция обрабатываемого символа в m_Expr
  QString m_Expr;  // Переданное выражение, обработанное функцией preprocess()
  TExpr_Error m_error;  // Последняя ошибка

  // m_VarAutoIndex = true: (по умолчанию)
  //   Если после буквы идут цифры, они будут распознаны как
  //   нижний индекс. (пример: a2 -> a₂)
  bool m_VarAutoIndex;

  // m_FuncAutoIndex = true: (по умолчанию)
  //   Если между буквой и скобкой '(' находятся цифры, они будут распознаны как
  //   нижний индекс. (пример: a2(x+1) -> a₂(x+1))
  bool m_FuncAutoIndex;

  // m_PostSymbols = true: (по умолчанию)
  // символы '^','_','!' и '`' установленные после буквы распазнаются как
  // 'верхний индекс', 'нижний индекс', 'факториал' и
  // 'черточка для обозначения производной' соответственно
  bool m_PostSymbols;

  bool isEnd() const {return m_Pos >= m_Expr.size();}

  QString preprocess(const QString& S) const;
  void SkipSpaces();
  TExpr_Class* ExprString(int _wrapper, bool AllowComma = false);
  TExpr_Class* Wrap(TExpr_Class* _base, int _wrapper, int _flags);
  TExpr_Class* BoolExpr(int& _flags);
  TExpr_Class* Expr(int& _flags);
  TExpr_Class* Trans(int& _flags);
  TExpr_Class* Factor(int& _flags);
  void  AddMult(TExpr_Class** _existing, TExpr_Class** _multiplier);
  void Val(const QString& _str, double& _result, int& _errPos);
  void Val(const QString& _str,int& __result, int& _errPos);
  TExpr_Class* MakeCap(TExpr_Class* _base, ExprDraw::TExprCapStyle _style, int _n);
  TExpr_Class* Func(int& _flags);
  void Decorate(TExpr_Class** _base);
  TExpr_Class* MakePower(TExpr_Class* _base, TExpr_Class* _exponent);
  TExpr_Class* MakeIndex(TExpr_Class* _base, TExpr_Class* _index);
  TExpr_Class* FuncName(const QString& _name, int& _flags, bool _needBrackets);
  TExpr_Class* Token(const QString& _name);
  int GreekLetter(const QString& _name);
  void LookForComma();
  bool Comma();
  void FindFuncIndex(QString& _name, int& _indx, bool& _wasIndex);

  TExpr_Class* fnLOG(const QString& _name, bool _needBrackets);
  TExpr_Class* fnPOW(int& _flags);
  TExpr_Class* fnROOT();
  TExpr_Class* fnIND();
  TExpr_Class* fnLIM(int& _flags, bool _needBrackets);
  TExpr_Class* fnFUNC(bool _needBrackets);
  TExpr_Class* fnSPACE();

  TExpr_Class* fnDIFF();
  TExpr_Class* fnPDIFF();
  TExpr_Class* fnDIFFN();
  TExpr_Class* fnPDIFFN();
  TExpr_Class* fnDIFFR();
  TExpr_Class* fnPDIFFR();
  TExpr_Class* fnDIFFRF();
  TExpr_Class* fnPDIFFRF();
  TExpr_Class* fnSTRING();
  TExpr_Class* fnSTROKES(int& _flags);
  TExpr_Class* fnFACT(int& _flags);
  TExpr_Class* fnAT();
  TExpr_Class* fnCAPTION(ExprDraw::TExprCapStyle _style);
  TExpr_Class* fnPOINTS();
  TExpr_Class* fnSTAND(ExprDraw::TExprHorAlign _align);
  TExpr_Class* fnMATRIX();
  TExpr_Class* fnGroupOp(TExprGroupOpType _type, bool _needBrackets);
  TExpr_Class* fnCASE();
  TExpr_Class* fnCOMMA();
  TExpr_Class* fnBRACKETS();
  TExpr_Class* fnSYSTEM();
  TExpr_Class* fnNUM();
  TExpr_Class* fnSYMBOL();
  TExpr_Class* fnANGLE();

  int GetErrorPos() const;
};

#endif // TEXPR_BUILDER_H
