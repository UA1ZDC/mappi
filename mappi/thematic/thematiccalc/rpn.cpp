#include "rpn.h"

#include <qfile.h>

#include <cross-commons/debug/tlog.h>
#include <qchar.h>

namespace mappi {
namespace thematic {

RPN::RPN() { }
RPN::~RPN() { }

QList<Token> RPN::parsingFile(const QString& filename)
{
  QFile file(filename);
  if(false == file.open(QFile::ReadOnly)) {
    error_log << QObject::tr("Не удалось открыть файл %1").arg(filename);
    stack_.clear();
    return stack_;
  }
  data_ = file.readAll();
  file.close();
  return parsingExpression(data_);
}

QList<Token> RPN::parsingExpression(const QString& data) {
  data_ = data;
  size_ = data_.size();
  position_ = 0;
  lastChar_ = ' ';
  lastToken_ = { Token::Type::kNull, "" };
  stack_.clear();

  if(!nextChar()) return stack_;

  QList<Token> operations;
  do {
    switch (nextToken()) {
    case Token::Type::kNumber:
    case Token::Type::kVariable:
      stack_.append(lastToken_);
      continue;
    case Token::Type::kUnary:
    case Token::Type::kBinary:
      while(!operations.isEmpty()
            && ((lastToken_.isUnary() && operations.last().getPriority() > lastToken_.getPriority())
                || (lastToken_.isBinary() && operations.last().getPriority() >= lastToken_.getPriority())))
        stack_.append(operations.takeLast());
      operations.append(lastToken_);
      continue;
    case Token::Type::kLeftModule:
      operations.append(lastToken_);
      continue;
    case Token::Type::kRightModule:
      while(!operations.isEmpty() && !operations.last().isLeftModule())
        stack_.append(operations.takeLast());
      if(false == operations.isEmpty()) operations.pop_back();
      stack_.append({ Token::Type::kUnary, lastToken_.value });
      continue;
    case Token::Type::kLeftBracket:
      operations.append(lastToken_);
      continue;
    case Token::Type::kRightBracket:
      while(!operations.isEmpty() && !operations.last().isLeftBracket())
        stack_.append(operations.takeLast());
      if(false == operations.isEmpty()) operations.pop_back();
      continue;
    case Token::Type::kInvalide:
    default:
      error_log<<QObject::tr("Нераспознанный токен. %1").arg(lastToken_.value);
      stack_.clear();
      return stack_;
    }

  } while (nextChar());
  while(!operations.isEmpty()) stack_.append(operations.takeLast());

  return stack_;
}


bool RPN::nextChar()
{
  while(position_ < size_) {
    lastChar_ = data_.at(position_);
    if(lastChar_.isSpace())
      position_++;
    else
      break;
  }
  bool result = position_ < size_;
  position_++;
  return result;
}

Token::Type RPN::nextToken()
{
  if(isValid(lastChar_)) {
    if(lastChar_.isDigit()){                // Цифровая терма (Число)
      lastToken_ = { Token::Type::kNumber, getNumber() };
    }
    else if(lastChar_.isLetter()) {         // Текстовая терма ( функция или переменная)
      QString str = getText();
      if(isUnary(str))
        lastToken_ = { Token::Type::kUnary, Token::getUnary(str), str };
      else if(isBinary(str))
        lastToken_ = { Token::Type::kBinary, Token::getBinary(str), str };
      else
        lastToken_ = { Token::Type::kVariable, str };
    }
    else if(isUnaryOrBinary(lastChar_)) {   // Унарный или бинарный оператор (+ -)
      Token::Type type = Token::Type::kBinary;
      Token::Operation operation = Token::getBinary(lastChar_);
      // Операция считается унарной если оператор стоит в начале выражения или вначале подвыражения (-5) |-1|.
      // Случаи когда унарный оператор стоит сразу за бинарным 1+-5 или 2^-3 не рассматриваются,
      // следует использовать круглые скобки 1+(-5), 2^(-3)
      if(lastToken_.isNull() || lastToken_.isLeftBracket() || lastToken_.isLeftModule()) {
        type = Token::Type::kUnary;
        operation = Token::getUnary(lastChar_);
      }
      lastToken_ = { type, operation, lastChar_ };
    }
    else if(isBinary(lastChar_)) {          // Бинарный оператор
      lastToken_ = { Token::Type::kBinary, Token::getBinary(lastChar_), lastChar_ };
    }
    else if(isUnary(lastChar_)) {           // Унарный опрератор
      Token::Type type = Token::Type::kUnary;
      if('|' == lastChar_) {
        type = inModule_ ? Token::Type::kRightModule : Token::Type::kLeftModule;
        inModule_ = !inModule_;
      }
      lastToken_ = { type, Token::getUnary(lastChar_), lastChar_ };
    }
    else if(isLeftBracket(lastChar_)) {     // Левая скобка (
      lastToken_ = { Token::Type::kLeftBracket, lastChar_ };
    }
    else if(isRightBracket(lastChar_)) {    // Правая скобка )
      lastToken_ = { Token::Type::kRightBracket, lastChar_ };
    }
    else {                                  // Не распознанный символ
      lastToken_ = { Token::Type::kInvalide, lastChar_ };
    }
  }
  else {                                    // Не валидный символ
    lastToken_ = { Token::Type::kInvalide, lastChar_ };
  }
  return lastToken_.type;
}

QString RPN::getNumber()
{
  // Функция понимает 5 5,5 5.5 5e-1 5e+1 5e1 5E-1 5E+1 5E1
  // , или . выбираются из функции isComma(QChar)
  QString str;
  bool hasComma = false;
  do {
    str.append(lastChar_);
    if(false == nextChar()) break;
    if(isComma(lastChar_)) {
      if(false == hasComma)
        hasComma = true;
      else {
        error_log << QObject::tr("Ошибка записи числа. Более одного дисятичного знака. %1").arg(str);
        str.append('0');
        return str;
      }
      str.append('.');
      if(false == nextChar()) break;
    }
    else if('e' == lastChar_ || 'E' == lastChar_) {
      str.append(lastChar_);
      if(false == nextChar()) break;
      if('+' == lastChar_ || '-' == lastChar_) {
        str.append(lastChar_);
        if(false == nextChar()) break;
      }
      if(lastChar_.isDigit()) {
        str.append(lastChar_);
      }
      else {
        error_log << QObject::tr("Ошибка записи числа. Нет степени в эксоненциальной записи. %1").arg(str);
        str.append('1');
        return str;
      }
      if(false == nextChar()) break;
    }
  } while(lastChar_.isDigit());

  if(position_ <= size_) lastChar_ = data_.at(--position_);

  return str;
}

QString RPN::getText()
{
  QString str;
  do {
    str.append(lastChar_);
    if(false == nextChar()) break;
  } while (lastChar_.isLetterOrNumber());

  if(position_ <= size_) lastChar_ = data_.at(--position_);

  return str;
}

bool RPN::isValid(const QChar &ch) {
  return ch.isDigit()      || ch.isLetter()      || isComma(ch)
      || isLeftBracket(ch) || isRightBracket(ch)
      || isUnary(ch)       || isBinary(ch)       || isLogic(ch);
}
bool RPN::isLeftBracket(const QString &str) { return "(" == str; }
bool RPN::isRightBracket(const QString &str) { return ")" == str; }
bool RPN::isComma(const QChar& ch) { return ',' == ch || '.' == ch; }

bool RPN::isUnary(const QString &str) {
  return "sin" == str || "cos" == str || "tg" == str || "ctg" == str
      || "!" == str || "|" == str;
}
bool RPN::isUnaryOrBinary(const QString& str) { return "+" == str || "-" == str; }
bool RPN::isBinary(const QString &str) {
  return "<<" == str || ">>" == str || "and" == str || "or" == str || "xor" == str
      || "-" == str || "+" == str || "*" == str || "/" == str || "%" == str || "^" == str;
}
bool RPN::isLogic(const QString &str) {
  return "&&" == str || "||" == str || ">" == str || "<" == str || "<=" == str || ">=" == str
      || "==" == str || "!=" == str;
}

} //commons
} //mappi
