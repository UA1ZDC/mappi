#ifndef TEXPR_CLASS_H
#define TEXPR_CLASS_H

#include <QColor>
#include <QFont>

#ifdef _MSC_BUILD
#   define GET_FUNC_NAME()      __FUNCTION__
#elif defined(__BORLANDC__)
#   define GET_FUNC_NAME()      __FUNC__
#else
#   define GET_FUNC_NAME()      __PRETTY_FUNCTION__
#endif

class QPainter;
class QPaintDevice;

namespace ExprDraw {

enum TExprOrigin {eoTop, eoBottom};

// Вертикальное выравнивание выражения.
// Указывает где находится координата X по отношению к выражению.
// Пример:
// координата          X
// ehLeft               "выражение"
// ehCentr        "выражение"
// ehRight  "выражение"
enum TExprHorAlign {ehLeft, ehCenter, ehRight};

// Горизонтальное выравнивание выражения.
// Указывает где находится координата Y по отношению к выражению.
// Пример:
//               evBottom
// координата Y  evCenter
//               evTop
enum TExprVertAlign {evTop, evCenter, evBottom};

// Типы скобок
//  ebNone   - нет скобки
//  ebRound  - ( или )
//  ebSquare - [ или ]
//  ebFigure - { или }
//  ebModule - |
enum TExprBracket {ebNone, ebRound, ebSquare, ebFigure, ebModule};

// Тип символа над выражением
enum TExprCapStyle {ecPoints, ecVector, ecCap, ecTilde, ecLine};

const int fsBold       = 1;   // 0000 0001
const int fsItalic     = 2;   // 0000 0010
const int fsUnderline  = 4;   // 0000 0100
const int fsStrikeOut  = 8;   // 0000 1000

const unsigned int CHANGED = 0xFFFFFFFF;
const int efLeft          = 1;   // 0000 0001
const int efRight         = 2;   // 0000 0010
const int efNegative      = 4;   // 0000 0100
const int efBrackets      = 16;  // 0001 0110
const int efRoundBrackets = 24;  // 0010 0100
const int efNumber        = 32;  // 0011 0010

const unsigned int tcWidth        = 1 << 0;
const unsigned int tcHeight       = 1 << 1;
const unsigned int tcPowerXPos    = 1 << 2;
const unsigned int tcPowerYPos    = 1 << 3;
const unsigned int tcIndexXPos    = 1 << 4;
const unsigned int tcIndexYPos    = 1 << 5;
const unsigned int tcCapDX        = 1 << 6;
const unsigned int tcCapDY        = 1 << 7;
const unsigned int tcMidLineUp    = 1 << 8;
const unsigned int tcMidLineDn    = 1 << 9;
const unsigned int tcCellSize     = 1 << 10;
const unsigned int tcSymbolWidth  = 1 << 10;
const unsigned int tcSymbolHeight = 1 << 11;

// Эти символы рисуются отдельно, несмотря на то, что
// в Unicode они есть.
const unsigned int esMuchLess       = 1;  // <<
const unsigned int esMuchGreater    = 2;  // >>
const unsigned int esApproxLess     = 3;  // ≲
const unsigned int esApproxGreater  = 4;  // ≳
const unsigned int esPlusMinus      = 5;  // ±
const unsigned int esMinusPlus      = 6;  // ∓
const unsigned int esAlmostEqual    = 7;  // ≃
const unsigned int esParallel       = 10; // ‖
const unsigned int esPerpendicular  = 11; // ⊥
const unsigned int esAngle          = 12; // ∠

// Коды дополнительных символов
// символы xxxxxU лучше не использовать,
// они могут по разному отображаться на разных системах
const unsigned int esMuchLessU      = 8810;
const unsigned int esMuchGreaterU   = 8811;
const unsigned int esPlusMinusU     = 177;
const unsigned int esParallelU      = 8214;  // B
const unsigned int esPerpendicularU = 8869;  // B
const unsigned int esAngleU         = 8736;  // B
const unsigned int esApproxLessU    = 8818;  // только Linux
const unsigned int esApproxGreaterU = 8819;  // только Linux
const unsigned int esMinusPlusU     = 8723;  // только Linux
const unsigned int esAlmostEqualU   = 8771;  // только Linux

const unsigned int esPlus           = 43;   // +
const unsigned int esMinus          = 8722; // -
const unsigned int esLess           = 60;   // <
const unsigned int esEqual          = 61;   // =
const unsigned int esGreater        = 62;   // >
const unsigned int esNotEqual       = 8800; // ≠
const unsigned int esMultiply       = 183;  // ·
const unsigned int esLessOrEqual    = 8804; // ≤
const unsigned int esGreaterOrEqual = 8805; // ≥
const unsigned int esApproxEqual    = 8776; // ≈
const unsigned int esCrossMultiply  = 215;  // ×
const unsigned int esDivide         = 247;  // ÷
const unsigned int esTilde          = 126;  // ~
const unsigned int esEquivalent     = 8801; // ≡
const unsigned int esArrow          = 8594; // →
const unsigned int esSlash          = 47;   // /
const unsigned int esEllipsis       = 8230; // …
const unsigned int esInfinum        = 8734; // ∞
const unsigned int esPartDiff       = 8706; // ∂

}

class TExpr_Class
{
private:
  friend class TExpr_Parent;
  friend class TExpr_Matrix;     // m_toChange
  friend class TExpr_BigParent;
  friend class TExpr_CommonFunc; // ArgNeedBrackets()
  friend class TExpr_Cap;        // GetCapDXLeft(), GetCapDXRight(), CalcIndexXPos()
  friend class TExpr_Chain;      // m_toChange, SetParent()
  friend class TExpr_Bracketed;  // m_toChange
  friend class TExpr_Argument;   // m_toChange, NeedBrackets
  friend class TExpr_TwinParent; // m_toChange, SetParent()
  friend class TExpr_GroupOp;    // m_toChange

  TExpr_Class* m_parent;
  TExpr_Class* m_next;
  QColor m_color;

  int m_width;
  int m_height;
  int m_midLineUp;
  int m_midLineDn;
  int m_powerXPos;
  int m_powerYPos;
  int m_indexXPos;
  int m_indexYPos;
  int m_capDY;
  int m_capDXLeft;
  int m_capDXRight;
  unsigned int m_toChange;

  void SetLineWidth();
  int GetWidth();
  int GetHeight();
  int GetMidLineUp();
  int GetMidLineDn();
  int GetPowerXPos();
  int GetPowerYPos();
  int GetIndexXPos();
  int GetIndexYPos();
  int GetCapDXLeft();
  int GetCapDXRight();
  int GetCapDY();
protected:
  int m_wx;
  int m_wy;
  double m_rwx;
  double m_rwy;

  QFont m_font;

  QPaintDevice* m_paintDevice;

  void SetParent(TExpr_Class *value);
  TExpr_Class* parent() {return m_parent;}

  virtual void DynaSetFont();
  virtual void DynaSetColor();
  virtual void DynaSetPaintDevice();
  virtual void SetPenAndBrush(QPainter* painter);
  virtual void SetPainterFont(QPainter* painter);
  void ConvertCoords(int& X, int& Y, ExprDraw::TExprHorAlign hAligment, ExprDraw::TExprVertAlign vAligment);
  void AssignPaintDevice(QPaintDevice *_paintDevice, int _wx, int _wy, long double _rwx, long double _rwy);
  void AssignFont(const QFont& _font, int _wx, int _wy, long double _rwx, long double _rwy);
  void AssignColor(const QColor& _color);
  virtual void Paint(QPainter* painter, int X, int Y);
  virtual bool NeedBrackets();
  virtual bool ArgNeedBrackets();
  virtual int CalcWidth();
  virtual int CalcHeight();
  virtual int CalcMidLine(ExprDraw::TExprOrigin origin);
  virtual int CalcPowerXPos();
  virtual int CalcIndexXPos();
  virtual int CalcPowerYPos();
  virtual int CalcIndexYPos();
  virtual int CalcCapDY();
  virtual void CalcCapDX(int& DLeft, int& DRight);
  virtual QFont GetFont();

public:
  TExpr_Class();
  virtual ~TExpr_Class();

  void SetNext(TExpr_Class *value);
  void SetFont(const QFont& value);
  void SetPaintDevice(QPaintDevice* _paintDevice);
  void SetColor(const QColor& value);
  TExpr_Class* next()                 {return m_next;}
  QFont font() {return GetFont();}
  const QColor color() const;
  QPaintDevice* paintDevice()         {return m_paintDevice;}
  bool hasNext() const   {return m_next != 0;}
  bool hasParent() const {return m_parent != 0;}

  int width() {return GetWidth();}
  int height() {return GetHeight();}
  int midLineUp() {return GetMidLineUp();}
  int midLineDn() {return GetMidLineDn();}
  int powerXPos() {return GetPowerXPos();}
  int powerYPos() {return GetPowerYPos();}
  int indexXPos() {return GetIndexXPos();}
  int indexYPos() {return GetIndexYPos();}
  int capDXLeft() {return GetCapDXLeft();}
  int capDXRight() {return GetCapDXRight();}
  int capDY() {return GetCapDY();}

  virtual int FTType();
  TExpr_Class* CutOff();
  void AddNext(TExpr_Class* value);
  void Draw(int x, int y, ExprDraw::TExprHorAlign hAligment, ExprDraw::TExprVertAlign vAligment);
  void Draw(QPainter* painter, int x, int y, ExprDraw::TExprHorAlign hAligment, ExprDraw::TExprVertAlign vAligment);
};


#endif // TEXPR_CLASS_H
