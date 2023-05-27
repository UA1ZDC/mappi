#include "loader.h"
#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/paths.h>
#include <cross-commons/debug/tlog.h>
#include <commons/textproto/tprototext.h>
#include <commons/textproto/pbtools.h>
#include <meteo/commons/global/global.h>
#include <meteo/commons/msgparser/tlgparser.h>
#include <meteo/commons/proto/msgparser.pb.h>
#include <meteo/commons/proto/msgcenter.pb.h>
#include <meteo/commons/settings/settings.h>
#include <meteo/commons/msgparser/msgmetainfo.h>

#include <qdatetime.h>
#include <qapplication.h>
#include <qmimedatabase.h>


namespace meteo {
const int kChunkSize = 64*16;
const int kRowDirs = 0;
const int kRowFiles = 1;
const int kRowSended = 2;
const std::string kImageType = "image-satelite";

Loader::Loader(QObject *parent) : QObject(parent)
{
}

Loader::~Loader()
{
  delete msgServ_;
  msgServ_ = nullptr;
}

void Loader::slotSetYMD(int year, int month, int day)
{
  year_ = year;
  month_ = month;
  day_ = day;
}

void Loader::slotSetMaskFileList(QStringList maskFileList)
{
  maskFileList_ = maskFileList;
}

void Loader::slotSetTemplList(QStringList templList)
{
  templList_ = templList;
}

void Loader::slotSetDir(QDir dir)
{
  dir_ = dir;
}

void Loader::slotRun()
{
  run_ = true;
  createConnection();
  recursiveLoadFromDir(dir_);
  emit finished();
}

void Loader::slotStop()
{
 run_ = false;
// emit finished();
}

void Loader::recursiveLoadFromDir(const QDir &directory)
{
  if ( false == run_) {
    return;
  }
  emit signalAddDirs(directory.absolutePath());
  emit signalTableItemIncrement(kRowDirs);
  QStringList files = directory.entryList(QDir::Filter::Files);
  foreach (QString file, files) {
    QString absolutePath = QString(directory.absolutePath()+"/"+file);
    fileHandler(absolutePath);
  }
  if ( false == run_ ) {
    return;
  }
  QStringList dirs = directory.entryList(QDir::Filter::Dirs | QDir::NoDotAndDotDot);
  foreach (QString dir, dirs) {
    QDir d;
    d.setPath(directory.absolutePath()+"/"+dir);
    recursiveLoadFromDir(d);
  }
}

void Loader::fileHandler(const QString &absolutePath)
{
  if (false == run_) {
    return;
  }
  if ( nullptr == msgServ_ ) {
    if ( false == createConnection() ) {
      emit signalFileIncrement();
      return;
    }
  }
  QString name = absolutePath.split("/").last();
  if ( false == checkFileName(name) ) {
    error_log << QObject::tr("Ошибка. Имя файла %1 невалидно").arg( name );
    emit signalFileIncrement();
    return;
  }
  QFile file(absolutePath);
  file.open( QIODevice::ReadOnly);
  QMimeDatabase mdb;
  QFileInfo fInf(file);
  auto mimeinf = mdb.mimeTypeForFile(fInf);
  if ( mimeinf.name().contains("image") ) {
    return imageTlg(absolutePath);
  }
  QDateTime tlgDt;
  tlgDt.setTimeSpec(Qt::UTC);
  if ( true == templList_.isEmpty() ) {
   tlgDt.setDate(QDate( year_, month_, day_) );
  }
  if ( false == templList_.isEmpty() ) {
    bool chekFile = false;
    foreach (QString templ, templList_) {
      MsgMetaInfo msgMI(templ, name);
      if ( true == msgMI.parseError() ) {
        continue;
      }
      else {
        if ( false == msgMI.calcConvertedDt().isValid() ) {
          error_log << QObject::tr("Ошибка. Имя файла %1 соответствует шаблону. Но дата время не валидны.")
                       .arg(name);
          continue;
        }
        chekFile = true;
        tlgDt.setDate( msgMI.calcConvertedDt().date() );
        tlgDt.setTime( msgMI.calcConvertedDt().time() );
        break;
      }
    }
    if ( false == chekFile) {
      error_log << QObject::tr("Ошибка. Имя файла %1 не соответствует шаблону.")
                   .arg(name);
      emit signalFileIncrement();
      return;
    }
  }
  QByteArray filearr;
  meteo::TlgParser parser(filearr);
  while ( true && run_) {
    QByteArray arr(kChunkSize,'\0');
    if ( false == file.atEnd() ) {
      qint64 fsz = file.read( arr.data(), kChunkSize );
      parser.appendData( arr, fsz );
    }
    meteo::tlg::MessageNew newmsg;
    while ( true == parser.parseNextMessage(&newmsg) ) {
      qApp->processEvents();
      if ( false == run_) {
        emit signalFileIncrement();
        return;
      }
//      if ( true == templList_.isEmpty() ) {
        if ( true == newmsg.header().has_yygggg() ) {
          QString YYGGgg = QString::fromStdString(newmsg.header().yygggg());
          int day = YYGGgg.mid(0,2).toInt();
          int hour = YYGGgg.mid(2,2).toInt();
          int minute = YYGGgg.mid(4,2).toInt();
          if ( 0 == tlgDt.date().day() ) {
            tlgDt.setDate(QDate(year_, month_, day));
          }
          tlgDt.setTime(QTime(hour, minute, 0));
        }
        else {
          tlgDt.setTime(QTime(0,0,0));
        }
//      }
      if ( true == tlgDt.isValid() ) {
        tlg::MetaInfo* metaInf = newmsg.mutable_metainfo();
        metaInf->set_converted_dt(tlgDt.toString(Qt::ISODate).toStdString());
        if ( false == dts_.isValid() ) {
          dts_ = tlgDt;
        }
        if ( false == dte_.isValid() ) {
          dte_ = tlgDt;
        }
        if (tlgDt < dts_)  {
          dts_ = tlgDt;
        }
        if ( tlgDt > dte_ ) {
          dte_ = tlgDt;
        }
        emit signalUpdateDT(dts_, dte_);
      }
      else {
        error_log << QObject::tr("Время телеграммы невалидно.");
        continue;
      }
      msgcenter::ProcessMsgRequest req;
      meteo::tlg::MessageNew* msg = req.mutable_msg();
      msg->CopyFrom(newmsg);
      if ( !msgServ_->remoteCall(&msgcenter::MsgCenterService::ProcessMsg, req, this, &Loader::slotProcessed) ) {
        continue;
      }
      emit signalTableItemIncrement(kRowSended);
    }
    if ( true == file.atEnd() ) {
      break;
    }
  }
  emit signalAddFiles(name);
  emit signalTableItemIncrement(kRowFiles);
  emit signalFileIncrement();
}

bool Loader::createConnection()
{
  msgServ_ = meteo::global::serviceChannel(meteo::settings::proto::kMsgCenter);
  if(nullptr == msgServ_) {
    error_log << QObject::tr("Не удалось установить соединение с сервисом телеграмм");
    return false;
  }
  return  true;
}

bool Loader::checkFileName( QString& name)
{
  if ( true == maskFileList_.isEmpty() ) {
    return  true;
  }
  bool result = false;
  QRegExp exp;
  foreach (QString templ, maskFileList_) {
    exp.setPattern(templ);
    exp.setPatternSyntax(QRegExp::Wildcard);
    result = result || exp.exactMatch(name);
  }
  return result;
}

void Loader::imageTlg(const QString& absolutePath)
{
  QDateTime tlgDt;
  tlgDt.setTimeSpec(Qt::UTC);
  if ( true == templList_.isEmpty() ) {
   tlgDt.setDate(QDate( year_, month_, day_) );
  }
  QFile file(absolutePath);
  file.open( QIODevice::ReadOnly);
  QByteArray filearr = file.readAll();
  meteo::tlg::MessageNew newmsg;
  newmsg.set_format(tlg::kWMO);
  newmsg.set_msg(pbtools::toBytes(filearr));
  auto name = absolutePath.split("/").last();
  auto header = newmsg.mutable_header();
  if ( false == templList_.isEmpty() ) {
    bool chekFile = false;
    foreach (QString templ, templList_) {
      MsgMetaInfo msgMI(templ, name);
      if ( true == msgMI.parseError() ) {
        continue;
      }
      else {
        if ( false == msgMI.calcConvertedDt().isValid() ) {
          error_log << QObject::tr("Ошибка. Имя файла %1 соответствует шаблону. Но дата время не валидны.")
                       .arg(name);
          continue;
        }
        chekFile = true;
        tlgDt.setDate( msgMI.calcConvertedDt().date() );
        tlgDt.setTime( msgMI.calcConvertedDt().time() );
        header->CopyFrom(msgMI.getHeader());
        break;
      }
    }
    if ( false == chekFile) {
      error_log << QObject::tr("Ошибка. Имя файла %1 не соответствует шаблону.")
                   .arg(name);
      emit signalFileIncrement();
      return;
    }
  }
  if ( !header->has_t1()) {
    header->set_t1(std::string("x"));
  }
  if ( !header->has_t2()) {
    header->set_t2(std::string("x"));
  }
  if ( !header->has_a1()) {
    header->set_a1(std::string("x"));
  }
  if ( !header->has_a2()) {
    header->set_a2(std::string("x"));
  }
  if ( !header->has_ii()) {
    header->set_ii(0);
  }
  if ( !header->has_cccc()) {
    header->set_cccc(std::string("cccc"));
  }
  if ( !header->has_yygggg()) {
    header->set_yygggg(std::string("000000"));
  }
  if ( true == templList_.isEmpty() ) {
    if ( true == newmsg.header().has_yygggg() ) {
      QString YYGGgg = QString::fromStdString(newmsg.header().yygggg());
      int day = YYGGgg.mid(0,2).toInt();
      int hour = YYGGgg.mid(2,2).toInt();
      int minute = YYGGgg.mid(4,2).toInt();
      if ( 0 == tlgDt.date().day() ) {
        tlgDt.setDate(QDate(year_, month_, day));
      }
      tlgDt.setTime(QTime(hour, minute, 0));
    }
    else {
      tlgDt.setTime(QTime(0,0,0));
    }
  }
  if ( true == tlgDt.isValid() ) {
    tlg::MetaInfo* metaInf = newmsg.mutable_metainfo();
    metaInf->set_converted_dt(tlgDt.toString(Qt::ISODate).toStdString());
    if ( false == dts_.isValid() ) {
      dts_ = tlgDt;
    }
    if ( false == dte_.isValid() ) {
      dte_ = tlgDt;
    }
    if (tlgDt < dts_)  {
      dts_ = tlgDt;
    }
    if ( tlgDt > dte_ ) {
      dte_ = tlgDt;
    }
    emit signalUpdateDT(dts_, dte_);
  }
  else {
    error_log << QObject::tr("Время телеграммы невалидно.");
    emit signalFileIncrement();
    return;
  }
  msgcenter::ProcessMsgRequest req;
  meteo::tlg::MessageNew* msg = req.mutable_msg();
  msg->CopyFrom(newmsg);
  msg->set_msgtype(kImageType);
  if ( msgServ_->remoteCall(&msgcenter::MsgCenterService::ProcessMsg, req, this, &Loader::slotProcessed) ) {
    emit signalTableItemIncrement(kRowSended);
  }
  emit signalAddFiles(name);
  emit signalTableItemIncrement(kRowFiles);
  emit signalFileIncrement();
}

void Loader::slotProcessed(msgcenter::ProcessMsgReply* r)
{
  delete r;
}


}
