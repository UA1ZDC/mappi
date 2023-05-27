#include "multichoosedlg.h"
#include "multichoosebtn.h"

#include <qlayout.h>
#include <qtoolbutton.h>
#include <qcolordialog.h>
#include <cmath>

#include <cross-commons/debug/tlog.h>

MultiChooseDlg::MultiChooseDlg(QWidget* parent)
  : QDialog(parent),
  layout_(new QGridLayout(this)),
  cols_(kCols),
  allbtn_(new QToolButton(this))

{
  setWindowFlags(Qt::Popup);
  layout_->setMargin(1);
  layout_->setSpacing(1);
  allbtn_->setCheckable(true);
  allbtn_->setAutoRaise(true);
  allbtn_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  connect(allbtn_,SIGNAL(toggled(bool)), this, SLOT(slotSelectAll(bool)));
}

MultiChooseDlg::~MultiChooseDlg()
{
  delete allbtn_; allbtn_ = 0;
}

void MultiChooseDlg::setIcons( const QList< QPair< QString, QString > >& icons )
{
  for ( int i = 0, sz = buttons_.size(); i < sz; ++i ) {
    QToolButton* btn = buttons_[i];
    layout_->removeWidget(btn);
    delete btn;
  }
  buttons_.clear();
  icons_ = icons;
  QSize btnsize( kIconMinWidth, kIconMinHeight );
  int curCount = 0;
  int count = icons_.size();
  int maxRow = ceil(((double)count)/((double)cols_));
  for (int row = 0, rsz = maxRow + 1; row < rsz; ++row )
  {
    if (0 == row) //!< первая строка - служебные кнопки
    {
      for (int col = 0, csz = cols_ + 1; col < csz; ++col )
      {
        if (0 == col)
        {
          allbtn_->setText("*");
          if (indexes_.size() == count)
          {
            allbtn_->setChecked(true);
          }
          layout_->addWidget(allbtn_, row, col );
        }
        else
        {
          QToolButton* btn = new QToolButton(this);
          btn->setCheckable(true);
          btn->setAutoRaise(true);
          btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
          btn->setText("*/"+QString::number(col));
          vbtns_.append(btn);
          layout_->addWidget(btn, row, col );
          connect(btn, SIGNAL(toggled(bool)), this, SLOT(slotSelectCol(bool)));
        }
      }
    }
    else
    {
      for (int col = 0, csz = cols_ + 1; col < csz; ++col )
      {
        if (0 == col)
        {
          QToolButton* btn = new QToolButton(this);
          btn->setCheckable(true);
          btn->setAutoRaise(true);
          btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
          btn->setText(QString::number(curCount) + "-" + QString::number(curCount + cols_ - 1));
          hbtns_.append(btn);
          layout_->addWidget(btn, row, col );
          connect(btn, SIGNAL(toggled(bool)), this, SLOT(slotSelectRow(bool)));
        }
        else
        {
          QToolButton* btn = new QToolButton(this);
          btn->setCheckable(true);
          btn->setMinimumSize(btnsize);
          btn->setText(icons_[curCount].first);
          btn->setToolTip( icons_[curCount].second );
          btn->setAutoRaise(true);
          buttons_.append(btn);
          int curIndex = buttons_.indexOf(btn);
          if (true == indexes_.contains(curIndex))
          {
            btn->setChecked(true);
          }
          layout_->addWidget(btn, row, col );
          ++curCount;
          QObject::connect( btn, SIGNAL(toggled(bool)), SLOT(slotChooseIcon()) );
          if (curCount == count)
          {
            break;
          }
        }
      }
      if (curCount == count)
      {
        break;
      }
    }
  }
  adjustSize();
}

void MultiChooseDlg::slotSelectAll(bool select)
{
  for (int i = 0, isz = buttons_.size(); i < isz; ++i)
  {
    QToolButton* btn = buttons_[i];
    if (0 != btn)
    {
      btn->setChecked(select);
    }
  }
}

void MultiChooseDlg::slotSelectRow(bool select)
{
  QToolButton* btn = qobject_cast<QToolButton*>(sender());
  if ( 0 == btn )
  {
    return;
  }
  int curIndex = hbtns_.indexOf(btn);
  if (-1 == curIndex) { return; }
  int minIndex = cols_*curIndex;
  int maxIndex = minIndex + cols_;
  for (int i = minIndex; i < maxIndex; ++i)
  {
    QToolButton* btn = buttons_[i];
    if (0 != btn)
    {
      btn->setChecked(select);
    }
  }
}

void MultiChooseDlg::slotSelectCol(bool select)
{
  QToolButton* btn = qobject_cast<QToolButton*>(sender());
  if ( 0 == btn )
  {
    return;
  }
  int curIndex = vbtns_.indexOf(btn);
  if (-1 == curIndex) { return; }
  for (int i = curIndex, isz = buttons_.size(); i < isz; i = i+cols_)
  {
    QToolButton* btn = buttons_[i];
    if (0 != btn)
    {
      btn->setChecked(select);
    }
  }
}

void MultiChooseDlg::setCols(int cols)
{
  if (cols > 0)
  {
    cols_ = cols;
  }
}

void MultiChooseDlg::setIndexes(const QList<int> &list)
{
  indexes_.clear();
  indexes_ = list;
}

void MultiChooseDlg::slotChooseIcon()
{
  QToolButton* btn = qobject_cast<QToolButton*>(sender());
  if ( 0 == btn )
  {
    return;
  }
  int curIndex = buttons_.indexOf(btn);
  if (true == btn->isChecked())
  {
    if (indexes_.contains(curIndex))
    {
      return;
    }
    indexes_ << curIndex;
  }
  else if (false == btn->isChecked())
  {
    if (false == indexes_.contains(curIndex))
    {
      return;
    }
    indexes_.removeAt(indexes_.indexOf(curIndex));
  }
}
