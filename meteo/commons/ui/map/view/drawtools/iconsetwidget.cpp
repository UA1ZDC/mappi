#include "iconsetwidget.h"

#include <qlayout.h>
#include <qtoolbutton.h>
#include <qdir.h>

#include <cross-commons/debug/tlog.h>

namespace meteo {
namespace map {

IconsetWidget::IconsetWidget(QWidget* parent)
  : QWidget(parent),
  index_(-1),
  layout_(new QGridLayout(this))
{
  setWindowFlags(Qt::Tool);

  layout_->setMargin(1);
  layout_->setSpacing(1);
}

void IconsetWidget::setIcons( const QList< QPair< QIcon, QString > >& icons )
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
  }
  adjustSize();
}

void IconsetWidget::loadIconsFromPaths(const QStringList &paths )
{
  if ( true == paths.isEmpty() ) {
    error_log << QObject::tr("Директория для загрузки не задана. Загузка иконок невозможна");
    return;
  }
  QList< QPair< QIcon, QString > > ilist;
  foreach (const QString& path, paths) {
    QDir dir( path );
    if ( false == dir.exists() ) {
      continue;
    }
    QFileInfoList list = dir.entryInfoList( QDir::Files | QDir::NoDotAndDotDot );
    for ( int i = 0, sz = list.size(); i < sz; ++i ) {
      const QFileInfo& info = list[i];
      QIcon ico( info.absoluteFilePath() );
      if ( false == ico.isNull() ) {
        ilist.append( qMakePair( ico, info.absoluteFilePath() ) );
      }
    }
  }
  setIcons(ilist);
}


QIcon IconsetWidget::currentIcon() const
{
  if ( index_ >= icons_.size() || 0 > index_ ) {
    return QIcon();
  }
  return icons_[index_].first;
}

QString IconsetWidget::currentIconPath() const
{
  if ( index_ >= icons_.size() || 0 > index_ ) {
    return "";
  }
  return icons_[index_].second;
}

bool IconsetWidget::setCurrentIconIndx(int index)
{
  if ( index < buttons_.size()) {
    index_ = index;
    return true;
  }
  return false;
}

void IconsetWidget::slotChangeIcon()
{
  QToolButton* btn = qobject_cast<QToolButton*>(sender());
  if ( 0 == btn ) {
    index_ = -1;
    return;
  }
  index_ = buttons_.indexOf(btn);
  emit iconChanged(index_);
}

}
}
