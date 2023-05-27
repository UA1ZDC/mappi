#include "multichoosebtn.h"
#include "multichoosedlg.h"

#include <cross-commons/debug/tlog.h>

MultiChooseBtn::MultiChooseBtn( QWidget* p)
  : QToolButton(p)
{
  setDefault();
  QObject::connect( this, SIGNAL( clicked() ), this, SLOT( slotClicked() ) );
}

MultiChooseBtn::~MultiChooseBtn()
{
}

void MultiChooseBtn::setIcons(const QList<QPair<QString, QString> > &i )
{
  icons_ = i;
  if ( 0 == icons_.size() ) {
    setDefault();
    return;
  }
}

void MultiChooseBtn::setIndexes(const QList<int> &list)
{
  indexes_.clear();
  indexes_ = list;

  QStringList strList;
  for (int i = 0, isz = icons_.size(); i < isz; ++i )
  {
    if (indexes_.contains(i))
    {
      strList << icons_[i].second;

    }
  }
  QString tooltip = strList.join(", ");
  QToolButton::setToolTip( tooltip );
  emit changed(indexes_);
}

void MultiChooseBtn::setCols(int cols)
{
  if (cols > 0)
  {
    cols_ = cols;
  }
}

QStringList MultiChooseBtn::indexesToStringList() const
{  
  QStringList strList;
  for (int i = 0, isz = indexes_.size(); i < isz; ++i )
  {
     strList << QString::number(indexes_.at(i));
  }
  return strList;
}

MultiChooseBtn* MultiChooseBtn::addIco(const QPair<QString, QString> &pair )
{
  icons_.append(pair);
  return this;
}

void MultiChooseBtn::slotClicked()
{
  MultiChooseDlg* dlg = new MultiChooseDlg(this);
  dlg->move( QWidget::mapToGlobal( QPoint(0,0) ) + QPoint( 0, height() ) );
  dlg->setCols(cols_);
  dlg->setIndexes(indexes_);
  dlg->setIcons(icons_);
  dlg->exec();
  QList<int> list = dlg->indexes();
  delete dlg;
  setIndexes(list);
}

void MultiChooseBtn::setDefault()
{
  cols_ = kCols;
  setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
//  QToolButton::setIcon( QIcon(":/meteo/icons/misc/critical-message.png") );
//  QToolButton::setToolTip( QObject::tr("Набор иконок не установлен") );
}
