#include "antfollow.h"

#include <unistd.h>

#include <qtimer.h>
#include <qmutex.h>

#include <cross-commons/app/paths.h>
#include <cross-commons/debug/tlog.h>
#include <cross-commons/funcs/mn_funcs.h>

#include <mappi/device/antenna/mantcoord.h>
#include <mappi/device/antenna/prbl-ant.h>
#include <mappi/device/antenna/smallant.h>
#include <mappi/device/antenna/suzhetant.h>
#include <mappi/device/antenna/cafant.h>

namespace mappi {
  namespace receive {

    //! шаг времени между двумя точками траектории, мс
    //static constexpr float kTrackPointStep = 0.5f;

    //static QString DEBUG_DIR = MnCommon::varPath("mappi") + "/debug/spsctrld";       //!< путь к файлам с траекторией сеанса

    AntFollow::AntFollow()
      : editMutex_(new QMutex())
      , workMutex_(new QMutex())
      , timer_(new QTimer(this))
    {
      qRegisterMetaType<meteo::GeoPoint>("meteo::GeoPoint");
      qRegisterMetaType<conf::AntennaConf>("conf::Antenna");
      QObject::connect(timer_, &QTimer::timeout,             this,   &AntFollow::setNextPosition);
      QObject::connect(this,   &AntFollow::startTimer,       timer_, static_cast<void (QTimer::*)(int)>(&QTimer::start));
      QObject::connect(this,   &AntFollow::stopTimer,        timer_, &QTimer::stop);
      QObject::connect(this,   &AntFollow::check,            this,   &AntFollow::slotCheck);
      QObject::connect(this,   &AntFollow::init,             this,   &AntFollow::slotInit);
      QObject::connect(this,   &AntFollow::setStartPosition, this, &AntFollow::slotStartPosition);
    }

    AntFollow::~AntFollow()
    {
      delete track_;
      delete editMutex_;
      delete workMutex_;
      delete ant_;
    }

    //инициализация (safe-thread, if(editMutex.isLock()) return false; )
    bool AntFollow::slotInit(const meteo::GeoPoint& coord, const conf::AntennaConf& conf)
    {
      if(false == editMutex_->tryLock(1000)) return false;
      bool ok;
      stationCoord_ = coord;

      ant_ = AntFollow::createAntenna(conf.type());
      ant_->setZeroAzimut(conf.azimut_correct());
      trackStep_ = conf.track_step();
    
      track_ = new MnDevice::MAntCoord(ant_);
      track_->setAngleType( MnDevice::DEGREE );

      ok = ant_->openPort(QString::fromStdString(conf.port()));

      editMutex_->unlock();

      if(!ok) {
	error_log << QObject::tr("Ошибка открытия порта");
      }
      return ok;
    }

    bool AntFollow::setSTLE(const MnSat::STLEParams &stle)
    {
      if(false == editMutex_->tryLock(1000)) return false;
      stle_ = stle;
      editMutex_->unlock();
      return true;
    }


    //TODO проверка тока (safe-thread, if(editMutex.isLock()) return false; )
    bool AntFollow::slotCheck()
    {
      if(false == editMutex_->tryLock(1000)) return false;

      bool ok = true;

      ok &= ant_->isOpenPort();
      if(!ok) {
	error_log << QObject::tr("Ошибка открытия порта");
	//Q_EMIT(checked(ok));
	//return ok;
      }
      else {
	ant_->setPower(true);

	//----
	float angle;
	for(auto idx = 0; idx < 15; ++idx) {
	  ok = ant_->currentAzimut(&angle);
	  if (ok) break;
	  usleep(100000);
	}

	if (!ok) {
	  error_log << QObject::tr("Ошибка получения текущего азимута антенны");
	  //Q_EMIT(checked(ok));
	  //return ok;
	}

	antAzimut_ = angle;

	for(auto idx = 0; idx < 5; ++idx) {
	  ok = ant_->currentElevat(&angle);
	  if (ok) break;
	  usleep(100000);
	}

	if (!ok) {
	  error_log << QObject::tr("Ошибка получения текущего угла места антенны");
	}
	antElevation_ = angle;
	//----

	Q_EMIT(positionChanged(azimut_, antAzimut_, elevation_, antElevation_));
      }

      editMutex_->unlock();
      Q_EMIT(checked(true));
      return true;
    }

    bool AntFollow::isWorking()
    {
      return worked_;
    }

    // (safe-thread, if(editMutex.isLock()) return false; )
    /*!
     * \brief AntFollow::setStartPosition - Установка начального положения
     * \param session - Сеанс
     * \param wait    - true - подождать пока антенна будет устновлена в начальное положение
     */
    bool AntFollow::slotStartPosition(const QDateTime& aos, const QDateTime& los, bool wait)
    {
      if(false == editMutex_->tryLock(1000)) return false;


      ant_->setPower(true);

      bool ok = getTrackSatellite(aos, los);
      if(ok) {
	trackIdx_ = 0;
	azimut_ = track_->startPointAzimut(ant_->zeroAzimut());
	elevation_ = (*track_)[trackIdx_].elevat();
	++trackIdx_;

	debug_log << "start pos = " << track_->dt(0).toString("hh:mm:ss.zzz") << azimut_ << elevation_ << "azCorr=" << ant_->zeroAzimut();

	ok = ant_->setPosition(azimut_, elevation_, wait);
	editMutex_->unlock();
	if (!ok) {
	  error_log << QObject::tr("Ошибка установки антенны в начальное положение");
	}
      }
      editMutex_->unlock();
      return ok;
    }

    //! Запуск движения по траектории (public, editMutex_.lock())
    bool AntFollow::startBlocking()
    {
      editMutex_->lock();
      if (!ant_->isOpenPort()) {
	editMutex_->unlock();
	return false;
      }
      worked_ = true;
      Q_EMIT(startTimer(trackStep_ * 1000));
      return true;
    }

    //! Запуск движения по траектории (public, editMutex_.tryLock(msec))
    bool AntFollow::start(int msec)
    {
      if(false == editMutex_->tryLock(msec)){
	error_log << QObject::tr("Работа с потоком управления антенны заблокирована");
	return false;
      }
      if (!ant_->isOpenPort()) {
	editMutex_->unlock();
	return false;
      }
      worked_ = true;
      Q_EMIT(startTimer(trackStep_ * 1000));
      return true;
    }

    //! Установка следующего положения траектории
    void AntFollow::setNextPosition()
    {
      if(false == workMutex_->tryLock()) return;

      int64_t mtcount = track_->count();

      /*  var(trackIdx_);
	  var(mtcount);*/

      QDateTime cur = QDateTime::currentDateTimeUtc();
      while (trackIdx_ < mtcount &&
	     cur.msecsTo(track_->dt(trackIdx_)) < trackStep_ * 500) {
	++trackIdx_;
      }

      int state = 0;
      if ( trackIdx_ < mtcount ) {
	MnDevice::MAntPoint point = (*track_)[ trackIdx_ ];
	if (!ant_->movePosition(point.azimut(), point.elevat(), &antAzimut_, &antElevation_)) {
	  state = ant_->state(); //TODO в smallant не всегда удачно было при чтении текущего положения. для повторной передачи координат
	}

	//if (trackIdx_ % 10 == 0) {
/*	  debug_log << trackIdx_ << "(" << mtcount << ")"
		    << track_->dt(trackIdx_ - 1).toString("hh:mm:ss.zzz") << azimut_ << elevation_
		    << QDateTime::currentDateTimeUtc().toString("hh:mm:ss.zzz") << antAzimut_ << antElevation_
		    << "|\t" << fabs(azimut_ - antAzimut_) << fabs(elevation_ - antElevation_);*/
	  debug_log << trackIdx_ << "(" << mtcount << ")"
		    << azimut_ << elevation_ << "\t"
		    << antAzimut_ << antElevation_
		    << "|\t" << fabs(azimut_ - antAzimut_) << fabs(elevation_ - antElevation_) << "\n";
	
	  //}

	elevation_ = point.elevat();
	azimut_ = point.azimut();

	++trackIdx_;

        if (trackIdx_ % int(1./trackStep_) == 0) { //чтоб не отправлять слишком часто на гую
	  Q_EMIT(positionChanged(azimut_, antAzimut_, elevation_, antElevation_));
	}
      }
      
      if (1 == state) {
	QTimer::singleShot(100, this, SLOT(setNextPosition()));
      }

      workMutex_->unlock();
    }

    //! Остановка движения по траектории (public, editMutex_.unlock())
    void AntFollow::stop()
    {
      Q_EMIT(stopTimer());

      //Ожидаем завершения функции setNextPosition
      if(worked_) {
	workMutex_->lock();
	ant_->stop(&antAzimut_, &antElevation_);
	workMutex_->unlock();
	worked_ = false;
      }
      //Пробуем заблокировать mutex если вдруг он не заблокирован(для устранение UB)

      editMutex_->tryLock();
      editMutex_->unlock();

      Q_EMIT(positionChanged(azimut_, antAzimut_, elevation_, antElevation_));
    }

    void AntFollow::finish()
    {
      //Остонавливаем управление антеной и посылаем сигнал завершения
      stop();
      Q_EMIT(finished());
    }

    /*!
     * \brief Получение траектории спутника для заданного сеанса
     * \return true если траектория получена.
     */
    bool AntFollow::getTrackSatellite(const QDateTime& aos, const QDateTime& los)
    {
      Satellite satellite;
      bool is_OK = satellite.readTLE(stle_);
      if ( is_OK ) {
	//if ( MnCommon::mnMkdir( DEBUG_DIR.toUtf8() ) == 0 )
	{
	  QDateTime dt = QDateTime::currentDateTime();
	  //QString pathFile = ( QString( "%1/" ).arg( DEBUG_DIR ) + dt.toString( "dd.MM.yyyy_hh:mm:ss" ) );
	  dt = QDateTime::currentDateTimeUtc();
	  QList< MnSat::TrackTopoPoint > track;
	  is_OK = satellite.trajectory((aos < dt ? dt : aos),
				       los,
				       trackStep_,
				       stationCoord_,
				       //pathFile,
				       track);

	  if ( is_OK ) {
	    track_->setTrack( track );
	  } else {
	    error_log << QObject::tr( "Траектория спутника не определена" );
	  }
	}
      }

      return is_OK;
    }

    bool AntFollow::getTrackGeo( double az, double el )
    {
      QValueList<MnSat::TrackTopoPoint> track;
      MnSat::TrackTopoPoint point;
      point.az = az;
      point.el = el;
      track.append(point);
      track_->setTrack(track);

      return true;
    }


    PrblAnt* AntFollow::createAntenna(conf::AntennaType type)
    {
      switch (type) {
      case conf::kSuzhetAntenna:
	return new SuzhetAnt;
	break;
      case conf::kSmallAntenna:
	return new SmallAnt;
	break;
      case conf::kCafAntenna:
	return new CafAnt;
	break;

      default: {
	error_log << QObject::tr("Ошибка создания антенны типа %1. Создание по умолчанию антенны kSuzhetAntenna").arg(type);
	return new SuzhetAnt;
      }
      }
    }

  
  } // receive
} // mappi
