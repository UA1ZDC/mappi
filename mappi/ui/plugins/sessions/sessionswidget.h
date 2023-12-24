#ifndef MAPPI_UI_PLUGIN_SESSION_SESSIONWIDGET_H
#define MAPPI_UI_PLUGIN_SESSION_SESSIONWIDGET_H

#include <memory>

#include <qwidget.h>
#include <qdatetime.h>

#include <meteo/commons/rpc/rpc.h>

#include <mappi/schedule/session.h>
#include <mappi/proto/reception.pb.h>

class QListView;
class QButtonGroup;
class QStringListModel;

namespace Ui {
  class SessionsWidget;
}



namespace meteo {
  namespace app {
    class AppState_Proc;
  }
}

namespace mappi {
  namespace conf {
    class DeviceStateReply;
  }
  namespace schedule {
    class Session;
  }
  class SatelliteTrackerScene;
  class ReceiverImageView;
  class ReceiverImageScene;
  class ScheduleTable;

class SessionsWidget : public QWidget
{
  Q_OBJECT
public:
  explicit SessionsWidget(QWidget *parent = 0);
  ~SessionsWidget();

  QDateTime currentDtUtc() { return QDateTime::currentDateTimeUtc().addSecs(timeOffset_); }
  QDateTime currentDt() { return QDateTime::currentDateTime().addSecs(timeOffset_); }
  
protected:
  void resizeEvent(QResizeEvent* e);
  void showEvent(QShowEvent* e);

private:
  void init();                   //!< Инициализация
  void loadSettings();           //!< Загрузка настройки плагина
  void saveSettings() const;     //!< Записать настройки плагина

  // void deleteReceiverChannel();
  void deleteAntennaChannel();
  bool subscribe();                             //!< Подписка на получение данных от сервиса приема
  void callbackAntenna(conf::AntennaResponse *reply); //!< обработка данных о приемнике и антенне

  void deleteAppManagerChannel();
  bool subscribeAppManager();
  void callbackAppManager(meteo::app::AppState_Proc *reply);

  void resizeMap(const QSize& size);                  //!< Задать размер сцене

  bool selectSatellite(const QString& satelliteName);   //!< Выборать активного спутника

  void addMsgToLog (const QString& message);            //!< Добавить сообщение об ошибки в модель списка ошибок
  void setMsgFromReceiver(const QString& message);      //!< Установить сообщения от приемника
  void setMsgFromAntenna (const QString& message);      //!< Установить сообщения от антенны

  void refreshReceiverControls();


  bool getSessions();

private slots:
  void slotChangedNear(std::shared_ptr<schedule::Session> near);
  void slotChangedSession(std::shared_ptr<schedule::Session> session, const ::mappi::conf::TleItem&);

  void slotInitAntennaChannel();                        //!< Инициализация сервиса приёма
  void slotConnectToAntenna();                          //!< Подключение к сервису приёма
  void slotDisconnectedFromAntenna();                   //!< Отключение от сервиса приёма

  /*
  void slotInitReceiverChannel();                        //!< Инициализация сервиса приёма
  void slotConnectToReceiver();                          //!< Подключение к сервису приёма
  void slotDisconnectedFromReceiver();                   //!< Отключение от сервиса приёма
*/
  void slotInitAppManagerChannel();
  void slotConnectToAppManager();
  void slotDisconnectedFromAppManager();

  void slotResizeImage(const QSize& size);              //!< Измененить размера рисунка полученного от спутника
  void slotTimeout();                                   //!< Итерация таймера

  void slotToggleShowErrorList();                       //!< Скрыть/показать список ошибок
  void slotToggleMapMode(int index, bool state);        //!< Изменить режим отображения карты
  void slotToggleVisibleMap();                          //!< Спрятать/показать карту сессии

  void slotChangedSchedule(int row);                    //!< Сменить задание для визуализации
  void slotSetChannels(const QStringList& channels);

  void slotClickReceiverCtrl() const;
  void slotClickGeoCtrl() const;

private:
  Ui::SessionsWidget*         ui_;
  SatelliteTrackerScene*      satelliteTracker_;   //!< Сцена траектории спутника
  ReceiverImageView*          receiverImageView_;
  ReceiverImageScene*         receiverImage_;      //!< Сцена принятого снимка
  ScheduleTable*              scheduleTable_;      //!< Таблица с расписанием
  QDialog*                    errorDialog_;        //!< Окно лога ошибок
  QButtonGroup*               mapModeGroup_;       //!< Группа кнопок выбора проекции

  bool                        showMap_;            //!< Флаг отображения карты
  bool                        showErrorList_;      //!< Флаг отображения диалогового окна с логом ошибок
  bool                        loadedSettings_;     //!< Флаг били ли загружены настройки
  bool                        receiverRunning_;

  QListView*                  errorList_;          //!< Список ошибок дл Dialog лога ошибок
  QStringListModel*           errorsModel_;        //!< Модел для списка ошибок


 // meteo::rpc::Channel*               chReceiver_;         //!< Канал подписки к информации об антенне
  meteo::rpc::Channel*               chAntenna_;         //!< Канал подписки к информации об антенне
  //  rpc::TController*           ctrlReceiver_;       //!< Контроллер подписки к информации об антенне
  //  conf::DeviceStateReply*     replyReceiver_;

  meteo::rpc::Channel*              chAppManager_;       //!< Канал подписки на получение расписания
  //  rpc::TController*          ctrlAppManager_;     //!< Контроллер подписки на получение расписания
  //  meteo::app::AppState_Proc* replyAppManager_;

  std::shared_ptr<schedule::Session> near_;        //!< Ближайщая/текущая сессия


  int timeOffset_ = 0; //!< Разница между реальным временем и виртуальным (для тестового приёма)

  QTimer*                     updateTimer_;        //!< Таймер для часов на нижней панели

private:
  static void startReceiverApp();
  static void stopReceiverApp();
  static bool getReceiverServiceRunning();

  static QPixmap getEyeIcon(bool active);          //!< Создать обычный / серый глаз
};

} //mappi

#endif // MAPPI_UI_PLUGIN_SESSION_SESSIONWIDGET_H
