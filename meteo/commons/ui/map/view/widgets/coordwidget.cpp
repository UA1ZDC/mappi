#include "coordwidget.h"
#include <meteo/commons/ui/mainwindow/widgethandler.h>

namespace meteo {
namespace map {

namespace{
  QWidget* createDialog( QWidget* parent, const QString& options = 0 )
  {
    Q_UNUSED(options);
    CoordWidget* dlg = new CoordWidget( parent );
    dlg->initWidget();
    dlg->show();
    return dlg;
  }

  bool registerDialog()
  {
    mapauto::WidgetHandler* hndl = WidgetHandler::instance();
    return hndl->registerWidget( "CoordWidget", createDialog );
  }

static bool res = registerDialog();
}
const int pik_height = 13;

CoordWidget::CoordWidget(QWidget *parent) :
  QDialog(parent),
  close_(0),
  lock_(0),
  label_(0),
  color_(56,123,248)
{
  setMinimumSize(QSize(200, 30));
  //setWindowFlags(Qt::FramelessWindowHint);
  setAttribute( Qt::WA_TranslucentBackground, true );
 // setAttribute(Qt::WA_DeleteOnClose);
  
  close_ = new QToolButton();
  close_->setMaximumSize(QSize(16,16));
  close_->setMinimumSize(QSize(16,16));
  close_->setIcon(QIcon(":/meteo/icons/map/close.png"));
  close_->setIconSize(QSize(8,8));
  
  lock_ = new QToolButton();
  lock_->setMaximumSize(QSize(16,16));
  lock_->setMinimumSize(QSize(16,16));
  lock_->setIcon(QIcon(":/meteo/icons/map/lock-open.png"));
  lock_->setIconSize(QSize(8,8));
  lock_->setCheckable(true);
  connect(lock_, SIGNAL(clicked(bool)), SLOT(slotClicked(bool)));
  connect(close_, SIGNAL(clicked()), this, SLOT(slotClose()));
  setBtnVisible(false);
}

void CoordWidget::slotClose(){
  //close();
  emit sclosed();
}  

void CoordWidget::initWidget(){
  layout_ = new QHBoxLayout(this);
  layout_->setSpacing(0);
  layout_->setContentsMargins(1,1,1,pik_height-2);
  label_ = new QLabel;
  layout_->addWidget(label_);
  layout_->addWidget(lock_);
  layout_->addWidget(close_);
}



CoordWidget::~CoordWidget()
{
}

bool CoordWidget::lock() const
{
  return lock_->isChecked();
}

void CoordWidget::slotClicked(bool on)
{
  setLock(on);
}


void CoordWidget::mousePressEvent(QMouseEvent*e)
{
  QWidget::mousePressEvent(e);
}



void CoordWidget::moveEvent(QMoveEvent*e)
{
//  WidgetHandler::instance()->setValue(objectName(), viewportPos_);
  QDialog::moveEvent(e);
}

void CoordWidget::setLock(bool on)
{
  if( 0 == lock_ ){
    return;
  }
  lock_->setChecked(on);
  if( true == on ){
    setBtnVisible(false);
    //setIcon(QIcon(":/meteo/icons/map/lock.png"));
  }else{
    setLockVisible(true);
    lock_->setIcon(QIcon(":/meteo/icons/map/lock-open.png"));
  }
}

void CoordWidget::setText(const QString& text)
{
  if(0 != label_){
   label_->setText(text);
   label_->adjustSize();
   adjustSize(label_->width(),label_->height());
  }
}

void CoordWidget::adjustSize(int width,int height) {
  width += contentsMargins().right();
  width += contentsMargins().left();
  width += layout_->contentsMargins().left();
  width += layout_->contentsMargins().right();
  
  height += contentsMargins().bottom();
  height += contentsMargins().top();
  height += layout_->contentsMargins().top();
  height += layout_->contentsMargins().bottom();
  
  setMinimumSize(width,height+pik_height);
  setMaximumSize(width,height+pik_height);
  resize(width,height+pik_height);
  QDialog::adjustSize();
  
}

void CoordWidget::adjustSize() {
  QDialog::adjustSize();
}

const QString CoordWidget::text() const
{
  if(0 != label_){
    return label_->text();
  }
  return QString();
}

void CoordWidget::setBtnVisible(bool on)
{
  lock_->setVisible(on);
  close_->setVisible(on);
}

void CoordWidget::setLockVisible(bool on)
{
  lock_->setVisible(on);
}

void CoordWidget::setCloseVisible(bool on)
{
  close_->setVisible(on);
}



void CoordWidget::setViewportPos(const QPoint& pos)
{
  viewportPos_ = pos;
}

QPoint CoordWidget::viewportPos() const
{
  return viewportPos_;
}

QLabel*CoordWidget::label() const
{
  return label_;
}

void CoordWidget::setFrameColor(const QColor& color)
{
  color_ = color;
}

}
}

