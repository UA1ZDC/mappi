#include "gv.h"

namespace meteo {
namespace puanson {

Gv::Gv( QWidget* p )
  : QGraphicsView(p)
{
}

Gv::~Gv()
{
}

void Gv::scrollContentsBy( int dx, int dy )
{
  QGraphicsView::scrollContentsBy( dx, dy );
}

}
}
