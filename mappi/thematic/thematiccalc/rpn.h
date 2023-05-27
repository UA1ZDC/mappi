#ifndef MAPPI_THEMATIC_THEMATICCALC_RPN_H
#define MAPPI_THEMATIC_THEMATICCALC_RPN_H

#include <qlist.h>

namespace mappi {
namespace thematic {

struct Token {
  enum class Type : int
  { kInvalide,
    kNumber, kVariable, kNull, kExpression,
    kUnary, kBinary, kLeftModule, kRightModule, kLeftBracket, kRightBracket
  };
  enum class Operation : int
  {
    kUnk, kNot,
    kAdd, kSub, kMul, kDiv, kExp, kMod,
    kAbs, kFac, kSin, kCos, kTan, kCtg, kPos, kNeg
  };
  enum Priority
  {
    kAdditional = 1,  kSubstruct      = 1,  kMultiplication = 2,  kDivision  = 2,
    kModule     = 2,  kExponentiation = 3,  kAbs            = 10, kFactorial = 20,
    kSin        = 11, kCos            = 11, kTan            = 11, kCtg       = 11,
    kPositive   = 11, kNegation       = 11
  };

  Type type = Type::kInvalide;
  Operation operation = Operation::kNot;
  QString value;

  Token() {}
  Token(Type _type) : type(_type) { }
  Token(Type _type, QString _value) : type(_type), value(_value) {}
  Token(Type _type, Operation _operation) : type(_type), operation(_operation) {}
  Token(Type _type, Operation _operation, QString _value) : type(_type), operation(_operation), value(_value) {}

  bool isValid()        const { return Type::kInvalide     != type; }

  bool isNumber()       const { return Type::kNumber       == type; }
  bool isVariable()     const { return Type::kVariable     == type; }
  bool isNull()         const { return Type::kNull         == type; }
  bool isUnary()        const { return Type::kUnary        == type; }
  bool isBinary()       const { return Type::kBinary       == type; }

  bool isLeftModule()   const { return Type::kLeftModule   == type; }
  bool isRightModule()  const { return Type::kRightModule  == type; }
  bool isLeftBracket()  const { return Type::kLeftBracket  == type; }
  bool isRightBracket() const { return Type::kRightBracket == type; }

  bool isNot() const { return Operation::kNot == operation; }
  bool isUnk() const { return Operation::kUnk == operation; }
  bool isAdd() const { return Operation::kAdd == operation; }
  bool isSub() const { return Operation::kSub == operation; }
  bool isMul() const { return Operation::kMul == operation; }
  bool isDiv() const { return Operation::kDiv == operation; }
  bool isExp() const { return Operation::kExp == operation; }
  bool isMod() const { return Operation::kMod == operation; }

  bool isAbs() const { return Operation::kAbs == operation; }
  bool isFac() const { return Operation::kFac == operation; }
  bool isSin() const { return Operation::kSin == operation; }
  bool isCos() const { return Operation::kCos == operation; }
  bool isTan() const { return Operation::kTan == operation; }
  bool isCtg() const { return Operation::kCtg == operation; }
  bool isPos() const { return Operation::kPos == operation; }
  bool isNeg() const { return Operation::kNeg == operation; }

  static Operation getBinary(QString str) {
         if("+" == str) return Operation::kAdd;
    else if("-" == str) return Operation::kSub;
    else if("*" == str) return Operation::kMul;
    else if("/" == str) return Operation::kDiv;
    else if("^" == str) return Operation::kExp;
    else if("%" == str) return Operation::kMod;
    else                return Operation::kUnk;
  }
  static Operation getUnary(QString str) {
         if("|"   == str) return Operation::kAbs;
    else if("!"   == str) return Operation::kFac;
    else if("sin" == str) return Operation::kSin;
    else if("cos" == str) return Operation::kCos;
    else if("tg"  == str) return Operation::kTan;
    else if("ctg" == str) return Operation::kCtg;
    else if("+"   == str) return Operation::kPos;
    else if("-"   == str) return Operation::kNeg;
    else                  return Operation::kUnk;
  }

  int getPriority()
  {
    if(isBinary()) {
           if(value == "+") return Priority::kAdditional;
      else if(value == "-") return Priority::kSubstruct;
      else if(value == "*") return Priority::kMultiplication;
      else if(value == "/") return Priority::kDivision;
      else if(value == "%") return Priority::kModule;
      else if(value == "^") return Priority::kExponentiation;
    }
    else if(isUnary()) {
           if(value == "!")   return Priority::kFactorial;
      else if(value == "|")   return Priority::kAbs;
      else if(value == "+")   return Priority::kPositive;
      else if(value == "-")   return Priority::kNegation;
      else if(value == "sin") return Priority::kSin;
      else if(value == "cos") return Priority::kCos;
      else if(value == "tan")  return Priority::kTan;
      else if(value == "ctg") return Priority::kCtg;
    }
    return 0;
  }
  bool isLeftAssociate()
  {
    if(isBinary()) {
           if(value == "+") return true;
      else if(value == "-") return true;
      else if(value == "*") return true;
      else if(value == "/") return true;
      else if(value == "%") return true;
      else if(value == "^") return true;
    }
    else if(isUnary()) {
           if(value == "!")   return true;
      else if(value == "-")   return false;
      else if(value == "+")   return false;
      else if(value == "sin") return false;
      else if(value == "cos") return false;
      else if(value == "tg")  return false;
      else if(value == "ctg") return false;
    }
    return false;
  }

};

class RPN
{
public:
  explicit RPN();
  ~RPN();

  QList<Token> parsingFile(const QString& filename);
  QList<Token> parsingExpression(const QString& data_);

  QList<Token> getParsedExpression() { return stack_; }

private:
  static bool isValid        (const QChar&   ch );
  static bool isLeftBracket  (const QString& str);
  static bool isRightBracket (const QString& str);
  static bool isComma        (const QChar&   ch );

  static bool isUnary        (const QString& str);
  static bool isUnaryOrBinary(const QString& str);
  static bool isBinary       (const QString& str);
  static bool isLogic        (const QString &str);

  static int getPriority(const Token& operation);

private:
  bool nextChar();
  Token::Type nextToken();

  QString getNumber();
  QString getText();

private:
  bool inModule_ = false;
  size_t size_ = 0;
  size_t position_ = 0;

  QChar lastChar_;
  Token lastToken_;
  QString data_;

  QList<Token> stack_;
};

} //thematic
} //mappi

#endif // MAPPI_THEMATIC_THEMATICCALC_RPN_H
