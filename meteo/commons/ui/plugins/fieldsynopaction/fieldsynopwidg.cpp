#include "fieldsynopwidg.h"
#include <meteo/commons/ui/mainwindow/widgethandler.h>

namespace meteo {
namespace map {

namespace{
  QWidget* createDialog( QWidget* parent, const QString& options = 0 )
  {
    Q_UNUSED(options);
    FieldSynopWidget* dlg = new FieldSynopWidget( parent );
    dlg->show();
    return dlg;
  }

  bool registerDialog()
  {
    mapauto::WidgetHandler* hndl = WidgetHandler::instance();
    return hndl->registerWidget( "fieldsynopwidget", createDialog );
  }

static bool res = registerDialog();
}

FieldSynopWidget::FieldSynopWidget(QWidget *parent) :
  QDialog(parent),
  label_(0),
  close_(0),
  color_(56,123,248)
{
  setAttribute( Qt::WA_TranslucentBackground, true );
  layout_ = new QHBoxLayout(this);
  layout_->setSpacing(0);
  layout_->setContentsMargins(2,2,2,12);
  label_ = new QLabel;
  close_ = new QToolButton();
  close_->setMaximumSize(QSize(16,16));
  close_->setMinimumSize(QSize(16,16));
  close_->setIcon(QIcon(":/meteo/icons/map/close.png"));
  close_->setIconSize(QSize(8,8));

  layout_->addWidget(label_);
  layout_->addWidget(close_);

  connect(close_, SIGNAL(clicked()), this, SLOT(close()));
  setBtnVisible(false);
}

FieldSynopWidget::~FieldSynopWidget()
{
}

void FieldSynopWidget::setText(const QString &text)
{
  label_->setText(text);
  adjustSize();
}

const QString FieldSynopWidget::text() const
{
  return label_->text();

}

FieldSynopWidget::Type FieldSynopWidget::typeWidget() const
{
  return type_;

}

void FieldSynopWidget::setCloseVisible(bool on)
{
  close_->setVisible(on);
}

void FieldSynopWidget::setBtnVisible(bool on)
{

  close_->setVisible(on);
}

void FieldSynopWidget::setTypeWidget(FieldSynopWidget::Type type)
{
  type_ = type;

}

void FieldSynopWidget::setViewportPos(const QPoint &pos)
{
  viewportPos_ = pos;

}

QPoint FieldSynopWidget::viewportPos() const
{
  return viewportPos_;

}

void FieldSynopWidget::setFrameColor(const QColor &color)
{
  color_ = color;

}

void FieldSynopWidget::slotClose()
{

}

void FieldSynopWidget::mousePressEvent(QMouseEvent *e)
{
  QWidget::mousePressEvent(e);

}

void FieldSynopWidget::paintEvent(QPaintEvent *)
{
  QPainter painter(this);
  QPen pen(color_);
  pen.setWidth(1);
  QBrush brush(Qt::white);
  brush.setStyle(Qt::SolidPattern);
  painter.setPen(pen);
  painter.setBrush(brush);

  if( CloseButton == type_ ) {
    QPolygon poly;
    poly.append(rect().topLeft());
    poly.append(rect().topRight());
    poly.append(rect().bottomRight() - QPoint(0, 10));
    poly.append(rect().bottomLeft() - QPoint(-20, 10));
    poly.append(rect().bottomLeft() - QPoint(-13, 0));
    poly.append(rect().bottomLeft() -  QPoint(-7, 10));
    poly.append(rect().bottomLeft() -  QPoint(0, 10));
    painter.drawPolygon(poly);
  }
  if( DoubleButton == type_ ) {
    painter.drawRect(0,0, width(), height()-10);
  }
}

void FieldSynopWidget::moveEvent(QMoveEvent *e)
{
//  WidgetHandler::instance()->setValue(objectName(), viewportPos_);
  QDialog::moveEvent(e);

}


}
}
