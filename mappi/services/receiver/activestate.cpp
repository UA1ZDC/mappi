#include "activestate.h"
#include "filestorage.h"
#include <mappi/global/global.h>
#include <sql/nosql/nosqlquery.h>


namespace mappi {

namespace receiver {


static const QString SQL_SESSION_INSERT = "session_insert";

ActiveState::ActiveState(Context* ctx, StateMachine* fsm) :
  ReceiverState(ctx, fsm)
{
}

ActiveState::~ActiveState()
{
}

State::id_t ActiveState::branch(Primitive::id_t prim, Primitive::queue_t* input)
{
  switch (prim) {
    case Primitive::ALERT_IND :
    case Primitive::WARMING_COMPLETED : {
      // TODO активация выхода облучателя, заисит от диапазона
      // feedhornOutput(1, true);

      Parameters param = ctx->conf.parameters(ctx->session.data().satellite);
      info_log << param.toString();

      global::StreamHeader header;
      header.site = ctx->conf.siteName;
      header.siteCoord = ctx->conf.point;
      header.satellite = ctx->session.data().satellite;
      header.direction = ctx->session.data().direction;
      header.mode = ctx->conf.mode;
      header.tle = ctx->satellite.getTLEParams();

      actualAos = ctx->now();
      if (ctx->conf.demoMode)
        actualAos = ctx->sessionOriginal.data().aos.addSecs(::abs(ctx->session.secsToAos(ctx->now())));

      header.start = actualAos;
      header.stop = ctx->sessionOriginal.data().los;

      FileStorage fs(ctx);
      fs.setHeader(header);
      debug_log << QObject::tr("set stream header");

      if (ctx->receiver->start(&ctx->satellite, param)) {
        int msec = ctx->session.msecsToLos(ctx->now());
        fsm->startTimer_msec(msec);

        info_log << QObject::tr("receiver start: %1, duration(%2, %3)")
          .arg(SessionData::dateToString(actualAos))
          .arg(ctx->session.duration())
          .arg(msec / 1000.0);

        break ;
      }

      // если не получилось включить приёмник
      input->append(Primitive::FAIL_IND);
      return State::SKIPPING;
    }

    case Primitive::TIMEOUT :
      ctx->receiver->stop();

      info_log << QObject::tr("receiver stop: %1")
        .arg(SessionData::dateToString(ctx->sessionOriginal.data().los));

      // TODO деактивация выхода облучателя, заисит от диапазона
      // неочень понятла логика работы с выходами облучателя, их нужно вкл\выкл для каждого диапазона или достаночно включить все выходы один раз
      // feedhornOutput(1, false);
      saveToDb();

      // уведомление о новом сеансе получено раньше завершения текущего
      if (!ctx->isReplay())
        input->append(Primitive::ACCEPT_SESSION);

      return State::IDLING;

    case Primitive::SERVICE_MODE_ON :
      fsm->stopTimer();
      ctx->receiver->stop();
      info_log << QObject::tr("receiver terminate");

      return State::SERVICE;

    default :
      break ;
  }

  return id();
}

void ActiveState::saveToDb()
{
  // TODO перенести в отдельный RPC вызов
  QScopedPointer<Dbi> db(global::dbMappi());
  if (db.get() == nullptr) {
    error_log << QObject::tr("Ошибка подключения к базе данных");
    return ;
  }

  auto query = db->queryptrByName(SQL_SESSION_INSERT);
  if (query == nullptr) {
    error_log << QObject::tr("Ошибка в запросе: %1").arg(SQL_SESSION_INSERT);
    return ;
  }

  query->arg("satellite", ctx->sessionOriginal.data().satellite);
  query->argDt("date_start", schedule::SessionData::dateToString(ctx->sessionOriginal.data().aos));
  query->argDt("date_end", schedule::SessionData::dateToString(ctx->sessionOriginal.data().los));
  query->arg("date_start_offset", static_cast<int32_t>(ctx->sessionOriginal.data().aos.secsTo(actualAos)));
  query->arg("revol", static_cast<int32_t>(ctx->sessionOriginal.data().revol));
  query->arg("direction", static_cast<int32_t>(ctx->sessionOriginal.data().direction));
  query->arg("elevat_max", static_cast<float>(MnMath::rad2deg(ctx->sessionOriginal.data().elevatMax)));
  query->arg("site", ctx->conf.siteName);
  query->arg("fpath", ctx->sessionOriginal.data().fileName());

  QByteArray buf;
  QDataStream out(&buf, QIODevice::WriteOnly);
  out << ctx->satellite.getTLEParams();
  query->arg("tle", QString(buf.toBase64()));

  // debug_log << query->query();
  if (query->exec() == false) {
    error_log << QObject::tr("Ошибка выполнения запроса: %1").arg(SQL_SESSION_INSERT);
    return ;
  }

  info_log << QObject::tr("В базу данных добавлена запись о принятом сеансе");
}

void ActiveState::feedhornOutput(int n, bool turnOn)
{
  QScopedPointer<rpc::Channel> ch(channel(settings::proto::kAntenna));
  if (ch == nullptr) {
    warning_log << QObject::tr("%1 недоступен")
      .arg(global::serviceTitle(settings::proto::kAntenna));
    return ;
  }

  conf::Feedhorn req;
  switch (n) {
    case 1 : req.set_output_1(turnOn); break ;
    case 2 : req.set_output_2(turnOn); break ;
    case 3 : req.set_output_3(turnOn); break ;
    case 4 : req.set_output_4(turnOn); break ;

    default :
      break ;
  }

  QScopedPointer<conf::AntennaResponse> resp(ch->remoteCall(&conf::AntennaService::FeedhornOutput, req, 10000, true));
  if (resp.isNull()) {
    error_log << QObject::tr("Сервис вернул пустой ответ");
    return ;
  }

  if (resp->result() == false) {
    if (resp->has_comment())
      error_log << resp->comment();

    return ;
  }

  info_log << QObject::tr("feedhorn output(%1): %2")
    .arg(n)
    .arg(turnOn);
}

}

}
