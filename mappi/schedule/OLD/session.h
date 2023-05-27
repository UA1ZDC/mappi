#ifndef MAPPI_SCHEDULE_SESSION_H
#define MAPPI_SCHEDULE_SESSION_H

#include <mappi/schedule/sat_schedule.h>
#include <mappi/proto/schedule.pb.h>

namespace mappi {
  namespace schedule {

    class Session :
      public satellite::Schedule {


    public :

      //! стадия сеанса
      enum stage_t {
        MISSING       = 0,  //!< пропущен
        ALREADY_TAKEN = 1,  //!< уже принимается
        EXPECTED      = 2   //!< ожидается
      };

      Session();
      Session( const Session& object );
      Session( const satellite::Schedule& object );
      Session( const conf::Session& object);
      Session& operator=( const Session& object );
      ~Session();

      Session& operator=( const satellite::Schedule& object );
      bool operator==( const Session& object ) const;

      /*! \brief Получить название спутника. */
      QString getNameSat() const { return name; }
      /*! \brief Получить время начала сеанса. */
      QDateTime getAos() const { return aos; }
      /*! \brief Получить время завершения сеанса. */
      QDateTime getLos() const { return los; }
      /*! \brief Получить угол места в кульминации. */
      double getMaxEl() const { return maxEl; }
      /*! \brief Получить номер витка. */
      unsigned long int getRevol() const { return revol; }
      /*! \brief Получить направление спутника. */
      conf::SatDirection getDirection() const { return direction; }
      /*! \brief Получить состояние конфликта. */
      conf::ConflState getState() const { return state; }

      /*! \brief Установить название спутника. */
      void setNameSat( const QString& value ) { name = value; }
      /*! \brief Установить время начала сеанса. */
      void setAos( const QDateTime& value ) { aos = value; }
      /*! \brief Установить время завершения сеанса. */
      void setLos( const QDateTime& value ) { los = value; }
      /*! \brief Установить угол места в кульминации. */
      void setMaxEl( const double& value ) { maxEl = value; }
      /*! \brief Установить номер витка. */
      void setRevol( const unsigned long int& value ) { revol = value; }
      /*! \brief Установить направление спутника. */
      void setDirection( const conf::SatDirection& value ) { direction = value; }
      /*! \brief Установить состояние конфликта. */
      void setState( const conf::ConflState& value ) { state = value; }

      void defaults();
      bool isDefaults() const;
      bool adopted() const;
      stage_t currentStage() const;
      stage_t dtStage(const QDateTime& dt) const;
      int secsToAos() const;
      int secsToLos() const;
      int length() const;

      bool toSession( const QString& line );
      QString toLine() const;

    };

  }
}

#endif //_SESSION_
