#ifndef DOCOPTIONS_H
#define DOCOPTIONS_H

#include <qwidget.h>
#include <qmap.h>
#include <meteo/commons/proto/map_document.pb.h>
#include <commons/geobasis/geopoint.h>

class QString;

namespace Ui{
   class DocOptions;
}

class DocOptions : public QWidget {
  Q_OBJECT
  public:
    DocOptions( QWidget * parent = nullptr);
    ~DocOptions();

    void setCoord(const meteo::GeoPoint &pnt);
    void setProjection(const meteo::ProjectionType &proj);
    void setScale( double scale);

    void setPunchOrient( int orient );
    void setRuleVeiw( int qual );
    void setIsoLabel( int orient );
    void setIsoSmooth( bool fl );
    void setLoader( const QString& loader );

    void setCrossType( int type );
    void setOutline( bool fl );
    void setOutlineColor( QRgb color );
    void setBackgroundColor( QRgb color );

    void loadMapSettings();
    bool saveMapSettings();

    void setCache( bool fl );

    bool setDoc(const meteo::map::proto::Document& doc);

    meteo::ProjectionType projection() const ;
    double scale() const ;
    double lat_radian() const ;
    double lon_radian() const ;
    double lat_degree() const;
    double lon_degree() const;

    int punchOrient() const ;
    int ruleVeiw() const ;
    int isoLabel() const ;
    bool isoSmooth() const;

    QString loader() const ;

    int crossType() const ;
    bool outline() const ;
    QRgb outlineColor() const ;
    QRgb backgroundColor() const;

    bool cache() const ;

    QString projectionText() const ;
    QString scaleText() const ;

    meteo::map::proto::Document doc() const ;

  private slots:
      void slotChanged();
      void slotSwitch();

  private:
      Ui::DocOptions* ui_;
      QMap< QString, meteo::Property > settings_;

  signals:
      void changed();
};


#endif
