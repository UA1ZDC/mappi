#include "multichoosewgt.h"

#include <qlayout.h>
#include <qtoolbutton.h>
#include <qcolordialog.h>
#include <cmath>

#include <cross-commons/debug/tlog.h>

const int kCols = 5;
const int kIconMinWidth = 64;
const int kIconMinHeight = 64;

MultiChooseWgt::MultiChooseWgt(QWidget* parent)
  : QWidget(parent),
  layout_(new QGridLayout(this)),
  cols_(kCols),
  allbtn_(new QToolButton(this))
{
  setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
//  layout_->setMargin(1);
//  layout_->setSpacing(1);
  allbtn_->setCheckable(true);
  allbtn_->setAutoRaise(true);
  QFont font = allbtn_->font();
  font.setBold(true);
  allbtn_->setFont(font);
  allbtn_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  connect(allbtn_,SIGNAL(toggled(bool)), this, SLOT(slotSelectAll(bool)));
  setStyleSheet("QWidget.MultiChooseWgt{ border: 1px solid rgb(194, 194, 194) } ");
}

MultiChooseWgt::~MultiChooseWgt()
{
  delete allbtn_; allbtn_ = 0;
}


QStringList MultiChooseWgt::indexesToStringList() const
{
  QStringList strList;
  for (int i = 0, isz = indexes_.size(); i < isz; ++i )
  {
     strList << QString::number(indexes_.at(i));
  }
  return strList;
}

void MultiChooseWgt::init()
{
  setBtns();
}

MultiChooseWgt* MultiChooseWgt::addIco(const QPair<QString, QString> &pair )
{
  icons_.append(pair);
  return this;
}

void MultiChooseWgt::setBtns()
{
  if( 0 == layout_ ){
    return;
  }
  for ( int i = 0, sz = buttons_.size(); i < sz; ++i ) {
    QToolButton* btn = buttons_[i];
    layout_->removeWidget(btn);
    delete btn;
  }
  buttons_.clear();
  hbtns_.clear();
  vbtns_.clear();
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
          QFont font = btn->font();
          font.setBold(true);
          btn->setFont(font);
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
          btn->setEnabled(true);
          btn->setAutoRaise(true);
          btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
          btn->setText(QString::number(curCount) + "-" + QString::number(curCount + cols_ - 1));
          btn->setToolTip(btn->text());
          QFont font = btn->font();
          font.setBold(true);
          btn->setFont(font);
          hbtns_.append(btn);
          layout_->addWidget(btn, row, col );
          connect(btn, SIGNAL(toggled(bool)), this, SLOT(slotSelectRow(bool)));
        }
        else
        {
          QToolButton* btn = new QToolButton(this);
          btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
          btn->setCheckable(true);
//          btn->setMinimumSize(btnsize);
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

void MultiChooseWgt::slotSelectAll(bool select)
{
  for (int i = 0, isz = buttons_.size(); i < isz; ++i)
  {
    QToolButton* btn = buttons_[i];
    if (0 != btn)
    {
      btn->setChecked(select);
    }
  }
  emit changed();
}

void MultiChooseWgt::slotSelectRow(bool select)
{
  trc;
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
  emit changed();
}

void MultiChooseWgt::slotSelectCol(bool select)
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
  emit changed();
}

void MultiChooseWgt::setCols(int cols)
{
  if (cols > 0)
  {
    cols_ = cols;
  }
}

void MultiChooseWgt::setIndexes(const QList<int> &list)
{
  indexes_.clear();
  indexes_ = list;
}

void MultiChooseWgt::slotChooseIcon()
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
  emit changed();
}
