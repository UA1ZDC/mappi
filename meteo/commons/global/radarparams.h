#ifndef METEO_COMMONS_GLOBAL_RADARPARAMS_H
#define METEO_COMMONS_GLOBAL_RADARPARAMS_H

#include <qstring.h>

#include <commons/funcs/tcolorgrad.h>
#include <meteo/commons/proto/map_radar.pb.h>

namespace meteo {
namespace map {

class RadarParams {
  public:
    RadarParams( const QString& path );
    ~RadarParams();

    void loadParamsFromDir();

    const proto::RadarColors& protoParams() const { return params_; }
    const proto::RadarColor& protoParams( int descr ) const ;
    const proto::RadarColor& protoParams( const QString& name ) const ;

    bool contains( int descr ) const ;
    bool contains( const QString& name ) const ;

    TColorGradList gradParams( int descr );

    void setParams( const proto::RadarColor& isoline );
    void setParams( const proto::RadarColors& isoline );
    void removeParams( int descr );

    bool saveParams();

    static float gradMin( const proto::RadarColor& color );
    static float gradMax( const proto::RadarColor& color );

    static float gradStep( const proto::RadarColor& color );

    static proto::RadarColor gradlist2gradientcolor( const TColorGradList& gradlist );
    static TColorGradList gradParams( const proto::RadarColor& fieldcolor );

    static void setGradColor( const TColorGradList& grad, proto::RadarColor* color );
    static void setGradStep( float step, proto::RadarColor* color );

  private:
    const QString path_;
    proto::RadarColors params_;

    proto::RadarColor emptycolor_;
};

}
}

#endif
