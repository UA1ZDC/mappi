#include "exprguidepaintwidget.h"

#include <qpainter.h>
#include <qevent.h>

#include <mappi/ui/exprguid/exprmake/texpr_builder.h>
#include <mappi/ui/exprguid/exprdraw/expr_draw.h>

ExprGuidePaintWidget::ExprGuidePaintWidget(QWidget *parent)
  : QWidget(parent)
  , showAxisX_(false), showAxisY_(false)
  , hAligment_(ExprDraw::ehCenter), vAligment_(ExprDraw::evCenter)
  , exprClass_(nullptr)
{
  fontFamily_ = font().family();
  fontSize_ = font().pointSize();
}
ExprGuidePaintWidget::~ExprGuidePaintWidget() {  delete exprClass_; }

QString ExprGuidePaintWidget::expr() { return expr_; }

void ExprGuidePaintWidget::showAxisX(bool value)
{
  showAxisX_ = value;
  repaint();
}

void ExprGuidePaintWidget::showAxisY(bool value)
{
  showAxisY_ = value;
  repaint();
}

void ExprGuidePaintWidget::setFontFamily(const QString& family)
{
  fontFamily_ = family;
  repaint();
}

void ExprGuidePaintWidget::setFontSize(int size)
{
  fontSize_ = size;
  repaint();
}

void ExprGuidePaintWidget::paintEvent(QPaintEvent* event)
{
  QPainter p(this);
  p.setBackground(QBrush(Qt::green));
  QWidget::paintEvent(event);
  if (showAxisX_ || showAxisY_)
    paintAxiss();
  paintExpr();
}

void ExprGuidePaintWidget::setExpr(const QString &_expr)
{
  expr_ = _expr;
  exprClass_ = exprBuilder_.BuildExpr(_expr);

  if (exprBuilder_.lastError().isValid())
    exprClass_->SetColor(Qt::red);
  else
    exprClass_->SetColor(palette().color(QPalette::Text));

  Q_EMIT finishBuildExpr(exprBuilder_.lastError());
  repaint(rect());
}

void ExprGuidePaintWidget::paintAxiss()
{
  int w = width()/2, h = height()/2;
  QPainter p;
  p.begin(this);
  QPen pen((QColor(Qt::red)));
  p.setPen(pen);
  if (showAxisX_) p.drawLine(w, 0, w, height());
  if (showAxisY_) p.drawLine(0, h, width(), h);
}

void ExprGuidePaintWidget::paintExpr()
{
  if (nullptr == exprClass_) return;

  int x = width() / 2, y = height() / 2;

  QFont fn = exprClass_->font();
  fn.setFamily(fontFamily_);
  fn.setPointSize(fontSize_);
  exprClass_->SetFont(fn);

  exprClass_->SetPaintDevice(this);
  exprClass_->Draw(x, y, hAligment_, vAligment_);
}
