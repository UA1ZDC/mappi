#ifndef METEO_COMMONS_UI_MAP_VIEW_PUANSON_H
#define METEO_COMMONS_UI_MAP_VIEW_PUANSON_H

#include <qgraphicsview.h>

namespace meteo {
namespace puanson {

class Gv : public QGraphicsView
{
  Q_OBJECT
  public:
    Gv( QWidget* parent = 0 );
    ~Gv();

  protected:
    void scrollContentsBy( int dx, int dy );
};

}
}

#endif
