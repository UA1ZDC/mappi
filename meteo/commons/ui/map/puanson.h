#ifndef METEO_COMMONS_UI_MAP_PUANSON_H
#define METEO_COMMONS_UI_MAP_PUANSON_H

#include "label.h"

#include <qimage.h>

#include <commons/meteo_data/meteo_data.h>
#include <commons/meteo_data/tmeteodescr.h>
#include <meteo/commons/proto/puanson.pb.h>
#include <meteo/commons/proto/surface.pb.h>

namespace meteo {
namespace map {

class PuansonAirport;
class Wind;
class Flow;

class Puanson : public Label
{
  public:
    Puanson( Layer* layer );
    Puanson( Object* parent );
    Puanson( Projection* proj );
    Puanson( const meteo::Property& prop );
    Puanson();
    ~Puanson();

    enum {
      Type = kPuanson
    };
    virtual int type() const { return Type; }

    Object* copy( Layer* l ) const ;
    Object* copy( Object* o ) const ;
    Object* copy( Projection* proj ) const ;

    bool visible( float scalepower ) const ;

    virtual const puanson::proto::Puanson& punch() const ;
    bool hasPunch() const { return ( true == punch().IsInitialized() ); }
    bool hasWind() const ;
    bool hasFlow() const ;
    TMeteoParam paramValue( const QString& name, bool* ok = 0 ) const ;
    TMeteoParam paramValue( const puanson::proto::Id& id, bool* ok = 0 ) const ;
    TMeteoParam dd( bool* ok = 0 ) const;
    TMeteoParam ff( bool* ok = 0 ) const;

    TMeteoParam dn( bool* ok = 0 ) const;
    TMeteoParam cn( bool* ok = 0 ) const;

    const TMeteoData& meteodata() const { return meteodata_; }

    void loadTestMeteoData();

    virtual void setPunch( const puanson::proto::Puanson& p );
    bool setPunch( const QString& path );
    bool setPunchById( const QString& id );

    void setUV(float u, float v);
    void setDdff( int dd, int ff, control::QualityControl ddqual = control::RIGHT, control::QualityControl ffqual = control::RIGHT );
    void setDncn( int dn, int cn, control::QualityControl dnqual = control::RIGHT, control::QualityControl cnqual = control::RIGHT );
    void setParamValue( const QString& name, const TMeteoParam& param );
    void setParamValue( const puanson::proto::Id& id, const TMeteoParam& param );
    void setMeteodata( const TMeteoData& m );
    void windCorrection( float angle );

    //прямоугольники пуансона на документе
    QList<QRect> boundingRect( const QTransform& transform ) const ;
    //прямоугольник пуансона относительно центра пуансона
    QRect boundingRect() const ;


    //прямоугольник элемента пуансона относительно центра пуансона
    QRect boundingRect( const puanson::proto::CellRule& rule ) const ;
    //прямоугольник элемента пуансона относительно центра пуансона
    QRect boundingRect( const puanson::proto::Id& id, const QPoint& pos, puanson::proto::Align align, const puanson::proto::CellRule::NumbeRule& rule ) const ;
    QRect boundingRect( const puanson::proto::Id& id, const QPoint& pos, puanson::proto::Align align, const puanson::proto::CellRule::SymbolRule& rule ) const ;

    QList<GeoVector> skeletInRect( const QRect& rect, const QTransform& transform ) const ;


//    bool render( QPainter* painter, const QRect& target, const QTransform& transform );

    bool hasValue() const { return false; }
    float value() const { return 0.0; }
    void setValue( float value, const QString& format = "4' '.2'0'", const QString& unit = QString() )
    { Q_UNUSED(value); Q_UNUSED(format); Q_UNUSED(unit); }

    int minimumScreenDistance( const QPoint& pos, QPoint* cross = 0 ) const ;

    static QMap<QString, QString> stringForCloud(const meteo::surf::MeteoDataProto* data = nullptr, const QString& punch = QString());

    QString stringForParam( const QString& name ) const ;
    QStringList stringForParamList( const QString& name ) const ;
    QFont fontForParam( const QString& name ) const ;
    QColor colorForParam( const QString& name ) const ;
    std::string fontForParamHtml( const std::string& name ) const ;
    std::string colorForParamHtml( const std::string& name ) const ;

    //дополнительные функции для работы с идентификатором метеодескриптора
    //! Получить имя дескриптора по номеру descr и индексу index.
    //! Если величина с прибабахом, а индекс не найден,
    //! то в d_index будет записан первый валидный индекс, а вернется его наименование
    static QString descriptorName( int64_t descr, QString* description, int index = -1, int* dindex = 0 );
//    static puanson::proto::Id descr2Id( int64_t descr, int index = -1, int* dindex = 0 );
    static bool fakeDescriptor( const QString& name );
    static void setMeteoParam( const QString& name, const TMeteoParam& param, TMeteoData* data );

    GeoVector windSkelet( bool* ok = 0 ) const ;
    GeoVector flowSkelet( bool* ok = 0 ) const ;

    GeoVector bindedSkelet( bool* ok = 0 ) const ;

    Wind* wind() const { return wind_; }
    Flow* flow() const { return flow_; }

    const std::string& punchString() const ;

    bool bindedToSkelet() const { return bindedtoskelet_; }
    void setBindedToSkelet( bool fl ) { bindedtoskelet_ = fl; }

    float bindedAngle( const QPoint& pnt, bool* res ) const ;

    void setLenft( float l );
    float lenft() const { return lenft_; }

    int32_t dataSize() const ;
    int32_t serializeToArray( char* arr ) const ;
    int32_t parseFromArray( const char* data );

  protected:
    bool isEmpty() const ;
    QList< QPair< QPoint, float > > screenPoints( const QTransform& transform ) const ;
    puanson::proto::Puanson punch_;

  private:
    TMeteoData meteodata_;
    Wind* wind_;
    Flow* flow_;
    mutable QRect cached_rect_;
    mutable std::string cached_punch_string_;
    bool bindedtoskelet_;
    float lenft_;

    QSize size() const ;
    bool preparePixmap();


    QList<QPoint> plist_;//экранных координат, полученных при отрисовке на посл. документе

    friend PuansonAirport;
};

}
}

#endif
