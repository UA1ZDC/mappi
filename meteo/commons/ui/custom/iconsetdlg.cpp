#include "iconsetdlg.h"

#include <qlayout.h>
#include <qtoolbutton.h>
#include <qcolordialog.h>

#include <cross-commons/debug/tlog.h>

IconsetDlg::IconsetDlg(QWidget* parent)
  : QDialog(parent),
  index_(-1),
  layout_(new QGridLayout(this))
{
  setWindowFlags(Qt::Popup);

  layout_->setMargin(1);
  layout_->setSpacing(1);
}

void IconsetDlg::setIcons( const QList< QPair< QIcon, QString > >& icons )
{
  for ( int i = 0, sz = buttons_.size(); i < sz; ++i ) {
    QToolButton* btn = buttons_[i];
    layout_->removeWidget(btn);
    delete btn;
  }
  buttons_.clear();
  icons_ = icons;
  QSize btnsize( 32, 32 );
  if ( 0 != parent() ) {
    QToolButton* prntbtn = qobject_cast<QToolButton*>( parent() );
    if ( 0 != prntbtn ) {
      btnsize = prntbtn->size();
    }
  }

  int j = 0;
  for ( int i = 0, sz = icons_.size(); i < sz; ++i ) {
    j = i/10;
    QToolButton* btn = new QToolButton(this);
    btn->setMinimumSize(btnsize);
    btn->setMaximumSize(btnsize);
    btn->setIcon( icons_[i].first );
    btn->setToolTip( icons_[i].second );
    btn->setAutoRaise(true);
    buttons_.append(btn);
    layout_->addWidget(btn, i-j*10, j );
    QObject::connect( btn, SIGNAL(clicked(bool)), SLOT(slotChangeIcon()) );
    QObject::connect( btn, SIGNAL(clicked(bool)), SLOT(accept()) );
  }
  adjustSize();
}

QIcon IconsetDlg::currentIcon() const
{
  if ( index_ >= icons_.size() || 0 > index_ ) {
    return QIcon();
  }
  return icons_[index_].first;
}

void IconsetDlg::slotChangeIcon()
{
  QToolButton* btn = qobject_cast<QToolButton*>(sender());
  if ( 0 == btn ) {
    index_ = -1;
    return;
  }
  index_ = buttons_.indexOf(btn);
}
