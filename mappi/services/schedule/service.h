#pragma once

#include "context.h"
#include "tlemonitor.h"
#include <mappi/proto/schedule.pb.h>
#include <qmutex.h>


namespace mappi {

namespace schedule {

/**
 * Основная задача сервиса - работа с расписанием:
 * - формирование согласно конфигурации;
 * - формирование по требованию;
 * - выдача расписания;
 * - выдача следующего и ближайшего сеанса (вместе с сеансом передаются tle-параметры спутника);
 * - отслеживание изменение tle-файла (при изменении обновляется расписание);
 * - выдача tle-параметров заданного спутника из tle-файла;
 * - выдача информации о всех спутниках;
 * - автоматическое\ручное решение конфликтов.
 *
 * Сервис работает по принципу "ленивой инициализации", т.е. пока кому-нибудь не понадобится расписание оно не будет сформировано.
 * Сформированное текущее расписание сохраняется в файл.
 *
 * В случае изменения расписания другие сервисы НЕ УВЕДОМЛЯЮТСЯ об этом.
 * Расписание может обновится по следующим причинам:
 * - ручное решение конфликтов, влияет на то будет приниматься сеанс или нет;
 * - изменение tle-файла.
 *
 * !!! TODO
 * - При ручном решении конфликтов целесообразно останавливать приём по расписанию (перевод сервисов отвечающих за приём в сервисный режим),
 * т.к. может получиться ситуация, когда активный сеанс исключен пользователем из расписания, а по логике работы playback.service
 * этот сеанс все равно будет принят.
 * - На сегодняшний день при формировании нового расписания по требованию, если до этого, конфликты решались вручную, то они будут потеряны.
 * - Конфликты решенные пользователем не сохраняются в следующих случаях (со слов Юры):
 *   + при обнвленни tle-файла, файл может сильно устареть, поэтому расписание сильно может "разъехаться";
 *   + при регенерации расписания (раз в сутки добавление следующего дня), на стыке дней могут произойти новые конфликты.
 * - Расмотреть возможность передачи информации о tle-файле (когда обновлен, когда сформирован и т.д.)
 */
class ServiceHandler :
  public QObject
{
  Q_OBJECT
public :
  explicit ServiceHandler(QObject* parent = nullptr);
  virtual ~ServiceHandler();

  bool init();
  bool start();

  void makeSchedule(const Dummy* req, conf::ScheduleResponse* resp);
  void currentSchedule(const Dummy* req, conf::ScheduleResponse* resp);

  void editSession(const conf::Session* req, conf::SessionResponse* resp);
  void nearSession(const Dummy* req, conf::SessionResponse* resp);
  void nextSession(const conf::Session* req, conf::SessionResponse* resp);

  void tleSatellite(const conf::SessionInfo* req, conf::SessionResponse* resp);
  void tleSatelliteList(const Dummy* req, conf::SatelliteResponse* resp);

private :
  bool tleToMessage(const QString& satellite, conf::TleItem* tle);
  void scheduleToMessage(conf::ScheduleResponse* resp);

private slots :
  void tleUpdate();

private :
  Context* ctx_;
  TleMonitor* tleMonitor_;      // QObject
};

}

}
