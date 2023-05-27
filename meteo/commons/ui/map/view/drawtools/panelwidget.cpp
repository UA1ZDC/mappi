#include "panelwidget.h"

#include <qevent.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>

#include <cross-commons/debug/tlog.h>


namespace meteo {
namespace map {

PanelWidget::PanelWidget(QWidget* parent)
  : QWidget(parent),
    movePanel_(false),
    header_(0),
    body_(0)
{
  setWindowFlags(windowFlags() | Qt::Dialog);
  setWindowFlags(windowFlags() | Qt::FramelessWindowHint);

  header_ = new PanelHeader(this);

  connect( header_, SIGNAL(collapsed()), SLOT(slotToggleCollapse()) );
  connect( header_, SIGNAL(expanded()), SLOT(slotToggleCollapse()) );

  QVBoxLayout* l = new QVBoxLayout(this);
  l->setMargin(2);
  l->insertWidget(0, header_);
}

void PanelWidget::setMainWidget(QWidget* widget)
{
  body_ = widget;

  QVBoxLayout* l = qobject_cast<QVBoxLayout*>(layout());
  l->insertWidget(1, widget);
}

void PanelWidget::setOption(PanelWidget::Option opt, bool enable)
{
  Q_UNUSED( enable );

  header_->setOption(opt, enable);
}

bool PanelWidget::isCollapsed() const
{
  return header_->isCollapsed();
}

void PanelWidget::slotToggleCollapse()
{
  slotSetCollapse(header_->isCollapsed());
}

void PanelWidget::slotSetCollapse(bool collapse)
{
  if ( 0 == body_ ) { return; }

  body_->setVisible(!collapse);
  header_->slotSetCollapse(collapse);

  int w = width();
  adjustSize();
  resize(w, size().height());
}

void PanelWidget::mousePressEvent(QMouseEvent* e)
{
  pressOffset_ = e->pos();
  movePanel_ = ( header_->label_ == childAt(pressOffset_) ) ;
  if ( movePanel_ ) {
    setCursor(Qt::SizeAllCursor);
  }
  QWidget::mousePressEvent(e);
}

void PanelWidget::mouseReleaseEvent(QMouseEvent* e)
{
  movePanel_ = false;
  unsetCursor();
  QWidget::mouseReleaseEvent(e);
}

void PanelWidget::mouseMoveEvent(QMouseEvent* e)
{
  if ( movePanel_ && e->buttons() & Qt::LeftButton ) {
    QPoint newPos = pos() + e->pos() - pressOffset_;
    move(newPos);
  }
  QWidget::mouseMoveEvent(e);
}
//
// PanelHeader
//
PanelHeader::PanelHeader(QWidget* parent)
  : QWidget(parent),
    layout_(0),
    label_(0),
    collapseBtn_(0),
    collapse_(false)
{
  layout_ = new QGridLayout(this);
  layout_->setMargin(2);
  layout_->setSpacing(2);

  label_ = new QLabel(this);
  label_->setStyleSheet("QLabel { background-color: #5555dd; }");
  label_->setMaximumHeight(20);
  layout_->addWidget(label_, 0, 0);

  collapseBtn_ = new QPushButton("-", this);
  collapseBtn_->setMaximumSize(QSize(20,20));
  collapseBtn_->setMinimumSize(QSize(20,20));
  collapseBtn_->setVisible(false);
  collapseBtn_->setFocusPolicy(Qt::NoFocus);
  layout_->addWidget(collapseBtn_, 0, 1);

  connect( collapseBtn_, SIGNAL(clicked(bool)), SLOT(slotCollapseBtnClicked()) );
}

void PanelHeader::setOption(PanelWidget::Option opt, bool enable)
{
  switch ( opt ) {
    case PanelWidget::kCollapseButton: {
      collapseBtn_->setVisible(enable);
    } break;
    default:
    {}
  }
}

void PanelHeader::slotSetCollapse(bool collapse)
{
  collapse_ = collapse;
  collapseBtn_->setText(collapse_ ? "+" : "-");
}

void PanelHeader::slotCollapseBtnClicked()
{
  slotSetCollapse(!collapse_);
  if ( collapse_ ) { emit collapsed(); } else { emit expanded(); }
}

} // map
} // meteo
