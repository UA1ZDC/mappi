#include "iconsetbtn.h"

#include <qdir.h>
#include <qfileinfo.h>

#include <cross-commons/debug/tlog.h>
#include <meteo/commons/global/common.h>
#include "iconsetdlg.h"

IconsetButton::IconsetButton( QWidget* p )
  : QToolButton(p),
  index_(-1)
{
  setDefault();
  QObject::connect( this, SIGNAL( clicked() ), this, SLOT( slotClicked() ) );
}

IconsetButton::~IconsetButton()
{
}

void IconsetButton::setIcons( const QList< QPair< QIcon, QString> >& i )
{
  icons_ = i;
  if ( 0 == icons_.size() ) {
    setDefault();
    return;
  }
  setCurrentIndex(0);
}

void IconsetButton::setIcons( const QList<QIcon>& list )
{
  icons_.clear();
  for ( int i = 0, sz = list.size(); i < sz; ++i ) {
    if ( false == list[i].isNull() ) {
      icons_.append( qMakePair( list[i], list[i].name() ) );
    }
  }
  if ( 0 == icons_.size() ) {
    setDefault();
    return;
  }
  setCurrentIndex(0);
}

void IconsetButton::loadIconsFromPath( const QString& path )
{
  QDir dir( path );
  if ( false == dir.exists() ) {
    error_log << QObject::tr("Директория %1 не существует. Загузка иконок невозможна")
      .arg( dir.absolutePath() );
    return;
  }
  QFileInfoList list = dir.entryInfoList( QDir::Files | QDir::NoDotAndDotDot );
  QList< QPair< QIcon, QString > > ilist;
  for ( int i = 0, sz = list.size(); i < sz; ++i ) {
    const QFileInfo& info = list[i];
    QIcon ico( info.absoluteFilePath() );
    if ( false == ico.isNull() ) {
      ilist.append( qMakePair( ico, info.absoluteFilePath() ) );
    }
  }
  setIcons(ilist);
}

void IconsetButton::setCurrentIndex( int indx )
{
  if ( indx == index_ ) {
    return;
  }
  if ( indx >= icons_.size() || 0 > indx ) {
    setDefault();
    return;
  }
  index_ = indx;
  QToolButton::setIcon( icons_[indx].first );
  QToolButton::setToolTip( icons_[indx].second );
  emit indexChanged(index_);
}

QIcon IconsetButton::currentIcon() const
{
  if ( index_ < 0 || icons_.size() <= index_ ) {
    return QIcon();
  }
  return icons_[index_].first;
}

QString IconsetButton::currentText() const
{
  if ( index_ < 0 || icons_.size() <= index_ ) {
    return QString();
  }
  return icons_[index_].second;
}

IconsetButton* IconsetButton::addIco( const QPair< QIcon, QString >& pair )
{
  icons_.append(pair);
  if ( 0 > index_ ) {
    setCurrentIndex(0);
  }
  return this;
}

void IconsetButton::setPoint(QPoint point)
{
  point_ = point;
  setPoint_ = true;
}

void IconsetButton::slotClicked()
{
  IconsetDlg dlg;
  if ( false != setPoint_ ) {
    dlg.move( mapToGlobal( point_ ) );
  }
  else {
    dlg.move( QWidget::mapToGlobal( QPoint(0,0) ) + QPoint( 0, height() ) );
  }
  dlg.setIcons(icons_);
  int res = dlg.exec();
  int indx = dlg.currentIndex();
  if ( QDialog::Accepted == res ) {
    setCurrentIndex(indx);
  }
}

void IconsetButton::setDefault()
{
  index_ = -1;
  QToolButton::setIcon( QIcon(":/meteo/icons/misc/critical-message.png") );
  QToolButton::setToolTip( QObject::tr("Набор иконок не установлен") );
}
