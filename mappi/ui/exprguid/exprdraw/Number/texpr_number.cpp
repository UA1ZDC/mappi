#include "exprdraw/Number/texpr_number.h"

#include <QString>
#include <QFontMetrics>
#include <QPainter>


TExpr_Number::TExpr_Number(double _number, bool _exponentForm)
  : TExpr_Class()
{
  m_exponentForm = _exponentForm;
  _number == 0.0 ? m_number = 1.0 : m_number = 0.0;
  m_local = QLocale::system();
  setNumber(_number);
}

QString TExpr_Number::numberToString()
{
  if (m_exponentForm)
    return m_local.toString(m_number,'E',14);
  return m_local.toString(m_number, 'g', 14);
}

void TExpr_Number::setNumber(double _number)
{
  if (m_number == _number)
    return;

  m_number = _number;
  QString str = numberToString();
  int posE = str.lastIndexOf('E',-1,Qt::CaseInsensitive);

  if (posE == -1){
    m_sMantis = m_local.toString(m_local.toDouble(str),'g',14);  // Обрезаем '0' вконце
    m_sExponent.clear();
  }
  else{
    m_sMantis = str.mid(0,posE);  // Мантиса - всё что до 'E' ("3.33333333333333E+00")
    m_sExponent = str.mid(posE+1);//Экспонента - всё что после 'E'
    m_sMantis = m_local.toString(m_local.toDouble(m_sMantis),'g',14);   // Обрезаем '0' вконце

    //Удаляем '+', если он есть
    if (m_sExponent[0] == '+')
      m_sExponent.remove(0,1);

    //Удаляем все '0' спереди
    m_sExponent = QString().number(m_sExponent.toInt());
  }
}

int TExpr_Number::CalcWidth()
{
  QFontMetrics fm(font());
  if (!m_sExponent.isEmpty()){
    int result = 0;
    if (m_sMantis == "1")
      result = fm.width("10");
    else
      result = fm.width(m_sMantis + "·10");

    QFontMetrics fm_small(smallFont());
    result += fm_small.width(m_sExponent);
    return result;
  }
  else
    return fm.width(m_sMantis);
}

int TExpr_Number::CalcHeight()
{
  QFontMetrics fm(font());
  int result = fm.height();
  if (!m_sExponent.isEmpty())
    result = qRound(1.2 * result);

  return result;
}

int TExpr_Number::CalcCapDY()
{
  if (m_sExponent.isEmpty())
    return qRound(8 * m_rwy);
  return qRound(3 * m_rwy);
}

int TExpr_Number::CalcMidLine(ExprDraw::TExprOrigin origin)
{
  if (m_sExponent.isEmpty())
    return TExpr_Class::CalcMidLine(origin);

  QFontMetrics fm(font());
  int H = fm.height();
  if (origin == ExprDraw::eoTop)
    return (H / 2) + qRound(H * 0.2);
  else
    return -((H-1) / 2);
}

int TExpr_Number::FTType()
{
  return ExprDraw::efRight | (ExprDraw::efNegative * static_cast<int>(m_number < 0));
}

void TExpr_Number::Paint(QPainter* painter, int x, int y)
{
  QFontMetrics fm(font());
  QPoint pos(x, y + fm.ascent());

  painter->save();
  SetPenAndBrush(painter);
  SetPainterFont(painter);

  // Только мантиса
  if (m_sExponent.isEmpty()){
    painter->drawText(pos, m_sMantis);
    painter->restore();
    return;
  }

  //Мантиса + Экспонента
  int H = qRound(0.2 * fm.height());
  int W = 0;
  if (m_sMantis == "1"){
    painter->drawText(pos.x(), pos.y() + H, "10");
    W = fm.width("10");
  }
  else{
    painter->drawText(pos.x(), pos.y() + H, m_sMantis + "·10");
    W = fm.width(m_sMantis + "·10");
  }

  painter->setFont(smallFont());
  QFontMetrics fm_small(painter->font());
  pos.setY(y + fm_small.ascent());
  painter->drawText(pos.x() + W, pos.y(), m_sExponent);
  painter->restore();
}

QFont TExpr_Number::smallFont()
{
  QFont small_font = font();
  int s = small_font.pointSize();
  s = qRound(0.7 * s);
  small_font.setPointSize(s);
  return small_font;
}
