#include "freezedlabel.h"

#include "label.h"

#include "document.h"

namespace meteo {
namespace map {

FreezedLabelRect::FreezedLabelRect( Label* parent, const QRect& rect, const QPoint& pntnew, const QPoint& pntold, float angle )
  : parent_(parent),
  rect_(rect),
  pntnew_(pntnew),
  pntold_(pntold),
  angle_(angle)
{
}

FreezedLabelRect::~FreezedLabelRect()
{
}

bool FreezedLabelRect::render( QPainter* pntr ) const
{
  return parent_->render( pntr, rect_, pntnew_, pntold_, angle_ );
}

FreezedLabel::FreezedLabel( Label* lbl )
  : label_(lbl)
{
}

FreezedLabel::~FreezedLabel()
{
}

void FreezedLabel::render( QPainter* pntr, const QRect& target, Document* doc )
{
  for ( auto r : rects_ ) {
    r.render(pntr);
  }
  for ( auto o : label_->childs() ) {
    o->render( pntr, target, doc );
  }
}

void FreezedLabel::addRect( const QRect& rect, const QPoint& pntnew, const QPoint& pntold, float angle )
{
  FreezedLabelRect flr( label_, rect, pntnew, pntold, angle );
  rects_.append(flr);
}

FreezedLabelList::FreezedLabelList( Document* doc, const QRect& t )
  : doc_(doc),
  target_(t)
{
}

FreezedLabelList::~FreezedLabelList()
{
}

void FreezedLabelList::render( QPainter* pntr )
{
  for ( auto it = labels_.begin(), end = labels_.end(); it != end; ++it ) {
    auto lmap = it.value();
    for ( auto lit = lmap.begin(), lend = lmap.end(); lit != lend; ++lit ) {
      lit.value().render( pntr, target_, doc_ );
    }
  }
}

bool FreezedLabelList::addRect( Label* lbl, const QRect& rect, const QPoint& pntnew, const QPoint& pntold, float angle )
{
  if ( false == lbl->drawAlways() ) {
    for ( auto r : labelrects_ ) {
      if ( true == r.intersects(rect) || true == rect.intersects(r) || true == r.contains(rect) || true == rect.contains(r) ) {
        return false;
      }
    }
  }
  labelrects_.append(rect);

  if ( nullptr == lastlbl_ || lastlbl_->label() != lbl ) {
    FreezedLabel& freezedlbl = labels_[ lbl->priority() ][lbl];
    freezedlbl.setLabel(lbl);
    lastlbl_ = &freezedlbl;
  }

  lastlbl_->addRect( rect, pntnew, pntold, angle );
  return true;
}

}
}
