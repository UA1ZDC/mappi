#include "texpr_error.h"
#include "QObject"

using namespace ExprMake;

TExpr_Error::TExpr_Error()
{
  m_number = erNull;
}

void TExpr_Error::SetNumber(int _number, int _arg)
{
  m_number = _number;
  m_errPos = _arg;

  switch (m_number) {
  case erNull:
    m_text.clear();
    break;
  case erExpectedDigit:
    m_text = QObject::tr("Ожидается цифра в позиции %1!").arg(_arg);
    break;
  case erExpectedSquareBracket:
    m_text = QObject::tr("Ожидается \"]\" в позиции %1!").arg(_arg);
    break;
  case erExpectedFigureBracket:
    m_text = QObject::tr("Ожидается \"}\" в позиции %1!").arg(_arg);
    break;
  case erExpectedRoundBracket:
    m_text = QObject::tr("Ожидается \")\" в позиции %1!").arg(_arg);
    break;
  case erExpectedModuleBracket:
    m_text = QObject::tr("Ожидается \"|\" в позиции %1!").arg(_arg);
    break;
  case erSymbol:
    m_text = QObject::tr("Недопустимый символ в позиции %1!").arg(_arg);
    break;
  case erUnfinishedString:
    m_text = QObject::tr("Незавершённая строка!");
    break;
  case erExpectedOpenBracket:
    m_text = QObject::tr("Ожидается знак открывающей скобки в позиции %1!").arg(_arg);
    break;
  case erExpectedCloseBracket:
    m_text = QObject::tr("Ожидается знак закрывающей скобки в позиции %1!").arg(_arg);
    break;
  case erExpectedNumber:
    m_text = QObject::tr("Ожидается число в позиции %1!").arg(_arg);
    break;
  case erExpectedComma:
    m_text = QObject::tr("Ожидается \",\" в позиции %1!").arg(_arg);
    break;
  default:
    m_text.clear();
    break;
  }
}
