/**
   Класс представляет собой реализацию алгоритма прогона.
   Действие "прогонка", постоянное передвижение антенны с постоянной скоростью,
   определяемую параметрами DSA и DSE. Параметр определяет сколько градусов в секунду
   проходит антенна. По достижении максимальных углов антенна начинает двигаться
   в обратном направлении и так до бесконечночти.

   @author	Бурыкин Е.А.
   @date	21.08.2009
*/
#ifndef MAPPI_DEVICE_PRBL_ANT_ANTPROGON_H
#define MAPPI_DEVICE_PRBL_ANT_ANTPROGON_H


namespace mappi {
  namespace receive {

    class AntProgon {

      float m_azimut;			// угол по азимуту
      float m_elevat;			// угол по углу места
      float m_azMin; //минимальный азимут
      float m_azMax; //максимальный азимут
      float m_elMin; //минимальный угол места
      float m_elMax; //максимальный угол места
      float m_DSA;			// количество градусов в секунду по азимуту
      float m_DSE;			// количество градусов в секунду по углу места
      float m_dsaMax;			// Максимальное значение количества градусов в секунду по азимуту
      float m_dseMax;			// Максимальное значение количества градусов в секунду по углу места
      
      // начальное направление хода
      bool m_isPositivDirectAzimut;	// true положительное
      bool m_isPositivDirectElevat;	// true положительное, т.е. ближе к 180 чем к 0
    
      /** Определение направления хода. */
      void direction();
    
      AntProgon( const AntProgon& object );
      AntProgon& operator=( const AntProgon& object );
    
    public :
    
      AntProgon(float dsaMax, float dseMax, float azMin, float azMax, float elMin, float elMax);
      ~AntProgon();
    
      /**
	 Установка начального положения антенны, градусы.
	 @return true - если значения допустимы.
      */
      void setStartPosition( const double& az, const double& el );
    
      /**
	 Устанвока шага прогона по азимуту, градусы.
	 Максимальное значение DSA = 9 град\сек.
	 @return true - если значение допустимо.
      */
      bool setDSA( const double& value );
    
      /**
	 Установка шага прогона по углу места, градусы.
	 Максимальное значение DSE = 10 град\сек.
	 @return true - если значение допустимо.
      */
      bool setDSE(const double& value );
    
      double azimut() { return m_azimut; }
      double elevat() { return m_elevat; }

      /** Следующие значение для азимута. */
      double nextAzimut();
    
      /** Следующие значение для угла места. */
      double nextElevat();
    
    };
  }
}


#endif //_MANTPROGON_
