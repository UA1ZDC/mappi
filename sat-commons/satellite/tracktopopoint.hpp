#ifndef SRC_TRACKTOPOPOINT_H
#define SRC_TRACKTOPOPOINT_H

#include <commons/mathtools/mnmath.h>

namespace MnSat {

    class Angle {
    private:
        float _value; // значение угла в радианах или радианах/с
    public:
        Angle(float v = 0) : _value(v){}; // то же самое что fromRad
        static Angle fromDeg(float v){
            return Angle(MnMath::deg2rad(v));
        }
        static Angle fromRad(float v){
            return Angle(v);
        }
        float toDeg() const {
            return MnMath::rad2deg(_value);
        }
        float toRad() const {
            return _value;
        }
        QString toString() const {
            return QString("MnSat::Angle %1deg(%2rad)").arg(toDeg()).arg(toRad());
        }
        ~Angle() {};
    };
    class AngleWithVelocity {
    public:
        Angle _value;
        Angle _velocity;
    public:
        AngleWithVelocity(float a=0, float v=0){
            _value = Angle(a);
            _velocity = Angle(v);
        };
        AngleWithVelocity(Angle a){
            _value = a;
            _velocity = Angle();
        };
        AngleWithVelocity(Angle a, Angle v){
            _value = a;
            _velocity = v;
        };
        ~AngleWithVelocity() {};

        Angle value() const {
            return _value;
        }
        AngleWithVelocity& setValueDeg(float v){
            _value = Angle::fromDeg(v);
            return *this;
        }
        AngleWithVelocity& setValueRad(float v){
            _value = Angle::fromRad(v);
            return *this;
        }
        AngleWithVelocity& setValue(Angle v){
            _value = v;
            return *this;
        }

        Angle velocity() const {
            return _velocity;
        }
        AngleWithVelocity& setVelocityDeg(float v){
            _velocity = Angle::fromDeg(v);
            return *this;
        }
        AngleWithVelocity& setVelocityRad(float v){
            _velocity = Angle::fromRad(v);
            return *this;
        }
        AngleWithVelocity& setVelocity(Angle v){
            _velocity = v;
            return *this;
        }
        QString toString() const{
            return QString("MnSat::AngleWithVelocity: %1[%2]").arg(_value.toString()).arg(_velocity.toString());
        }
    };

    //! Точка траектории спутника в топографических координатах.
    class TrackTopoPoint {
    private:
        AngleWithVelocity _azimut;                       //азимут, радиан
        AngleWithVelocity _elevation;                    //угол места, радиан
        QDateTime _time; //время прохождения точки
    public:
        TrackTopoPoint(AngleWithVelocity a, AngleWithVelocity e, QDateTime t = QDateTime::fromMSecsSinceEpoch(0)) :
                _azimut(a),
                _elevation(e),
                _time(t){};
        TrackTopoPoint(float a, float e, float a_vel, float e_vel, QDateTime t = QDateTime::fromMSecsSinceEpoch(0)){
            AngleWithVelocity azimut(a, a_vel);
            AngleWithVelocity elevation(e, e_vel);
            _azimut = azimut;
            _elevation = elevation;
            _time = t;
        };
        TrackTopoPoint(float a, float e, QDateTime t = QDateTime::fromMSecsSinceEpoch(0)){
            AngleWithVelocity azimut(a, 0);
            AngleWithVelocity elevation(e, 0);
            _azimut = azimut;
            _elevation = elevation;
            _time = t;
        };
        TrackTopoPoint(){
            AngleWithVelocity azimut(0, 0);
            AngleWithVelocity elevation(0, 0);
            _azimut = azimut;
            _elevation = elevation;
            _time = QDateTime::fromMSecsSinceEpoch(0);
        };
        ~TrackTopoPoint() {};

        TrackTopoPoint& setAzimut(AngleWithVelocity v){
            _azimut = v;
            return *this;
        }
        TrackTopoPoint& setAzimut(Angle a, Angle v){
            _azimut = AngleWithVelocity(a, v);
            return *this;
        }
        TrackTopoPoint& setAzimut(float a, float v){
            _azimut = AngleWithVelocity(a, v);
            return *this;
        }
        TrackTopoPoint& setAzimut(Angle v){
            _azimut.setValue(v);
            return *this;
        }
        TrackTopoPoint& setAzimut(float v){
            _azimut.setValueRad(v);
            return *this;
        }
        AngleWithVelocity getAzimut() const {
            return _azimut;
        }
        float az() const {
            return _azimut.value().toRad();
        }

        TrackTopoPoint& setElevation(AngleWithVelocity v){
            _elevation = v;
            return *this;
        }
        TrackTopoPoint& setElevation(Angle a, Angle v){
            _elevation = AngleWithVelocity(a, v);
            return *this;
        }
        TrackTopoPoint& setElevation(float a, float v){
            _elevation = AngleWithVelocity(a, v);
            return *this;
        }
        TrackTopoPoint& setElevation(Angle v){
            _elevation.setValue(v);
            return *this;
        }
        TrackTopoPoint& setElevation(float v){
            _elevation.setValueRad(v);
            return *this;
        }
        AngleWithVelocity getElevation() const {
            return _elevation;
        }
        float el() const {
            return _elevation.value().toRad();
        }

        QDateTime time() const {
            return _time;
        }
        QString seconds() const {
            return "T" + _time.toString("sszzz");
        }
        QString toString() const {
            return QString("MnSat::TrackTopoPoint: %1T az:%2 el:%3")
                .arg(_time.toString("hh:mm:ss.zzz"))
                .arg(_azimut.toString())
                .arg(_elevation.toString());
        }
    };
}
#endif //SRC_TRACKTOPOPOINT_H
