#ifndef METEO_COMMONS_GLOBAL_GRADIENTPARAMS_H
#define METEO_COMMONS_GLOBAL_GRADIENTPARAMS_H

#include <qstring.h>

#include <commons/funcs/tcolorgrad.h>
#include <meteo/commons/proto/map_isoline.pb.h>

namespace meteo {
namespace map {

class GradientParams {
  public:
    GradientParams( const QString& path );
    ~GradientParams();

    void loadParamsFromDir();

    const proto::FieldColors& protoParams() const { return params_; }
    const proto::FieldColor& protoParams( int descr ) const ;
    const proto::FieldColor& protoParams( const QString& name ) const ;

    bool contains( int descr ) const ;
    bool contains( const QString& name ) const ;

    TColorGrad isoParams( int descr );
    TColorGrad isoParams( int level, int type_level, int descr );
    TColorGrad isoParams( const QString& name );
    TColorGrad isoParams( int level, int type_level, const QString& name );

    TColorGradList gradParams( int descr );
    TColorGradList gradParams( int level, int type_level, int descr );
    TColorGradList gradParams( int level, int type_level, const QString& name );

    void setParams( const proto::FieldColor& isoline );
    void removeParams( int descr );

    bool saveParams();

    static proto::LevelColor levelProto( int level, int type_level, const proto::FieldColor& color );

    static void setLevelProto( int level, int type_level, const proto::LevelColor& clr, proto::FieldColor* color );

    static TColorGrad isoParams( int level, int type_level, const proto::FieldColor& color );
    static TColorGradList gradParams( int level, int type_level, const proto::FieldColor& color );

    static float gradMin( const proto::FieldColor& color );
    static float gradMin( int level, int type_level, const proto::FieldColor& color );
    static float gradMax( const proto::FieldColor& color );
    static float gradMax( int level, int type_level, const proto::FieldColor& color );

    static float isoMin( const proto::FieldColor& color );
    static float isoMin( int level, int type_level, const proto::FieldColor& color );
    static float isoMax( const proto::FieldColor& color );
    static float isoMax( int level, int type_level, const proto::FieldColor& color );

    static QColor isoColorMin( const proto::FieldColor& color );
    static QColor isoColorMax( const proto::FieldColor& color );
    static QColor isoColorMin( int level, int type_level, const proto::FieldColor& color );
    static QColor isoColorMax( int level, int type_level, const proto::FieldColor& color );

    static float gradStep( const proto::FieldColor& color );
    static float gradStep( int level, int type_level, const proto::FieldColor& color );

    static float isoStep( const proto::FieldColor& color );
    static float isoStep( int level, int type_level, const proto::FieldColor& color );

    static proto::LevelColor gradlist2gradientcolor( const TColorGradList& gradlist, int level = -1, int type_level = -1 );
    static proto::LevelColor grad2levelcolor( const TColorGrad& gradlist, int level = -1, int type_level = -1 );

    static void setIsoColor( int level, int type_level, const TColorGrad& grad, proto::FieldColor* color );
    static void setIsoStep( int level, int type_level, float step, proto::FieldColor* color );
    static void setIsoMin( int level, int type_level, float min, proto::FieldColor* color );
    static void setIsoMax( int level, int type_level, float max, proto::FieldColor* color );
    static void setIsoColorMin( int level, int type_level, const QColor& clr, proto::FieldColor* color );
    static void setIsoColorMax( int level, int type_level, const QColor& clr, proto::FieldColor* color );

    static void setIsoColor( const TColorGrad& grad, proto::FieldColor* color );
    static void setIsoStep( float step, proto::FieldColor* color );
    static void setIsoMin( float min, proto::FieldColor* color );
    static void setIsoMax( float max, proto::FieldColor* color );
    static void setIsoColorMin( const QColor& clr, proto::FieldColor* color );
    static void setIsoColorMax( const QColor& clr, proto::FieldColor* color );

    static void setGradColor( int level, int type_level, const TColorGradList& grad, proto::FieldColor* color );
    static void setGradStep( int level, int type_level, float step, proto::FieldColor* color );
    static void setGradColor( const TColorGradList& grad, proto::FieldColor* color );
    static void setGradStep( float step, proto::FieldColor* color );

  private:
    const QString path_;
    proto::FieldColors params_;

    proto::FieldColor emptycolor_;
};

}
}

#endif
