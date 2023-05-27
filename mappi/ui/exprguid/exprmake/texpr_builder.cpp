#include "texpr_builder.h"

#include "exprdraw/expr_draw.h"

#include <QMap>
#include <QRegExp>
#include <QVector>
#include <QObject>

using namespace ExprDraw;
using namespace ExprMake;

// "обертки" - для функции Wrap() (может enum TExprWrapper ???)
const int EChain      = 0;
const int EExpression = 1;
const int EBracketed  = 2;
const int EArgument   = 3;
const int EPower      = 4;
const int EAbs        = 5;
const int ESquared    = 6;
const int EFigured    = 7;

// доп флаги для "обертки"
const int flgPower   = 1;
const int flgTrans   = 3;

// ***********************************************
const ushort bsNotEqual           = 1;  // ≠  <>
const ushort bsGreaterOrEqual     = 2;  // ≥  >=
const ushort bsLessOrEqual        = 3;  // ≤  <=
const ushort bsMuchGreater        = 4;  // ≫ >>
const ushort bsMuchLess           = 5;  // ≪ <<
const ushort bsArrow              = 6;  // →  ->
const ushort bsApproxEqual        = 7;  // ≈  ~~
const ushort bsPlusMinus          = 8;  // ±  +-
const ushort bsMinusPlus          = 9;  // ∓  -+
const ushort bsEquivalent         = 10; // ≡  ==
const ushort bsDivide             = 11; // ÷  /+
const ushort bsMultiplyCross      = 12; // x  *+
const ushort bsEllipsis           = 13; //   ...
const ushort bsApproxGreater      = 14; // ≳ >~
const ushort bsApproxLess         = 15; // ≲ <~
const ushort bsDivideSlash        = 16; // / //
const ushort bsNecessarilyBracket = 17; // () !() обязательная скобка (закрывается обычной)
const ushort bsMultiplyDot        = 18; // · *.
const ushort bsMultiply           = 19; //   ** умножение без перестановки множителей (ab)
const ushort bsAlmostEqual        = 20; // ≃ =~
const ushort bsAmpersand          = 21; // & ' & ' только ради правильного определения позиции символа с ошибкой
// ***********************************************

const ushort bsPlambda = 1;  // для GreekLetter
const ushort bsNabla   = 2;

TExpr_Builder::TExpr_Builder()
{
  m_VarAutoIndex = true;
  m_FuncAutoIndex = true;
  m_PostSymbols = true;
}

TExpr_Class* TExpr_Builder::BuildExpr(const QString &_expr)
{
  if (_expr.isEmpty())
    return new TExpr_Empty();

  m_Expr = preprocess(_expr);
  m_Expr.append(' ');

  m_Pos = 0;

  return ExprString(EExpression, true);
}

QString TExpr_Builder::preprocess(const QString& S) const
{
  int i = 0;
  QString result;

  while (i < S.size()) {
    ushort ch = S.at(i).unicode();
    ushort ch_next = i+1 < S.size() ? S.at(i+1).unicode(): 0;

    switch (ch) {
    case '<':
      switch (ch_next) {
      case '<':                 // << - много меньше
        result.append(bsMuchLess);
        ++i;
        break;
      case '>':                 // <> - не равно
        result.append(bsNotEqual);
        ++i;
        break;
      case '=':                 // <= - меньше или равно
        result.append(bsLessOrEqual);
        ++i;
        break;
      case '~':                 // <~ - меньше или порядка
        result.append(bsApproxLess);
        ++i;
        break;
      default:
        result.append(ch);
        break;
      }
      break;

    case '>':
      switch (ch_next) {
      case '=':                 // >= - больше или равно
        result.append(bsGreaterOrEqual);
        ++i;
        break;
      case '>':                 // >> - много больше
        result.append(bsMuchGreater);
        ++i;
        break;
      case '~':                 // >~ - больше или порядка
        result.append(bsApproxGreater);
        ++i;
        break;
      default:
        result.append(ch);
        break;
      }
      break;

    case '=':
      switch (ch_next) {
      case '=':                 // == - тождественно
        result.append(bsEquivalent);
        ++i;
        break;
      case '~':                 // =~ - знак равенства с тильдой сверху
        result.append(bsAlmostEqual);
        ++i;
        break;
      default:
        result.append(ch);
        break;
      }
      break;

    case '~':
      if (ch_next == '~'){      // ~~ - примерно равно
        result.append(bsApproxEqual);
        ++i;
      }else
        result.append(ch);
      break;

    case '+':
      if (ch_next == '-'){      // +- - плюс-минус
        result.append(bsPlusMinus);
        ++i;
      }else
        result.append(ch);
      break;

    case '-':
      switch (ch_next) {
      case '+':                 // -+ - минус-плюс
        result.append(bsMinusPlus);
        ++i;
        break;
      case '>':                 // -> - стрелка (стремится к пределу)
        result.append(bsArrow);
        ++i;
        break;
      default:
        result.append(ch);
        break;
      }
      break;

    case '/':
      switch (ch_next) {
      case '/':                 // // - делить символом "/", без дроби
        result.append(bsDivideSlash);
        ++i;
        break;
      case '+':                 // /+ - знак ÷
        result.append(bsDivide);
        ++i;
        break;
      default:
        result.append(ch);
        break;
      }
      break;

    case '*':
      switch (ch_next) {
      case '+':                 // *+ - косой крест (a x b)
        result.append(bsMultiplyCross);
        ++i;
        break;
      case '*':                 // ** - умножение без перестановки множителей (ab)
        result.append(bsMultiply);
        ++i;
        break;
      case '.':                 // *. - умножение точкой (a · b)
        result.append(bsMultiplyDot);
        ++i;
        break;
      default:
        result.append(ch);
        break;
      }
      break;

    case '!':
      switch (ch_next) {
      case '(':                         // !( - обязательная скобка (закрывается обычной)
        result.append(bsNecessarilyBracket);
        ++i;
        break;
      case '=':                         // не равно в стиле C
        result.append(bsNotEqual);
        ++i;
        break;
      default:
        result.append(ch);
        break;
      }
      break;

    case ' ':{
      ushort ch_next_next = i+2 < S.size() ? S.at(i+2).unicode() : 0;
      if ((ch_next == '&') & (ch_next_next == ' ')){ // " & " - то же самое, что и "&"
        //result.append('&');
        result.append(bsAmpersand);
        i += 2;
      }
      else
        result.append(ch);
    }
      break;

    case '.':{
      ushort ch_next_next = i+2 < S.size() ? S.at(i+2).unicode() : 0;
      if ((ch_next == '.') & (ch_next_next == '.')){  // ... - эллипсис
        result.append(bsEllipsis);
        i += 2;
      }
      else
        result.append(ch);
    }
      break;

    default:
      result.append(ch);
      break;
    }
    ++i;
  }

  return result;
}

void TExpr_Builder::SkipSpaces()
{
  // Пропускаем пробелы после ','
  int size = m_Expr.size();
  while ((m_Pos < size) &&
         (m_Expr.at(m_Pos).unicode() == ' ')) {
    ++m_Pos;
  }
  if (m_Pos >= size)
    m_Pos = size-1;
}

TExpr_Class* TExpr_Builder::ExprString(int _wrapper, bool AllowComma)
{
  int flags;
  TExpr_Class* result = BoolExpr(flags);

  if (m_error.isValid() || isEnd())
    return Wrap(result, _wrapper, flags);

  ushort ch = m_Expr.at(m_Pos).unicode();
  while ((ch == '&') || (ch == bsAmpersand) || ((ch == ',') & AllowComma)) {
    ushort sep = m_Expr.at(m_Pos).unicode();
    ++m_Pos;

    if (sep == ',')  // Пропускаем пробелы после ','
      SkipSpaces();

    TExpr_Class* A = BoolExpr(flags);
    if ((sep == '&') || (sep == bsAmpersand))
      result->AddNext(A);
    else{
      result->AddNext(new TExpr_Comma());
      result->AddNext(new TExpr_Space(7));
      result->AddNext(A);
    }

    if (m_error.isValid() || isEnd())
      break;

    ch = m_Expr.at(m_Pos).unicode();
  }

  return Wrap(result, _wrapper, flags);
}

TExpr_Class* TExpr_Builder::Wrap(TExpr_Class *_base, int _wrapper, int _flags)
{
  // Возвращаем _base как бы в "обертке"
  // В качестве "обертки" используются TExpr_Chain(для вывода цепочки выражений)
  // и различные скобки

//  if (_base == 0)
//    return new TExpr_Empty;

  switch (_wrapper) {
  case EExpression:
    if (_base->hasNext())
      return new TExpr_Chain(_base);
    break;

  case EBracketed:
    return new TExpr_Round(_base);
    break;

  case EArgument:
    if (_base->hasNext())
      return new TExpr_Argument(_base);
    break;

  case EPower:
    if (_base->hasNext())
      return new TExpr_Base(_base);
    else
      if ((_flags & flgPower) == flgPower)
        return new TExpr_Bracketed(_base, ebRound, ebRound);
    break;

  case EAbs:
    return new TExpr_Bracketed(_base, ebModule, ebModule);
    break;

  case ESquared:
    return new TExpr_Bracketed(_base, ebSquare, ebSquare);
    break;

  case EFigured:
    return new TExpr_Bracketed(_base, ebFigure, ebFigure);
    break;
  }

  return _base;
}

TExpr_Class* TExpr_Builder::BoolExpr(int &_flags)
{
  int flags;
  TExpr_Class* result = Expr(flags);

  if (m_error.isValid() || isEnd())
    return result;

  QMap<ushort, unsigned int> map;
  map.insert(bsNotEqual, esNotEqual);
  map.insert(bsGreaterOrEqual, esGreaterOrEqual);
  map.insert(bsLessOrEqual, esLessOrEqual);
  map.insert(bsMuchGreater, esMuchGreater);
  map.insert(bsMuchLess, esMuchLess);
  map.insert(bsArrow, esArrow);
  map.insert(bsApproxEqual, esApproxEqual);
  map.insert(bsEquivalent, esEquivalent);
  map.insert(bsApproxGreater, esApproxGreater);
  map.insert(bsApproxLess, esApproxLess);
  map.insert(bsAlmostEqual, esAlmostEqual);
  map.insert('<', esLess);
  map.insert('=', esEqual);
  map.insert('>', esGreater);
  map.insert('~', esTilde);

  ushort ch = m_Expr.at(m_Pos).unicode();
  while (map.contains(ch)) {
    int sign = map.value(ch);
    ++m_Pos;

    TExpr_Class* A = Expr(flags);
    result->AddNext(new TExpr_Sign(sign));
    result->AddNext(A);

    if (m_error.isValid() || isEnd())
      break;

    ch = m_Expr.at(m_Pos).unicode();
  }

  if (result->hasNext())
    _flags = flgPower;
  else
    _flags = flags;

  return result;
}

TExpr_Class* TExpr_Builder::Expr(int &_flags)
{
  int flags;
  TExpr_Class* result = Trans(flags);

  if (m_error.isValid() || isEnd())
    return result;

  QMap<ushort, unsigned int> map;
  map.insert(bsPlusMinus, esPlusMinus);
  map.insert(bsMinusPlus, esMinusPlus);
  map.insert('-', esMinus);
  map.insert('+', esPlus);

  ushort ch = m_Expr.at(m_Pos).unicode();
  while (map.contains(ch)) {
    int sign = map.value(ch);
    ++m_Pos;

    TExpr_Class* A = Trans(flags);
    if ((flags & flgTrans) == flgTrans)
      A = new TExpr_Bracketed(A, ebRound, ebRound);
    result->AddNext(new TExpr_Sign(sign));
    result->AddNext(A);

    if (m_error.isValid() || isEnd())
      break;

    ch = m_Expr.at(m_Pos).unicode();
  }

  if (result->hasNext())
    _flags = flgPower;
  else
    _flags = flags;

  return result;
}

TExpr_Class* TExpr_Builder::Trans(int &_flags)
{
  int flags;
  TExpr_Class* D1 = Factor(flags);
  TExpr_Class* D2 = 0;

  if (m_error.isValid() || isEnd())
    return D1;

  QMap<ushort, unsigned int> map;
  map.insert(bsDivide, esDivide);
  map.insert(bsMultiplyCross, esCrossMultiply);
  map.insert(bsDivideSlash, esSlash);
  map.insert(bsMultiplyDot, esMultiply);
  map.insert(bsMultiply, 0);
  map.insert('*', 0);
  map.insert('/', 0);

  ushort ch = m_Expr.at(m_Pos).unicode();
  while (map.contains(ch)) {
    int sign = map.value(ch);
    ++m_Pos;

    switch (ch) {
    case bsMultiply:{
      TExpr_Class* A = Factor(flags);
      D1->AddNext(A);
    }
      break;

    case '*':{
      //D1 = AddMult(D1, Factor(flags));
      TExpr_Class* A = Factor(flags);
      AddMult(&D1, &A);
    }break;

    case '/':{
      //D2 = AddMult(D2, Factor(flags));
      TExpr_Class* A = Factor(flags);
      AddMult(&D2, &A);
    }break;

    default:{
      TExpr_Class* A = Factor(flags);
      D1->AddNext(new TExpr_Sign(sign));
      D1->AddNext(A);
    }
      break;
    }

    if (m_error.isValid() || isEnd())
      break;

    ch = m_Expr.at(m_Pos).unicode();
  }

  flags = 0;
  if ((D2 == 0) & !D1->hasNext())
    _flags = flags;

  if (D2 != 0){               // D2 только для дроби
    if (D1->hasNext())
      D1 = new TExpr_Chain(D1);
    if (D2->hasNext())
      D2 = new TExpr_Chain(D2);

    _flags = flgPower;
    if ((D1->FTType() & efRoundBrackets) == efRoundBrackets){
      TExpr_Bracketed* B = dynamic_cast<TExpr_Bracketed*>(D1);
      if (B != 0) B->removeBrackets();
    }
    if ((D2->FTType() & efRoundBrackets) == efRoundBrackets){
      TExpr_Bracketed* B = dynamic_cast<TExpr_Bracketed*>(D2);
      if (B != 0) B->removeBrackets();
    }
    return new TExpr_Ratio(D1, D2);
  }
  else
    return D1;
}

TExpr_Class* TExpr_Builder::Factor(int &_flags)
{
  _flags = 0;
  m_error.SetNumber(erNull);
  TExpr_Class* result = 0;
  int D;

  //  if (isEnd())                   // врятле здесь будет конец строки
  //    return new TExpr_Empty();    // ????

  QChar ch = m_Expr.at(m_Pos);
  switch (ch.unicode()) {
  case '0': case '1': case '2': case '3': case '4':
  case '5': case '6': case '7': case '8': case '9':{
    double R;
    int J;
    Val(m_Expr.mid(m_Pos),R,D);
    J = D;
    Val(m_Expr.mid(m_Pos,J-1),R,D);
    result = new TExpr_Number(R,false);
    m_Pos += J-1;
  }break;

  case '#':{
    ++m_Pos;
    if (m_Expr.at(m_Pos).isDigit()){
      double R;
      int J;
      Val(m_Expr.mid(m_Pos),R,D);
      J = D;
      Val(m_Expr.mid(m_Pos,J-1),R,D);
      result = new TExpr_Number(R,true);
      m_Pos += J-1;
    }else
      m_error.SetNumber(erExpectedNumber, GetErrorPos());
  }break;

  case '+':{
    ++m_Pos;
    _flags = flgTrans;
    result = new TExpr_Sign(esPlus);
    result->AddNext(Factor(D));
  }break;

  case '-':{
    ++m_Pos;
    result = new TExpr_Sign(esMinus);
    result->AddNext(Factor(D));
  }break;

  case bsPlusMinus:{
    ++m_Pos;
    _flags = flgTrans;
    result = new TExpr_Sign(esPlusMinus);
    result->AddNext(Factor(D));
  }break;

  case bsMinusPlus:{
    ++m_Pos;
    _flags = flgTrans;
    result = new TExpr_Sign(esMinusPlus);
    result->AddNext(Factor(D));
  }break;

  case '[':{
    ++m_Pos;
    result = ExprString(ESquared, true);
    if (m_Expr.at(m_Pos).unicode() == ']')
      ++m_Pos;
    else
      m_error.SetNumber(erExpectedSquareBracket, GetErrorPos());
  }break;

  case '{':{
    ++m_Pos;
    result = ExprString(EFigured, true);
    if (m_Expr.at(m_Pos).unicode() == '}')
      ++m_Pos;
    else
      m_error.SetNumber(erExpectedFigureBracket, GetErrorPos());
  }break;

  case '(':{
    ++m_Pos;
    result = ExprString(EArgument, true);
    if (m_Expr.at(m_Pos).unicode() == ')')
      ++m_Pos;
    else
      m_error.SetNumber(erExpectedRoundBracket, GetErrorPos());
  }break;

  case '|':{
    ++m_Pos;
    result = ExprString(EAbs, true);
    if (m_Expr.at(m_Pos).unicode() == '|')
      ++m_Pos;
    else
      m_error.SetNumber(erExpectedModuleBracket, GetErrorPos());
  }break;

  case bsNecessarilyBracket:{
    ++m_Pos;
    result = ExprString(EBracketed, true);
    if (m_Expr.at(m_Pos).unicode() == ')')
      ++m_Pos;
    else
      m_error.SetNumber(erExpectedRoundBracket, GetErrorPos());
  }break;

  case bsEllipsis:{
    ++m_Pos;
    result = new TExpr_ExtSymbol(esEllipsis);
  }break;

  case '_':{
    ++m_Pos;
    result = MakeCap(Factor(D), ecVector, 0);
  }break;

  default:{
    QRegExp rxp = QRegExp(QObject::tr("[A-Z]|[a-z]|[А-я]|[Ёё]"));
    if (QString(ch).contains(rxp))
      result = Func(_flags);
    else
      m_error.SetNumber(erSymbol, GetErrorPos());
  }break;
  }

  if (m_error.isValid() || isEnd()){
    if (result == 0)
      return new TExpr_Empty();
    else
      return result;
  }

  if (m_PostSymbols){
    ushort c = m_Expr.at(m_Pos).unicode();
    while ((c == '^') || (c == '_') || (c == '!') || (c == '`')){
      switch (c) {
      case '^':{
        ++m_Pos;
        TExpr_Class* B = Factor(D);
        TExpr_Argument* Arg = dynamic_cast<TExpr_Argument*>(B);
        if (Arg != 0)
          Arg->removeBrackets();
        Decorate(&result);
        result = MakePower(result, B);
        _flags = flgPower;
      }break;
      case '_':{
        ++m_Pos;
        m_PostSymbols = false;
        TExpr_Class* B = Factor(D);
        m_PostSymbols = true;
        TExpr_Argument* Arg = dynamic_cast<TExpr_Argument*>(B);
        if (Arg != 0)
          Arg->removeBrackets();
        Decorate(&result);
        result = MakeIndex(result, B);
      }break;
      case '!':{
        ++m_Pos;
        Decorate(&result);
        result->AddNext(new TExpr_Simple('!'));
      }break;
      case '`':{
        ++m_Pos;
        D = 1;
        while (m_Expr.at(m_Pos).unicode() == '`') { // считаем кол. символов '`'
          ++m_Pos;
          ++D;
        }
        Decorate(&result);
        result = MakePower(result, new TExpr_Strokes(D));
      }break;
      }

      if (isEnd())
        break;

      c = m_Expr.at(m_Pos).unicode();
    }
  }

  return result;
}

void TExpr_Builder::Decorate(TExpr_Class **_base)
{
  TExpr_Chain* chain = dynamic_cast<TExpr_Chain*>(*_base);
  if ((chain != 0) && (chain->hasSon()) && (chain->son()->hasNext())){
    TExpr_Class* A = chain->CutOffSon();
    delete *_base;
    *_base = new TExpr_Bracketed(A, ebRound, ebRound);
  }
}

TExpr_Class* TExpr_Builder::MakeCap(TExpr_Class *_base, TExprCapStyle _style, int _n)
{
  // _base - это результат функции Factor(), а она 0 никогда не возвращает
  //  if (_base == 0)
  //    return new TExpr_Cap(new TExpr_Empty(), _style, _n);

  TExpr_CommonFunc* com = dynamic_cast<TExpr_CommonFunc*>(_base);
  if (com != 0){
    // Пример: _log(β,x+1) - Cap будет только над logᵦ (x+1)
    TExpr_Class* A = MakeCap(com->CutOffSon(), _style, _n);
    com->SetSon(A);
    return _base;
  }

  TExpr_Index* indx = dynamic_cast<TExpr_Index*>(_base);
  if ((indx != 0) && (indx->hasSecondTwin())){
    // Пример: _s^2 -> добавляем cap к s, которая возведена в степень
    TExpr_Class* A = new TExpr_Cap(indx->CutOffSon(), _style, _n);
    indx->SetSon(A);
    return _base;
  }

  return new TExpr_Cap(_base, _style, _n);
}

TExpr_Class* TExpr_Builder::MakePower(TExpr_Class *_base, TExpr_Class *_exponent)
{
  TExpr_CommonFunc* cmFunc = dynamic_cast<TExpr_CommonFunc*>(_base);
  if (cmFunc != 0){
    //Пример: log(β,x+1)^2 -> log²ᵦ (x+1)
    TExpr_Index* sIndx = dynamic_cast<TExpr_Index*>(cmFunc->son());
    if ((sIndx != 0) && (!sIndx->hasSecondTwin())){
      sIndx->setSecondTwin(_exponent);
      return _base;
    }

    if (sIndx == 0){
      //Пример: func(f,a)^2 -> f²(a)
      TExpr_Class* A = new TExpr_Index(cmFunc->CutOffSon(), 0, _exponent);
      cmFunc->SetSon(A);
      return _base;
    }
  }

  TExpr_Index* indx = dynamic_cast<TExpr_Index*>(_base);
  if ((indx != 0) && (!indx->hasSecondTwin())){
    // Пример: s_2^2 -> s₂² (s с нижним индексом, возводим в степень)
    indx->setSecondTwin(_exponent);
    return _base;
  }

  return new TExpr_Index(_base, 0, _exponent);
}

TExpr_Class* TExpr_Builder::MakeIndex(TExpr_Class *_base, TExpr_Class *_index)
{
  TExpr_CommonFunc* cmFunc = dynamic_cast<TExpr_CommonFunc*>(_base);
  if (cmFunc != 0){
    //Пример: func(f^2,a)_2 -> f₂²(a)
    TExpr_Index* sIndx = dynamic_cast<TExpr_Index*>(cmFunc->son());
    if ((sIndx != 0) && (!sIndx->hasFirstTwin())){
      sIndx->setFirstTwin(_index);
      return _base;
    }

    if (sIndx == 0){
      //Пример: func(f,a)_2 -> f₂(a)
      TExpr_Class* A = new TExpr_Index(cmFunc->CutOffSon(), _index, 0);
      cmFunc->SetSon(A);
      return _base;
    }
  }

  TExpr_Index* indx = dynamic_cast<TExpr_Index*>(_base);
  if ((indx != 0) && (!indx->hasFirstTwin())){
    // Пример: (s^2)_2 -> s₂² (к s, возведенную в степень, добавляем нижний индекс)
    indx->setFirstTwin(_index);
    return _base;
  }

  return new TExpr_Index(_base, _index, 0);
}

TExpr_Class* TExpr_Builder::Func(int &_flags)
{
  // Func() вызывается из Factor(), во время вызова, m_error.numbe = erNull
  // В момент вызова этой функции m_Pos указывает на символ в строке выражения,
  // и этот символ один из [A-Z]|[a-z]|[А-я]|[Ёё]
  QString fncName;
  QRegExp rxp = QRegExp(QObject::tr("[A-Z]|[a-z]|[А-я]|[Ёё]|[0-9]"));
  QChar ch = m_Expr.at(m_Pos);
  while (QString(ch).contains(rxp)) {
    fncName.append(ch);
    ++m_Pos;

    //    if (isEnd())  // Последний символ в строке выражения всегда пробел
    //      break;      // поэтому эта проверка лишняя
    ch = m_Expr.at(m_Pos);
  }

  TExpr_Class* result = 0;

  //if (!isEnd() &&  // это тоже лишнее
  // Если после имени мы остановились на символе '(' или '!('
  // значит это функция
  if ((ch.unicode() == '(') || (ch.unicode() == bsNecessarilyBracket)){
    ++m_Pos;
    result = FuncName(fncName, _flags, ch.unicode() == bsNecessarilyBracket);

    if (m_error.isValid())
      return result;

    if (!isEnd() && (m_Expr.at(m_Pos).unicode() == ')'))
      ++m_Pos;
    else
      m_error.SetNumber(erExpectedRoundBracket, GetErrorPos());
    return result;
  }

  // Если скобки не было, значит это или Token или
  // буква и возможно сразу цифра, которая будет распознана как
  // нижний индекс, если m_VarAutoIndex = true. (пример: a2 -> a₂)
  bool wasIndex = false;
  int indxNumber = 0;
  if (m_VarAutoIndex)
    FindFuncIndex(fncName, indxNumber, wasIndex);

  result = Token(fncName);
  if (wasIndex)
    result = new TExpr_Index(result, new TExpr_Number(indxNumber, false), 0);

  return result;
}

void TExpr_Builder::FindFuncIndex(QString &_name, int &_indx, bool &_wasIndex)
{
  _indx = 0.0;
  _wasIndex = false;

  int i = _name.size();
  ushort c = _name.at(i-1).unicode();
  while ((c >= '0') && (c <= '9')) {
    --i;
    //      if (fncNameSize <= 0)  // поидеи такого быть не должно
    //        break;               // т.к. как минимум первый символ не цифра
    c = _name.at(i-1).unicode();
  }

  if (i < _name.size()){  // Значит в имени функции были цифры, это индекс
    _wasIndex = true;
    _indx = QString(_name.mid(i)).toInt();
    _name = _name.left(i);
  }
}

TExpr_Class* TExpr_Builder::FuncName(const QString &_name, int &_flags, bool _needBrackets)
{
  _flags = 0;
  QString nameU = _name.toUpper();

  if (nameU == "SQRT"){        // Корень квадратный
    TExpr_Class* A = ExprString(EExpression);
    return new TExpr_Root(A, 0);
  }

  if (nameU == "SQR"){         // Возведение выражения в квадрат
    _flags = flgPower;
    return MakePower(ExprString(EPower), new TExpr_Number(2.0, false));
  }

  if (nameU == "LOG")
    return fnLOG(_name, _needBrackets);

  if (nameU == "ABS")          // ABS(E) - Модуль E
    return ExprString(EAbs);

  if (nameU == "POW")
    return fnPOW(_flags);

  if (nameU == "ROOT")
    return fnROOT();

  if (nameU == "IND")
    return fnIND();

  if (nameU == "LIM")
    return fnLIM(_flags, _needBrackets);

  if (nameU == "FUNC")
    return fnFUNC(_needBrackets);

  if (nameU == "SPACE")
    return fnSPACE();

  if (nameU == "DIFF")
    return fnDIFF();

  if (nameU == "PDIFF")
    return fnPDIFF();

  if (nameU == "DIFFN")
    return fnDIFFN();

  if (nameU == "PDIFFN")
    return fnPDIFFN();

  if (nameU == "DIFFR")
    return fnDIFFR();

  if (nameU == "PDIFFR")
    return fnPDIFFR();

  if (nameU == "DIFFRF")
    return fnDIFFRF();

  if (nameU == "PDIFFRF")
    return fnPDIFFRF();

  if (nameU == "STRING")
    return fnSTRING();

  if (nameU == "STROKES")
    return fnSTROKES(_flags);

  if (nameU == "FACT")
    return fnFACT(_flags);

  if (nameU == "AT")
    return fnAT();

  if (nameU == "LINE")
    return fnCAPTION(ecLine);

  if (nameU == "VECT")
    return fnCAPTION(ecVector);

  if (nameU == "CAP")
    return fnCAPTION(ecCap);

  if (nameU == "TILDA")
    return fnCAPTION(ecTilde);

  if (nameU == "POINTS")
    return fnPOINTS();

  if (nameU == "STANDL")
    return fnSTAND(ehLeft);

  if (nameU == "STANDC")
    return fnSTAND(ehCenter);

  if (nameU == "STANDR")
    return fnSTAND(ehRight);

  if (nameU == "MATRIX")
    return fnMATRIX();

  if (nameU == "SUMMA")
    return fnGroupOp(gtSumma, _needBrackets);

  if (nameU == "PROD")
    return fnGroupOp(gtProd, _needBrackets);

  if (nameU == "CIRC")
    return fnGroupOp(gtCirc, _needBrackets);

  if (nameU == "INT")
    return fnGroupOp(gtInt, _needBrackets);

  if (nameU == "INTM")
    return fnGroupOp(gtIntM, _needBrackets);

  if (nameU == "CASE")
    return fnCASE();

  if (nameU == "COMMA")
    return fnCOMMA();

  if (nameU == "BRACKETS")
    return fnBRACKETS();

  if (nameU == "SYSTEM")
    return fnSYSTEM();

  if (nameU == "NUM")
    return fnNUM();

  if (nameU == "SYMBOL")
    return fnSYMBOL();

  if (nameU == "ANGLE")
    return fnANGLE();

  // для всего остального
  TExpr_Class* D = ExprString(EChain, true);
  TExpr_Argument* arg = new TExpr_Argument(D);
  if (_needBrackets)
    arg->SetBrackets();

  int indxNumber;
  bool wasIndex = false;
  QString nameFunc = _name;
  if (m_FuncAutoIndex)
    FindFuncIndex(nameFunc, indxNumber, wasIndex);

  int GI = GreekLetter(nameFunc);
  if (GI == 0){
    TExpr_Class* result = new TExpr_Func(nameFunc, arg);
    if (wasIndex)
      result = MakeIndex(result, new TExpr_Number(indxNumber, false));
    return result;
  }

  switch (GI) {
  case bsPlambda:
    D = new TExpr_Lambda();
    break;
  case bsNabla:
    D = new TExpr_Nabla();
    break;
  default:
    D = new TExpr_ExtSymbol(GI);
    break;
  }

  TExpr_Class* R = new TExpr_CommonFunc(D, arg);
  if (wasIndex)
    R = MakeIndex(R, new TExpr_Number(indxNumber, false));
  return R;
}

TExpr_Class* TExpr_Builder::fnANGLE()
{
  TExpr_Class* result = new TExpr_Sign(esAngleU);
  result->AddNext(fnSTRING());
  return result;
}

TExpr_Class* TExpr_Builder::fnSYMBOL()
{
  m_error.SetNumber(erNull);

  int n;
  int J, T;
  Val(m_Expr.mid(m_Pos),n,T);
  if (T == 1){
    m_error.SetNumber(erExpectedNumber, GetErrorPos());
    return new TExpr_Empty();
  }

  J = T;
  Val(m_Expr.mid(m_Pos,J-1),n,T);
  m_Pos += J-1;

  return new TExpr_ExtSymbol(n);
}

TExpr_Class* TExpr_Builder::fnNUM()
{
  //Num(R[n1[,n2[,n3]]])
  m_error.SetNumber(erNull);

  double R;
  int J, T;
  Val(m_Expr.mid(m_Pos),R,T);
  if (T == 1){
    m_error.SetNumber(erExpectedNumber, GetErrorPos());
    return new TExpr_Empty();
  }

  J = T;
  Val(m_Expr.mid(m_Pos,J-1),R,T);
  m_Pos += J-1;

  int n1 = 4;  // Pr
  int n2 = 4;  // Dig
  int n3 = 2;  // MD

  if (Comma()){
    // ******** n1
    Val(m_Expr.mid(m_Pos),n1,T);
    if (T == 1){
      m_error.SetNumber(erExpectedNumber, GetErrorPos());
      return new TExpr_ExpNumber(R, n1, n2, n3);
    }

    J = T;
    Val(m_Expr.mid(m_Pos,J-1),n1,T);
    m_Pos += J-1;

    // ************ n2
    if (Comma()){
      Val(m_Expr.mid(m_Pos),n2,T);
      if (T == 1){
        m_error.SetNumber(erExpectedNumber, GetErrorPos());
        return new TExpr_ExpNumber(R, n1, n2, n3);
      }

      J = T;
      Val(m_Expr.mid(m_Pos,J-1),n2,T);
      m_Pos += J-1;

      // ************ n3
      if (Comma()){
        Val(m_Expr.mid(m_Pos),n3,T);
        if (T == 1){
          m_error.SetNumber(erExpectedNumber, GetErrorPos());
          return new TExpr_ExpNumber(R, n1, n2, n3);
        }

        J = T;
        Val(m_Expr.mid(m_Pos,J-1),n3,T);
        m_Pos += J-1;
      }
    }
  }

  return new TExpr_ExpNumber(R, n1, n2, n3);
}

TExpr_Class* TExpr_Builder::fnSYSTEM()
{
  TExpr_Class* A = ExprString(EExpression);

  if (m_error.isValid())
    return new TExpr_Bracketed(A, ebFigure, ebNone);

  while (Comma()) {
    TExpr_Class* D = ExprString(EExpression);
    A->AddNext(D);

    if (m_error.isValid())
      break;
  }

  A = new TExpr_Stand(A, ehLeft);
  return new TExpr_Bracketed(A, ebFigure, ebNone);
}

TExpr_Class* TExpr_Builder::fnBRACKETS()
{
  m_error.SetNumber(erNull);
  TExprBracket leftBr;
  switch (m_Expr.at(m_Pos).unicode()) {
  case '(':
    leftBr = ebRound;
    break;
  case '[':
    leftBr = ebSquare;
    break;
  case '{':
    leftBr = ebFigure;
    break;
  case '|':
    leftBr = ebModule;
    break;
  case '0':
    leftBr = ebNone;
    break;
  default:
    m_error.SetNumber(erExpectedOpenBracket, GetErrorPos());
    return new TExpr_Empty();
  }

  ++m_Pos;
  if (isEnd()){
    m_error.SetNumber(erUnfinishedString, GetErrorPos());
    return new TExpr_Empty();
  }

  TExprBracket rightBr;
  switch (m_Expr.at(m_Pos).unicode()) {
  case ')':
    rightBr = ebRound;
    break;
  case ']':
    rightBr = ebSquare;
    break;
  case '}':
    rightBr = ebFigure;
    break;
  case '|':
    rightBr = ebModule;
    break;
  case '0':
    rightBr = ebNone;
    break;
  default:
    m_error.SetNumber(erExpectedCloseBracket, GetErrorPos());
    return new TExpr_Empty();
  }

  ++m_Pos;
  LookForComma();
  if (m_error.isValid())
    return new TExpr_Bracketed(0, leftBr, rightBr);

  TExpr_Class* A = ExprString(EExpression);
  return new TExpr_Bracketed(A, leftBr, rightBr);
}

TExpr_Class* TExpr_Builder::fnCOMMA()
{
  // Comma(n) - вставляет в выражение запятую, а после неё n пробелов.
  int n, J, T;
  Val(m_Expr.mid(m_Pos),n,T);
  J = T;
  Val(m_Expr.mid(m_Pos,J-1),n,T);
  m_Pos += J-1;

  TExpr_Class* result = new TExpr_Comma();
  result->AddNext(new TExpr_Space(n));
  return result;
}

TExpr_Class* TExpr_Builder::fnCASE()
{
  TExpr_Class* A = ExprString(EExpression);

  if (m_error.isValid())
    return new TExpr_Case(A);

  while (Comma()) {
    TExpr_Class* D = ExprString(EExpression);
    A->AddNext(D);

    if (m_error.isValid())
      break;
  }

  return new TExpr_Case(A);
}

TExpr_Class* TExpr_Builder::fnGroupOp(TExprGroupOpType _type, bool _needBrackets)
{
  // Summa(E1[,E2[,E3]]) - сумма выражений E1.
  // Prod(E1[,E2[,E3]]) - произведение выражений E1.
  // Circ(E1[,E2[,E3]]) - циркуляция выражений E1.
  // Int(E1[,E2[,E3]]) - интеграл выражения E1.
  // IntM(n,E1[,E2[,E3]]) - n-кратный интеграл выражения E1.
  // Под знаком E2, над ним E3. (E2 и E3 не обязательны)

  int n = 0;
  if (_type == gtIntM){
    int J, T;
    Val(m_Expr.mid(m_Pos),n,T);
    J = T;
    Val(m_Expr.mid(m_Pos,J-1),n,T);
    m_Pos += J-1;

    LookForComma();
    if (m_error.isValid())
      return new TExpr_Empty(); // функция недописана IntM(n
  }

  TExpr_Class* A = ExprString(EArgument);

  if (_needBrackets){
    TExpr_Argument* arg = dynamic_cast<TExpr_Argument*>(A);
    if (arg != 0)
      arg->SetBrackets();
    else
      A = new TExpr_Round(A);
  }

  TExpr_Class* D = 0;
  if (Comma())
    D = ExprString(EExpression);

  TExpr_Class* D2 = 0;
  if (Comma())
    D2 = ExprString(EExpression);

  switch (_type) {
  case gtSumma:
    return new TExpr_Summa(A, D, D2);
  case gtProd:
    return new TExpr_Prod(A, D, D2);
  case gtCirc:
    return new TExpr_Circ(A, D, D2);
  case gtInt:
    return new TExpr_Integral(A, D, D2);
  case gtIntM:
    return new TExpr_Integral(A, D, D2,n);
  }

  return A; // только чтоб убрать предупреждение компилятора [-Wreturn-type]
}

TExpr_Class* TExpr_Builder::fnMATRIX()
{
  // Matrix(n, m, E1[,E2,...E] - выводит выражения Е в матрицу
  // размером n на m;
  int n, J, D;
  Val(m_Expr.mid(m_Pos),n,D);
  J = D;
  Val(m_Expr.mid(m_Pos,J-1),n,D);
  m_Pos += J-1;

  LookForComma();
  if (m_error.isValid())
    return new TExpr_Empty();

  int m;
  Val(m_Expr.mid(m_Pos),m,D);
  J = D;
  Val(m_Expr.mid(m_Pos,J-1),m,D);
  m_Pos += J-1;

  LookForComma();
  if (m_error.isValid())
    return new TExpr_Empty();

  TExpr_Class* C = ExprString(EExpression);
  while (Comma()) {
    TExpr_Class* A = ExprString(EExpression);
    C->AddNext(A);

    if (m_error.isValid())
      break;
  }

  return new TExpr_Matrix(C, n, m);
}

TExpr_Class* TExpr_Builder::fnSTAND(TExprHorAlign _align)
{
  // standX(a,b,c) - выводит выражения a, b, c друг над другом
  // выравнивая их слева, справа или поцентру
  TExpr_Class* D = ExprString(EExpression);

  if (m_error.isValid())
    return D;

  while (Comma()) {
    TExpr_Class* A = ExprString(EExpression);
    D->AddNext(A);

    if (m_error.isValid())
      break;
  }

  return new TExpr_Stand(D, _align);
}

TExpr_Class* TExpr_Builder::fnPOINTS()
{
  TExpr_Class* A = ExprString(EExpression);

  int R = 1;
  if (Comma()){                  // если есть запятая, то после неё указано
    int J, D;                    // кол. точек
    Val(m_Expr.mid(m_Pos),R,D);
    J = D;
    Val(m_Expr.mid(m_Pos,J-1),R,D);
    m_Pos += J-1;
  }

  return MakeCap(A, ecPoints, R);
}

TExpr_Class* TExpr_Builder::fnCAPTION(TExprCapStyle _style)
{
  // Line(x) - горизонтальная линия над x
  TExpr_Class* A = ExprString(EExpression);
  return MakeCap(A, _style, 0);
}

TExpr_Class* TExpr_Builder::fnAT()
{
  // At(E1, E2) - значение E1 при условии E2.
  TExpr_Class* A = ExprString(EArgument);
  TExpr_Class* D;

  LookForComma();
  if (m_error.isValid())
    D = new TExpr_Empty();
  else
    D = ExprString(EExpression);

  return new TExpr_AtValue(A, D);
}

TExpr_Class* TExpr_Builder::fnFACT(int &_flags)
{
  // fact(n) -> n!
  _flags = flgPower;
  TExpr_Class* result = ExprString(EPower);
  result->AddNext(new TExpr_Simple('!'));
  return result;
}

TExpr_Class* TExpr_Builder::fnSTROKES(int &_flags)
{
  //strokes(f(x)) -> f'(x)
  //strokes(f(x), 3) -> f'''(x)
  _flags = flgPower;
  TExpr_Class* A = ExprString(EPower);

  int R = 1;
  if (Comma()){                  // если есть запятая, то после неё указано
    int J, D;                    // кол. strokes
    Val(m_Expr.mid(m_Pos),R,D);
    J = D;
    Val(m_Expr.mid(m_Pos,J-1),R,D);
    m_Pos += J-1;
  }

  return MakePower(A, new TExpr_Strokes(R));
}

TExpr_Class* TExpr_Builder::fnSTRING()
{
  m_error.SetNumber(erNull);
  QString str("");
  QChar ch = m_Expr.at(m_Pos);
  if (ch.unicode() == '"'){    // Если после string( идет символ "
    ++m_Pos;
    ch = m_Expr.at(m_Pos);     // берем символ после "
    do {                       // и повторяем операции ниже до следующего символа != "
      if (ch.unicode() != '"') // или до конца строки
        str.append(ch);
      else
        if (m_Expr.at(m_Pos+1).unicode() == '"'){ // если нам попадется " добавим его в строку, только зачем ???
          str.append('"');                       // string("""строка") -> "строка
          ++m_Pos;                               // но string("""строка""") -> ошибка, ожидается ')' в поз 18
        }                                        //                     ^ тут ожидается ')'

      ++m_Pos;
      if (isEnd()){
        m_error.SetNumber(erUnfinishedString, GetErrorPos());
        break;
      }

      ch = m_Expr.at(m_Pos);
    } while (ch.unicode() != '"');

    ++m_Pos;
  }
  else
    while (ch.unicode() != ')') { // Если после string( нет символа "
      str.append(ch);             // собираем все символы в строку до след. )
      ++m_Pos;                    // или до конца строки
      if (isEnd()){
        m_error.SetNumber(erUnfinishedString, GetErrorPos());
        break;
      }

      ch = m_Expr.at(m_Pos);
    }

  return new TExpr_Simple(str);
}

TExpr_Class* TExpr_Builder::fnPDIFFRF()
{
  // DiffRF(y,x[,n]) - d(^n)y/dx^n
  // аргумент n не обязателен
  TExpr_Class* D2 = ExprString(EPower);
  TExpr_Class* D;

  LookForComma();
  if (m_error.isValid())
    D = new TExpr_Empty();
  else
    D = ExprString(EPower);

  TExpr_Class* result = new TExpr_ExtSymbol(esPartDiff);
  TExpr_Class* C;

  if (Comma()){
    int P0 = m_Pos;
    TExpr_Class* A = ExprString(EExpression);
    result->AddNext(new TExpr_Index(D, 0, A));
    m_Pos = P0;
    A = ExprString(EExpression);
    result = new TExpr_Chain(result);
    C = new TExpr_Index(new TExpr_ExtSymbol(esPartDiff), 0, A);
    C->AddNext(D2);
    C = new TExpr_Chain(C);
    result = new TExpr_Ratio(C, result);
    return result;
  }

  result->AddNext(D);
  result = new TExpr_Chain(result);
  C = new TExpr_ExtSymbol(esPartDiff);
  C->AddNext(D2);
  C = new TExpr_Chain(C);
  result = new TExpr_Ratio(C, result);
  return result;
}

TExpr_Class* TExpr_Builder::fnDIFFRF()
{
  // DiffRF(y,x[,n]) - d(^n)y/dx^n
  // аргумент n не обязателен
  TExpr_Class* D2 = ExprString(EPower);
  TExpr_Class* D;

  LookForComma();
  if (m_error.isValid())
    D = new TExpr_Empty();
  else
    D = ExprString(EPower);

  TExpr_Class* result = new TExpr_Var('d');
  TExpr_Class* C;

  if (Comma()){
    int P0 = m_Pos;
    TExpr_Class* A = ExprString(EExpression);
    result->AddNext(new TExpr_Index(D, 0, A));
    m_Pos = P0;
    A = ExprString(EExpression);
    result = new TExpr_Chain(result);
    C = new TExpr_Index(new TExpr_Var('d'), 0, A);
    C->AddNext(D2);
    C = new TExpr_Chain(C);
    result = new TExpr_Ratio(C, result);
    return result;
  }

  result->AddNext(D);
  result = new TExpr_Chain(result);
  C = new TExpr_Var('d');
  C->AddNext(D2);
  C = new TExpr_Chain(C);
  result = new TExpr_Ratio(C, result);
  return result;
}

TExpr_Class* TExpr_Builder::fnPDIFFR()
{
  // DiffR(x[,n]) - d(^n)/dx^n - частный дифференциал
  // аргумент n не обязателен
  TExpr_Class* D = ExprString(EPower);

  TExpr_Class* result = new TExpr_ExtSymbol(esPartDiff);

  if (Comma()){
    int P0 = m_Pos;
    TExpr_Class* A = ExprString(EExpression);
    result->AddNext(new TExpr_Index(D, 0, A));
    m_Pos = P0;
    A = ExprString(EExpression);
    result = new TExpr_Chain(result);
    result = new TExpr_Ratio(new TExpr_Index(new TExpr_ExtSymbol(esPartDiff), 0, A), result);
    return result;
  }

  result->AddNext(D);
  result = new TExpr_Chain(result);
  result = new TExpr_Ratio(new TExpr_ExtSymbol(esPartDiff), result);
  return result;
}

TExpr_Class* TExpr_Builder::fnDIFFR()
{
  // DiffR(x[,n]) - d(^n)/dx^n
  // аргумент n не обязателен
  TExpr_Class* D = ExprString(EPower);

  TExpr_Class* result = new TExpr_Var('d');

  if (Comma()){
    int P0 = m_Pos;
    TExpr_Class* A = ExprString(EExpression);
    result->AddNext(new TExpr_Index(D, 0, A));
    m_Pos = P0;
    A = ExprString(EExpression);
    result = new TExpr_Chain(result);
    result = new TExpr_Ratio(new TExpr_Index(new TExpr_Var('d'), 0, A), result);
    return result;
  }

  result->AddNext(D);
  result = new TExpr_Chain(result);
  result = new TExpr_Ratio(new TExpr_Var('d'), result);
  return result;
}

TExpr_Class* TExpr_Builder::fnPDIFFN()
{
  // PDiffN(x[,n]) - d(^n)x - "частный дифференциал" в степени n
  // аргумент n не обязателен
  TExpr_Class* D = ExprString(EPower);

  TExpr_Class* result = new TExpr_Space(4);

  if (Comma()){
    TExpr_Class* A = ExprString(EPower);
    result->AddNext(new TExpr_Index(new TExpr_ExtSymbol(esPartDiff), 0, A));
    result->AddNext(D);
    return result;
  }

  result->AddNext(new TExpr_ExtSymbol(esPartDiff));
  result->AddNext(D);
  return result;
}

TExpr_Class* TExpr_Builder::fnDIFFN()
{
  // DiffN(x[,n]) - d(^n)x
  // аргумент n не обязателен
  TExpr_Class* D = ExprString(EPower);

  TExpr_Class* result = new TExpr_Space(4);

  if (Comma()){
    TExpr_Class* A = ExprString(EPower);
    result->AddNext(new TExpr_Index(new TExpr_Var('d'), 0, A));
    result->AddNext(D);
    return result;
  }

  result->AddNext(new TExpr_Var('d'));
  result->AddNext(D);
  return result;
}

TExpr_Class* TExpr_Builder::fnPDIFF()
{
  // PDiff(x[,n]) - "частный дифференциал" от dx^n
  // аргумент n не обязателен
  TExpr_Class* D = ExprString(EPower);

  TExpr_Class* result = new TExpr_Space(4);
  result->AddNext(new TExpr_ExtSymbol(esPartDiff));

  if (Comma()){
    TExpr_Class* A = ExprString(EExpression);
    result->AddNext(new TExpr_Index(D, 0, A));
    return result;
  }

  result->AddNext(D);
  return result;
}

TExpr_Class* TExpr_Builder::fnDIFF()
{
  // Diff(x[,n]) - дифференциал от dx^n
  // аргумент n не обязателен
  TExpr_Class* D = ExprString(EPower);

  TExpr_Class* result = new TExpr_Space(4);
  result->AddNext(new TExpr_Var('d'));

  if (Comma()){
    TExpr_Class* A = ExprString(EExpression);
    result->AddNext(new TExpr_Index(D, 0, A));
    return result;
  }

  result->AddNext(D);
  return result;
}

TExpr_Class* TExpr_Builder::fnSPACE()
{
  int R, J, D;
  Val(m_Expr.mid(m_Pos),R,D);
  J = D;
  Val(m_Expr.mid(m_Pos,J-1),R,D);
  m_Pos += J-1;

  return new TExpr_Space(R);
}

TExpr_Class* TExpr_Builder::fnFUNC(bool _needBrackets)
{
  TExpr_Class* A = ExprString(EExpression);
  LookForComma();

  TExpr_Class* D = 0;
  if (!m_error.isValid()){
    D = ExprString(EChain);

    while (Comma()) {
      TExpr_Class* B = ExprString(EChain);
      D->AddNext(new TExpr_Comma());
      D->AddNext(B);
    }
  }

  TExpr_Argument* arg = new TExpr_Argument(D);

  if (_needBrackets)
    arg->SetBrackets();

  return new TExpr_CommonFunc(A, arg);
}

TExpr_Class* TExpr_Builder::fnLIM(int &_flags, bool _needBrackets)
{
  // Lim(E1, E2) - предел выражения E2 при условии E1
  _flags = flgPower;
  TExpr_Class* D = ExprString(EExpression);
  LookForComma();

  TExpr_Class* A = 0;
  if (!m_error.isValid())
    A = ExprString(EArgument);

  if (_needBrackets){
    TExpr_Argument* arg = dynamic_cast<TExpr_Argument*>(D);
    if (D != 0)
      arg->SetBrackets();
    else
      D = new TExpr_Round(D);
  }

  return new TExpr_CommonFunc(new TExpr_Lim(D), A);
}

TExpr_Class* TExpr_Builder::fnIND()
{
  TExpr_Class* D = ExprString(EPower);
  LookForComma();

  TExpr_Class* A = 0;
  if (!m_error.isValid())
    A = ExprString(EExpression);

  return MakeIndex(D,A);
}

TExpr_Class* TExpr_Builder::fnPOW(int &_flags)
{
  // POW(E1, E2) - возведение E1 в степнь E2
  _flags = flgPower;
  TExpr_Class* D = ExprString(EPower);
  LookForComma();

  TExpr_Class* A = 0;
  if (!m_error.isValid())
    A = ExprString(EExpression);

  return MakePower(D,A);
}

TExpr_Class* TExpr_Builder::fnROOT()
{
  // Root(E1, E2) - Извлечение корня степени E1 из выражения E2
  TExpr_Class* D = ExprString(EExpression);
  LookForComma();

  TExpr_Class* A = 0;
  if (!m_error.isValid())
    A = ExprString(EExpression);

  return new TExpr_Root(A, D);
}

TExpr_Class* TExpr_Builder::fnLOG(const QString &_name, bool _needBrackets)
{
  // Log(E1, E2) Логарифм E2 по основанию E1
  TExpr_Class* A = ExprString(EExpression);
  LookForComma();
  TExpr_Class* D = 0;

  // если запятая была найдена
  if (!m_error.isValid()){
    D = ExprString(EArgument);

    if (_needBrackets){
      TExpr_Argument* arg = dynamic_cast<TExpr_Argument*>(D);
      if (D != 0)
        arg->SetBrackets();
      else
        D = new TExpr_Round(D);
    }
  }

  A = new TExpr_Index(new TExpr_FuncName(_name), A, 0);
  return new TExpr_CommonFunc(A, D);
}

void TExpr_Builder::LookForComma()
{
  m_error.SetNumber(erNull);
  if (!Comma())
    m_error.SetNumber(erExpectedComma, GetErrorPos());
}

bool TExpr_Builder::Comma()
{
  bool result = m_Expr.at(m_Pos).unicode() == ',';
  if (result){
    ++m_Pos;
    SkipSpaces();
  }
  return result;
}

TExpr_Class* TExpr_Builder::Token(const QString &_name)
{
  int GI = GreekLetter(_name);
  if (GI != 0){
    switch (GI) {
    case bsPlambda:
      return new TExpr_Lambda();
      break;
    case bsNabla:
      return new TExpr_Nabla();
      break;
    default:
      return new TExpr_ExtSymbol(GI);
      break;
    }
  }

  QString nameU = _name.toUpper();
  if (nameU == "INF")
    return new TExpr_ExtSymbol(esInfinum);
  else if (nameU == "PLANK")
    return new TExpr_Plank();
  else if (nameU == "NIL")
    return new TExpr_Class();
  else if (nameU == "COMMA")
    return new TExpr_Comma();
  else if (nameU == "CONST"){
    TExpr_Class* A = new TExpr_Simple(_name);
    A->AddNext(new TExpr_Space(3));
    return A;}
  else if (nameU == "ASTERIX")
    return new TExpr_Asterix();
  else if (nameU == "MINUS")
    return new TExpr_ExtSymbol(esMinus);
  else if (nameU == "PARALLEL")
    return new TExpr_Sign(esParallelU);
  else if (nameU == "PERPENDICULAR")
    return new TExpr_Sign(esPerpendicularU);
  else if (nameU == "ANGLE")
    return new TExpr_Sign(esAngleU);
  else if (nameU == "EMPTY")
    return new TExpr_Empty();
  else{
    QRegExp rxp = QRegExp("[A-Z]|[a-z]");
    if (QString(_name.at(0)).contains(rxp))
      return new TExpr_Var(_name);
  }

  return new TExpr_Simple(_name);
}

int TExpr_Builder::GreekLetter(const QString &_name)
{
  QMap<QString, ushort> map;
  map.insert("ALPHA", 913);   // 'Α'
  map.insert("BETA", 914);    // 'Β'
  map.insert("GAMMA", 915);   // 'Γ'
  map.insert("DELTA", 916);   // 'Δ'
  map.insert("EPSILON", 917); // 'Ε'
  map.insert("ZETA", 918);    // 'Ζ'
  map.insert("ETA", 919);     // 'Η'
  map.insert("THETA", 920);   // 'Θ'
  map.insert("IOTA", 921);    // 'Ι'
  map.insert("KAPPA", 922);   // 'Κ'
  map.insert("LAMBDA", 923);  // 'Λ'
  map.insert("MU", 924);      // 'Μ'
  map.insert("NU", 925);      // 'Ν'
  map.insert("XI", 926);      // 'Ξ'
  map.insert("OMICRON", 927); // 'Ο'
  map.insert("PI", 928);      // 'Π'
  map.insert("RHO", 929);     // 'Ρ'
  map.insert("SIGMA", 931);   // 'Σ'
  map.insert("TAU", 932);     // 'Τ'
  map.insert("UPSILON", 933); // 'Υ'
  map.insert("PHI", 934);     // 'Φ'
  map.insert("CHI", 935);     // 'Χ'
  map.insert("PSI", 936);     // 'Ψ'
  map.insert("OMEGA", 937);   // 'Ω'
  map.insert("PLAMBDA", bsPlambda);
  map.insert("NABLA", bsNabla);

  int DS = _name.at(0).isLower() ? 32: 0; // Смешение между большими и мал. символами
  int result = map.value(_name.toUpper(), 0);

  if ((result != 0) && !((result == bsPlambda) || (result == bsNabla)))
    result += DS;
  return result;
}

void TExpr_Builder::Val(const QString &_str, int& _result, int &_errPos)
{
  //Это аналог старой функции Delphi Var().
  //Преобразует переданную строку в int. errPos после преобразования
  //указывает на номер символа(начиная с 1), который будет мешать
  //преобразованию. При успешном преобразовании _errPos = 0.
  //Есть небольшое отличие от функции Delphi, если результат преобразования
  //больше допустимого диапазона int, вернет 0(Delphi возвращает, то что вместилось),
  //но для наших целей это не имеет значения.

  for (_errPos = 0; _errPos < _str.size(); ++_errPos) {
    QChar ch = _str.at(_errPos);
    if (!ch.isDigit())
      break;
  }

  QString tmp = _str.left(_errPos);
  ++_errPos;

  bool b;
  _result = tmp.toInt(&b);
  if (b && (_errPos > _str.size()))
    _errPos = 0;
}

void TExpr_Builder::Val(const QString &_str, double& _result, int &_errPos)
{
  //Это аналог старой функции Delphi Var().
  //Преобразует переданную строку в double. errPos после преобразования
  //указывает на номер символа(начиная с 1), который будет мешать
  //преобразованию. При успешном преобразовании _errPos = 0.

  //пропускаем цифры и одну точку
  char dot_count = 0;
  for (_errPos = 0; _errPos < _str.size(); ++_errPos) {
    QChar ch = _str.at(_errPos);
    if (!ch.isDigit()){
      if ((dot_count == 0) && (ch.unicode() == '.'))
        ++dot_count;
      else
        break;
    }
  }

  //если мы остановились на символе 'e' или 'E', но недостигли конца строки
  // и была хотябы одна цифра
  // продолжаем пропускать только цифры, при этом пропустим
  // один символ '-' или '+'.
  if ((_errPos > 0) && (_errPos < _str.size())){
    ushort ch = _str.at(_errPos).unicode();      //символ на котором остановились

    if ((ch == 'e') || (ch == 'E')){             //если это 'e' продолжим поиск чисел
      if ((_errPos+1) < _str.size()){            //если сл. символ '-' или '+', пропустим
        ushort ch_next = _str.at(_errPos+1).unicode();
        if ((ch_next == '-') || (ch_next == '+'))
          ++_errPos;
      }

      ++_errPos;
      for (; _errPos < _str.size(); ++_errPos) {
        QChar c = _str.at(_errPos);
        if (!c.isDigit())
          break;
      }
    }
  }

  QString tmp = _str.left(_errPos);
  ++_errPos;

  bool b;
  _result = tmp.toDouble(&b);
  if (b && (_errPos > _str.size()))
    _errPos = 0;
}

void TExpr_Builder::AddMult(TExpr_Class** _existing, TExpr_Class** _multiplier)
{
  TExpr_Class* _exist = *_existing;
  TExpr_Class* _mult = *_multiplier;
  // И _existing и _multiplier могут быть равны 0
  if (_mult == 0)
    return;

  if (_exist == 0){
    *_existing = *_multiplier;
    return;
  }

  TExpr_Class* ELast = _exist;
  while (ELast->hasNext()) {      // Последний next в _existing
    ELast = ELast->next();
  }

  TExpr_Class* MLast = _mult;
  while (MLast->hasNext()) {      // Последний next в _multiplier
    MLast = MLast->next();
  }

  // пример: a*b = ab
  if (((ELast->FTType() & efRight) > 0) && ((_mult->FTType() & efLeft) > 0)){
    _exist->AddNext(_mult);
    return;
  }

  // пример: a*2 = 2a
  if (((MLast->FTType() & efRight) > 0) && ((_exist->FTType() & efLeft) > 0)){
    _mult->AddNext(_exist);
    *_existing = *_multiplier;
    return;
  }

  // пример: 2*4 = 6
  TExpr_Number* _existNum = dynamic_cast<TExpr_Number*>(_exist);
  TExpr_Number* MLastNum = dynamic_cast<TExpr_Number*>(MLast);
  if ((_existNum != 0) && (MLastNum != 0)){
    MLastNum->setNumber(MLastNum->number() * _existNum->number());
    MLast->SetNext(_exist->CutOff());
    delete _exist;
    *_existing = _mult;
    return;
  }

  // пример: ???
  if (((_mult->FTType() & efLeft) > 0) && ((MLast->FTType() & efRight) > 0)){
    TExpr_Class* tmp = _exist;
    while (tmp->hasNext()) {
      if (((tmp->FTType() & efRight) > 0) && ((tmp->next()->FTType() & efLeft) > 0))
        break;
      tmp = tmp->next();
    }

    if (tmp->hasNext()){
      MLast->SetNext(tmp->CutOff());
      tmp->SetNext(_mult);
    }

    return;
  }

  // пример: sin(x)*cos(x)
  _exist->AddNext(new TExpr_Sign(esMultiply));
  _exist->AddNext(_mult);
}

int TExpr_Builder::GetErrorPos() const
{
  QVector<ushort> vec;
  vec.push_back(bsNotEqual);
  vec.push_back(bsGreaterOrEqual);
  vec.push_back(bsLessOrEqual);
  vec.push_back(bsMuchGreater);
  vec.push_back(bsMuchLess);
  vec.push_back(bsArrow);
  vec.push_back(bsApproxEqual);
  vec.push_back(bsPlusMinus);
  vec.push_back(bsMinusPlus);
  vec.push_back(bsEquivalent);
  vec.push_back(bsDivide);
  vec.push_back(bsMultiplyCross);
  vec.push_back(bsEllipsis);
  vec.push_back(bsApproxGreater);
  vec.push_back(bsApproxLess);
  vec.push_back(bsDivideSlash);
  vec.push_back(bsNecessarilyBracket);
  vec.push_back(bsMultiplyDot);
  vec.push_back(bsMultiply);
  vec.push_back(bsAlmostEqual);
  vec.push_back(bsAmpersand);

  int i = 0;
  int dPos = 0;
  while (i <= m_Pos) {
    ushort ch = m_Expr.at(i).unicode();
    if (vec.contains(ch)){
      ++dPos;
      if ((ch == bsEllipsis) || (ch == bsAmpersand)) ++dPos;
    }

    ++i;
  }

  return m_Pos+dPos+1;
}
