#ifndef MAPPI_SERVICES_SESSIONMANAGER_SESSIONMANAGER_H
#define MAPPI_SERVICES_SESSIONMANAGER_SESSIONMANAGER_H

#include <memory>

#include <qobject.h>
#include <qmutex.h>

#include <sat-commons/satellite/satellitebase.h>

#include <meteo/commons/global/global.h>
#include <meteo/commons/global/requestcard.h>

#include <mappi/schedule/session.h>
#include <mappi/proto/reception.pb.h>



namespace mappi {
  namespace po {
    class Handler;
  }
  namespace schedule {
    class Schedule;
  }
  namespace receive {
    class Ctrl;
    class AntFollow;
    class RecvStream;
  }

  namespace receive {

    //! Менеджер приёма

    /*! \brief The SessionManager class- Менеджер приёма
     *  Внимание. Менеджер запрашивает расписание, после подключения к сервису расписаний
     * в методе slotConnectToService вызывается метод checkSchedule. Это позволяет запросить свежее расписание,
     * если сервис расписаний был недоступен на момент запуска менеджера приёма.
     *  Таким образом, для использования менеджера приема, достаточно его проинициализировать.
     */

    class SessionManager : public QObject, public conf::ReceiveService {
      Q_OBJECT

      using DataMultiCard   = RequestCard<const conf::DataRequest, conf::DataReply>;
      using DeviceMultiCard = RequestCard<const conf::DataRequest, conf::DeviceStateReply>;

      //! Состояние менеджера приёма
      enum State { kWaiting = 0, kRecv = 1, kNoSession = 2 };

    public:
      SessionManager();
      virtual ~SessionManager();

      bool init();
      bool checkDevices();

      void GetDataMulti(google::protobuf::RpcController *controller
			, const conf::DataRequest *request
			, conf::DataReply *response
			, google::protobuf::Closure *done);

      void GetDeviceStateMulti(google::protobuf::RpcController *controller
			       , const conf::DataRequest *request
			       , conf::DeviceStateReply *response
			       , google::protobuf::Closure *done);

  
      virtual QDateTime currentDt()const { return QDateTime::currentDateTimeUtc().addSecs(timeOffset_); }
      void setOffsetTime(int offset) { timeOffset_ = offset; }

    public slots:
      void slotClientSubscribed(meteo::rpc::Controller*);
      void slotClientUnsubscribed(meteo::rpc::Controller*);
      
    protected:
      const schedule::Session& near() const { return   near_; }
      virtual schedule::Session getNextNear(const schedule::Schedule& sched);
      bool checkSchedule();
      void recreateSchedule();

      virtual void stopProcess();
      void stopProcessWithDt();
			     
    private slots:
      void slotDeviceUncheck(bool check);

      void slotTimeout();
      void slotFileSubscr(const QString& fileName, uint64_t sessionId);
      void slotDataSubscr(const QByteArray& data, int status);
      void slotDeviceSubscr(float azimut, float antAzimut, float elevation, float antElevation);
      void sendNearSession();

      void slotInitChannel();
      void slotConnectToService();
      void slotDisconnectedFromService();

    private:

      bool checkSession(const schedule::Schedule& sched);

      void process();
      

      bool startReceiver();
      bool stopReceiver();

      void deleteChannel();
      bool subscribe();
      void callbackSchedule(conf::ScheduleResponse* reply);

      void saveImg(const QByteArray& data);

    private:
      AntFollow*            ant_;       //!< Объект удаляет сам себя см manager.h стр: ~43
      Ctrl*                 rctrl_;     //!< Объект удаляет сам себя см manager.h стр: ~289
      RecvStream*           receiver_;  //!< Объект удаляет сам себя см manager.h стр: ~51
      po::Handler*          handler_;   //!< Обработка данных для отображения при приёме
      MnSat::STLEParams     stle_;

      meteo::rpc::Channel*         chReceiver_;         //!< Канал подписки на получение расписания
      //rpc::TController*     ctrlReceiver_;       //!< Контроллер подписки на получение расписания
      //      conf::ScheduleReply*  replySchedule_;

      QTimer*               timer_;
      QMutex mutex_;

      State                 state_;

      conf::Reception       conf_;
      schedule::Session     near_;


      QMap<meteo::rpc::Channel*, google::protobuf::Closure*> subs_; //!< Подписчики
      QMap<meteo::rpc::Channel*, DataMultiCard>              dataMultiCards_;    //!< Подписчики на данные
      bool sendingDataMulti_    = false;
      QMap<meteo::rpc::Channel*, DeviceMultiCard>            deviceMultiCards_;    //!< Подписчики на состояние железа
      bool sendingDeviceMulti_  = false;

      QByteArray            tmpdata_;
      int                   tmprows_;
      int                   timeOffset_ = 0; //!< Разница между реальным временем и виртуальным (для тестового приёма)
    };

  } // receiver
} // mappi

#endif 
