#ifndef TEXPR_ERROR_H
#define TEXPR_ERROR_H

#include <QString>

namespace ExprMake {
  const int erNull                  = 0;
  const int erExpectedDigit         = 1;

  const int erExpectedSquareBracket = 2;
  const int erExpectedFigureBracket = 3;
  const int erExpectedRoundBracket  = 4;
  const int erExpectedModuleBracket = 5;

  const int erSymbol                = 6;
  const int erUnfinishedString      = 7;

  const int erExpectedOpenBracket   = 8;
  const int erExpectedCloseBracket  = 9;

  const int erExpectedNumber        = 10;
  const int erExpectedComma         = 11;
}

class TExpr_Error
{
public:
  TExpr_Error();
  //TExpr_Error(const TExpr_Error& other);

  bool isValid() const {return m_number != ExprMake::erNull;}
  QString text() const {return m_text;}
  int number() const   {return m_number;}
  int errorPos() const {return m_errPos;}
  void SetNumber(int _number, int _arg = 0);
private:
  QString m_text;
  int m_number;
  int m_errPos;
};

#endif // TEXPR_ERROR_H
