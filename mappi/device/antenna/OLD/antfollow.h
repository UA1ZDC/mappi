#ifndef MAPPI_DEVICE_MANAGER_ANTFOLLOW_H
#define MAPPI_DEVICE_MANAGER_ANTFOLLOW_H

#include <memory>

#include <qobject.h>

#include <commons/geobasis/geopoint.h>
#include <sat-commons/satellite/satellitebase.h>
#include <mappi/proto/reception.pb.h>

#include <qmutex.h>
#include <qtimer.h>

namespace MnDevice {
  class MAntCoord;
}

namespace mappi {
  namespace schedule {
    class Session;
  }
}
namespace mappi {
  namespace receive {

    class PrblAnt;

    /*!
     * \brief The AntFollow class - Класс управления антенной
     *  Для инициализации класса используется init
     *  Для старта процесса управления используй bool startBlocking или bool start(int timeout)
     * Внимание startBlcoking не safeThread. Она заблакирует вызывающий поток если управление антенной включено.
     *  start(int msec), если управление заблокировано будет ждать msec(милисекунд).
     * Если управление антенной заблокировано и не разблокировалось в течении timeout функция вернет false, иначе true
     *  Для остановки управления вызывай void stop() или void finish();
     * void stop() - остонавливает управление и разблакирует его.
     * void finish() - останавливает управление, разблакирует его и испускает сигнал finished().
     *  Значение углов можно узнать из сигнала positionChanged(float, float, float, float)
     *  Для Корректного завершения работы в потоке нужно связать сигнал finished() со слотом deleteLater()
     *
     *  Функции init, setStartPosition, check временно блокируют управление, и всегда возвращают false,
     * если управление антенной работает
     *  safe-thread функции: init, setStartPosition, check, start, stop, finish.
     *
     *  slotCheck использовать только из одного потока
     *
     *  Пример:
     *  void Foo::connectToAntFollow() {
     *    ant_ = new AntFollow();
     *    QThread* antThread = new QThread();
     *    connect(ant_, &AntFollow::positionChanged, this, &SessionManager::runDeviceSubscr);
     *    connect(ant_, &AntFollow::finished, ant, &AntFollow::deleteLater);
     *    connect(ant_, &AntFollow::destroyed, antThread, &QThread::quit);
     *    connect(antThread, &QThread::finished, antThread, &QThread::deleteLater);
     *    ant_->moveToThread(antThread);
     *    antThread->start();
     * }
     */
    class AntFollow : public QObject {
      Q_OBJECT
    public:
      AntFollow();
      ~AntFollow();

      //!* Safe-Thread  */
      bool setSTLE(const MnSat::STLEParams& stle);
      bool isWorking();

      static PrblAnt* createAntenna(conf::AntennaType type);
      
    signals:
      //Входящие
      void init(const meteo::GeoPoint& coord,  const conf::AntennaConf& conf);
      void setStartPosition(const QDateTime& aos, const QDateTime& los, bool wait);
      void check();             //!< Проверить связь с антенной
      void startTimer(int msec);
      void stopTimer();
      //Исходящие
      void checked(bool);       //!< Результат проверки связи
      void positionChanged(float azimut, float antAzimut, float elevation, float antElevation);
      void finished();

    public slots:
      bool slotInit(const meteo::GeoPoint& coord, const conf::AntennaConf& conf);
      bool slotCheck();
      bool slotStartPosition(const QDateTime& aos, const QDateTime& los, bool wait);

      bool startBlocking();     //!< Блокирующий запуск управления антенной
      bool start(int msec);     //!< Запуск управления антенной с ожиданием
      void stop();              //!< Остановка управления антенной
      void finish();            //!< Остановка управления антеной и испускание сигнала finish

    private slots:
      void setNextPosition();

    private:
      bool getTrackSatellite(const QDateTime& aos, const QDateTime& los);
      bool getTrackGeo(double az, double el);

    private:
      MnDevice::MAntCoord* track_;
      QMutex*              editMutex_;
      QMutex*              workMutex_;
      PrblAnt*             ant_;
      QTimer*              timer_;

      bool                 worked_ = false;

      MnSat::STLEParams    stle_;
      meteo::GeoPoint      stationCoord_;        //!< координаты пункта приема
      float                azimut_       = 0.0f; //!< Устанавливаемый азимут
      float                elevation_    = 0.0f; //!< Устанавливаемый угол места
      float                antAzimut_    = 0.0f; //!< Азимут полученные от антенны
      float                antElevation_ = 0.0f; //!< Угол места полученный от антенны
      int64_t              trackIdx_     = 0;    //!< Текущий номер точки траектории
      float                trackStep_ = 0.5;  //!< Шаг при построении траектории, секунды
    };
 
  }
}

#endif
