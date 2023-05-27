/**
   Класс предназначен для преобразования координат полученных из траектории спутника
   в координаты допустимые для анетнны. Корректировки подвергается только азимут.
   По умолчанию мера угла радианая. Для антенны азимут может быть в пределах от -270 до 270.
   Траектория спутника по азимуту от 0 до 360. Угол места в пределах от 0 до 179.
   Траектория спутника от 0 до 90.

   Значение угла по азимуту разбивается на четверти согласно следующему правилу:
   I   - всегда положительна;
   II  - может быть положительной так и отрицательной, все зависит от предыдущей четверти;
   III - может быть положительной так и отрицательной, все зависит от предыдущей четверти;
   IV  - всегда отрицательна.
   Знак спорной четверти определяется знаком предыдущей четверти.
   Переход II, III, IV и IV III II считается отрицательным.
   Переход I, II, III и III, II, I считается положительным
   Смена знака возможна только при переходе через ноль.

   !!!ВНИМАНИЕ если начальная четверть перехода была признана отрицательной и начальный угол будет больше -270
   то значения свыше -270 будут проигнорированы. Также это касается положительной начальной четверти все значения свыше 270
   будут проигнорированы. Такое ограничение -270 и 270 связано с конструктивной особенностью антенны.

             0
             |
         IV  |  I
   270 ______|______ 90
             |
         III |  II
             |
            180

   @author	Бурыкин Е.А.
   @date	26.02.09
*/
#ifndef _MANTCOORD_
#define _MANTCOORD_


#include <sat-commons/satellite/satellite.h>
#include <qlist.h>

namespace mappi {
  namespace receive {
    class PrblAnt;
  }
}

namespace MnDevice {
  
  enum angle_t {
    RADIAN,	// ...радианы
    DEGREE	// ...градусы
  };


  //class MAntCoord;

  // положение антенны в пространстве, точка (азимут и угол места)
  class MAntPoint {

    //friend class MAntCoord;

    double m_azimut;
    double m_elevat;

  public :

    MAntPoint() {
      m_azimut = 0.0;
      m_elevat = 0.0;
    };

    ~MAntPoint() {
    };

    /** Устаноить значение точки. */
    inline void setPoint( const double& azimut, const double& elevat ) {
      m_azimut = azimut;
      m_elevat = elevat;
    };

    /** Метод возвращает значение азимута. */
    inline double azimut() const { return m_azimut; };

    /** Метод возвращает значение угла места. */
    inline double elevat() const { return m_elevat; };

  };


  // координаты антенны
  class MAntCoord {
    const  mappi::receive::PrblAnt* m_ant = 0;
         
    QList< MnSat::TrackTopoPoint > m_listTrack;	// ...список координат полученных от класса Satellite
    angle_t m_angleType;	// ...мера угла в которой будут возвращаться значения

    unsigned int m_indexMaxEl;
    double m_maxEl;
    double m_azFullRate;	// в радианах

    void print();
    void findMaxEl();
    void correctRacingIntervalEl();
    bool isCoup();
    bool isMaxRateAzimut();
    void coup();
    int signCoup();
    void convertTransitionAz();
    bool isAzJumpZero( unsigned int* position = 0 );
    bool isAzQuarterIV( unsigned int* position = 0 );

    MAntCoord( const MAntCoord& object );
    MAntCoord& operator=( const MAntCoord& object );

  public :

    MAntCoord( mappi::receive::PrblAnt* ant);
    ~MAntCoord();

    /** Установить меру в которой будет возвращаться угол. */
    void setAngleType( const angle_t& type );

    /** Возвращает количество точек траектории спутника. */
    unsigned int count() const;

    /** Доступ к точке с индексом index. */
    MAntPoint& operator[]( unsigned int index );
    QDateTime& dt(int idx) { return m_listTrack[ idx ].time; }

    /** Метод возвращает допустимый угол для начальной точки траектории по азимуту. */
    double startPointAzimut(float zeroCorrect);

    /** Установить траекторию, для дальнейшего пересчета. */
    void setTrack( const QList< MnSat::TrackTopoPoint >& track );
  };
};

#endif //_MANTCOORD_
