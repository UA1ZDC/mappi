#include "filterheader.h"

#include <qmenu.h>
#include <qcursor.h>
#include <qwidgetaction.h>

#include <cross-commons/debug/tlog.h>

#include "filteredit.h"

FilterHeader::FilterHeader( QTreeWidget* t )
  : QHeaderView( Qt::Horizontal, t ),
  tree_(t),
  clickedcolumn_(-1),
  order_( Qt::AscendingOrder ),
  sortcolumn_(-1)
{
  QTreeWidgetItem* item = tree_->headerItem();
  for ( int i = 0, sz = tree_->columnCount(); i < sz; ++i ) {
    defnames_.insert( i, item->text(i) );
  }
  QHeaderView::setStretchLastSection(true);
  QObject::connect( this, SIGNAL( sectionClicked( int ) ), this, SLOT( slotSectionClicked( int ) ) );
}

FilterHeader::~FilterHeader()
{
}

// при сортивовке из кода, параметры становятся не актуальными. возможно, помогла бы обработка каких-то сигналов об изменении сортировки. 
void FilterHeader::setCortColumn(int col, Qt::SortOrder order)
{
  clickedcolumn_ = order;
  sortcolumn_ = col;
}

const QString &FilterHeader::getFilter(int column)
{
  return filters_[column];
}

//! установка колонки, по которой сортировка отключена
void FilterHeader::setUnsortedColumn(int col)
{
  unsorted_.clear();
  unsorted_ << col;
}

void FilterHeader::reloadFilters()
{
  for ( int i = 0, sz = tree_->topLevelItemCount(); i < sz; ++i ) {
    QTreeWidgetItem* item = tree_->topLevelItem(i);
    if ( 0 == filters_.size() ) {
      item->setHidden(false);
    }
    else  {
      QMapIterator<int,QString> it(filters_);
      bool fl = false;
      while ( true == it.hasNext() ) {
        it.next();
        int clmn = it.key();
        const QString& f = it.value();
        if ( -1 == item->text(clmn).indexOf( f, 0, Qt::CaseInsensitive ) ) {
          fl = true;
          break;
        }
      }
      item->setHidden(fl);
    }
  }
}

void FilterHeader::slotSectionClicked( int indx )
{
  if (unsorted_.contains(indx)) {
    if (sortcolumn_ != -1) {
      setSortIndicator(sortcolumn_, order_);
    } else {
      setSortIndicator(-1, Qt::AscendingOrder);
    }
    return;
  }

  if ( true == QHeaderView::isSortIndicatorShown() ) {
    QHeaderView::setSortIndicator( sortcolumn_, order_ );
  }
  clickedcolumn_ = indx;
  QMenu* menu = new QMenu(this);
  QWidgetAction* a = new QWidgetAction(menu);
  FilterEdit* fe = new FilterEdit(menu, a);
  fe->setMinimumWidth( sectionSize(indx) );
  QTreeWidgetItem* header = tree_->headerItem();
  if ( header->text(clickedcolumn_) != defnames_[clickedcolumn_] ) {
    fe->setFilter( header->text(clickedcolumn_) );
  }
  int x = 0;
  while ( 0 != indx ) {
    x += sectionSize(--indx);
  }
  QPoint pnt(x, height() );
  a->setDefaultWidget(fe);
  QObject::connect( a, SIGNAL( triggered() ), this, SLOT( slotSetFilter() ) );
  menu->addAction(a);
  menu->addAction( QObject::tr("По возрастанию"), this, SLOT( slotSortAscent() ) );
  menu->addAction( QObject::tr("По убыванию"), this, SLOT( slotSortDescent() ) );
  menu->setActiveAction(a);
  menu->setDefaultAction(a);
  fe->setFocusPolicy( Qt::StrongFocus );
  fe->setFocus( Qt::OtherFocusReason );
  menu->setFocusProxy(fe);
  menu->exec( mapToGlobal(pnt) );
  delete menu;
  if ( -1 != tree_->sortColumn() ) {
    QHeaderView::setSortIndicatorShown(true);
    QHeaderView::setSortIndicator( sortcolumn_, order_ );
  }
  clickedcolumn_ = -1;
}

void FilterHeader::slotSortAscent()
{
  if ( -1 == clickedcolumn_ || unsorted_.contains(clickedcolumn_)) {
    return;
  }
  tree_->sortByColumn( clickedcolumn_, Qt::AscendingOrder );
  QHeaderView::setSortIndicatorShown(true);
  order_ = Qt::AscendingOrder;
  QHeaderView::setSortIndicator( clickedcolumn_, order_ );
  sortcolumn_ = clickedcolumn_;
  emit sortChanged( clickedcolumn_, order_ );
}

void FilterHeader::slotSortDescent()
{
  if ( -1 == clickedcolumn_  || unsorted_.contains(clickedcolumn_)) {
    return;
  }
  tree_->sortByColumn( clickedcolumn_, Qt::DescendingOrder );
  QHeaderView::setSortIndicatorShown(true);
  order_ = Qt::DescendingOrder;
  QHeaderView::setSortIndicator( clickedcolumn_, order_ );
  sortcolumn_ = clickedcolumn_;
  emit sortChanged( clickedcolumn_, order_ );
}

void FilterHeader::slotSetFilter()
{
  if ( -1 == clickedcolumn_ ) {
    return;
  }
  QWidgetAction* a = qobject_cast<QWidgetAction*>( sender() );
  if ( 0 == a ) {
    return;
  }
  FilterEdit* fe = qobject_cast<FilterEdit*>( a->defaultWidget() );
  if ( 0 == fe ) {
    return;
  }
  QTreeWidgetItem* header = tree_->headerItem();
  QFont f = header->font(clickedcolumn_);
  if ( false == fe->hasFilter() ) {
    header->setText( clickedcolumn_, defnames_[clickedcolumn_] );
    f.setBold(false);
    filters_.remove(clickedcolumn_);
  }
  else {
    header->setText( clickedcolumn_, fe->filter() );
    f.setBold(true);
    filters_.insert( clickedcolumn_, fe->filter() );
  }
  header->setFont( clickedcolumn_, f );
  reloadFilters();
}
