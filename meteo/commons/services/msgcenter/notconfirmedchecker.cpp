#include "notconfirmedchecker.h"

#include "msgcenter.h"

#include <sql/nosql/nosqlquery.h>
#include <sql/dbi/gridfs.h>
#include <sql/dbi/gridfile.h>
#include <meteo/commons/global/global.h>
#include <meteo/commons/msgdata/msggetter.h>
#include <meteo/commons/msgparser/tlgfuncs.h>
#include <meteo/commons/msgparser/tlgparser.h>
#include <meteo/commons/msgparser/common.h>
#include <meteo/commons/msgdata/msggetter.h>

namespace meteo {

static constexpr int32_t kCheckTimeout = 30*1000; //!< период проверки неподтвержденных сообщений - 1 мин.
static constexpr int32_t kExpireTimeout = 10*24*60*60; //!< период ожидания подтверждения  - 10 дней

static const QString kTelegramsdb = QObject::tr("telegramsdb");

static const QString kGridPrefix = "fs";

NotConfirmedChecker::NotConfirmedChecker( MsgCenter* srv )
  : service_(srv),
  tm_( new QTimer(this) )
{
  slotTimeout();
  QObject::connect( tm_, SIGNAL( timeout() ), this, SLOT( slotTimeout() ) );
  tm_->start(kCheckTimeout);

}

NotConfirmedChecker::~NotConfirmedChecker()
{
  delete tm_; tm_ = nullptr;
  service_ = nullptr;
}

void NotConfirmedChecker::loadUnconfimedMessages()
{

  const ConnectProp param = global::mongodbConfTelegram();
//  QString host = param.host();
//  int port = param.port();
//  auto login = param.login();
//  auto pass = param.pass();
//  QString dbName = param.name();

  GridFs gridfs;
  if ( false == gridfs.connect( param ) ) {
    error_log << gridfs.lastError();
    return;
  }
  gridfs.use( param.name(), kGridPrefix);

  std::unique_ptr<Dbi> db(meteo::global::dbTelegram());
  if ( nullptr == db.get() ) {
     error_log << QObject::tr("Не удалось подключиться к БД");
     return;
  }

  auto query = db->queryptrByName("get_unconfirmed_msg");
  if ( nullptr == query ) {
    error_log << QObject::tr("Не найдена функция для получения неподтвержденых адресованых сообщений = 'get_unconfirmed_msg'");
    return;
  }

  QDateTime dt = QDateTime::currentDateTimeUtc().addSecs( -kExpireTimeout );
  query->arg("dt", dt );
  query->arg("cccc", global::hmsId() );
  QString err;
  if ( false == query->execInit(&err) ) {
    //error_log << rr;
    return;
  }

  QByteArray ba;
  TlgParser parser(ba);
//  QList<int64_t> ids;
  while ( true == query->next() ) {
    const DbiEntry& doc = query->entry();
    tlg::MessageNew tlg;
    QString qq = QString("{ \"_id\": \"%1\" }" ).arg( doc.valueOid("file_id") );

    GridFile file = gridfs.findOne(qq);
    if ( !file.isValid() || !file.hasFile() ) {
      error_log << QObject::tr("Ошибка при поиске файла = %1. Ошибка = %2")
	      .arg( qq )
	      .arg( gridfs.lastError() );
      continue;
    }

    bool ok = false;
    QByteArray raw = file.readAll(&ok);
    if ( true == ok ) {
      parser.setData(raw);
      if ( false == parser.parseNextMessage(&tlg) ) {
        error_log << QObject::tr("Не удалось загрузить телеграмму с идентификатором %1.").arg(doc.valueInt64("_id"));
        continue;
      }
    }

    MsgGetter::fillMessage( doc, &tlg);
    tlg::MessageNew confirm;
    makeConfirmation(tlg,&confirm );
    service_->sendMessage(confirm);
  }
}

bool NotConfirmedChecker::makeConfirmation(const tlg::MessageNew& msg, tlg::MessageNew* confirm)
{

  QString dddd = tlg::creator(msg);

  QString kkkk = "00"; // FIXME:
  QString num = "999"; // FIXME: использовать счётчик для номеров

  QByteArray raw;
  raw += 001;
  raw += QString::fromUtf8("010 %1/=Н%2").arg(kkkk).arg(num).toUtf8();
  raw += 002;
  raw += "\r\r\n";
  raw += QString::fromUtf8("БА%1 %2 %3\r\r\n").arg(dddd, global::hmsId(),
                                                   QDateTime::currentDateTimeUtc().toString("ddhhmm")).toUtf8();
  raw += "CONFIRM MD5 ";
  raw += tlg::md5hash(msg).toHex();
  raw += "\r\r\n";
  raw += 003;

  TlgParser parser(raw);
  if ( !parser.parseNextMessage(confirm) ) {
    var(raw);
    return false;
  }

  confirm->mutable_metainfo()->set_converted_dt(QDateTime::currentDateTimeUtc().toString(Qt::ISODate).toStdString());

  return true;
}
void NotConfirmedChecker::slotTimeout()
{
  loadUnconfimedMessages();
}

}
