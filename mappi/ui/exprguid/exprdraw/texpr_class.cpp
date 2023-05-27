#include "exprdraw/texpr_class.h"

#include <QtGlobal>
#include <QPainter>
#include <QFontMetrics>
#include <QPaintDevice>
#include <QPen>
#include <QBrush>

#include <QDebug>

using namespace ExprDraw;

//static int _creat = 0;
//static int c = 0;

TExpr_Class::TExpr_Class()
{
  m_parent = 0;
  m_next = 0;
  m_paintDevice = 0;

  m_width = 0;
  m_height = 0;
  m_midLineUp = 0;
  m_midLineDn = 0;
  m_powerXPos = 0;
  m_powerYPos = 0;
  m_indexXPos = 0;
  m_indexYPos = 0;
  m_capDXLeft = 0;
  m_capDXRight = 0;
  m_capDY = 0;
  m_wx = 0;
  m_wy = 0;
  m_rwx = 0;
  m_rwy = 0;
  m_toChange = 0;
  //m_font.setPointSize(m_font.pointSize()+1);

//  ++_creat;
}

TExpr_Class::~TExpr_Class()
{
  delete m_next;
//  --_creat;
//  if (_creat <=0){
//    ++c;
//    qDebug() << QString("%1. All Texpr_Class deleted! i = %2.").arg(c).arg(_creat);
//  }
}

void TExpr_Class::SetNext(TExpr_Class *value)
{
  //if (hasNext())
  delete m_next;
  m_next = value;
  if (hasNext())
    m_next->SetParent(m_parent);
}

void TExpr_Class::SetParent(TExpr_Class *value)
{
  if (m_parent == value)
    return;
  m_parent = value;
  if (hasNext())
    m_next->SetParent(m_parent);
}

const QColor TExpr_Class::color() const
{
  if (m_color.isValid())
    return m_color;
  if (hasParent())
    return m_parent->color();
  return QColor(Qt::black);
}

void TExpr_Class::SetPaintDevice(QPaintDevice * _paintDevice)
{
  if (m_paintDevice == _paintDevice)
    return;

  m_paintDevice = _paintDevice;
  m_toChange = CHANGED;
  SetLineWidth();
  DynaSetPaintDevice();
}

void TExpr_Class::AssignPaintDevice(QPaintDevice* _paintDevice,
                                    int _wx, int _wy,
                                    long double _rwx, long double _rwy)
{
  if (m_paintDevice == _paintDevice)
    return;

  m_paintDevice = _paintDevice;
  m_toChange = CHANGED;
  m_wx = _wx;
  m_wy = _wy;
  m_rwx = _rwx;
  m_rwy = _rwy;
  DynaSetPaintDevice();
}

void TExpr_Class::SetLineWidth()
{
  QFontMetrics fm(font());
  double H = fm.height();

  m_rwx = H/27.6;

  double logicalDpi = 1.0;
  if (m_paintDevice != 0){
    logicalDpi = m_paintDevice->logicalDpiY()/m_paintDevice->logicalDpiX();
  }
  m_rwy = m_rwx * logicalDpi;

  m_wy = fm.boundingRect('_').height();

  logicalDpi = 1.0;
  if (m_paintDevice != 0){
    logicalDpi = m_paintDevice->logicalDpiX()/m_paintDevice->logicalDpiY();
  }
  m_wx = qRound(m_wy * logicalDpi);
}

void TExpr_Class::SetFont(const QFont &value)
{
  if (value == m_font)
    return;
  m_font = value;
  SetLineWidth();
  DynaSetFont();
}

void TExpr_Class::DynaSetFont()
{
  m_toChange = CHANGED;
  if (hasNext())
    m_next->AssignFont(m_font, m_wx, m_wy, m_rwx, m_rwy);
}

void TExpr_Class::AssignFont(const QFont &_font,
                             int _wx, int _wy,
                             long double _rwx, long double _rwy)
{
  if (m_font == _font) // если font не изменился,то m_wx, m_wy, m_rwx и m_rwy
    return;            // не изменились тоже

  m_font = _font;
  m_wx = _wx;
  m_wy = _wy;
  m_rwx = _rwx;
  m_rwy = _rwy;
  DynaSetFont();
}

QFont TExpr_Class::GetFont()
{
  return m_font;
}

void TExpr_Class::SetColor(const QColor &value)
{
  if (m_color == value)
    return;
  m_color = value;
  DynaSetColor();
}

void TExpr_Class::DynaSetColor()
{
  if (hasNext())
    m_next->AssignColor(m_color);
}

void TExpr_Class::AssignColor(const QColor &_color)
{
  m_color = _color;
  DynaSetColor();
}

void TExpr_Class::DynaSetPaintDevice()
{
  if (hasNext())
    m_next->AssignPaintDevice(m_paintDevice, m_wx, m_wy, m_rwx, m_rwy);
}

void TExpr_Class::AddNext(TExpr_Class *value)
{
  if (value == 0)
    return;

  TExpr_Class* p;
  p = this;
  while (p->hasNext())
    p = p->m_next;    // добираемся до последнего Next
  // Может вместо цикла хранить ссылку на послендий Next ???

  p->m_next = value;
  p->m_next->SetFont(m_font);
  p->m_next->SetPaintDevice(paintDevice());
}

TExpr_Class* TExpr_Class::CutOff()
{
  TExpr_Class* result = m_next;
  m_next = 0;
  return result;
}

bool TExpr_Class::NeedBrackets()
{
  return false;
}

bool TExpr_Class::ArgNeedBrackets()
{
  return true;
}

int TExpr_Class::FTType()
{
  return efLeft | efRight;
}

int TExpr_Class::CalcPowerXPos()
{
  return width()+m_wx;
}

int TExpr_Class::CalcIndexXPos()
{
  return width()+m_wx;
}

int TExpr_Class::CalcPowerYPos()
{
  QFontMetrics fm(font());
  return fm.height() / 2;
}

int TExpr_Class::CalcIndexYPos()
{
  QFontMetrics fm(font());
  return height() - (fm.height() / 2) - 2;
}

int TExpr_Class::CalcCapDY()
{
  return 0;
}

void TExpr_Class::CalcCapDX(int &DLeft, int &DRight)
{
  DLeft = 0;
  DRight = 0;
}

int TExpr_Class::CalcMidLine(TExprOrigin origin)
{
  if (origin == eoTop)
    return height() / 2;
  return -((height() - 1) / 2);
}

void TExpr_Class::SetPenAndBrush(QPainter* painter)
{
  QPen pen;
  pen.setStyle(Qt::SolidLine);
  pen.setWidth(1);
  pen.setColor(m_color);
  painter->setPen(pen);

  QBrush brush;
  brush.setStyle(Qt::SolidPattern);
  brush.setColor(m_color);
  painter->setBrush(brush);

  //painter.setRenderHint(QPainter::Antialiasing); // <- !!!!!!!!!!!!
}

void TExpr_Class::SetPainterFont(QPainter *painter)
{
  painter->setFont(font());
}

void TExpr_Class::ConvertCoords(int &X, int &Y,
                                TExprHorAlign hAligment,
                                TExprVertAlign vAligment)
{
  switch (hAligment) {
  case ehCenter:
    X = X - (width() / 2);
    break;
  case ehRight:
    X = X - width() - 1;
    break;
  case ehLeft:
    break;
  }
  switch (vAligment) {
  case evCenter:
    Y = Y - midLineUp();
    break;
  case evBottom:
    Y = Y - height() - 1;
    break;
  case evTop:
    break;
  }
}

int TExpr_Class::CalcWidth()
{
  return 0;
}

int TExpr_Class::CalcHeight()
{
  return 0;
}

void TExpr_Class::Paint(QPainter* /*painter*/, int /*x*/, int /*y*/)
{

}

void TExpr_Class::Draw(QPainter* painter, int x, int y,
                       ExprDraw::TExprHorAlign hAligment,
                       ExprDraw::TExprVertAlign vAligment)
{
  SetPaintDevice(painter->device());

  if (m_paintDevice == 0){
    qWarning() << QString("PaintDevice is null! (%1: %2)").arg(GET_FUNC_NAME()).arg(__LINE__);
    return;
  }
  ConvertCoords(x,y,hAligment,vAligment);
  Paint(painter, x, y);
}

void TExpr_Class::Draw(int x, int y,
                       TExprHorAlign hAligment,
                       TExprVertAlign vAligment)
{
  if (m_paintDevice == 0){
    qWarning() << QString("PaintDevice is null! (%1: %2)").arg(GET_FUNC_NAME()).arg(__LINE__);
    return;
  }

  QPainter painter(paintDevice());
  ConvertCoords(x,y,hAligment,vAligment);
  Paint(&painter, x, y);
}

int TExpr_Class::GetWidth()
{
  if ((m_toChange & tcWidth) != 0){
    m_width = CalcWidth();
    m_toChange = m_toChange & ~tcWidth;
  }
  return m_width;
}

int TExpr_Class::GetHeight()
{
  if ((m_toChange & tcHeight) !=0 ){
    m_height = CalcHeight();
    m_toChange = m_toChange & ~tcHeight;
  }
  return m_height;
}

int TExpr_Class::GetMidLineUp()
{
  if ((m_toChange & tcMidLineUp) != 0){
    m_midLineUp = CalcMidLine(eoTop);
    m_toChange = m_toChange & ~tcMidLineUp;
  }
  return m_midLineUp;
}

int TExpr_Class::GetMidLineDn()
{
  if ((m_toChange & tcMidLineDn) != 0){
    m_midLineDn = CalcMidLine(eoBottom);
    m_toChange = m_toChange & ~tcMidLineDn;
  }
  return m_midLineDn;
}

int TExpr_Class::GetPowerXPos()
{
  if ((m_toChange & tcPowerXPos) != 0){
    m_powerXPos = CalcPowerXPos();
    m_toChange = m_toChange & ~tcPowerXPos;
  }
  return m_powerXPos;
}

int TExpr_Class::GetPowerYPos()
{
  if ((m_toChange & tcPowerYPos) != 0){
    m_powerYPos = CalcPowerYPos();
    m_toChange = m_toChange & ~tcPowerYPos;
  }
  return m_powerYPos;
}

int TExpr_Class::GetIndexXPos()
{
  if ((m_toChange & tcIndexXPos) != 0){
    m_indexXPos = CalcIndexXPos();
    m_toChange = m_toChange & ~tcIndexXPos;
  }
  return m_indexXPos;
}

int TExpr_Class::GetIndexYPos()
{
  if ((m_toChange & tcIndexYPos) != 0){
    m_indexYPos = CalcIndexYPos();
    m_toChange = m_toChange & ~tcIndexYPos;
  }
  return m_indexYPos;
}

int TExpr_Class::GetCapDXLeft()
{
  if ((m_toChange & tcCapDX) != 0){
    CalcCapDX(m_capDXLeft, m_capDXRight);
    m_toChange = m_toChange & ~tcCapDX;
  }
  return m_capDXLeft;
}

int TExpr_Class::GetCapDXRight()
{
  if ((m_toChange & tcCapDX) != 0){
    CalcCapDX(m_capDXLeft, m_capDXRight);
    m_toChange = m_toChange & ~tcCapDX;
  }
  return m_capDXRight;
}

int TExpr_Class::GetCapDY()
{
  if ((m_toChange & tcCapDY) != 0){
    m_capDY = CalcCapDY();
    m_toChange = m_toChange & ~tcCapDY;
  }
  return m_capDY;
}

