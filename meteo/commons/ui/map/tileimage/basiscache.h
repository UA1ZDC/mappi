#ifndef METEO_COMMONS_UI_MAP_TILEIMAGE_BASISCASCHE_H
#define METEO_COMMONS_UI_MAP_TILEIMAGE_BASISCASCHE_H

#include <qmap.h>
#include <qstring.h>
#include <qrect.h>

#include <meteo/commons/proto/map_document.pb.h>

class QPainter;

namespace meteo {
namespace map {

class Document;
class Layer;

class BasisCache
{
  public:
    BasisCache();
    ~BasisCache();

    bool flushBasis( Document* d );
    bool renderBasis( Document* doc, QPainter* pntr );
    void loadGeoBasis( Document* doc );

    bool hasTile( const proto::Document& doc, int x, int y ) const ;
    QByteArray getTile( const proto::Document& doc, int x, int y ) const ;
    QString tilePath( const proto::Document& doc, int x, int y ) const ;

  private:
    inline QString rectFileName( Document* d, const QRect& r ) const ;

    bool containsRect( Document* d, const QRect& r ) const ;

    void flushRect( Document* d, const QRect& r );

    bool renderRect( const QRect& r, Document* d, QPainter* p );
};

}
}

#endif
