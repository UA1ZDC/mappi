#ifndef MAPPI_UI_POS_POS_H 
#define MAPPI_UI_POS_POS_H 

#include <commons/geobasis/projection.h>
#include <commons/geobasis/coords.h>

#include <qdatetime.h>

class SatViewPoint;
class SatelliteBase;

namespace MnSat{
struct TLEParams;
}

namespace meteo
{

class POSproj: public Projection
{
  public:
    POSproj();
    ~POSproj();

    enum {
      Type = POLARORBSAT
    };
    int type() const { return Type; }


    /*! 
     * \brief установка параметров спутника
     * \ts         - время начала сеанса
     * \te         - время окончания сеанса
     * \params     - параметры орбиты спутника
     * \sa         - угол сканирования
     * \w          - количество значений в строке сканирования
     * \lps        - скорость приема (строк/сек)
     */
    void setSatellite( const QDateTime& ts, const QDateTime& te, SatelliteBase* params,
                       double sa, int w, double lps );
    /*! 
     * \param brief установка параметров спутника
     * \param i          - угол наклонения орбиты
     * \param rn         - кол-во витков в сутки
     * \param ts         - время начала сеанса
     * \param te         - время окончания сеанса
     * \param sa         - угол сканирования
     * \param w          - количество значений в строке сканирования
     * \param lps        - скорость приема (строк/сек)
     */
    void setSatellite( double i, double rn, const QDateTime& ts, const QDateTime& te,
                       const Coords::EciPoint& satcoord, double sa, int w, double lps );
    void setSatellite( const QDateTime& aos, const QDateTime& los,
                       const MnSat::TLEParams& tle, double sa, int w, double lps );

    bool F2X( const GeoVector& geoPoints, QVector<QPolygon>* scrPoints, bool isClosed ) const;
    bool F2X_one(const GeoPoint& geoCoord, QPoint* meterCoord ) const;
    bool X2F_one(const QPoint& meterCoord, GeoPoint* geoCoord ) const;

    bool F2X_one(const GeoPoint& geoCoord, QPointF* meterCoord ) const;
    bool X2F_one(const QPointF& meterCoord, GeoPoint* geoCoord ) const;
    bool X2F_one(int i, GeoPoint* geoCoord ) const;
    double bt(int num) const;
    bool zenithCos(int num, double *retval) const;//косинус зенитного угла радиометра

    QDateTime date(int num) const;

    Projection* copy() const ;

    const GeoPoint& startpos() const { return startpos_; }
    const GeoPoint& endpos() const { return endpos_; }

    const QDateTime& timestart() const { return ts_; }
    const QDateTime& timestop() const { return te_; }
      
    const QDateTime& startdate() const { return ts_; }
    const QDateTime& enddate() const { return te_; }

    bool isUpward() const { return up_; }

    const SatViewPoint* satViewPoint() const{return sat_;};
    bool countGridCorners(meteo::GeoPoint* gp ) const;


  protected:
    void initRamka();

  private:
    bool calcPointKA( double du, GeoPoint* kapoint ) const ;
    bool calcDuKA( const GeoPoint& gp, double* du ) const ;
    bool calcScanAngleKA( const GeoPoint& kapoint, const GeoPoint& gp, double du, double* sa ) const ;

    SatViewPoint* sat_ = nullptr;

  private:
    double i_   = 0.0;          //!< наклонение орбиты
    double lam0_= 0.0;          //!< долгота восходящего узла
    double lam0_PI = 0.0;       //!< долгота нисходящего узла
    double rn_  = 0.0;          //!< количество витков/сутки
    double tka_ = 0.0;          //!< период обращения спутника (секунды)
    double U_ = 0.0;            //!< длина дуги от GeoPos(0, lam0) до startpos_
    bool up_ = true;
    QDateTime t0_;              //!< время, когда спутник пересекал lam0
    QDateTime t0_PI_;           //!< время, когда спутник сделал пол-витка с тех пор, как пересек lam0
    QDateTime t0_PI_2_;         //!< время, когда спутник сделал четверть-витка с тех пор, как пересек lam0
    QDateTime ts_;              //!< время начала сеанса
    QDateTime te_;              //!< время окончания сеанса
    double dt_ = 0.0;           //!< продолжительность сеанса (сек)
    GeoPoint startpos_;         //!< координата подспутниковой точки в мемент начала приема
    GeoPoint endpos_;           //!< координата подспутниковой точки в мемент окончания приема
    Coords::EciPoint satcoord_;         //!< координаты, скорость спутника в геоцентрической системе координат
    double scan_angle_ = 0.0;         //!< угол сканирования прибора
    double cos_fi_s_ = 0.0;
    double cos_fi_e_ = 0.0;
    double sin_fi_s_ = 0.0;
    double sin_fi_e_ = 0.0;
    double sin_fi_c_ = 0.0;
    double cos_fi_c_ = 0.0;
    double sin_la_c_ = 0.0;
    double cos_la_c_ = 0.0;
    double alpham_ = 0.0;
    double CC_ = 0.0;
    double DD_ = 0.0;
    int32_t width_ = 0;
    int32_t height_ = 0;
    double lps_ = 6.0;
    double duseans_ = 0.0;
};

}

#endif
